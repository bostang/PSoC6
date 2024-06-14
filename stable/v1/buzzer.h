/*******************************************************************************
* File Name:   buzzer.h (WMS v3.0)
* Description: implementasi buzzer sebagai bagian dari sub-sistem Display
* 				apabila button ditekan, maka buzzer akan berkedipa selama tiga kali dengan jeda 500 ms
* Programmer: Bostang
*******************************************************************************/

// Header Guard
#ifndef BUZZER_H
#define BUZZER_H

/*******************************************************************************
* Header Files
*******************************************************************************/
#include "cyhal.h"
#include "cybsp.h"
#include "cy_retarget_io.h"
#include "FreeRTOS.h"
#include "task.h"

/*******************************************************************************
* Macros
*******************************************************************************/
#define DEBOUNCING_DELAY_MS         (1000)   /* milliseconds */
#define GPIO_INTERRUPT_PRIORITY 	(7u)

#define BUZZER_BLINKING_COUNT		(20)	// 20/2 = 10 kali buzzing
#define BUZZ_TASK_NAME         		("Buzzer Task")
#define BUZZ_TASK_STACK_SIZE   		(1024)
#define BUZZ_TASK_PRIORITY     		(1)
//#define BUZZ_TASK_PRIORITY     		(2)	// DEBUG 11 juni
#define BUZZER_BEEP_DURATION_MS		(100)
#define BUZZ_PIN P12_3

#define BUZZ_NORMAL_COUNT			(1)
#define	BEEP_SAME_PERIOD_COUNT		(3)
#define BUZZ_ON						(1)
#define BUZZ_OFF					(0)

UBaseType_t __attribute__((used)) uxTopUsedPriority;
#define uxTopReadyPriority uxTopUsedPriority

/*******************************************************************************
* Global Variables
*******************************************************************************/
int BUZZER_BLINKING_DELAY_MS = 1000;

/*******************************************************************************
* Function Prototypes
*******************************************************************************/
void buzzer_task(void *pvParameters);
void Buzzer_selfDestruction();
void Buzzer_mainOperation();

/*******************************************************************************
* Function Definitions
*******************************************************************************/

/*******************************************************************************
* Function Name: buzzer_task
********************************************************************************
* Summary:
*   Task to handle button events.
*
* Parameters:
*   void *arg: Task arguments (unused)
*
*******************************************************************************/
void buzzer_task(void *pvParameters)
{
    TaskParameters *params = (TaskParameters *)pvParameters;
    int* state = &(params->state);
    bool* scanner_selector = &(params->scanner_mode);

    printf("%s started!\r\n", BUZZ_TASK_NAME);
    for(;;)
    {
    	// Ketika state SCAN menggunakan RFID, saat terima notifikasi dari task data acquisition maka buzzer berbunyi sekali
    	// Ketika state SEND, saat data sudah terkirim ke back-end : terima notifikasi dari task oled maka buzzer berbunyi tiga kali
    	if (ulTaskNotifyTake(pdTRUE, portMAX_DELAY))
		{
			if (*state == STATE_SCAN)
			{
			// melakukan pengiriman data ketika memasuki state SEND
				Buzzer_mainOperation();

				// kirim notifikasi kembali ke task oled

				xTaskNotifyGive(oled_taskHandle);

				printf("berhasil kirim notifikasi ke task oled\r\n");	// DEBUG
			}
			else if (*state == STATE_SEND)
			{
				for (int i = 0;i<3;i++)
				{
					Buzzer_mainOperation();
					vTaskDelay(pdMS_TO_TICKS(BUZZER_BEEP_DURATION_MS)) ;
				}

				// kirim notifikasi kembali ke task oled
				xTaskNotifyGive(oled_taskHandle);
			}
			else if (*state == STATE_IDLE)	// mode diubah saat masih di state IDLE
			{
				for (int i = 0;i<2;i++)
				{
					Buzzer_mainOperation();
					vTaskDelay(pdMS_TO_TICKS(BUZZER_BEEP_DURATION_MS)) ;
				}

				// kirim notifikasi kembali ke task oled
				xTaskNotifyGive(oled_taskHandle);
			}
		}
    	taskYIELD(); // Yield to other tasks
    }
}

/*******************************************************************************
* Function Name: Buzzer_selfDestruction
********************************************************************************
* Summary:
*   Pola bunyi buzzer saat count-down 10 sampai 0 (test).
*
* Parameters:
*   void *arg: Task arguments (unused)
*
*******************************************************************************/
void Buzzer_selfDestruction()
{
	// Buzzing normal dua kali
	for (int k = 0;k<BUZZ_NORMAL_COUNT;k++)
	{
		cyhal_gpio_write((cyhal_gpio_t)BUZZ_PIN, BUZZ_ON);
		vTaskDelay(pdMS_TO_TICKS(BUZZER_BEEP_DURATION_MS)) ;
		cyhal_gpio_write((cyhal_gpio_t)BUZZ_PIN, BUZZ_OFF);
		vTaskDelay(pdMS_TO_TICKS(BUZZER_BLINKING_DELAY_MS)) ;
	}

	// Buzzing makin cepat
	printf("Buzzer Berbunyi!\r\n");
	for(int i = 0;i<BUZZER_BLINKING_COUNT;i++)
	{
		for (int j = 0;j<BEEP_SAME_PERIOD_COUNT;j++)
		{
			cyhal_gpio_write((cyhal_gpio_t)BUZZ_PIN, BUZZ_ON);
			vTaskDelay(pdMS_TO_TICKS(BUZZER_BEEP_DURATION_MS)) ;
			cyhal_gpio_write((cyhal_gpio_t)BUZZ_PIN, BUZZ_OFF);
			vTaskDelay(pdMS_TO_TICKS(BUZZER_BLINKING_DELAY_MS)) ;
		}
		BUZZER_BLINKING_DELAY_MS /= 2; // delay makin lambat
	}
}

/*******************************************************************************
* Function Name: Buzzer_mainOperation
********************************************************************************
* Summary:
*   Pola bunyi buzzer saat transisi state (main).
*
* Parameters:
*   void *arg: Task arguments (unused)
*
*******************************************************************************/
void Buzzer_mainOperation()
{
	// Buzzing normal sekali
	cyhal_gpio_write((cyhal_gpio_t)BUZZ_PIN, BUZZ_ON);
	vTaskDelay(pdMS_TO_TICKS(BUZZER_BEEP_DURATION_MS)) ;
	cyhal_gpio_write((cyhal_gpio_t)BUZZ_PIN, BUZZ_OFF);

//	// vTaskDelay untuk debouncing
//	vTaskDelay(pdMS_TO_TICKS(DEBOUNCING_DELAY_MS)) ;
}

#endif /* BUZZER_H */

/* [] END OF FILE */
