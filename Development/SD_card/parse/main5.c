// konversi array of string heksadesimal menjadi byte heksadesimal
    // misalkan "AA" menjadi 0xAA
    // 2024-03-15 07.17 pagi

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Fungsi untuk mengubah string heksadesimal menjadi nilai heksadesimal
int hexToDecimal(const char* hexString) {
    int result;
    sscanf(hexString, "%x", &result);
    return result;
}

int main() {
    // String heksadesimal yang akan diubah
    const char* hexStrings[] = {"00", "01", "EF", "FF"};

    printf("Konversi string heksadesimal ke nilai heksadesimal:\n");
    for (int i = 0; i < sizeof(hexStrings) / sizeof(hexStrings[0]); i++) {
        printf("%s menjadi 0x%02X\n", hexStrings[i], hexToDecimal(hexStrings[i]));
    }

    return 0;
}