#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>

#define RED "\x1b[31m"
#define LIGHT_BLUE "\033[94m"
#define LIGHT_GREEN "\033[92m"
#define RESET "\x1b[0m"
#define GB 1073741824.0
#define MB 1048576.0
#define KB 1024.0

char* format_file_weight(const double weight) {
    char* str = malloc(30 * sizeof(char));
    if (str == NULL) {
        perror("malloc failed");
        return NULL;
    }

    if (weight > GB) {
        sprintf(str, "%.2f %sGB%s", weight / GB, RED, RESET);
    } else if (weight > MB) {
        sprintf(str, "%.2f MB", weight / MB);
    } else if (weight > KB) {
        sprintf(str, "%.2f KB", weight / KB);
    } else {
        sprintf(str, "%.2f B", weight);
    }

    return str;
}

double calculate_directory_size(const char *path) {
    double total_size = 0.0;
    struct dirent *entry;
    DIR *dir = opendir(path);

    if (dir == NULL) {
        perror("Failed to open directory");
        return 0.0;
    }

    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }

        char full_path[PATH_MAX];
        snprintf(full_path, sizeof(full_path), "%s/%s", path, entry->d_name);

        struct stat file_info;
        if (stat(full_path, &file_info) != 0) {
            perror("Failed to stat file");
            continue;
        }

        if (S_ISDIR(file_info.st_mode)) {
            total_size += calculate_directory_size(full_path);
        } else {
            total_size += file_info.st_size;
        }
    }

    closedir(dir);
    return total_size;
}

void print_directory_info() {
    struct dirent *entry;
    DIR *dir = opendir(".");

    if (dir == NULL) {
        perror("Failed to open directory");
        return;
    }

    char *files[1024];
    char *dirs[1024];
    int file_count = 0, dir_count = 0;
    double total_size = 0;

    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }

        struct stat file_info;
        if (stat(entry->d_name, &file_info) != 0) {
            perror("Failed to stat file");
            continue;
        }

        char *file_name = strdup(entry->d_name);
        if (file_name == NULL) {
            perror("strdup failed");
            continue;
        }

        char *formatted;
        if (S_ISDIR(file_info.st_mode)) {
            double dir_size = calculate_directory_size(entry->d_name);
            formatted = format_file_weight(dir_size);
            if (formatted == NULL) {
                perror("format failed");
                free(file_name);
                continue;
            }

            dirs[dir_count++] = malloc(strlen(file_name) + strlen(": ") + strlen(formatted) + 1);
            if (dirs[dir_count - 1] == NULL) {
                perror("malloc failed");
                free(file_name);
                free(formatted);
                continue;
            }
            total_size += dir_size;
            snprintf(dirs[dir_count - 1], strlen(file_name) + strlen(": ") + strlen(formatted) + 1, "%s: %s", file_name, formatted);
        } else {
            formatted = format_file_weight(file_info.st_size);
            if (formatted == NULL) {
                perror("format failed");
                free(file_name);
                continue;
            }

            files[file_count++] = malloc(strlen(file_name) + strlen(": ") + strlen(formatted) + 1);
            if (files[file_count - 1] == NULL) {
                perror("malloc failed");
                free(file_name);
                free(formatted);
                continue;
            }
            snprintf(files[file_count - 1], strlen(file_name) + strlen(": ") + strlen(formatted) + 1, "%s: %s", file_name, formatted);

            total_size += file_info.st_size;
        }

        free(file_name);
        free(formatted);
    }

    closedir(dir);

    for (int i = 0; i < file_count; i++) {
        printf(LIGHT_GREEN "FILE %s%s\n", RESET, files[i]);
        free(files[i]);
    }

    for (int i = 0; i < dir_count; i++) {
        printf(LIGHT_BLUE "DIR %s%s\n", RESET, dirs[i]);
        free(dirs[i]);
    }

    char* formatted_total = format_file_weight(total_size);
    printf("Total size: %s\n", formatted_total);
    free(formatted_total);
}

int main() {
    print_directory_info();
    return 0;
}
