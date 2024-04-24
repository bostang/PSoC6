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
#include <string.h>
#include "FreeRTOS.h"
#include "task.h"
#include "cyabs_rtos.h"


/*******************************************************************************
* Macros
*******************************************************************************/
#define DEBOUNCING_DELAY_MS          (1000)   /* milliseconds */

#define RFID_TASK_NAME         ("RFID scanner Task")
#define RFID_TASK_STACK_SIZE   (1024)
#define RFID_TASK_PRIORITY     (5)

#define DATA_BITS_8     8
#define STOP_BITS_1     1
#define BAUD_RATE       115200
#define UART_DELAY      10u
#define RFID_RX_BUF_SIZE     1
#define RFID_TX_BUF_SIZE     6
#define LEN_EPC			12 // 12 double

/*******************************************************************************
* Global Variables
*******************************************************************************/
cyhal_uart_t uart_rfid; // UART object for RFID scanner
uint32_t actualbaud;
uint8_t rfid_tx_buf[RFID_TX_BUF_SIZE] = {0xA0, 0x04, 0x01, 0x89, 0x01, 0xD1}; // cmd_read byte array
uint8_t rfid_rx_buf[RFID_RX_BUF_SIZE];
uint8_t rfid_rx_buf2[RFID_RX_BUF_SIZE];
size_t rfid_tx_length = RFID_TX_BUF_SIZE;
size_t rfid_rx_length = RFID_RX_BUF_SIZE;
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
    .rx_buffer = rfid_rx_buf,
    .rx_buffer_size = RFID_RX_BUF_SIZE
};

// extern variable agar scope-nya menjadi global
extern char** hexStringArray;

/*******************************************************************************
* Function Prototypes
*******************************************************************************/
void byteToHexString(uint8_t byte, char* hexString);
char** byteArrayToHexStringArray(uint8_t* byteArray, int arrayLength);
//void rfid_task( void * arg );
void rfid_task(void *pvParameters);


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
//void rfid_task( void * arg )
void rfid_task(void *pvParameters)
{
    TaskParameters *params = (TaskParameters *)pvParameters;
    int* state = &(params->state);

    // Lock the mutex before accessing/modifying epc
    cy_rtos_mutex_get(&epc_mutex, CY_RTOS_NEVER_TIMEOUT);

    // params->epc[0] // kalau mau memodifikasi epc

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
			cyhal_uart_write(&uart_rfid, (void*)rfid_tx_buf, &rfid_tx_length);

			/* DEBUG */
			char** hexStringArray_command = byteArrayToHexStringArray(rfid_tx_buf,rfid_tx_length);
			printf("Sent command:\n");
			for (int i = 0; i < rfid_tx_length; i++)
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
			    	if (CY_RSLT_SUCCESS == cyhal_uart_getc(&uart_rfid, &rfid_rx_buf, 0))
			    	{
			    		flag_tag_detect = 1;
			    	}
			    		// tulis tag pertama
			    	if (rfid_rx_buf[0] == 0xA0)
			    	{
			    		flag_header_detect = 1;
			    	}
			    	if (flag_header_detect)
			    	{
			    		if (index_tag > 0)
			    		{
			    			epc[index_tag] = rfid_rx_buf[0];
			    		}
			    		index_tag++;
			    	}
			    }

			    // terus baca sampai akhir
			    while((CY_RSLT_SUCCESS == cyhal_uart_getc(&uart_rfid, &rfid_rx_buf, 0))&&(!flag_finish_read))
				{

					// tulis ke terminal
			    	if (rfid_rx_buf[0] == 0xA0)
			    	{
			    		flag_header_detect = 1;
			    	}
			    	if (flag_header_detect)
			    	{
			    		if (index_tag >= 0)
			    		{
			    			epc[index_tag] = rfid_rx_buf[0];
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
				// menampilkan epc ke terminal
				printf("0x%s ", hexStringArray[i]);

			}

//			// DEBUG : memastikan bahwa hexStringArray telah tersalin ke epc
//			for (int i = 0;i<arrayLength;i++)
//			{
//				// salin hexStringArray ke epc untuk ditampilkan ke OLED
//				strcpy(params->epc[i],hexStringArray[i]);
////				strncpy(params->epc[i],hexStringArray[i],2);
//				printf("%s ",params->epc[i]);
//			}

			strcpy(params->epc[0],hexStringArray[0]);

			for (int i = 0;i<arrayLength;i++)
			{
				// salin hexStringArray ke epc untuk ditampilkan ke OLED
				printf("%s ",params->epc[i]);
			}

		 // Unlock the mutex after done with epc
			cy_rtos_mutex_set(&epc_mutex);
			printf("\r\n");
			printf("%d\r\n",strlen(hexStringArray[0]));

			// Free allocated memory
			for (int i = 0; i < arrayLength; i++)
			{
				free(hexStringArray[i]);
			}
			free(hexStringArray);
	        vTaskDelay(pdMS_TO_TICKS(DEBOUNCING_DELAY_MS)); // Debouncing delay
		}
	}
}
