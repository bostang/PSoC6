/*******************************************************************************
* File Name:   fsm.h (Integrasi 1)
* Description: implementasi fsm (transisi state) pada pengujian integrasi sistem
* Programmer: Bostang
* Tanggal : 2024/04/04
*******************************************************************************/

 // HEADER GUARD
#ifndef FSM_H
#define FSM_H

/*******************************************************************************
* Header Files
*******************************************************************************/
#include <stdio.h>
#include <stdbool.h>

/*******************************************************************************
* Macros
*******************************************************************************/
// State
#define STATE_IDLE		(0)
#define STATE_SCAN		(1)
#define STATE_DISPLAY	(2)
#define STATE_SEND		(4)

//#define len_epc 14 // versi asli
#define len_epc 12 // versi lama

// tipe data untuk parameter task : struct berisi state dan juga epc
typedef struct
{
    int state;
    char* epc[len_epc];
} TaskParameters;

// global mutex
cy_mutex_t epc_mutex;


/*******************************************************************************
* Function Prototypes
*******************************************************************************/
void display_state(int* state);
int handle_acquire_button(int *current_state);
int handle_previous_button(int *current_state);

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
*	  |------------------------------------------------------^
*
*******************************************************************************/
//State handle_previous_button(State *current_state)
//{
//	switch (*current_state)
//	{
//		case STATE_IDLE:
//			return STATE_SEND;
//		case STATE_SCAN:
//			return STATE_IDLE;
//		case STATE_DISPLAY:
//			return STATE_SCAN;
//		case STATE_SEND:
//			return STATE_DISPLAY;
//	}
//}
//
//
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

#endif // FSM_H
