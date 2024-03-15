// mengemas program main5 menjadi sebuah fungsi
    // input : array string
    // output : array byte (uint8_t)
    // 2023-03-15 07.19 pagi

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

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

int main() {
    // String heksadesimal yang akan diubah
    const char* hexStrings[] = {"00", "01", "EF", "FF"};
    int numStrings = sizeof(hexStrings) / sizeof(hexStrings[0]);

    printf("Konversi string heksadesimal ke array uint8_t:\n");
    uint8_t* result = hexStringsToUint8Array(hexStrings, numStrings);
    if (result != NULL) {
        for (int i = 0; i < numStrings; i++) {
            printf("%s menjadi 0x%02X\n", hexStrings[i], result[i]);
        }
        free(result);
    } else {
        printf("Alokasi memori gagal.\n");
    }

    return 0;
}
