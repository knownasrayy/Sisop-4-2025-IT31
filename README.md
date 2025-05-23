# Sisop-4-2025-IT31

- Erlangga Valdhio Putra Sulistio_5027241030
- Rayhan Agnan Kusuma_5027241102
- S. Farhan Baig_5027241097


# Soal 1_

## Deskripsi Soal
Shorekeeper menemukan teks hexadecimal anomali di Tethys' Deep. Tugas:
1. Ekstraksi file teks
2. Konversi hex ke gambar
3. Penamaan file dengan timestamp
4. Pencatatan log aktivitas.

---

### **1a: Ekstraksi File Anomali**
**Tujuan**:  
Mengambil file ZIP dari sumber, mengekstrak ke direktori `anomali`, dan menghapus file ZIP.

**Langkah**:
1. # Salin file ZIP ke direktori kerja
```bash
cp '/path/anomali.zip' ~/blackshores/
```
2. # Unzip dan hapus file
```bash
unzip anomali.zip -d anomali
rm anomali.zip
```

Struktur Hasil:
```
anomali/
├── 1.txt
├── 2.txt
├── 3.txt
├── 4.txt
├── 5.txt
├── 6.txt
└── 7.txt
```

### 1b: Konversi Hex ke Gambar
**Tujuan**
Mengubah string hexadecimal dalam file `.txt` menjadi file gambar.

Script Awal `(hex_to_image.sh)`:
```bash
#!/bin/bash
INPUT="anomali/1.txt"
OUTPUT="image/output.png"
HEXDATA=$(cat $INPUT | tr -d ' \n\r')
echo $HEXDATA | xxd -r -p > $OUTPUT
echo "File gambar berhasil dibuat di $OUTPUT"
```

**Cara Jalankan**
```bash
chmod +x hex_to_image.sh
./hex_to_image.sh
```

### 1c: Penamaan File dengan Timestamp
**Tujuan**
Memberi nama file dengan format:
`[nama_file]_image_[YYYY-mm-dd]_[HH-MM-SS].png`

Script (`hex_to_image_timestamp.sh`):
```bash
#!/bin/bash
mkdir -p image
for input_file in anomali/*.txt; do
    base_name=$(basename "$input_file" .txt)
    timestamp=$(date +"%Y-%m-%d_%H-%M-%S")
    output_file="image/${base_name}_image_${timestamp}.png"
    hex_data=$(cat "$input_file" | tr -d ' \n\r')
    echo "$hex_data" | xxd -r -p > "$output_file"
    echo "Berhasil mengonversi $base_name.txt → $(basename "$output_file")"
done
```


# Soal_2

## Deskripsi Soal
Buatlah sebuah virtual filesystem menggunakan FUSE yang memunculkan file Baymax.jpeg secara utuh dengan cara menggabungkan 14 potongan file berukuran 1 KB yang ada di direktori relics (Baymax.jpeg.000 hingga Baymax.jpeg.013). Sistem harus memungkinkan pembuatan file baru di direktori mount dengan otomatis memecahnya menjadi potongan 1 KB dan menyimpannya di relics. Saat file dihapus dari mount, semua potongan terkait di relics juga dihapus. Semua aktivitas baca, tulis, hapus, dan salin file harus dicatat dalam file activity.log.

---

### Kode Lengkap

```bash
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
#define MAX_LOG_FRAG_DETAIL_LEN (LOG_BUFFER_SIZE - NAME_MAX - 100)


static const char *relics_dir = "relics";
static const char *log_file    = "activity.log";
static FILE *log_fp            = NULL;
static pthread_mutex_t log_mutex = PTHREAD_MUTEX_INITIALIZER;

typedef struct {
    char *name;
    int was_written;
    int max_fragment_idx_written;
} tracked_file_ctx;


static void init_logging_internal() {
    if (!log_fp) {
        log_fp = fopen(log_file, "a");
        if (!log_fp) {
            perror("CRITICAL: Failed to open log file");
            return;
        }
        setvbuf(log_fp, NULL, _IOLBF, LOG_BUFFER_SIZE);
    }
}

static void log_event(const char *event_type, const char *details) {
    pthread_mutex_lock(&log_mutex);
    if (!log_fp) {
        init_logging_internal();
        if (!log_fp) {
            pthread_mutex_unlock(&log_mutex);
            return;
        }
    }

    time_t now = time(NULL);
    struct tm *tm_info = localtime(&now);
    char timestamp[20];
    strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", tm_info);

    if (details) {
        fprintf(log_fp, "[%s] %s: %s\n", timestamp, event_type, details);
    } else {
        fprintf(log_fp, "[%s] %s\n", timestamp, event_type);
    }
    pthread_mutex_unlock(&log_mutex);
}

static void cleanup_resources() {
    pthread_mutex_lock(&log_mutex);
    if (log_fp) {
        fclose(log_fp);
        log_fp = NULL;
    }
    pthread_mutex_unlock(&log_mutex);
    pthread_mutex_destroy(&log_mutex);
}

static void remove_base_fragments(const char *base_filename) {
    DIR *dir_p = opendir(relics_dir);
    if (!dir_p) return;

    struct dirent *dir_entry;
    char fragment_path[PATH_MAX];
    size_t base_len = strlen(base_filename);

    while ((dir_entry = readdir(dir_p)) != NULL) {
        if (strncmp(dir_entry->d_name, base_filename, base_len) == 0 &&
            dir_entry->d_name[base_len] == '.' &&
            strlen(dir_entry->d_name) == base_len + 4 &&
            isdigit(dir_entry->d_name[base_len + 1]) &&
            isdigit(dir_entry->d_name[base_len + 2]) &&
            isdigit(dir_entry->d_name[base_len + 3])) {

            snprintf(fragment_path, sizeof(fragment_path), "%s/%s", relics_dir, dir_entry->d_name);
            unlink(fragment_path);
        }
    }
    closedir(dir_p);
}


static int baymax_getattr(const char *path, struct stat *stbuf) {
    memset(stbuf, 0, sizeof(struct stat));

    if (strcmp(path, "/") == 0) {
        stbuf->st_mode = S_IFDIR | 0755;
        stbuf->st_nlink = 2;
        stbuf->st_uid = getuid();
        stbuf->st_gid = getgid();
        stbuf->st_atime = stbuf->st_mtime = stbuf->st_ctime = time(NULL);
        return 0;
    }

    const char *base_filename = path + 1;
    off_t total_size = 0;
    int fragment_count = 0;
    char fragment_path[PATH_MAX];
    struct stat frag_stat;
    time_t last_mod_time = 0;

    for (int i = 0; i < MAX_FRAGMENTS; i++) {
        snprintf(fragment_path, sizeof(fragment_path), "%s/%s.%03d", relics_dir, base_filename, i);
        if (stat(fragment_path, &frag_stat) == -1) {
            if (errno == ENOENT) break;
            return -errno;
        }
        total_size += frag_stat.st_size;
        fragment_count++;
        if (frag_stat.st_mtime > last_mod_time) {
            last_mod_time = frag_stat.st_mtime;
        }
    }

    if (fragment_count == 0) return -ENOENT;

    stbuf->st_mode = S_IFREG | 0644;
    stbuf->st_nlink = 1;
    stbuf->st_size = total_size;
    stbuf->st_uid = getuid();
    stbuf->st_gid = getgid();
    stbuf->st_atime = stbuf->st_mtime = stbuf->st_ctime = last_mod_time ? last_mod_time : time(NULL);
    return 0;
}

static int baymax_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
                         off_t offset, struct fuse_file_info *fi) {
    (void)offset; (void)fi;
    if (strcmp(path, "/") != 0) return -ENOENT;

    filler(buf, ".", NULL, 0);
    filler(buf, "..", NULL, 0);

    DIR *dir_p = opendir(relics_dir);
    if (!dir_p) return -errno;

    struct dirent *dir_entry;
    char *added_basenames[MAX_FRAGMENTS] = {0};
    int basename_count = 0;
    int ret = 0;

    while ((dir_entry = readdir(dir_p)) != NULL) {
        if (dir_entry->d_name[0] == '.') continue;

        char *dot_ptr = strrchr(dir_entry->d_name, '.');
        if (dot_ptr && strlen(dot_ptr) == 4 &&
            isdigit(dot_ptr[1]) && isdigit(dot_ptr[2]) && isdigit(dot_ptr[3])) {

            size_t base_len = dot_ptr - dir_entry->d_name;
            if (base_len == 0 || base_len > NAME_MAX) continue;

            char base_name[NAME_MAX + 1];
            strncpy(base_name, dir_entry->d_name, base_len);
            base_name[base_len] = '\0';

            int found = 0;
            for (int j = 0; j < basename_count; j++) {
                if (strcmp(added_basenames[j], base_name) == 0) {
                    found = 1;
                    break;
                }
            }

            if (!found) {
                if (basename_count < MAX_FRAGMENTS) {
                    added_basenames[basename_count] = strdup(base_name);
                    if (!added_basenames[basename_count]) {
                        ret = -ENOMEM; break;
                    }
                    filler(buf, base_name, NULL, 0);
                    basename_count++;
                } else {
                    break;
                }
            }
        }
    }

    for (int j = 0; j < basename_count; j++) free(added_basenames[j]);
    closedir(dir_p);
    return ret;
}

static int baymax_access(const char *path, int mask) {
    struct stat st;
    int res = baymax_getattr(path, &st);
    if (res != 0) return res;

    if ((mask & X_OK) && S_ISREG(st.st_mode)) return -EACCES;

    return 0;
}

static int baymax_open(const char *path, struct fuse_file_info *fi) {
    const char *base_filename = path + 1;
    struct stat st;

    if ((fi->flags & O_ACCMODE) == O_RDONLY) {
        if (baymax_getattr(path, &st) == 0) {
            log_event("READ", base_filename);
        } else {
            return -ENOENT;
        }
    }

    tracked_file_ctx *ctx = calloc(1, sizeof(tracked_file_ctx));
    if (!ctx) return -ENOMEM;

    ctx->name = strdup(base_filename);
    if (!ctx->name) {
        free(ctx);
        return -ENOMEM;
    }
    ctx->was_written = 0;
    ctx->max_fragment_idx_written = -1;

    if (fi->flags & O_TRUNC) {
        if ((fi->flags & O_ACCMODE) == O_RDONLY) {
            free(ctx->name); free(ctx);
            return -EINVAL;
        }
        remove_base_fragments(base_filename);
    }

    fi->fh = (uintptr_t)ctx;
    return 0;
}

static int baymax_create(const char *path, mode_t mode, struct fuse_file_info *fi) {
    (void)mode;
    const char *base_filename = path + 1;

    remove_base_fragments(base_filename);

    tracked_file_ctx *ctx = calloc(1, sizeof(tracked_file_ctx));
    if (!ctx) return -ENOMEM;

    ctx->name = strdup(base_filename);
    if (!ctx->name) {
        free(ctx);
        return -ENOMEM;
    }
    ctx->was_written = 0;
    ctx->max_fragment_idx_written = -1;

    fi->fh = (uintptr_t)ctx;
    return 0;
}


static int baymax_read(const char *path, char *buf, size_t size,
                      off_t offset, struct fuse_file_info *fi) {
    (void)fi;
    const char *base_filename = path + 1;

    size_t total_bytes_read = 0;
    int current_fragment_idx = offset / CHUNK_SIZE;
    off_t offset_in_fragment = offset % CHUNK_SIZE;
    char fragment_path[PATH_MAX];

    while (total_bytes_read < size && current_fragment_idx < MAX_FRAGMENTS) {
        snprintf(fragment_path, sizeof(fragment_path), "%s/%s.%03d",
                 relics_dir, base_filename, current_fragment_idx);

        int fd = open(fragment_path, O_RDONLY);
        if (fd < 0) {
            if (errno == ENOENT) break;
            return -errno;
        }

        if (flock(fd, LOCK_SH) == -1) {
            close(fd); return -errno;
        }

        size_t bytes_to_read_from_chunk = CHUNK_SIZE - offset_in_fragment;
        if (bytes_to_read_from_chunk > size - total_bytes_read) {
            bytes_to_read_from_chunk = size - total_bytes_read;
        }

        ssize_t bytes_read_this_op = pread(fd, buf + total_bytes_read,
                                           bytes_to_read_from_chunk, offset_in_fragment);

        flock(fd, LOCK_UN);
        close(fd);

        if (bytes_read_this_op < 0) return -errno;
        if (bytes_read_this_op == 0) break;

        total_bytes_read += bytes_read_this_op;
        offset_in_fragment = 0;
        current_fragment_idx++;
    }
    return total_bytes_read;
}

static int baymax_write(const char *path, const char *buf, size_t size,
                       off_t offset, struct fuse_file_info *fi) {
    (void)path; //path is not directly used, ctx->name is used from fi
    tracked_file_ctx *ctx = (tracked_file_ctx*)fi->fh;
    if (!ctx) return -EIO;

    size_t total_bytes_written = 0;
    int current_fragment_idx = offset / CHUNK_SIZE;
    off_t offset_in_fragment = offset % CHUNK_SIZE;
    char fragment_path[PATH_MAX];

    ctx->was_written = 1;

    while (total_bytes_written < size && current_fragment_idx < MAX_FRAGMENTS) {
        snprintf(fragment_path, sizeof(fragment_path), "%s/%s.%03d",
                 relics_dir, ctx->name, current_fragment_idx);

        int fd = open(fragment_path, O_WRONLY | O_CREAT, 0644);
        if (fd < 0) return -errno;

        if (flock(fd, LOCK_EX) == -1) {
            close(fd); return -errno;
        }

        size_t bytes_to_write_to_chunk = CHUNK_SIZE - offset_in_fragment;
        if (bytes_to_write_to_chunk > size - total_bytes_written) {
            bytes_to_write_to_chunk = size - total_bytes_written;
        }

        ssize_t bytes_written_this_op = pwrite(fd, buf + total_bytes_written,
                                               bytes_to_write_to_chunk, offset_in_fragment);

        flock(fd, LOCK_UN);
        close(fd);

        if (bytes_written_this_op < 0) return -errno;
        if ((size_t)bytes_written_this_op != bytes_to_write_to_chunk) return -EIO;

        total_bytes_written += bytes_written_this_op;
        if (current_fragment_idx > ctx->max_fragment_idx_written) {
            ctx->max_fragment_idx_written = current_fragment_idx;
        }

        offset_in_fragment = 0;
        current_fragment_idx++;
        if (current_fragment_idx >= MAX_FRAGMENTS && total_bytes_written < size) {
            break;
        }
    }
    return total_bytes_written;
}

static int baymax_release(const char *path, struct fuse_file_info *fi) {
    (void)path;
    tracked_file_ctx *ctx = (tracked_file_ctx*)fi->fh;

    if (ctx) {
        if (ctx->was_written && ctx->max_fragment_idx_written >= 0) {
            char *frag_details_buffer = malloc(MAX_LOG_FRAG_DETAIL_LEN);
            char log_msg_buffer[LOG_BUFFER_SIZE];

            if (!frag_details_buffer) {
                snprintf(log_msg_buffer, sizeof(log_msg_buffer), "%s (%d fragments created)",
                         ctx->name, ctx->max_fragment_idx_written + 1);
                log_event("WRITE", log_msg_buffer);
            } else {
                frag_details_buffer[0] = '\0';
                size_t current_len = 0;
                int first_frag = 1;

                for (int i = 0; i <= ctx->max_fragment_idx_written; i++) {
                    char frag_name_part[NAME_MAX + 10];
                    snprintf(frag_name_part, sizeof(frag_name_part), "%s.%03d", ctx->name, i);

                    size_t part_len = strlen(frag_name_part);
                    size_t needed_len = part_len + (first_frag ? 0 : 2);

                    if (current_len + needed_len >= MAX_LOG_FRAG_DETAIL_LEN - 4) {
                        if (current_len < MAX_LOG_FRAG_DETAIL_LEN - 4) {
                           strncat(frag_details_buffer, "...", MAX_LOG_FRAG_DETAIL_LEN - current_len - 1);
                        }
                        break;
                    }

                    if (!first_frag) {
                        strncat(frag_details_buffer, ", ", MAX_LOG_FRAG_DETAIL_LEN - current_len - 1);
                        current_len += 2;
                    }
                    strncat(frag_details_buffer, frag_name_part, MAX_LOG_FRAG_DETAIL_LEN - current_len - 1);
                    current_len += part_len;
                    first_frag = 0;
                }

                snprintf(log_msg_buffer, sizeof(log_msg_buffer), "%s -> %s", ctx->name, frag_details_buffer);
                log_event("WRITE", log_msg_buffer);
                free(frag_details_buffer);
            }
        }
        free(ctx->name);
        free(ctx);
        fi->fh = 0;
    }
    return 0;
}
static int baymax_unlink(const char *path) {
    const char *base_filename = path + 1;
    int fragments_unlinked_count = 0;
    char fragment_path[PATH_MAX];
    char first_frag_name_for_log[NAME_MAX + 5] = "";
    char last_frag_name_for_log[NAME_MAX + 5] = "";

    for (int i = 0; i < MAX_FRAGMENTS; i++) {
        snprintf(fragment_path, sizeof(fragment_path), "%s/%s.%03d",
                 relics_dir, base_filename, i);

        struct stat st_frag_check;
        if (stat(fragment_path, &st_frag_check) == -1) {
            if (errno == ENOENT) break;
            return -errno;
        }

        int fd = open(fragment_path, O_WRONLY);
        if (fd >= 0) {
            if (flock(fd, LOCK_EX) == -1) {
                close(fd);
                return -errno;
            }

            if (unlink(fragment_path) == 0) {
                 if (fragments_unlinked_count == 0) {
                    snprintf(first_frag_name_for_log, sizeof(first_frag_name_for_log), "%s.%03d", base_filename, i);
                }
                snprintf(last_frag_name_for_log, sizeof(last_frag_name_for_log), "%s.%03d", base_filename, i);
                fragments_unlinked_count++;
            }
            close(fd);
        } else {
            if (errno == ENOENT) continue;
            return -errno;
        }
    }

    if (fragments_unlinked_count == 0) return -ENOENT;

    char log_details[NAME_MAX * 2 + 20];
    if (fragments_unlinked_count == 1) {
        snprintf(log_details, sizeof(log_details), "%s", first_frag_name_for_log);
    } else {
        snprintf(log_details, sizeof(log_details), "%s - %s", first_frag_name_for_log, last_frag_name_for_log);
    }
    log_event("DELETE", log_details);

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
    struct stat st_relics;
    
    if (stat(relics_dir, &st_relics) == -1) {
        if (errno == ENOENT) {
            if (mkdir(relics_dir, 0755) == -1) {
                perror("ERROR: Failed to create relics directory");
                return 1;
            }
            
        } else {
            perror("ERROR: Failed to stat relics directory");
            return 1;
        }
        
    } else if (!S_ISDIR(st_relics.st_mode)) {
        fprintf(stderr, "ERROR: '%s' exists but is not a directory.\n", relics_dir);
        return 1;
    }
    
    pthread_mutex_lock(&log_mutex);
    init_logging_internal();
    pthread_mutex_unlock(&log_mutex);

    atexit(cleanup_resources);

    return fuse_main(argc, argv, &baymax_oper, NULL);
}
```

### Penjelasan Kode

#### A. Sistem file virtual FUSE menampilkan file utuh dari fragment di folder relics

```bash
static int baymax_getattr(const char *path, struct stat *stbuf) {
    memset(stbuf, 0, sizeof(struct stat));
    
    if (strcmp(path, "/") == 0) {
        stbuf->st_mode = S_IFDIR | 0755;
        stbuf->st_nlink = 2;
        stbuf->st_uid = getuid();
        stbuf->st_gid = getgid();
        stbuf->st_atime = stbuf->st_mtime = stbuf->st_ctime = time(NULL);
        return 0;
    }

    const char *base_filename = path + 1;
    off_t total_size = 0;
    int fragment_count = 0;
    char fragment_path[PATH_MAX];
    struct stat frag_stat;
    time_t last_mod_time = 0;

    for (int i = 0; i < MAX_FRAGMENTS; i++) {
        snprintf(fragment_path, sizeof(fragment_path), "%s/%s.%03d", relics_dir, base_filename, i);
        if (stat(fragment_path, &frag_stat) == -1) {
            if (errno == ENOENT) break;
            return -errno;
        }
        total_size += frag_stat.st_size;
        fragment_count++;
        if (frag_stat.st_mtime > last_mod_time) last_mod_time = frag_stat.st_mtime;
    }

    if (fragment_count == 0) return -ENOENT;

    stbuf->st_mode = S_IFREG | 0644;
    stbuf->st_nlink = 1;
    stbuf->st_size = total_size;
    stbuf->st_uid = getuid();
    stbuf->st_gid = getgid();
    stbuf->st_atime = stbuf->st_mtime = stbuf->st_ctime = last_mod_time ? last_mod_time : time(NULL);
    return 0;
}

static int baymax_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
                         off_t offset, struct fuse_file_info *fi) {
    (void)offset; (void)fi;
    if (strcmp(path, "/") != 0) return -ENOENT;

    filler(buf, ".", NULL, 0);
    filler(buf, "..", NULL, 0);

    DIR *dir_p = opendir(relics_dir);
    if (!dir_p) return -errno;

    struct dirent *dir_entry;
    char *added_basenames[MAX_FRAGMENTS] = {0};
    int basename_count = 0;
    int ret = 0;

    while ((dir_entry = readdir(dir_p)) != NULL) {
        if (dir_entry->d_name[0] == '.') continue;

        char *dot_ptr = strrchr(dir_entry->d_name, '.');
        if (dot_ptr && strlen(dot_ptr) == 4 &&
            isdigit(dot_ptr[1]) && isdigit(dot_ptr[2]) && isdigit(dot_ptr[3])) {

            size_t base_len = dot_ptr - dir_entry->d_name;
            if (base_len == 0 || base_len > NAME_MAX) continue;

            char base_name[NAME_MAX + 1];
            strncpy(base_name, dir_entry->d_name, base_len);
            base_name[base_len] = '\0';

            int found = 0;
            for (int j = 0; j < basename_count; j++) {
                if (strcmp(added_basenames[j], base_name) == 0) {
                    found = 1;
                    break;
                }
            }

            if (!found) {
                if (basename_count < MAX_FRAGMENTS) {
                    added_basenames[basename_count] = strdup(base_name);
                    if (!added_basenames[basename_count]) {
                        ret = -ENOMEM; break;
                    }
                    filler(buf, base_name, NULL, 0);
                    basename_count++;
                } else {
                    break;
                }
            }
        }
    }

    for (int j = 0; j < basename_count; j++) free(added_basenames[j]);
    closedir(dir_p);
    return ret;
}

```
- Fungsi baymax_getattr() menghitung ukuran file utuh dari total fragmen di folder relics.
- Fungsi baymax_readdir() menampilkan nama base file dari fragmen agar user lihat file utuh.

#### B. File di mount point terlihat utuh walau aslinya pecahan
```bash
static int baymax_read(const char *path, char *buf, size_t size,
                      off_t offset, struct fuse_file_info *fi) {
    (void)fi;
    const char *base_filename = path + 1;

    size_t total_bytes_read = 0;
    int current_fragment_idx = offset / CHUNK_SIZE;
    off_t offset_in_fragment = offset % CHUNK_SIZE;
    char fragment_path[PATH_MAX];

    while (total_bytes_read < size && current_fragment_idx < MAX_FRAGMENTS) {
        snprintf(fragment_path, sizeof(fragment_path), "%s/%s.%03d",
                 relics_dir, base_filename, current_fragment_idx);

        int fd = open(fragment_path, O_RDONLY);
        if (fd < 0) {
            if (errno == ENOENT) break;
            return -errno;
        }

        if (flock(fd, LOCK_SH) == -1) {
            close(fd); return -errno;
        }

        size_t bytes_to_read = CHUNK_SIZE - offset_in_fragment;
        if (bytes_to_read > size - total_bytes_read) bytes_to_read = size - total_bytes_read;

        ssize_t bytes_read = pread(fd, buf + total_bytes_read, bytes_to_read, offset_in_fragment);

        flock(fd, LOCK_UN);
        close(fd);

        if (bytes_read < 0) return -errno;
        if (bytes_read == 0) break;

        total_bytes_read += bytes_read;
        offset_in_fragment = 0;
        current_fragment_idx++;
    }
    return total_bytes_read;
}

```
- Baymax_read() baca fragmen satu per satu sesuai offset dan ukuran baca, gabungkan hasilnya jadi satu kesatuan.

#### C. Membuat file baru otomatis pecah ke fragmen 1KB di relics
```bash
static int baymax_create(const char *path, mode_t mode, struct fuse_file_info *fi) {
    (void)mode;
    const char *base_filename = path + 1;

    remove_base_fragments(base_filename);

    tracked_file_ctx *ctx = calloc(1, sizeof(tracked_file_ctx));
    if (!ctx) return -ENOMEM;

    ctx->name = strdup(base_filename);
    if (!ctx->name) {
        free(ctx);
        return -ENOMEM;
    }
    ctx->was_written = 0;
    ctx->max_fragment_idx_written = -1;

    fi->fh = (uintptr_t)ctx;
    return 0;
}

static int baymax_write(const char *path, const char *buf, size_t size,
                       off_t offset, struct fuse_file_info *fi) {
    (void)path;
    tracked_file_ctx *ctx = (tracked_file_ctx*)fi->fh;
    if (!ctx) return -EIO;

    size_t total_bytes_written = 0;
    int current_fragment_idx = offset / CHUNK_SIZE;
    off_t offset_in_fragment = offset % CHUNK_SIZE;
    char fragment_path[PATH_MAX];

    ctx->was_written = 1;

    while (total_bytes_written < size && current_fragment_idx < MAX_FRAGMENTS) {
        snprintf(fragment_path, sizeof(fragment_path), "%s/%s.%03d",
                 relics_dir, ctx->name, current_fragment_idx);

        int fd = open(fragment_path, O_WRONLY | O_CREAT, 0644);
        if (fd < 0) return -errno;

        if (flock(fd, LOCK_EX) == -1) {
            close(fd); return -errno;
        }

        size_t bytes_to_write = CHUNK_SIZE - offset_in_fragment;
        if (bytes_to_write > size - total_bytes_written) bytes_to_write = size - total_bytes_written;

        ssize_t bytes_written = pwrite(fd, buf + total_bytes_written, bytes_to_write, offset_in_fragment);

        flock(fd, LOCK_UN);
        close(fd);

        if (bytes_written < 0) return -errno;
        if ((size_t)bytes_written != bytes_to_write) return -EIO;

        total_bytes_written += bytes_written;
        if (current_fragment_idx > ctx->max_fragment_idx_written) ctx->max_fragment_idx_written = current_fragment_idx;

        offset_in_fragment = 0;
        current_fragment_idx++;
    }
    return total_bytes_written;
}

```
- baymax_create() dan baymax_open() siapkan context dan hapus fragmen lama.
- baymax_write() tulis data pecahan 1024 byte ke fragmen di folder relics.

#### D. Menghapus file di mount point juga menghapus semua fragmen di relics
```bash
static int baymax_unlink(const char *path) {
    const char *base_filename = path + 1;
    int fragments_unlinked_count = 0;
    char fragment_path[PATH_MAX];
    char first_frag_name_for_log[NAME_MAX + 5] = "";
    char last_frag_name_for_log[NAME_MAX + 5] = "";

    for (int i = 0; i < MAX_FRAGMENTS; i++) {
        snprintf(fragment_path, sizeof(fragment_path), "%s/%s.%03d",
                 relics_dir, base_filename, i);

        struct stat st_frag_check;
        if (stat(fragment_path, &st_frag_check) == -1) {
            if (errno == ENOENT) break;
            return -errno;
        }

        int fd = open(fragment_path, O_WRONLY);
        if (fd >= 0) {
            if (flock(fd, LOCK_EX) == -1) {
                close(fd);
                return -errno;
            }

            if (unlink(fragment_path) == 0) {
                if (fragments_unlinked_count == 0) {
                    snprintf(first_frag_name_for_log, sizeof(first_frag_name_for_log), "%s.%03d", base_filename, i);
                }
                snprintf(last_frag_name_for_log, sizeof(last_frag_name_for_log), "%s.%03d", base_filename, i);
                fragments_unlinked_count++;
            }
            close(fd);
        } else {
            if (errno == ENOENT) continue;
            return -errno;
        }
    }

    if (fragments_unlinked_count == 0) return -ENOENT;

    char log_details[NAME_MAX * 2 + 20];
    if (fragments_unlinked_count == 1) {
        snprintf(log_details, sizeof(log_details), "%s", first_frag_name_for_log);
    } else {
        snprintf(log_details, sizeof(log_details), "%s - %s", first_frag_name_for_log, last_frag_name_for_log);
    }
    log_event("DELETE", log_details);

    return 0;
}

```
- baymax_unlink() hapus semua file fragmen berurutan dari folder relics dengan nama [filename].000, [filename].001, dst.



