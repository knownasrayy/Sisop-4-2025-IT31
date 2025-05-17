#define FUSE_USE_VERSION 29
#include <fuse.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <limits.h>
#include <time.h>
#include <ctype.h>
#include <sys/file.h>
#include <pthread.h>

#define CHUNK_SIZE 1024
#define MAX_FRAGMENTS 1000
#define LOG_BUFFER_SIZE 4096

static const char *relics_dir = "relics";
static const char *log_file    = "activity.log";
static FILE *log_fp            = NULL;
static pthread_mutex_t log_mutex = PTHREAD_MUTEX_INITIALIZER;

typedef struct {
    char *name;
    int is_new;
    int fragments_created;
} tracked_file;

static void init_logging() {
    pthread_mutex_lock(&log_mutex);
    if (!log_fp) {
        log_fp = fopen(log_file, "a");
        if (!log_fp) { perror("open log"); exit(1); }
        setvbuf(log_fp, NULL, _IOLBF, LOG_BUFFER_SIZE);
    }
    pthread_mutex_unlock(&log_mutex);
}

static int baymax_access(const char *path, int mask) {
    return 0;
}


static void log_event(const char *event, const char *details) {
    pthread_mutex_lock(&log_mutex);
    if (!log_fp) init_logging();
    time_t now = time(NULL);
    struct tm *tm = localtime(&now);
    char ts[20];
    strftime(ts, sizeof(ts), "%Y-%m-%d %H:%M:%S", tm);
    if (details)
        fprintf(log_fp, "[%s] %s: %s\n", ts, event, details);
    else
        fprintf(log_fp, "[%s] %s\n", ts, event);
    pthread_mutex_unlock(&log_mutex);
}

static void cleanup_resources() {
    if (log_fp) fclose(log_fp);
    pthread_mutex_destroy(&log_mutex);
}

static void remove_fragments(const char *base) {
    DIR *dp = opendir(relics_dir);
    if (!dp) return;
    struct dirent *de;
    char path[PATH_MAX];
    size_t bl = strlen(base);
    while ((de = readdir(dp))) {
        if (strncmp(de->d_name, base, bl)==0 && de->d_name[bl]=='.') {
            snprintf(path, sizeof(path), "%s/%s", relics_dir, de->d_name);
            unlink(path);
        }
    }
    closedir(dp);
}

static int baymax_getattr(const char *path, struct stat *stbuf) {
    memset(stbuf, 0, sizeof(*stbuf));
    if (strcmp(path, "/")==0) {
        stbuf->st_mode = S_IFDIR | 0755;
        stbuf->st_nlink = 2;
        return 0;
    }
    const char *b = path + 1;
    off_t total = 0;
    struct stat st;
    char frag[PATH_MAX];
    int i;
    for (i=0; i<MAX_FRAGMENTS; i++) {
        snprintf(frag, sizeof(frag), "%s/%s.%03d", relics_dir, b, i);
        if (stat(frag, &st)==-1) break;
        total += st.st_size;
    }
    if (i==0) return -ENOENT;
    stbuf->st_mode = S_IFREG | 0644;
    stbuf->st_nlink = 1;
    stbuf->st_size = total;
    return 0;
}

static int baymax_readdir(const char *path, void *buf,
    fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi) {
    (void)offset; (void)fi;
    if (strcmp(path, "/")!=0) return -ENOENT;
    filler(buf, ".", NULL, 0);
    filler(buf, "..", NULL, 0);
    DIR *dp = opendir(relics_dir);
    if (!dp) return -ENOENT;
    struct dirent *de;
    char *names[MAX_FRAGMENTS] = {0};
    int n=0, ret=0;
    while ((de = readdir(dp))) {
        char *p = strrchr(de->d_name, '.');
        if (!p || strlen(p)!=4) continue;
        if (!isdigit(p[1])||!isdigit(p[2])||!isdigit(p[3])) continue;
        size_t L = p - de->d_name;
        char base[NAME_MAX+1];
        strncpy(base, de->d_name, L); base[L]='\0';
        int found=0;
        for (int j=0; j<n; j++)
            if (!strcmp(names[j], base)) { found=1; break; }
        if (!found && n<MAX_FRAGMENTS) {
            names[n] = strdup(base);
            if (!names[n]) { ret=-ENOMEM; break; }
            filler(buf, base, NULL, 0);
            n++;
        }
    }
    for (int j=0; j<n; j++) free(names[j]);
    closedir(dp);
    return ret;
}

static int baymax_open(const char *path, struct fuse_file_info *fi) {
    const char *b = path + 1;
    if ((fi->flags & O_ACCMODE)==O_RDONLY) log_event("READ", b);
    tracked_file *ctx = calloc(1, sizeof(*ctx));
    if (!ctx) return -ENOMEM;
    ctx->name = strdup(b); if (!ctx->name) { free(ctx); return -ENOMEM; }
    ctx->is_new = (fi->flags & O_TRUNC)?1:0;
    ctx->fragments_created = 0;
    if (ctx->is_new) remove_fragments(b);
    fi->fh = (uintptr_t)ctx;
    return 0;
}

static int baymax_create(const char *path, mode_t mode, struct fuse_file_info *fi) {
    (void)mode; const char *b = path + 1;
    remove_fragments(b);
    tracked_file *ctx = calloc(1, sizeof(*ctx));
    if (!ctx) return -ENOMEM;
    ctx->name = strdup(b); if (!ctx->name) { free(ctx); return -ENOMEM; }
    ctx->is_new = 1; ctx->fragments_created = 0;
    fi->fh = (uintptr_t)ctx; return 0;
}

static int baymax_read(const char *path, char *buf, size_t sz,
    off_t off, struct fuse_file_info *fi) {
    (void)fi; const char *b = path + 1;
    size_t rd=0; int idx=off/CHUNK_SIZE; off_t o=off%CHUNK_SIZE;
    char frag[PATH_MAX];
    while (rd<sz && idx<MAX_FRAGMENTS) {
        snprintf(frag, sizeof(frag), "%s/%s.%03d", relics_dir, b, idx);
        int fd = open(frag, O_RDONLY); if (fd<0) break;
        flock(fd, LOCK_SH);
        size_t c = CHUNK_SIZE-o; if (c>sz-rd) c=sz-rd;
        ssize_t r = pread(fd, buf+rd, c, o);
        flock(fd, LOCK_UN); close(fd);
        if (r<=0) break;
        rd+=r; o=0; idx++;
    }
    return rd;
}

static int baymax_write(const char *path, const char *buf, size_t sz,
    off_t off, struct fuse_file_info *fi) {
    tracked_file *ctx = (tracked_file*)fi->fh;
    const char *b = path + 1;
    size_t wr=0; int idx=off/CHUNK_SIZE; off_t o=off%CHUNK_SIZE;
    char frag[PATH_MAX];
    while (wr<sz && idx<MAX_FRAGMENTS) {
        snprintf(frag, sizeof(frag), "%s/%s.%03d", relics_dir, b, idx);
        int fd = open(frag, O_CREAT|O_WRONLY, 0644); if (fd<0) return -EIO;
        flock(fd, LOCK_EX);
        size_t c = CHUNK_SIZE-o; if (c>sz-wr) c=sz-wr;
        ssize_t w = pwrite(fd, buf+wr, c, o);
        flock(fd, LOCK_UN); close(fd);
        if (w!=c) return -EIO;
        wr+=w; o=0; idx++;
        if (idx>ctx->fragments_created) ctx->fragments_created=idx;
    }
    return wr;
}

static int baymax_release(const char *path, struct fuse_file_info *fi) {
    tracked_file *ctx = (tracked_file*)fi->fh;
    if (ctx) {
        if (ctx->is_new && ctx->fragments_created>0) {
            char det[4096]=""; int first=1;
            for (int i=0; i<ctx->fragments_created; i++) {
                char tmp[64]; snprintf(tmp,sizeof(tmp),"%s.%03d",ctx->name,i);
                if (!first) strncat(det,", ",sizeof(det)-strlen(det)-1);
                first=0; strncat(det,tmp,sizeof(det)-strlen(det)-1);
            }
            char ev[PATH_MAX+64];
            snprintf(ev,sizeof(ev),"%s -> %s",ctx->name,det);
            log_event("WRITE",ev);
        }
        free(ctx->name); free(ctx);
    }
    return 0;
}

static int baymax_unlink(const char *path) {
    const char *b = path + 1; int cnt=0; char frag[PATH_MAX];
    for (int i=0; i<MAX_FRAGMENTS; i++) {
        snprintf(frag,sizeof(frag),"%s/%s.%03d",relics_dir,b,i);
        if (access(frag,F_OK)) break;
        int fd=open(frag,O_WRONLY);
        if (fd>=0){ flock(fd,LOCK_EX); unlink(frag); flock(fd,LOCK_UN); close(fd); }
        cnt++;
    }
    if (cnt==0) return -ENOENT;
    char ev[PATH_MAX];
    snprintf(ev,sizeof(ev),"%s.000 - %s.%03d",b,b,cnt-1);
    log_event("DELETE",ev);
    return 0;
}

static struct fuse_operations baymax_oper = {
    .getattr = baymax_getattr,
    .readdir = baymax_readdir,
    .access  = baymax_access,
    .open    = baymax_open,
    .read    = baymax_read,
    .write   = baymax_write,
    .create  = baymax_create,
    .release = baymax_release,
    .unlink  = baymax_unlink,
};

int main(int argc, char *argv[]) {
    struct stat st;
    if (stat(relics_dir,&st)==-1) {
        if (mkdir(relics_dir,0755)==-1) { perror("mkdir relics"); return 1; }
    } else if (!S_ISDIR(st.st_mode)) {
        fprintf(stderr,"'%s' exists but not dir\n", relics_dir); return 1;
    }
    init_logging();
    atexit(cleanup_resources);
    return fuse_main(argc,argv,&baymax_oper,NULL);
}

