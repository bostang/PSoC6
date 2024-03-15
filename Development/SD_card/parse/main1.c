// 2024-03-15 06.56 pagi
    // melakukan parsing dari E2 00 00 17 57 0C 00 44 19 90 47 26, E2 11 00 17 57 0C 10 47 19 90 47 26, E3 11 00 17 57 0C 10 47 19 90 47 26
    // menjadi beberapa array of string
`// (FAIL)
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Fungsi untuk memisahkan string dengan delimiter ","
char **parseString(const char *inputString, const char *delimiter, int *count) {
    // Menghitung jumlah substring yang dihasilkan
    int numSubstrings = 1;
    const char *temp = inputString;
    while ((temp = strstr(temp, delimiter))) {
        numSubstrings++;
        temp += strlen(delimiter);
    }

    // Mengalokasikan memori untuk array pointer ke string
    char **substrings = (char **)malloc(numSubstrings * sizeof(char *));
    if (substrings == NULL) {
        fprintf(stderr, "Gagal mengalokasikan memori\n");
        exit(EXIT_FAILURE);
    }

    // Menyalin dan memisahkan setiap substring
    int i = 0;
    char *token = strtok((char *)inputString, delimiter);
    while (token != NULL) {
        substrings[i] = (char *)malloc(strlen(token) + 1);
        if (substrings[i] == NULL) {
            fprintf(stderr, "Gagal mengalokasikan memori\n");
            exit(EXIT_FAILURE);
        }
        strcpy(substrings[i], token);
        token = strtok(NULL, delimiter);
        i++;
    }

    // Mengatur jumlah substring yang dihasilkan
    *count = numSubstrings;

    return substrings;
}

int main() {
    const char *inputString = "E2 00 00 17 57 0C 00 44 19 90 47 26, E2 11 00 17 57 0C 10 47 19 90 47 26, E3 11 00 17 57 0C 10 47 19 90 47 26";
    const char *delimiter = ", ";
    int count;
    char **substrings = parseString(inputString, delimiter, &count);

    // Mencetak setiap substring yang dipisahkan
    printf("Jumlah substring: %d\n", count);
    for (int i = 0; i < count; i++) {
        printf("Substring %d: %s\n", i + 1, substrings[i]);
        free(substrings[i]); // Membebaskan memori untuk setiap substring
    }

    free(substrings); // Membebaskan memori untuk array pointer ke string

    return 0;
}
