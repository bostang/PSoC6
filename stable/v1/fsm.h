/*******************************************************************************
* File Name:   fsm.h (WMS v3.0)
* Description: implementasi fsm (transisi state) pada pengujian integrasi sistem
* Programmer: Bostang
* Tanggal : 2024/05/08
*******************************************************************************/

// HEADER GUARD
#ifndef FSM_H
#define FSM_H

/*******************************************************************************
* Header Files
*******************************************************************************/
#include <stdio.h>
#include <stdbool.h>

#include "FreeRTOS.h"
#include "task.h"
#include "cyabs_rtos.h"
#include <pthread.h>

/*******************************************************************************
* Macros
*******************************************************************************/
// State
#define STATE_IDLE		(0)
#define STATE_SCAN		(1)
#define STATE_DISPLAY	(2)
#define STATE_SEND		(4)

// Scanner
#define	BARCODE_MODE	(false)	// sesuai kondisi P9_7 saat inisiasi
#define RFID_MODE		(true)

#define len_epc 		(14)
#define LEN_EPC			(len_epc)

#define MAX_EPC_ARRAY_SIZE 10 // jumlah epc maksimum yang disimpan di dalam array

#define ANYTHINGRESOURCE_INBOUND			"/hw-crud/inbound"
#define ANYTHINGRESOURCE_OUTBOUND			"/hw-crud/outbound"

// tipe data untuk parameter task : struct berisi state dan juga epc
typedef struct
{
    int state;
    bool scanner_mode;
//    uint8_t epc[len_epc];
} TaskParameters;

typedef struct
{
    int tag_count;
    uint8_t *epc_array[LEN_EPC];
} tags;

/*******************************************************************************
* Global Variables
*******************************************************************************/
//SemaphoreHandle_t Event_Semaphore;

// Task Handles
/* HTTP Client task handle. */
TaskHandle_t client_task_handle;
TaskHandle_t oled_taskHandle;
TaskHandle_t data_acq_taskHandle;
TaskHandle_t buzzer_taskHandle;
size_t epc_length = LEN_EPC;
QueueHandle_t dataQueue; // queue data untuk komuniksai antara task data_acquisition dan oled
QueueHandle_t dataQueue_array; // queue data untuk komuniksai antara task data_acquisition dan oled
QueueHandle_t ackQueue; // queue data (sukses kirim/tidak) dari task http_client ke oled
SemaphoreHandle_t xStateMutex;
extern SemaphoreHandle_t xModeMutex;

// Definisi variabel global
char ANYTHINGRESOURCE[30] = "/hw-crud/inbound";

//pthread_mutex_t anythingresource_mutex = PTHREAD_MUTEX_INITIALIZER;

/*******************************************************************************
* Function Prototypes
*******************************************************************************/
void display_state(int* state);
int handle_acquire_button(int *current_state);
int handle_previous_button(int *current_state);
void toggle_resource_path(void); // Deklarasi fungsi untuk mengubah nilai variabel

/*******************************************************************************
* Function Definitions
*******************************************************************************/

/*******************************************************************************
* Function Name: handle_acquire_button
********************************************************************************
* Summary:
* menangani perpindahan state akibat penekanan acquire_button
*
* Parameters:
*  uint32_t state - menyatakan state saat ini
*
* Return:
*  void
*
*	(IDLE) --ACQ--> (SCAN) --ACQ--> (DISPLAY) --ACQ--> (SEND)
*	  ^--------------------------------------------------|
*
*******************************************************************************/
int handle_acquire_button(int *current_state)
{
	switch (*current_state)
	{
		case STATE_IDLE:
			return STATE_SCAN;
		case STATE_SCAN:
			return STATE_DISPLAY;
		case STATE_DISPLAY:
			return STATE_SEND;
		case STATE_SEND:
			return STATE_IDLE;
	}
}

/*******************************************************************************
* Function Name: handle_prev_button
********************************************************************************
* Summary:
* menangani perpindahan state akibat penekanan acquire_button
*
* Parameters:
*  uint32_t state - menyatakan state saat ini
*
* Return:
*  void
*
*	(IDLE) <--PREV-- (SCAN) <--PREV-- (DISPLAY) <--PREV--> (SEND)
*	  |-------------------------x----------------------------^
*
*******************************************************************************/
int handle_previous_button(int *current_state)
{
	switch (*current_state)
		{
			case STATE_IDLE:
				return STATE_SEND;
//			case STATE_SCAN:
//	//			return STATE_DISPLAY;
//				return STATE_IDLE;
//			case STATE_DISPLAY:
//				return STATE_SEND;
//			case STATE_SEND:
//				return STATE_IDLE;
		}
}

/*******************************************************************************
* Function Name: display_state
********************************************************************************
* Summary:
* Menampilkan sekarang sedang berada di state apa
*
* Parameters:
*  uint32_t state - menyatakan state saat ini
*
* Return:
*  void
*
*******************************************************************************/
void display_state(int* state)
{
	if (*state == STATE_IDLE)
	{
		printf("STATE IDLE\r\n");
	}
	else if (*state == STATE_SCAN)
	{
		printf("STATE SCAN\r\n");
	}
	else if (*state == STATE_DISPLAY)
	{
		printf("STATE DISPLAY\r\n");
	}
	else if (*state == STATE_SEND)
	{
		printf("STATE SEND\r\n");
	}
	else
	{
		printf("[ERROR] STATE TIDAK VALID!\r\n"); // for debugging
	}
}

void toggle_resource_path(void)
{
//    pthread_mutex_lock(&anythingresource_mutex);
	if (xSemaphoreTake(xModeMutex, portMAX_DELAY) == pdTRUE)
	{

		if (strcmp(ANYTHINGRESOURCE, ANYTHINGRESOURCE_INBOUND) == 0)
		{
			strcpy(ANYTHINGRESOURCE, ANYTHINGRESOURCE_OUTBOUND);
		}
		else if (strcmp(ANYTHINGRESOURCE, ANYTHINGRESOURCE_OUTBOUND) == 0)
		{
			strcpy(ANYTHINGRESOURCE, ANYTHINGRESOURCE_INBOUND);
		}
		else
		{
			strcpy(ANYTHINGRESOURCE, "FAILED");
		}

        // Release the mutex after accessing the shared state
        xSemaphoreGive(xModeMutex);
	}
//    pthread_mutex_unlock(&anythingresource_mutex);
}


#endif // FSM_H

/* [] END OF FILE */
