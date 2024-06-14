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
void allocate_epc_memory(tags* tag_struct);
void free_epc_memory(tags* tag_struct);
char* generate_json(uint8_t *epc_array[], int num_epc);
bool is_tag_registered(uint8_t** epc_array, int tag_count, uint8_t* epc);

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

/*******************************************************************************
* Function Name: allocate_epc_memory
********************************************************************************
* Summary: alokasi memori untuk array epc (panjang dinamis)
*
* Parameters:
*   tags *tag_struct: struct tag yang mau dialokasi memori
*
*******************************************************************************/
void allocate_epc_memory(tags* tag_struct) {
    for (int i = 0; i < MAX_EPC_ARRAY_SIZE; i++) {
        tag_struct->epc_array[i] = (uint8_t*)malloc(LEN_EPC * sizeof(uint8_t));
        if (tag_struct->epc_array[i] == NULL) {
            printf("Memory allocation failed for epc_array[%d]\n", i);
            // Free previously allocated memory before returning
            for (int j = 0; j < i; j++) {
                free(tag_struct->epc_array[j]);
            }
            return;
        }
    }
}

/*******************************************************************************
* Function Name: free_epc_memory
********************************************************************************
* Summary: membebaskan memori untuk array epc (panjang dinamis)
*
* Parameters:
*   tags *tag_struct: struct tag yang mau dibebaskan memori
*
*******************************************************************************/
void free_epc_memory(tags* tag_struct) {
    for (int i = 0; i < MAX_EPC_ARRAY_SIZE; i++) {
        free(tag_struct->epc_array[i]);
    }
}


/*******************************************************************************
* Function Name: generate_json
********************************************************************************
* Summary: Function to generate JSON formatted string from UPC array
*
* Parameters:
*   uint8_t *epc_array: array epc sumber
*   int num_epc : jumlah epc
*
*******************************************************************************/
char* generate_json(uint8_t *epc_array[], int num_epc)
{
    // Estimate a buffer size. Adjust according to your needs.
    // Assuming each EPC is around 15 characters + 30 characters for JSON formatting
    int buffer_size = num_epc * 60 + 20; // Added some extra buffer for safety
    char* json_string = (char*)malloc(buffer_size);

    if (json_string == NULL) {
        printf("Memory allocation failed\n");
        return NULL;
    }

    // Initialize the JSON string
    strcpy(json_string, "{\"data\":[");

    // Iterate over the EPC array and format each entry as a JSON object
    for (int i = 0; i < num_epc; i++) {
        // Ensure null-termination
        char epc_str[LEN_EPC + 1] = {0}; // +1 for null-terminator

        strncpy(epc_str, (char*)epc_array[i], LEN_EPC);

        char json_object[100];
        snprintf(json_object, sizeof(json_object), "{\"kode_stok\":\"%s\",\"scanned\":true}", epc_str);

        strcat(json_string, json_object);

        if (i < num_epc - 1) {
            strcat(json_string, ",");
        }
    }

    // Close the JSON array and object
    strcat(json_string, "]}");

    return json_string;
}

/*******************************************************************************
* Function Name: is_tag_registered
********************************************************************************
* Summary: Function to check if a tag is already registered in the array
*
* Parameters:
*   uint8_t *epc_array: array epc sumber
*   int num_epc : jumlah epc
*
*******************************************************************************/
bool is_tag_registered(uint8_t** epc_array, int tag_count, uint8_t* epc)
{
    for (int j = 0; j < tag_count; j++)
    {
        if (memcmp(epc_array[j], epc, LEN_EPC) == 0)
        {
            return true; // Tag found in the array
        }
    }
    return false; // Tag not found in the array
}

/* [] END OF FILE */
#endif // UTILS_H
