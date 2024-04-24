/******************************************************************************
* File Name:   http_client.h
*
* Description: This file contains declaration of task related to HTTP client
* operation.
*
*******************************************************************************/

#ifndef DISPLAY_H_
#define DISPLAY_H_

/*******************************************************************************
* Macros
********************************************************************************/
#define OLED_TASK_NAME         ("OLED Task")
#define OLED_TASK_NAME         ("OLED Task")
#define OLED_TASK_STACK_SIZE   (1024)
//#define OLED_TASK_PRIORITY     (5)
#define OLED_TASK_PRIORITY     (4)		// diturunkan prioritasnya untuk dilihat apakah akan mengalah masalah mutex dengan rfid_task

#define OLED_HOR		(128)
#define OLED_VER		(64)

#define BLINKING_DELAY_MS		(1000)

const GUI_FONT * OldFont;

#define len_epc 12
//const char* epc_test[len_epc]= {"A0", "04", "01", "89", "01", "D1", "A0", "04", "08", "89", "31", "D1"};


/*******************************************************************************
* Function Prototypes
*******************************************************************************/
//void oled_task(void* arg);
//void oled_task(void* state);
//void oled_task(int* state);
void oled_task(void *pvParameters);

#endif /* DISPLAY_H_ */
