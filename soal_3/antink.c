#define FUSE_USE_VERSION 31
#include <fuse3/fuse.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <dirent.h>

static const char *source_dir = "/source";
static const char *log_file = "/logs/antink.log";


int is_berbahaya(const char *filename) {
	return strstr(filename, "nafis") || strstr(filename, "kimcun");
}


void reverse_string(char *str) {
	int len = strlen(str);
	for (int i = 0; i < len / 2; ++i) {
    	char tmp = str[i];
    	str[i] = str[len - i - 1];
    	str[len - i - 1] = tmp;
	}
}


void apply_rot13(char *buf, size_t size) {
	for (size_t i = 0; i < size; i++) {
    	if ('a' <= buf[i] && buf[i] <= 'z') {
        	buf[i] = ((buf[i] - 'a' + 13) % 26) + 'a';
    	} else if ('A' <= buf[i] && buf[i] <= 'Z') {
        	buf[i] = ((buf[i] - 'A' + 13) % 26) + 'A';
    	}
	}
}


static int antink_getattr(const char *path, struct stat *stbuf, struct fuse_file_info *fi) {
	char full_path[1024];
	snprintf(full_path, sizeof(full_path), "%s%s", source_dir, path);
	return lstat(full_path, stbuf);
}


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


    	if (is_berbahaya(name)) {
        	FILE *log = fopen(log_file, "a");
        	if (log) {
            	fprintf(log, "[WARNING] Detected dangerous file: %s\n", name);
            	fclose(log);
        	}
        	reverse_string(temp_name);
    	}

    	filler(buf, temp_name, NULL, 0, 0);
	}

	closedir(dp);
	return 0;
}


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

static int antink_read(const char *path, char *buf, size_t size, off_t offset,
                   	struct fuse_file_info *fi) {
	char full_path[1024];
	snprintf(full_path, sizeof(full_path), "%s%s", source_dir, path);

	int fd = open(full_path, O_RDONLY);
	if (fd == -1) return -errno;

	int res = pread(fd, buf, size, offset);
	if (res == -1) {
    	res = -errno;
	} else {

    	if (strstr(full_path, ".txt") && !is_berbahaya(full_path)) {
        	apply_rot13(buf, res);
    	}
	}

	close(fd);
	return res;
}

static struct fuse_operations antink_oper = {
	.getattr = antink_getattr,
	.readdir = antink_readdir,
	.open	= antink_open,
	.read	= antink_read,
};

int main(int argc, char *argv[]) {
	return fuse_main(argc, argv, &antink_oper, NULL);
}

