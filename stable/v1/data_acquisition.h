/*******************************************************************************
* File Name:   data_acquisition.h (WMS v3.0)
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
#include "utils.h"
#include "barcode.h"
#include "rfid.h"

/*******************************************************************************
* Macros
*******************************************************************************/
//#define DATA_ACQ_TASK_PRIORITY  (2) // DEBUG
#define DATA_ACQ_TASK_PRIORITY  (1)

// pin selektor RFID/barcode
#define SCANNER_SELECTOR_PIN	(P9_7)
#define barcode_mode			(false)
#define RFID_mode				(true)

#define DATA_ACQ_TASK_STACK_SIZE	(1024)

#define MULTIPLE_READ_RFID		(true)		// 1 : multiple read, 0 : single read

/*******************************************************************************
* Global Variables
*******************************************************************************/
//bool scanner_selector; // selektor rfid/barcode
uint8_t* resultArrayBarcode; // untuk barcode
uint8_t* resultArrayRFID; // untuk RFID

/*******************************************************************************
* Function Prototypes
********************************************************************************/
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
	// inisiasi pin selektor (P9_7)
	result = cyhal_gpio_init(SCANNER_SELECTOR_PIN, CYHAL_GPIO_DIR_INPUT, CYHAL_GPIO_DRIVE_NONE, false);

	/* User button init failed. Stop program execution */
	handle_error(result);

	// membaca tegangan pin selector (high : RFID, low : Barcode)
	bool read_val = cyhal_gpio_read(SCANNER_SELECTOR_PIN);

	return read_val;
}

/*******************************************************************************
 * Function Name: data_acquisition_task
 *******************************************************************************
 * Summary:
 *  Melakukan task data acquisition dengan barcode ataupun rfid
 *
 * Parameters:
 *  void.
 *
 * Return:
 *  bool : true -> RFID
 *  	   false -> Barcode
 *
 *******************************************************************************/
void data_acquisition_task(void *pvParameters)
{
    TaskParameters *params = (TaskParameters *)pvParameters;
    uint8_t epc[len_epc]; // variabel lokal epc yang nanti akan dikirim ke task display dengan queue
    int* state = &(params->state);
    bool* scanner_selector = &(params->scanner_mode);

    // selektor RFID/barcode
	*scanner_selector = initialize_selector();

    for(;;)
    {
		if (*state == STATE_SCAN)
		{
			if (ulTaskNotifyTake(pdTRUE, portMAX_DELAY))
			{
				switch (*scanner_selector)
				{
					// Proses scanning dengan RFID
					case RFID_mode:
					{
						uint8_t* resultArrayRFID = data_acquisition_rfid_task();

						// Menyimpan hasil pembacaan ke variabel epc
						for (int k = 0; k < len_epc; k++)
						{
							epc[k] = resultArrayRFID[k];
						}
						break;

					}
					// Proses scanning dengan barcode
					case barcode_mode:
					{
						uint8_t* resultArrayBarcode = data_acquisition_barcode();

						// Menyimpan hasil pembacaan ke variabel epc
						for (int k = 0; k < len_epc; k++)
						{
							epc[k] = resultArrayBarcode[k];
						}
						break;
					}
					default:
						// Handle the case where scanner_selector is neither RFID_mode nor barcode_mode (never reached under normal condition)
						break;
				}
				// saat sukses pindai data dengan RFID, kirim notifikasi ke task buzzer
				xTaskNotifyGive(buzzer_taskHandle);

				// Kirim hasil ke queue
//				printf("mengirim queue ke task oled\r\n");
				if (xQueueSend(dataQueue, &epc, portMAX_DELAY) != pdPASS)
				{
					printf("Failed to send data to queue.\r\n");
				}
			}
		}
	    taskYIELD(); // Yield to other tasks
    }
}

#endif // DATA_ACQUISITION_H

/* [] END OF FILE */
