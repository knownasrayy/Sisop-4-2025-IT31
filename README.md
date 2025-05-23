# Sisop-4-2025-IT31

- Erlangga Valdhio Putra Sulistio_5027241030
- Rayhan Agnan Kusuma_5027241102
- S. Farhan Baig_5027241097


# Soal 1

## Deskripsi Soal
Shorekeeper menemukan teks hexadecimal anomali di Tethys' Deep. Dimana kita disuruh untuk:
1. Ekstraksi file teks
2. Konversi hex ke gambar
3. Penamaan file dengan timestamp
4. Pencatatan log aktivitas.

---

### **1a. Ekstraksi File Anomali**
**Tujuan**:  
Mengambil file ZIP dari sumber, mengekstrak ke direktori `anomali`, dan menghapus file ZIP.

**Langkah**:
1. ### Salin file ZIP ke direktori kerja
```bash
cp '/path/anomali.zip' ~/blackshores/
```
2. ### Unzip dan hapus file
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

### 1b. Konversi Hex ke Gambar
**Tujuan**:

Mengubah string hexadecimal dalam file `.txt` menjadi file gambar.

Inti Program (`hexed.c`):
```bash
unsigned char hex_to_byte(const char *hex) {
    unsigned char byte = 0;
    for (int i = 0; i < 2; i++) {
        byte <<= 4;
        if (hex[i] >= '0' && hex[i] <= '9') {
            byte |= hex[i] - '0';
        } else if (hex[i] >= 'a' && hex[i] <= 'f') {
            byte |= hex[i] - 'a' + 10;
        } else if (hex[i] >= 'A' && hex[i] <= 'F') {
            byte |= hex[i] - 'A' + 10;
        }
    }
    return byte;
}
```
**Penjelasan Proses**:
1. Baca File Hex:
```bash
FILE *input = fopen(input_path, "r");
fread(hex_data, 1, file_size, input);
```
- Membaca seluruh isi file `.txt` ke dalam string `hex_data`.
2. Bersihkan Whitespace:
```bash
for (long j = 0; j < file_size; j++) {
    if (hex_data[j] != ' ' && hex_data[j] != '\n' && hex_data[j] != '\r') {
        clean_hex[clean_len++] = hex_data[j];
    }
}
```
- Menghapus spasi, newline (`\n`), dan carriage return (`\r`).

3. Konversi Hex ke Binary:
```bash
for (int j = 0; j < clean_len; j += 2) {
    bin_data[j/2] = hex_to_byte(&clean_hex[j]);
}
```
- Setiap 2 karakter hex (misal: `"FF"`) diubah ke 1 byte (misal: `0xFF`).

4. Tulis ke File Gambar:
```bash
FILE *output = fopen(output_path, "wb");
fwrite(bin_data, 1, clean_len / 2, output);
```
- Data binary langsung ditulis sebagai file PNG/JPEG.


### 1c. Penamaan File dengan Timestamp
**Tujuan**
Memberi nama file dengan format:
`[nama_file]_image_[YYYY-mm-dd]_[HH-MM-SS].png`

```bash
time_t now = time(NULL);
struct tm *tm = localtime(&now);
strftime(file_timestamp, sizeof(file_timestamp), "%Y-%m-%d_%H-%M-%S", tm);
snprintf(output_path, sizeof(output_path), "image/%d_image_%s.png", i, file_timestamp);
```

**Penjelasan**
1. Ambil Waktu Sekarang:
```bash
time_t now = time(NULL); 
struct tm *tm = localtime(&now); 
```

2. Format Timestamp:

- `%Y`: Tahun 4 digit (contoh: `2025`)
- `%m`: Bulan 2 digit (contoh: `05` untuk Mei)
- `%d`: Tanggal 2 digit (contoh: `17`)
- `%H`: Jam 24 jam (contoh: `22`)
- `%M`: Menit (contoh: `51`)
- `%S`: Detik (contoh: `04`)

Format akhir: `2025-05-17_22-51-04`. 

3. Pembuatan Nama File:
```bash
snprintf(output_path, ..., "image/%d_image_%s.png", i, file_timestamp);
```
Contoh hasil: `image/1_image_2025-05-17_22-51-04.png`.

### 1d. Pencatatan Log File
**Struktur Log**:
```[YYYY-mm-dd][HH:MM:SS]: Successfully converted hexadecimal text [X.txt] to [X_image_...png]```

Contoh:
```[2025-05-17][22:51:04]: Successfully converted hexadecimal text 1.txt to 1_image_2025-05-17_22-51-04.png```

### Cara Menjalankan:
1. Kompilasi:
```bash
gcc hexed.c -o hexec && chmod +x hexec
```

2. Eksekusi:
```bash
./hexec
```

3. Hasil:
```
Berhasil mengonversi anomali/1.txt → image/1_image_2025-05-22_10-40-27.png
Berhasil mengonversi anomali/2.txt → image/2_image_2025-05-22_10-40-27.png
Berhasil mengonversi anomali/3.txt → image/3_image_2025-05-22_10-40-27.png
Berhasil mengonversi anomali/4.txt → image/4_image_2025-05-22_10-40-27.png
Berhasil mengonversi anomali/5.txt → image/5_image_2025-05-22_10-40-27.png
Berhasil mengonversi anomali/6.txt → image/6_image_2025-05-22_10-40-27.png
Berhasil mengonversi anomali/7.txt → image/7_image_2025-05-22_10-40-27.png
```
### Validasi
1. Cek gambar
```bash                                                                                                                 
file image/1_image_2025-05-22_10-40-27.png
```

2. Cek log
```bash
cat conversion.log
```
3. Hasil pengubahan file `.txt.` ke `image`

![image](https://github.com/user-attachments/assets/4b53fdf7-786c-4455-b7e4-69a28047d973)



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
Use Control + Shift + m to toggle the tab key moving focus. Alternatively, use esc then tab to move to the next interactive element on the page.
No file chosen
Attach files by dragging & dropping, selecting or pasting them.
Editing Sisop-4-2025-IT31/README.md at main · knownasrayy/Sisop-4-2025-IT31

Translate

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
- Baymax_create() dan baymax_open() siapkan context dan hapus fragmen lama.
- Baymax_write() tulis data pecahan 1024 byte ke fragmen di folder relics.

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
- Baymax_unlink() hapus semua file fragmen berurutan dari folder relics dengan nama [filename].000, [filename].001, dst.

#### E. Mencatat aktivitas ke file activity.log (READ, WRITE, DELETE)
```bash
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

```
- log_event() menulis log event dengan timestamp ke file activity.log.
Di fungsi seperti baymax_open(), baymax_release(), dan baymax_unlink() dipanggil log_event() sesuai event.





# Soal 3 - AntiNK (Anti Napis Kimcun)

## Deskripsi

AntiNK adalah sistem berbasis FUSE yang digunakan untuk **mendeteksi file berbahaya** berdasarkan **nama file**, serta melakukan **transformasi nama file** dan **enkripsi isi file** sesuai kondisi tertentu. Sistem ini juga mencatat semua aktivitas yang terjadi di filesystem ke dalam log.

Filesystem ini akan berjalan di dalam container menggunakan Docker, dan mengakses file yang disediakan melalui folder mount host.

---

## Spesifikasi Soal

### A. Deteksi File Berbahaya
Jika ada file yang namanya mengandung substring **"nafis"** atau **"kimcun"**, maka:
1. **Nama file tersebut dibalik** saat ditampilkan pada mount folder.
2. Sistem **mencatat aktivitas ini dalam log** dengan format:

```
[WARNING] Detected dangerous file: <nama_file_asli>
```

**Implementasi di `antink.c`:**

- Fungsi pendeteksi nama berbahaya:
  ```c
  int is_berbahaya(const char *filename) {
	return strstr(filename, "nafis") || strstr(filename, "kimcun");
  }
  ```
- Pembalikan nama:
  ```c
  void reverse_string(char *str) {
	int len = strlen(str);
	for (int i = 0; i < len / 2; ++i) {
    	char tmp = str[i];
    	str[i] = str[len - i - 1];
    	str[len - i - 1] = tmp;
	}
  ```
- Penerapan dalam fungsi readdir:
  ```c
  static int antink_readdir(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset,
                      	struct fuse_file_info *fi, enum fuse_readdir_flags flags) {
	DIR *dp;
	struct dirent *de;
	char full_path[1024];
	snprintf(full_path, sizeof(full_path), "%s%s", source_dir, path);

	dp = opendir(full_path);
	if (dp == NULL) return -errno;

	while ((de = readdir(dp)) != NULL) {
    	const char *name = de->d_name;


    	if (strcmp(name, ".") == 0 || strcmp(name, "..") == 0)
        	continue;

    	char temp_name[1024];
    	strcpy(temp_name, name);
  ```

---

### B. Enkripsi ROT13 untuk File Aman `.txt`
Untuk **file dengan ekstensi `.txt`** yang **tidak dianggap berbahaya**, maka:
- Saat dibaca, isinya **dienkripsi dengan algoritma ROT13**.
- Tidak berlaku untuk file yang mengandung `nafis` atau `kimcun`.

**Implementasi di `antink.c`:**

- Fungsi ROT13:
  ```c
  void apply_rot13(char *buf, size_t size) {
	for (size_t i = 0; i < size; i++) {
    	if ('a' <= buf[i] && buf[i] <= 'z') {
        	buf[i] = ((buf[i] - 'a' + 13) % 26) + 'a';
    	} else if ('A' <= buf[i] && buf[i] <= 'Z') {
        	buf[i] = ((buf[i] - 'A' + 13) % 26) + 'A';
    	}
	}
  ```
- Penerapan di fungsi `read`:
  ```c
  if (strstr(full_path, ".txt") && !is_berbahaya(full_path)) {
      apply_rot13(buf, res);
  }
  ```

---

### C. Logging Akses File
Setiap file yang diakses (dibuka), akan dicatat ke file log `/logs/antink.log` dengan format:

```
[ACCESS] /<path_file>
```
**Implementasi di `antink.c`:**

- Logging dilakukan dalam:
  ```c
  static int antink_open(const char *path, struct fuse_file_info *fi) {
	char full_path[1024];
	snprintf(full_path, sizeof(full_path), "%s%s", source_dir, path);

	FILE *log = fopen(log_file, "a");
	if (log) {
    	fprintf(log, "[ACCESS] %s\n", path);
    	fclose(log);
	}

	int res = open(full_path, fi->flags);
	if (res == -1) return -errno;

	close(res);
	return 0;
  }
  ```

---

## Integrasi Docker

Filesystem ini dijalankan di container berbasis Ubuntu dengan FUSE. File akan dimount dari host ke dalam container agar bisa dipantau dan dimodifikasi.

### Struktur Docker:
- `Dockerfile`: Build image dengan dependensi FUSE.
- `docker-compose.yml`: Mount folder host ke `/source` dan `/logs` di container.
- Perintah mount: `./antink /home/antink_mount`

---

## Contoh Perilaku

### Struktur File Asli di Host `/source`
```
test.txt
nafis.csv
kimcun.txt
```

### Struktur Tampilan di Mount (`/home/antink_mount`)
```
test.txt  <- ROT13 saat dibaca
vsc.sifan   <- nama dibalik karena mengandung "nafis"
txt.nucmik  <- nama dibalik karena mengandung "kimcun"
```

### Isi Log `/logs/antink.log`
```
[WARNING] Detected dangerous file: nafis.csv
[WARNING] Detected dangerous file: kimcun.txt
[ACCESS] /test.txt
```

# Soal_4

```bash
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
```
