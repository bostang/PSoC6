#include <stdio.h>
#include <stdint.h>

void byteToHexString(uint8_t byte, char* hexString) {
    sprintf(hexString, "%02X", byte); // Convert byte to hexadecimal string
}

int main() {
    uint8_t byteArray[] = {0x00, 0x01, 0x02, 0xFE, 0xFF}; // Example byte array
    int arrayLength = sizeof(byteArray) / sizeof(byteArray[0]); // Get the length of the array

    // Allocate memory for character array to hold hexadecimal strings
    char hexStringArray[arrayLength][3]; // Each string can hold two characters plus null terminator
    
    // Convert each byte to its hexadecimal string representation
    for (int i = 0; i < arrayLength; i++) {
        byteToHexString(byteArray[i], hexStringArray[i]);
    }

    // Print the hexadecimal string array
    printf("Hexadecimal string array:\n");
    for (int i = 0; i < arrayLength; i++) {
        printf("%s\n", hexStringArray[i]);
    }

    return 0;
}
