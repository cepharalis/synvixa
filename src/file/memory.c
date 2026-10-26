#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/time.h>
long long line_count = 0;
long long line = 0;
// malloc
// pointers.
// bool          – true or false
// char          – single character, 1 byte
// wchar_t       – wide character
// char8_t       – UTF-8 code unit (C++20)
// char16_t      – UTF-16 code unit (C++11)
// char32_t      – UTF-32 code unit (C++11)
// short         – small integer, ~2 bytes
// int           – default integer, ~4 bytes
// long          – larger integer, ≥4 bytes
// long long     – very large integer, ~8 bytes
// float         – single-precision float, ~4 bytes
// double        – double-precision float, ~8 bytes
// long double   – extended-precision float, ~12–16 bytes
// void          – no value, used for return types

// int main() {
//     if ((sock_fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
//         perror("Socket creation failed");
//         exit(EXIT_FAILURE);
//     }
//     while (1) {
//         // --- STEP A: Read and format text.txt fresh every iteration ---
//         FILE *file = fopen("text.txt", "r");
//         if (file == NULL) {
//             sleep(1);
//             continue;
//         }
//         fseek(file, 0, SEEK_END);
//         long file_size = ftell(file);
//         rewind(file);
//
//         char *file_content = malloc(file_size * 2 + 3);
//         if (file_content == NULL) {
//             perror("Memory allocation failed");
//             fclose(file);
//             sleep(1);
//             continue;
//         }
//         long character_count = 0;
//         long pos = 0;
//         file_content[pos++] = '"';
//
//         int ch;
//         fgets();
//         //while ((ch = fgetc(file)) != EOF) {
//         //    if (ch == '\n') {
//         //        line_count++;
//         //    }
//             //    file_content[pos++] = '\\';
//             //    file_content[pos++] = 'n';
//             //    character_count += 2;
//             //}
//             else if (ch == '\r') {
//                 continue;
//             // } else {
//             //     file_content[pos++] = (char)ch;
//             //     character_count++;
//             }
//         }
//}


// while ((ch = fgetc(file)) != EOF) {
//     if (ch == '\n') {
//         line_count++;
//     }

char filename[1024];
int32_t target = 88432, window = 5;
int32_t main(void) {
    FILE *f = fopen(filename, "r");
    char *line = NULL;
    size_t cap = 0;


    char **saved = malloc(2*window+1);
    int32_t idx = 0;




    int32_t n = 0;
    while (getline(&line, &cap, f) != -1) {
        n++;
        if (n >= target - window && n <= target + window) {
            saved[idx++] = strduo(line);
        }
        if (n == target + window) break;

    }
    for (int i = 0; i < idx; i++) {
        printf("Line %d: %s", target - window + i, saved[i]);
        free(saved[i]);
    }
    free(saved);
    free(line);
    fclose(f);
    return 0;
}












