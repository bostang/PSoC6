/*******************************************************************************
* File Name:   utils.h (Integrasi 1)
* Description: implementasi fungsi-fungsi pembantu seperti error_handling
* Programmer: Bostang
* Tanggal : 2024/04/04
*******************************************************************************/

 // HEADER GUARD
#ifndef UTILS_H
#define UTILS_H

/*******************************************************************************
* Header Files
*******************************************************************************/
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h> // For malloc and free
#include <stdbool.h>
#include "cyhal.h"
#include "cybsp.h"
#include "cy_retarget_io.h"

/*******************************************************************************
* Function Prototypes
*******************************************************************************/
void handle_error(uint32_t status);
void byteToHexString(uint8_t byte, char* hexString);
char** byteArrayToHexStringArray(uint8_t* byteArray, int arrayLength);
char* uint8ArrayToHexString(const uint8_t* array, size_t length);

/*******************************************************************************
* Function Definitions
*******************************************************************************/

/*******************************************************************************
* Function Name: handle_error
********************************************************************************
* Summary:
* User defined error handling function
*
* Parameters:
*  uint32_t status - status indicates success or failure
*
* Return:
*  void
*
*******************************************************************************/
void handle_error(uint32_t status)
{
    if (status != CY_RSLT_SUCCESS)
    {
        CY_ASSERT(0);
    }
}

/*******************************************************************************
* Function Name: byteToHexString
********************************************************************************
* Summary:
*   Menampilkan RFID tag dalam bentuk array hexadecimal ke terminal dalam format string.
*
* Parameters:
*   char *hexString: array hexadecimal yang akan dicetak dalam bentuk string
*
*******************************************************************************/
void byteToHexString(uint8_t byte, char* hexString)
{
    sprintf(hexString, "%02X", byte); // Convert byte to hexadecimal string
}

/*******************************************************************************
* Function Name: byteArrayToHexStringArray
********************************************************************************
* Summary: melakukan konversi dari array byte menjadi array hexadecimal
*
* Parameters:
*   char *byteArray: array byte yang mau diubah ke hexadecimal
*   int arrayLength : panjang dari array yang mau diubah
*
*******************************************************************************/
char** byteArrayToHexStringArray(uint8_t* byteArray, int arrayLength)
{
    // Allocate memory for array of strings
    char** hexStringArray = (char**)malloc(arrayLength * sizeof(char*));

    if (hexStringArray == NULL)
    {
        // Memory allocation failed
        return NULL;
    }

    // Convert each byte to its hexadecimal string representation
    for (int i = 0; i < arrayLength; i++)
    {
        // Allocate memory for each string in the array
        hexStringArray[i] = (char*)malloc(3 * sizeof(char)); // Two characters + null terminator

        if (hexStringArray[i] == NULL)
        {
            // Memory allocation failed
            // Free memory allocated so far
            for (int j = 0; j < i; j++)
            {
                free(hexStringArray[j]);
            }
            free(hexStringArray);
            return NULL;
        }

        byteToHexString(byteArray[i], hexStringArray[i]);
    }

    return hexStringArray;
}

/*******************************************************************************
* Function Name: byteArrayToHexStringArray
********************************************************************************
* Summary: melakukan konversi dari array byte menjadi array string
*
* Parameters:
*   char *byteArray: array byte yang mau diubah ke hexadecimal
*   int arrayLength : panjang dari array yang mau diubah
*
*******************************************************************************/
char* uint8ArrayToHexString(const uint8_t* array, size_t length)
{
    char* hexString = (char*)malloc(length * 2 + 1); // Satu byte uint8_t = 2 karakter heksadesimal, tambahkan 1 untuk null-terminator
    if (hexString == NULL) {
        return NULL; // Handle alokasi gagal
    }

    for (size_t i = 0; i < length; i++) {
        sprintf(&hexString[i * 2], "%02X", array[i]); // %02X untuk format 2 karakter heksadesimal, huruf besar
    }

    hexString[length * 2] = '\0'; // Menambahkan null-terminator pada akhir string
    return hexString;
}



#endif // UTILS_H
