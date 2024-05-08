/*******************************************************************************
* File Name:   data_acquisition.h
* Description: implementasi Barcode sebagai bagian dari sub-sistem data acquisition
* 				apabila button ditekan, maka akan melakukan uart scanning (menunggu karakter input dari barcode scanner dan kemudian menampilkan bacaan pada terminal
* 				dan juga OLED
* Programmer: Bostang
* Tanggal awal : 23 April 2024
*******************************************************************************/

// HEADER GUARD
#ifndef DATA_ACQUISITION_H
#define DATA_ACQUISITION_H

/*******************************************************************************
* Header Files
*******************************************************************************/
#include <stdlib.h>
#include "cyhal.h"
#include "cybsp.h"
#include "cy_retarget_io.h"

/*******************************************************************************
* Macros
*******************************************************************************/
// untuk barcode
#define BAR_RX P9_0
#define BAR_TX P9_1
#define RX_BUF_SIZE     4
//#define TX_BUF_SIZE     14 // panjang tag = 14
#define TX_BUF_SIZE     12 // panjang tag = 14 (OLD)

// untuk rfid
#define DATA_BITS_8     8
#define STOP_BITS_1     1
#define BAUD_RATE       115200
#define UART_DELAY      10u
#define RFID_RX_BUF_SIZE     1
#define RFID_TX_BUF_SIZE     6
#define LEN_EPC			12 // 12 double

// pin selektor RFID/barcode
#define SCANNER_SELECTOR_PIN		(P9_7)
#define barcode_mode		(false)
#define RFID_mode			(true)

/*******************************************************************************
* Global Variables
*******************************************************************************/
// Untuk rfid
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


/*******************************************************************************
* Function Prototypes
********************************************************************************/
uint8_t* data_acquisition_task(); // barcode
char** data_acquisition_rfid_task();
bool initialize_selector(); // melakukan inisiasi selektor barcode/rfid

/*******************************************************************************
* Function Definitions
*******************************************************************************/

/*******************************************************************************
 * Function Name: initialize_selector
 *******************************************************************************
 * Summary:
 *  Melakukan inisiasi selektor barcode/rfid dengan melakukan pembacaan terhadap pin P9_7
 *
 * Parameters:
 *  void.
 *
 * Return:
 *  bool : true -> RFID
 *  	   false -> Barcode
 *
 *******************************************************************************/
bool initialize_selector()
{
	cy_rslt_t result;

	result = cyhal_gpio_init(SCANNER_SELECTOR_PIN, CYHAL_GPIO_DIR_INPUT, CYHAL_GPIO_DRIVE_NONE, false);
	/* User button init failed. Stop program execution */
	handle_error(result);

	bool read_val = cyhal_gpio_read(SCANNER_SELECTOR_PIN);

	if (read_val == RFID_mode) printf("P9_7 = 1 -> RFID");
	else printf("P9_7 = 0 -> Barcode");

	return read_val;

}

/*******************************************************************************
 * Function Name: data_acquisition_task
 *******************************************************************************
 * Summary:
 *  Task used to retrieve data from qrcode and rfid.
 *
 * Parameters:
 *  void *args : Task parameter defined during task creation (unused).
 *
 * Return:
 *  void
 *
 *******************************************************************************/

uint8_t* data_acquisition_task()
{
    cyhal_uart_t uart_obj;
    uint32_t     actualbaud;
    uint8_t      rx_buf[RX_BUF_SIZE];

    // Initialize the UART configuration structure
    const cyhal_uart_cfg_t uart_config =
    {
        .data_bits      = 8,
        .stop_bits      = 1,
        .parity         = CYHAL_UART_PARITY_NONE,
        .rx_buffer      = rx_buf,
        .rx_buffer_size = RX_BUF_SIZE
    };

    uint8_t read_data; /* Variable to store the received character
                        * through terminal */

    // Initialize the UART Block
    cyhal_uart_init(&uart_obj, BAR_TX, BAR_RX, NC, NC, NULL,
                           &uart_config);

    // Set the baud rate
    cyhal_uart_set_baud(&uart_obj, 115200, &actualbaud);

    __enable_irq();
    uint8_t* data_barcode = malloc(TX_BUF_SIZE * sizeof(uint8_t));

    if (data_barcode == NULL)
    {
        return NULL;
    }

    // melakukan pembacaan UART dari barcode scanner dan menyalinnya pada variabel data_barcode
    for (size_t i = 0;i<TX_BUF_SIZE;i++)
    {
    	if (CY_RSLT_SUCCESS == cyhal_uart_getc(&uart_obj,
    											   &read_data, 0))
		{
			data_barcode[i]=read_data;
		}
    }
    return data_barcode;
}

/*******************************************************************************
 * Function Name: data_acquisition_rfid_task
 *******************************************************************************
 * Summary:
 *  Task used to retrieve data from qrcode and rfid.
 *
 * Parameters:
 *  void *args : Task parameter defined during task creation (unused).
 *
 * Return:
 *  void
 *
*******************************************************************************/
char** data_acquisition_rfid_task()
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

//	for (;;)
//	{
//		// mengirimkan perintah baca apabila button acq (main) ditekan
//		if (true == gpio_intr_flag_acq)
//		{
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
//			printf("\x1b[2J\x1b[;H");
			printf("Hexadecimal string array: ");
			for (int i = 0; i < arrayLength; i++)
			{
				// menampilkan epc ke terminal
				printf("0x%s ", hexStringArray[i]);
			}
			printf("\r\n");

//			for (int i = 0;i<arrayLength;i++)
//			{
//				// salin hexStringArray ke epc untuk ditampilkan ke OLED
//				printf("%s ",params->epc[i]);
//			}
//			printf("\r\n");

//		 // Unlock the mutex after done with epc
//			cy_rtos_mutex_set(&epc_mutex);
			printf("\r\n");
			printf("%d\r\n",strlen(hexStringArray[0]));

			return hexStringArray;
//			// Free allocated memory
//			for (int i = 0; i < arrayLength; i++)
//			{
//				free(hexStringArray[i]);
//			}
//			free(hexStringArray);
//	        vTaskDelay(pdMS_TO_TICKS(DEBOUNCING_DELAY_MS)); // Debouncing delay
//		}
//	}

}

#endif // DATA_ACQUISITION_H
/* [] END OF FILE */
