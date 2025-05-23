#define FUSE_USE_VERSION 31
#include <fuse3/fuse.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <ctype.h>
#include <sys/wait.h>

#define BASE_ORIG_DIR "chiho"
#define BASE_FUSE_DIR "fuse_dir"

#define AES_KEY "12345678901234567890123456789012"
#define AES_IV  "1234567890123456"

static const char *areas[] = {
    "starter",
    "metro",
    "dragon",
    "blackrose",
    "heaven",
    "skystreet",
    "7sref"
};

#define AREA_COUNT 7

static int get_area_and_file(const char *path, char *area_out, char *file_out, size_t file_out_size) {
    if(path[0] != '/')
        return -1;
    const char *p = path + 1;
    for (int i=0; i<AREA_COUNT; i++) {
        size_t alen = strlen(areas[i]);
        if (strncmp(p, areas[i], alen) == 0 && (p[alen] == '/' || p[alen] == '\0')) {
            strcpy(area_out, areas[i]);
            const char *fname = p + alen;
            if (fname[0] == '/')
                fname++;
            else
                fname = "";
            strncpy(file_out, fname, file_out_size-1);
            file_out[file_out_size-1] = '\0';
            return 0;
        }
    }
    return -1;
}

static void rot13(char *s) {
    while (*s) {
        if ('a' <= *s && *s <= 'z')
            *s = 'a' + (*s - 'a' + 13) % 26;
        else if ('A' <= *s && *s <= 'Z')
            *s = 'A' + (*s - 'A' + 13) % 26;
        s++;
    }
}

static int exec_cmd(const char *cmd) {
    int ret = system(cmd);
    if (ret == -1) return -1;
    else if (WIFEXITED(ret) && WEXITSTATUS(ret) == 0) return 0;
    else return -1;
}

static int compose_orig_path(const char *area, const char *filename, char *out_path, size_t out_size, int for_write) {
    if (strcmp(area, "starter") == 0) {
        snprintf(out_path, out_size, "%s/%s/%s.mai", BASE_ORIG_DIR, area, filename);
    }
    else if (strcmp(area, "metro") == 0) {
        snprintf(out_path, out_size, "%s/%s/shifted/%s", BASE_ORIG_DIR, area, filename);
    }
    else if (strcmp(area, "dragon") == 0) {
        snprintf(out_path, out_size, "%s/%s/%s", BASE_ORIG_DIR, area, filename);
    }
    else if (strcmp(area, "blackrose") == 0) {
        snprintf(out_path, out_size, "%s/%s/%s", BASE_ORIG_DIR, area, filename);
    }
    else if (strcmp(area, "heaven") == 0) {
        snprintf(out_path, out_size, "%s/%s/%s.enc", BASE_ORIG_DIR, area, filename);
    }
    else if (strcmp(area, "skystreet") == 0) {
        snprintf(out_path, out_size, "%s/%s/%s.gz", BASE_ORIG_DIR, area, filename);
    }
    else if (strcmp(area, "7sref") == 0) {
        char area2[32];
        char file2[256];
        const char *us = strchr(filename, '_');
        if (!us) return -1;
        size_t a_len = us - filename;
        if (a_len >= sizeof(area2)) return -1;
        strncpy(area2, filename, a_len);
        area2[a_len] = '\0';
        strcpy(file2, us + 1);
        return compose_orig_path(area2, file2, out_path, out_size, for_write);
    } else {
        return -1;
    }
    return 0;
}

static int chiho_getattr(const char *path, struct stat *stbuf, struct fuse_file_info *fi) {
    (void) fi;
    memset(stbuf, 0, sizeof(struct stat));

    if (strcmp(path, "/") == 0) {
        stbuf->st_mode = S_IFDIR | 0755;
        stbuf->st_nlink = 2;
        return 0;
    }

    char area[32], filename[512];
    if (get_area_and_file(path, area, filename, sizeof(filename)) != 0) {
        return -ENOENT;
    }

    if (filename[0] == '\0') {
        stbuf->st_mode = S_IFDIR | 0755;
        stbuf->st_nlink = 2;
        return 0;
    }

    char orig_path[1024];
    if (compose_orig_path(area, filename, orig_path, sizeof(orig_path), 0) != 0) {
        return -ENOENT;
    }

    if (stat(orig_path, stbuf) == -1) {
        return -ENOENT;
    }

    return 0;
}

static ssize_t read_whole_file(const char *path, char **buf) {
    FILE *f = fopen(path, "rb");
    if (!f) return -1;
    if (fseek(f, 0, SEEK_END) != 0) {
        fclose(f);
        return -1;
    }
    long size = ftell(f);
    if (size < 0) {
        fclose(f);
        return -1;
    }
    rewind(f);
    *buf = malloc(size);
    if (!*buf) {
        fclose(f);
        return -1;
    }
    if (fread(*buf, 1, size, f) != (size_t)size) {
        free(*buf);
        fclose(f);
        return -1;
    }
    fclose(f);
    return size;
}

static int write_whole_file(const char *path, const char *buf, size_t size) {
    FILE *f = fopen(path, "wb");
    if (!f) return -1;
    if (fwrite(buf, 1, size, f) != size) {
        fclose(f);
        return -1;
    }
    fclose(f);
    return 0;
}

static int chiho_read(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fi) {
    (void) fi;

    char area[32], filename[512];
    if (get_area_and_file(path, area, filename, sizeof(filename)) != 0)
        return -ENOENT;

    if (filename[0] == '\0')
        return -EISDIR;

    char orig_path[1024];
    if (compose_orig_path(area, filename, orig_path, sizeof(orig_path), 0) != 0)
        return -ENOENT;

    if (strcmp(area, "starter") == 0) {
        char orig_file[1024];
        snprintf(orig_file, sizeof(orig_file), "%s/%s/%s.mai", BASE_ORIG_DIR, area, filename);
        FILE *f = fopen(orig_file, "rb");
        if (!f) return -ENOENT;
        if (fseek(f, offset, SEEK_SET) != 0) {
            fclose(f);
            return -EIO;
        }
        size_t res = fread(buf, 1, size, f);
        fclose(f);
        return res;
    }
    else if (strcmp(area, "metro") == 0) {
        char orig_file[1024];
        snprintf(orig_file, sizeof(orig_file), "%s/%s/shifted/%s", BASE_ORIG_DIR, area, filename);
        FILE *f = fopen(orig_file, "rb");
        if (!f) return -ENOENT;
        if (fseek(f, offset, SEEK_SET) != 0) {
            fclose(f);
            return -EIO;
        }
        size_t res = fread(buf, 1, size, f);
        fclose(f);
        return res;
    }
    else if (strcmp(area, "dragon") == 0) {
        char *data = NULL;
        ssize_t sz = read_whole_file(orig_path, &data);
        if (sz < 0) return -ENOENT;
        for (ssize_t i=0; i<sz; i++) {
            char c = data[i];
            if (('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z')) {
                if (('a' <= c && c <= 'z'))
                    data[i] = 'a' + (c - 'a' + 13) % 26;
                else
                    data[i] = 'A' + (c - 'A' + 13) % 26;
            }
        }
        if ((off_t)sz <= offset) {
            free(data);
            return 0;
        }
        size_t tocopy = sz - offset;
        if (tocopy > size) tocopy = size;
        memcpy(buf, data + offset, tocopy);
        free(data);
        return tocopy;
    }
    else if (strcmp(area, "blackrose") == 0) {
        FILE *f = fopen(orig_path, "rb");
        if (!f) return -ENOENT;
        if (fseek(f, offset, SEEK_SET) != 0) {
            fclose(f);
            return -EIO;
        }
        size_t res = fread(buf, 1, size, f);
        fclose(f);
        return res;
    }
    else if (strcmp(area, "heaven") == 0) {
        char tmpfile[] = "/tmp/chiho_heaven_decrypt_XXXXXX";
        int fd = mkstemp(tmpfile);
        if (fd < 0) return -EIO;
        close(fd);
        char cmd[2048];
        snprintf(cmd, sizeof(cmd),
                 "openssl enc -d -aes-256-cbc -K %s -iv %s -in '%s' -out '%s' 2>/dev/null",
                 "3132333435363738393031323334353637383930313233343536373839303132",
                 "31323334353637383930313233343536",
                 orig_path, tmpfile);
        int ret = exec_cmd(cmd);
        if (ret != 0) {
            unlink(tmpfile);
            return -EIO;
        }
        FILE *f = fopen(tmpfile, "rb");
        if (!f) {
            unlink(tmpfile);
            return -EIO;
        }
        if (fseek(f, offset, SEEK_SET) != 0) {
            fclose(f);
            unlink(tmpfile);
            return -EIO;
        }
        size_t res = fread(buf, 1, size, f);
        fclose(f);
        unlink(tmpfile);
        return res;
    }
    else if (strcmp(area, "skystreet") == 0) {
        char cmd[1024];
        snprintf(cmd, sizeof(cmd), "gzip -dc '%s'", orig_path);
        FILE *pipe = popen(cmd, "r");
        if (!pipe) return -EIO;
        if (offset > 0) {
            size_t discarded = 0;
            char discard_buf[4096];
            while (discarded < (size_t)offset) {
                size_t toread = sizeof(discard_buf);
                if (discarded + toread > (size_t)offset)
                    toread = (size_t)offset - discarded;
                size_t r = fread(discard_buf, 1, toread, pipe);
                if (r == 0) break;
                discarded += r;
            }
        }
        size_t res = fread(buf, 1, size, pipe);
        pclose(pipe);
        return res;
    }
    else if (strcmp(area, "7sref") == 0) {
        return chiho_read(path, buf, size, offset, fi);
    }
    return -ENOENT;
}

static int chiho_write(const char *path, const char *buf, size_t size, off_t offset, struct fuse_file_info *fi) {
    (void) fi;
    char area[32], filename[512];
    if (get_area_and_file(path, area, filename, sizeof(filename)) != 0)
        return -ENOENT;
    if (filename[0] == '\0')
        return -EISDIR;
    char orig_path[1024];
    if (compose_orig_path(area, filename, orig_path, sizeof(orig_path), 1) != 0)
        return -ENOENT;
    if (offset != 0) return -ENOTSUP;
    if (strcmp(area, "starter") == 0) {
        int fd = open(orig_path, O_WRONLY | O_CREAT | O_TRUNC, 0644);
        if (fd < 0) return -EIO;
        ssize_t written = write(fd, buf, size);
        close(fd);
        return written;
    }
    else if (strcmp(area, "metro") == 0) {
        char dirpath[512];
        snprintf(dirpath, sizeof(dirpath), "%s/%s/shifted", BASE_ORIG_DIR, area);
        mkdir(dirpath, 0755);
        int fd = open(orig_path, O_WRONLY | O_CREAT | O_TRUNC, 0644);
        if (fd < 0) return -EIO;
        ssize_t written = write(fd, buf, size);
        close(fd);
        return written;
    }
    else if (strcmp(area, "dragon") == 0) {
        char *tmpbuf = malloc(size);
        if (!tmpbuf) return -ENOMEM;
        memcpy(tmpbuf, buf, size);
        rot13(tmpbuf);
        int fd = open(orig_path, O_WRONLY | O_CREAT | O_TRUNC, 0644);
        if (fd < 0) {
            free(tmpbuf);
            return -EIO;
        }
        ssize_t written = write(fd, tmpbuf, size);
        close(fd);
        free(tmpbuf);
        return written;
    }
    else if (strcmp(area, "blackrose") == 0) {
        int fd = open(orig_path, O_WRONLY | O_CREAT | O_TRUNC, 0644);
        if (fd < 0) return -EIO;
        ssize_t written = write(fd, buf, size);
        close(fd);
        return written;
    }
    else if (strcmp(area, "heaven") == 0) {
        char tmpin[] = "/tmp/chiho_heaven_in_XXXXXX";
        int fd = mkstemp(tmpin);
        if (fd < 0) return -EIO;
        write(fd, buf, size);
        close(fd);
        char cmd[2048];
        snprintf(cmd, sizeof(cmd),
                 "openssl enc -aes-256-cbc -K %s -iv %s -in '%s' -out '%s' 2>/dev/null",
                 "3132333435363738393031323334353637383930313233343536373839303132",
                 "31323334353637383930313233343536",
                 tmpin, orig_path);
        int ret = exec_cmd(cmd);
        unlink(tmpin);
        if (ret != 0) return -EIO;
        return size;
    }
    else if (strcmp(area, "skystreet") == 0) {
        char tmpin[] = "/tmp/chiho_skystreet_in_XXXXXX";
        int fd = mkstemp(tmpin);
        if (fd < 0) return -EIO;
        write(fd, buf, size);
        close(fd);
        char tmpout[] = "/tmp/chiho_skystreet_out_XXXXXX.gz";
        int fd2 = mkstemps(tmpout, 3);
        if (fd2 < 0) {
            unlink(tmpin);
            return -EIO;
        }
        close(fd2);
        char cmd[2048];
        snprintf(cmd, sizeof(cmd),
                 "gzip -c '%s' > '%s'",
                 tmpin, tmpout);
        int ret = exec_cmd(cmd);
        if (ret != 0) {
            unlink(tmpin);
            unlink(tmpout);
            return -EIO;
        }
        ret = rename(tmpout, orig_path);
        unlink(tmpin);
        if (ret != 0) return -EIO;
        return size;
    }
    else if (strcmp(area, "7sref") == 0) {
        return chiho_write(path, buf, size, offset, fi);
    }
    return -ENOENT;
}

static int chiho_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
                         off_t offset, struct fuse_file_info *fi,
                         enum fuse_readdir_flags flags) {
    (void) offset;
    (void) fi;
    (void) flags;

    if (strcmp(path, "/") == 0) {
        for (int i=0; i<AREA_COUNT; i++) {
            filler(buf, areas[i], NULL, 0, 0);
        }
        return 0;
    }

    char area[32], filename[512];
    if (get_area_and_file(path, area, filename, sizeof(filename)) != 0) {
        return -ENOENT;
    }

    if (filename[0] != '\0') {
        return -ENOTDIR;
    }

    char dirpath[1024];
    if (strcmp(area, "starter") == 0) {
        snprintf(dirpath, sizeof(dirpath), "%s/%s", BASE_ORIG_DIR, area);
    }
    else if (strcmp(area, "metro") == 0) {
        snprintf(dirpath, sizeof(dirpath), "%s/%s/shifted", BASE_ORIG_DIR, area);
    }
    else if (strcmp(area, "7sref") == 0) {
        filler(buf, ".", NULL, 0, 0);
        filler(buf, "..", NULL, 0, 0);
        for (int i=0; i<AREA_COUNT-1; i++) { 
            if (strcmp(areas[i], "7sref") == 0) continue;
            snprintf(dirpath, sizeof(dirpath), "%s/%s", BASE_ORIG_DIR, areas[i]);
            DIR *d = opendir(dirpath);
            if (!d) continue;
            struct dirent *entry;
            while ((entry = readdir(d)) != NULL) {
                if (entry->d_name[0] == '.') continue;
                char name[512];
                snprintf(name, sizeof(name), "%s_%s", areas[i], entry->d_name);
                filler(buf, name, NULL, 0, 0);
            }
            closedir(d);
        }
        return 0;
    }
    else {
        snprintf(dirpath, sizeof(dirpath), "%s/%s", BASE_ORIG_DIR, area);
    }

    DIR *d = opendir(dirpath);
    if (!d) return -ENOENT;
    struct dirent *entry;
    while ((entry = readdir(d)) != NULL) {
        if (entry->d_name[0] == '.') continue;
        if (strcmp(area, "starter") == 0) {
            size_t len = strlen(entry->d_name);
            if (len > 4 && strcmp(entry->d_name + len - 4, ".mai") == 0) {
                char fname[512];
                strncpy(fname, entry->d_name, len-4);
                fname[len-4] = '\0';
                filler(buf, fname, NULL, 0, 0);
            }
        } else {
            filler(buf, entry->d_name, NULL, 0, 0);
        }
    }
    closedir(d);
    return 0;
}

static struct fuse_operations chiho_oper = {
    .getattr = chiho_getattr,
    .readdir = chiho_readdir,
    .read = chiho_read,
    .write = chiho_write,
};

int main(int argc, char *argv[]) {
    return fuse_main(argc, argv, &chiho_oper, NULL);
}
