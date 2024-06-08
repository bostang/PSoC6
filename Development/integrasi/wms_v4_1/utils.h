/*******************************************************************************
* File Name:   utils.h (WMS v2.0)
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
void hex_array_to_string(const uint8_t *hex_array, size_t array_length, uint8_t *output);

char hex_digit_to_char(uint8_t digit);
void hex_array_to_char_array(const unsigned char *hex_array, size_t array_length, char *output);
uint8_t CheckSum(unsigned char *uBuff, uint8_t uBuffLen);

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

    for (size_t i = 0; i < length; i++)
    {
        sprintf(&hexString[i * 2], "%02X", array[i]); // %02X untuk format 2 karakter heksadesimal, huruf besar
    }

    hexString[length * 2] = '\0'; // Menambahkan null-terminator pada akhir string
    return hexString;
}

/*******************************************************************************
* Function Name: RFIDTagProcess
********************************************************************************
* Summary: convert an array of hexadecimal values into a string
*
* Parameters:
*   char *hexArray: array byte yang mau diubah ke hexadecimal
*   int arrayLength : panjang dari array yang mau diubah
*
*******************************************************************************/
//void hex_array_to_string(const unsigned char *hex_array, size_t array_length, char *output)
void hex_array_to_string(const uint8_t *hex_array, size_t array_length, uint8_t *output)
{
    int i, j = 0;

    // Iterate through the array
    for (i = 0; i < array_length; i++) {
        // Convert each hexadecimal value into two characters and store them in the output array
        sprintf(output + j, "%02X", hex_array[i]);
        j += 2;
    }
    // Add null terminator to the output string
    output[j] = '\0';
}

/*******************************************************************************
* Function Name: hex_digit_to_char
********************************************************************************
* Summary: Function to convert a hexadecimal digit to a character
*
* Parameters:
*   char digit
*
*******************************************************************************/
char hex_digit_to_char(uint8_t digit)
{
    if (digit >= 0 && digit <= 9) {
        return digit + '0';
    } else {
        return digit - 10 + 'A';
    }
}

/*******************************************************************************
* Function Name: hex_array_to_char_array
********************************************************************************
* Summary: Function to convert an array of hexadecimal values into a string
*
* Parameters:
*   char *hexArray: array byte yang mau diubah ke hexadecimal
*   int arrayLength : panjang dari array yang mau diubah
*
*******************************************************************************/
void hex_array_to_char_array(const unsigned char *hex_array, size_t array_length, char *output)
{
    int i, j = 0;

    // Iterate through the array
    for (i = 0; i < array_length; i++) {
        // Convert each hexadecimal value into two characters and store them in the output array
        output[j++] = hex_digit_to_char(hex_array[i] >> 4);
        output[j++] = hex_digit_to_char(hex_array[i] & 0x0F);
    }
}

/*******************************************************************************
* Function Name: CheckSum
********************************************************************************
* Summary: Menghitung checksum dari response packet UHF RFID module IND903
*
* Parameters:
*   uint8_t *uBuff: array response packet yang mau dicek checksumnya
*   uint8_t arrayLength : panjang dari array yang mau dihitung checksumnya
*
*******************************************************************************/
uint8_t CheckSum(unsigned char *uBuff, uint8_t uBuffLen)
{
	uint8_t i,uSum=0;
    for(i=0;i<uBuffLen;i++)
    {
        uSum = uSum + uBuff[i];
    }
    uSum = (~uSum) + 1;
    return uSum;
}


/* [] END OF FILE */
#endif // UTILS_H
