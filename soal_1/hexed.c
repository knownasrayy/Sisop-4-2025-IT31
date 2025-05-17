#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/stat.h>

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

int main() {
    const char *input_dir = "anomali";
    const char *output_dir = "image";
    const char *log_file = "conversion.log";

    mkdir(output_dir, 0777);

    FILE *log = fopen(log_file, "a");
    if (!log) {
        perror("Gagal membuka log file");
        exit(1);
    }

    for (int i = 1; i <= 7; i++) {
        char input_path[256];
        snprintf(input_path, sizeof(input_path), "%s/%d.txt", input_dir, i);

        FILE *input = fopen(input_path, "r");
        if (!input) {
            perror("Gagal membuka file input");
            continue;
        }

        fseek(input, 0, SEEK_END);
        long file_size = ftell(input);
        fseek(input, 0, SEEK_SET);

        char *hex_data = malloc(file_size + 1);
        fread(hex_data, 1, file_size, input);
        hex_data[file_size] = '\0';
        fclose(input);

        char *clean_hex = malloc(file_size + 1);
        int clean_len = 0;
        for (long j = 0; j < file_size; j++) {
            if (hex_data[j] != ' ' && hex_data[j] != '\n' && hex_data[j] != '\r') {
                clean_hex[clean_len++] = hex_data[j];
            }
        }
        clean_hex[clean_len] = '\0';
        free(hex_data);

        unsigned char *bin_data = malloc(clean_len / 2);
        for (int j = 0; j < clean_len; j += 2) {
            bin_data[j/2] = hex_to_byte(&clean_hex[j]);
        }
        free(clean_hex);

        time_t now = time(NULL);
        struct tm *tm = localtime(&now);

        char file_timestamp[20];
        strftime(file_timestamp, sizeof(file_timestamp), "%Y-%m-%d_%H-%M-%S", tm);

        char log_timestamp[20];
        strftime(log_timestamp, sizeof(log_timestamp), "[%Y-%m-%d][%H:%M:%S]", tm);

        char output_path[256];
        snprintf(output_path, sizeof(output_path), "%s/%d_image_%s.png", output_dir, i, file_timestamp);

        FILE *output = fopen(output_path, "wb");
        if (!output) {
            perror("Gagal membuka file output");
            free(bin_data);
            continue;
        }
        fwrite(bin_data, 1, clean_len / 2, output);
        fclose(output);
        free(bin_data);
        fprintf(log, "%s: Successfully converted hexadecimal text %d.txt to %d_image_%s.png\n",
                log_timestamp, i, i, file_timestamp);
        fflush(log);

        printf("Berhasil mengonversi %s → %s\n", input_path, output_path);
    }

    fclose(log);
    return 0;

