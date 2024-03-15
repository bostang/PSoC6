#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

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

// Fungsi untuk mengonversi string heksadesimal menjadi nilai heksadesimal
uint8_t hexStringToUint8(const char* hexString) {
    uint8_t result;
    sscanf(hexString, "%hhx", &result);
    return result;
}

// Fungsi untuk mengonversi array string heksadesimal menjadi array uint8_t
uint8_t* hexStringsToUint8Array(const char** hexStrings, int numStrings) {
    uint8_t* result = (uint8_t*)malloc(numStrings * sizeof(uint8_t));
    if (result == NULL) {
        return NULL;
    }

    for (int i = 0; i < numStrings; i++) {
        result[i] = hexStringToUint8(hexStrings[i]);
    }

    return result;
}

// Fungsi untuk menggabungkan semua langkah pemrosesan
uint8_t** parseHexData(const char* data, const char delimiter, int* numRows, int* numCols) {
    // Memecah string menjadi beberapa string berdasarkan delimiter
    int numStrings;
    char** hexStringArray = splitString(data, delimiter, &numStrings);
    if (hexStringArray == NULL) {
        *numRows = 0;
        *numCols = 0;
        return NULL;
    }

    // Mengonversi setiap string heksadesimal menjadi array uint8_t
    uint8_t** result = (uint8_t**)malloc(numStrings * sizeof(uint8_t*));
    if (result == NULL) {
        *numRows = 0;
        *numCols = 0;
        return NULL;
    }

    for (int i = 0; i < numStrings; i++) {
        int numBytes;
        char** byteStrings = splitString(hexStringArray[i], ' ', &numBytes);
        if (byteStrings == NULL) {
            // Jika alokasi gagal, bebaskan memori yang sudah dialokasikan
            for (int j = 0; j < i; j++) {
                free(result[j]);
            }
            free(result);
            *numRows = 0;
            *numCols = 0;
            return NULL;
        }

        result[i] = hexStringsToUint8Array(byteStrings, numBytes);
        if (result[i] == NULL) {
            // Jika alokasi gagal, bebaskan memori yang sudah dialokasikan
            for (int j = 0; j < i; j++) {
                free(byteStrings[j]);
                free(result[j]);
            }
            free(byteStrings);
            free(result);
            *numRows = 0;
            *numCols = 0;
            return NULL;
        }

        // Bebaskan memori yang dialokasikan untuk array string byte
        for (int j = 0; j < numBytes; j++) {
            free(byteStrings[j]);
        }
        free(byteStrings);
    }

    *numRows = numStrings;
    *numCols = 12; // Jumlah kolom ditentukan berdasarkan jumlah byte dalam setiap string heksadesimal

    // Bebaskan memori yang dialokasikan untuk array string heksadesimal
    for (int i = 0; i < numStrings; i++) {
        free(hexStringArray[i]);
    }
    free(hexStringArray);

    return result;
}

// Fungsi untuk membebaskan memori yang dialokasikan untuk array uint8_t
void freeUint8Array(uint8_t** array, int numRows) {
    if (array != NULL) {
        for (int i = 0; i < numRows; i++) {
            free(array[i]);
        }
        free(array);
    }
}

int main() {
    const char* data = "E2 00 00 17 57 0C 00 44 19 90 47 26,E2 11 00 17 57 0C 10 47 19 90 47 26,E3 11 00 17 57 0C 10 47 19 90 47 26";
    int numRows, numCols;

    printf("Parsing data:\n");
    uint8_t** parsedData = parseHexData(data, ',', &numRows, &numCols);
    if (parsedData != NULL) {
        printf("Jumlah baris: %d\n", numRows);
        printf("Jumlah kolom: %d\n", numCols);
        printf("Data yang berhasil diproses:\n");
        for (int i = 0; i < numRows; i++) {
            printf("Data ke-%d:\n", i + 1);
            for (int j = 0; j < numCols; j++) {
                printf("0x%02X ", parsedData[i][j]);
            }
            printf("\n");
        }

        // Membebaskan memori yang dialokasikan untuk data yang berhasil diproses
        freeUint8Array(parsedData, numRows);
    } else {
        printf("Gagal memproses data.\n");
    }

    return 0;
}
