#include <stdio.h>
#include <stdint.h>
#include <stdlib.h> // For malloc and free

void byteToHexString(uint8_t byte, char* hexString) {
    sprintf(hexString, "%02X", byte); // Convert byte to hexadecimal string
}

char** byteArrayToHexStringArray(uint8_t* byteArray, int arrayLength) {
    // Allocate memory for array of strings
    char** hexStringArray = (char**)malloc(arrayLength * sizeof(char*));
    
    if (hexStringArray == NULL) {
        // Memory allocation failed
        return NULL;
    }

    // Convert each byte to its hexadecimal string representation
    for (int i = 0; i < arrayLength; i++) {
        // Allocate memory for each string in the array
        hexStringArray[i] = (char*)malloc(3 * sizeof(char)); // Two characters + null terminator

        if (hexStringArray[i] == NULL) {
            // Memory allocation failed
            // Free memory allocated so far
            for (int j = 0; j < i; j++) {
                free(hexStringArray[j]);
            }
            free(hexStringArray);
            return NULL;
        }

        byteToHexString(byteArray[i], hexStringArray[i]);
    }

    return hexStringArray;
}

int main() {
    uint8_t byteArray[] = {0x00, 0x01, 0x02, 0xFE, 0xFF}; // Example byte array
    int arrayLength = sizeof(byteArray) / sizeof(byteArray[0]); // Get the length of the array

    // Convert byte array to hexadecimal string array
    char** hexStringArray = byteArrayToHexStringArray(byteArray, arrayLength);

    if (hexStringArray == NULL) {
        printf("Memory allocation failed.\n");
        return -1;
    }

    // Print the hexadecimal string array
    printf("Hexadecimal string array:\n");
    for (int i = 0; i < arrayLength; i++) {
        printf("0x%s ", hexStringArray[i]);
    }
    printf("\r\n");

    // Free allocated memory
    for (int i = 0; i < arrayLength; i++) {
        free(hexStringArray[i]);
    }
    free(hexStringArray);

    return 0;
}
