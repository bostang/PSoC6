// 2024-03-15 07.07 pagi
    // melakukan parsing dari E2 00 00 17 57 0C 00 44 19 90 47 26, E2 11 00 17 57 0C 10 47 19 90 47 26, E3 11 00 17 57 0C 10 47 19 90 47 26
    // menjadi beberapa string
// (SUKSES)
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Fungsi untuk memecah string berdasarkan delimiter
char** splitString(const char* input, const char delimiter, int* count) {
    // Hitung jumlah string yang dihasilkan
    int numStrings = 1;
    for (int i = 0; input[i] != '\0'; i++) {
        if (input[i] == delimiter) {
            numStrings++;
        }
    }

    // Allokasi memori untuk array pointer ke string
    char** result = (char**)malloc(numStrings * sizeof(char*));
    if (result == NULL) {
        *count = 0;
        return NULL;
    }

    // Inisialisasi index dan pointer
    int index = 0;
    const char* start = input;
    const char* end = input;

    // Memecah string dan menyimpan hasilnya
    for (int i = 0; i < numStrings; i++) {
        while (*end && *end != delimiter) {
            end++;
        }
        int length = end - start;
        result[index] = (char*)malloc((length + 1) * sizeof(char));
        if (result[index] == NULL) {
            // Jika gagal alokasi, bebaskan memori yang sudah dialokasikan sebelumnya
            for (int j = 0; j < index; j++) {
                free(result[j]);
            }
            free(result);
            *count = 0;
            return NULL;
        }
        strncpy(result[index], start, length);
        result[index][length] = '\0';
        index++;
        start = ++end;
    }

    *count = numStrings;
    return result;
}

int main() {
    const char* input = "E2 00 00 17 57 0C 00 44 19 90 47 26,E2 11 00 17 57 0C 10 47 19 90 47 26,E3 11 00 17 57 0C 10 47 19 90 47 26";
    int count;
    char** strings = splitString(input, ',', &count);

    if (strings != NULL) {
        printf("Jumlah string: %d\n", count);
        for (int i = 0; i < count; i++) {
            printf("%s\n", strings[i]);
            free(strings[i]);
        }
        free(strings);
    } else {
        printf("Alokasi memori gagal\n");
    }

    return 0;
}
