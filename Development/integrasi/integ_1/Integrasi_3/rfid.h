/*******************************************************************************
* File Name:   rfid.h
* Description: implementasi RFID module sebagai bagian dari sub-sistem data acquisition
* 				apabila button ditekan, maka akan mengirimkan perintah mulai baca dan kemudian menampilkan bacaan pada terminal
* 				dan juga OLED
* Programmer: Bostang
* Tanggal awal : 14 April 2024
*******************************************************************************/

/*******************************************************************************
* Header Files
*******************************************************************************/
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h> // For malloc and free
#include "cyhal.h"
#include "cybsp.h"
#include "cy_retarget_io.h"

/*******************************************************************************
* Macros
*******************************************************************************/
#define RFID_TASK_NAME         ("RFID scanner Task")
#define RFID_TASK_STACK_SIZE   (1024)
#define RFID_TASK_PRIORITY     (5)

#define DATA_BITS_8     8
#define STOP_BITS_1     1
#define BAUD_RATE       115200
#define UART_DELAY      10u
#define RX_BUF_SIZE     1
#define TX_BUF_SIZE     6
#define LEN_EPC			12 // 12 double

/*******************************************************************************
* Global Variables
*******************************************************************************/
cyhal_uart_t uart_rfid; // UART object for RFID scanner
uint32_t actualbaud;
uint8_t tx_buf[TX_BUF_SIZE] = {0xA0, 0x04, 0x01, 0x89, 0x01, 0xD1}; // cmd_read byte array
uint8_t rx_buf[RX_BUF_SIZE];
uint8_t rx_buf2[RX_BUF_SIZE];
size_t tx_length = TX_BUF_SIZE;
size_t rx_length = RX_BUF_SIZE;
size_t epc_length = LEN_EPC;

// Variabel flag untuk menandakan telah terdeteksi tag
volatile int flag_tag_detect= 0;

// variabel flag untuk deteksi header (0x0A)
volatile int flag_header_detect= 0;

// variabel flag untuk menyatakan telah selesai deteksi EPC
volatile int flag_finish_read= 0;

// indeks tag
int index_tag = -7;

// variabel EPC
uint8_t epc[LEN_EPC];

/* Initialize the UART configuration structures */
const cyhal_uart_cfg_t uart_rfid_config =
{
    .data_bits = DATA_BITS_8,
    .stop_bits = STOP_BITS_1,
    .parity = CYHAL_UART_PARITY_NONE,
    .rx_buffer = rx_buf,
    .rx_buffer_size = RX_BUF_SIZE
};

// extern variable agar scope-nya menjadi global
extern char** hexStringArray;

/*******************************************************************************
* Function Prototypes
*******************************************************************************/
void byteToHexString(uint8_t byte, char* hexString);
char** byteArrayToHexStringArray(uint8_t* byteArray, int arrayLength);
void rfid_task( void * arg );


/*******************************************************************************
* Function Definitions
*******************************************************************************/

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
* Function Name: rfid_task
********************************************************************************
* Summary:
*   Task to handle button events untuk RFID scanner.
*
* Parameters:
*   void *arg: Task arguments (unused)
*
*******************************************************************************/
void rfid_task( void * arg )
{
	// Inisiasi
	cy_rslt_t result;
	#if defined(CY_DEVICE_SECURE)
		cyhal_wdt_t wdt_obj;
		/* Clear watchdog timer so that it doesn't trigger a reset */
		result = cyhal_wdt_init(&wdt_obj, cyhal_wdt_get_max_timeout_ms());
		CY_ASSERT(CY_RSLT_SUCCESS == result);
		cyhal_wdt_free(&wdt_obj);
	#endif

	/* Initialize the UART Blocks */
	result = cyhal_uart_init(&uart_rfid, P12_1, P12_0, NC, NC, NULL, &uart_rfid_config);
	handle_error(result);

	/* Set the baud rates */
	result = cyhal_uart_set_baud(&uart_rfid, BAUD_RATE, &actualbaud);
	handle_error(result);

	for (;;)
	{
		// mengirimkan perintah baca apabila button acq (main) ditekan
		if (true == gpio_intr_flag_acq)
		{
			/* Begin Tx Transfer */
			// Send the command to start reading from the RFID scanner
			cyhal_uart_write(&uart_rfid, (void*)tx_buf, &tx_length);

			/* DEBUG */
			char** hexStringArray_command = byteArrayToHexStringArray(tx_buf,tx_length);
			printf("Sent command:\n");
			for (int i = 0; i < tx_length; i++)
			{
				printf("0x%s ", hexStringArray_command[i]);
			}
			printf("\r\n");
			/* DEBUG */

			printf("sukses kirim perintah baca ke RFID \r\n"); // DEBUG
			 /* Begin Rx Transfer */
			    // Read the response data from the RFID scanner
			    	// tunggu sampai data pertama terdeteksi
			    while (!flag_tag_detect)
			    {
			    	if (CY_RSLT_SUCCESS == cyhal_uart_getc(&uart_rfid, &rx_buf, 0))
			    	{
			    		flag_tag_detect = 1;
			    	}
			    		// tulis tag pertama
			    	if (rx_buf[0] == 0xA0)
			    	{
			    		flag_header_detect = 1;
			    	}
			    	if (flag_header_detect)
			    	{
			    		if (index_tag > 0)
			    		{
			    			epc[index_tag] = rx_buf[0];
			    		}
			    		index_tag++;
			    	}
			    }

			    // terus baca sampai akhir
			    while((CY_RSLT_SUCCESS == cyhal_uart_getc(&uart_rfid, &rx_buf, 0))&&(!flag_finish_read))
				{

					// tulis ke terminal
			    	if (rx_buf[0] == 0xA0)
			    	{
			    		flag_header_detect = 1;
			    	}
			    	if (flag_header_detect)
			    	{
			    		if (index_tag >= 0)
			    		{
			    			epc[index_tag] = rx_buf[0];
			    		}
			    		index_tag++;
			    		if (index_tag > LEN_EPC)
			    		{
			    			flag_finish_read = 1;
			    		}
			    	}
				}

			// mengubah tag dari byte array menjadi array of string
			 int arrayLength = sizeof(epc) / sizeof(epc[0]); // Get the length of the array

			// Convert byte array to hexadecimal string array
			char** hexStringArray = byteArrayToHexStringArray(epc, arrayLength);

			if (hexStringArray == NULL)
			{
				printf("Memory allocation failed.\n");
				return -1;
			}

			// Print the hexadecimal string array
			/* \x1b[2J\x1b[;H - ANSI ESC sequence for clear screen */
			printf("\x1b[2J\x1b[;H");
			printf("Hexadecimal string array:\n");
			for (int i = 0; i < arrayLength; i++)
			{
				printf("0x%s ", hexStringArray[i]);
			}
			printf("\r\n");

			// Free allocated memory
			for (int i = 0; i < arrayLength; i++)
			{
				free(hexStringArray[i]);
			}
			free(hexStringArray);
		}
	}
}
