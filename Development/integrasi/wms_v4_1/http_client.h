/******************************************************************************
* File Name:   http_client.h (WMS v3.0)
*
* Description: This file contains declaration of task related to HTTP client
* operation.
*
* Programmer : Ikbar, Bostang
*
******************************************************************************/

// Header Guard
#ifndef HTTP_CLIENT_H_
#define HTTP_CLIENT_H_

/* Header file includes. */
#include "data_acquisition.h"
#include "fsm.h"

#include "cyhal.h"
#include "cybsp.h"
#include "cy_retarget_io.h"
#include "data_acquisition.h"

/* FreeRTOS header file. */
#include <FreeRTOS.h>
#include <task.h>
#include <semphr.h>

/* Standard C header file. */
#include <string.h>

/* Cypress secure socket header file. */
#include "cy_secure_sockets.h"

/* Wi-Fi connection manager header files. */
#include "cy_wcm.h"
#include "cy_wcm_error.h"

/* HTTP Client Library*/
#include "cy_http_client_api.h"

/*******************************************************************************
* Macros
********************************************************************************/
/* Wi-Fi Credentials: WIFI_SSID, WIFI_PASSWORD and WIFI_SECURITY_TYPE
 * disesuaikan dengan Wi-Fi network credentials yang digunakan.
 * Note: Maximum length of the Wi-Fi SSID and password is set to
 * CY_WCM_MAX_SSID_LEN and CY_WCM_MAX_PASSPHRASE_LEN as defined in cy_wcm.h file.
 */

#define WIFI_SSID							"bp"
#define WIFI_PASSWORD						"ntueee2025"

#define SERVERHOSTNAME						"89.116.191.171"
#define SERVERPORT							(3636)

/* Security type of the Wi-Fi access point. See 'cy_wcm_security_t' structure
 * in "cy_wcm.h" for more details.
 */

#define WIFI_SECURITY_TYPE                 CY_WCM_SECURITY_WPA2_AES_PSK

/* Maximum number of connection retries to the Wi-Fi network. */
#define MAX_WIFI_CONN_RETRIES             (10u)

/* Wi-Fi re-connection time interval in milliseconds */
#define WIFI_CONN_RETRY_INTERVAL_MSEC     (1000)

#define MAKE_IPV4_ADDRESS(a, b, c, d)     ((((uint32_t) d) << 24) | \
                                          (((uint32_t) c) << 16) | \
                                          (((uint32_t) b) << 8) |\
                                          ((uint32_t) a))
/* RTOS related macros. */
#define HTTP_CLIENT_TASK_STACK_SIZE        (5 * 1024)
#define HTTP_CLIENT_TASK_PRIORITY          (2)

#define BUFFERSIZE							(2048 * 2)
#define SENDRECEIVETIMEOUT					(5000)
#define ANYTHINGRESOURCE					"/crud/inbound"

/*******************************************************************************
* Function Prototype
********************************************************************************/
void http_client_task(void *pvParameters);
cy_rslt_t connect_to_wifi_ap(void);
void disconnect_callback(void *arg);

/*******************************************************************************
* Global Variables
********************************************************************************/
cy_rslt_t result;
bool connected;
bool flag_success_sent = false;

/*******************************************************************************
* Function Definitions
********************************************************************************/

/*******************************************************************************
 * Function Name: http_client_task
 *******************************************************************************
 * Summary:
 *  Task used to establish a connection to a remote TCP server and
 *  control the LED state (ON/OFF) based on the command received from TCP server.
 *
 * Parameters:
 *  void *args : Task parameter defined during task creation (unused).
 *
 * Return:
 *  void
 *
 *******************************************************************************/
void http_client_task(void *pvParameters)
{
	TaskParameters *params = (TaskParameters *)pvParameters;
	int* state = &(params->state);

	// variabel lokal (tidak dibagikan dengan task lain)
	uint8_t epc[len_epc];
    tags tagHTTP;

	for(;;)
	{
		// Menunggu sampai data sukses ditampilkan ke OLED
        if (xQueueReceive(dataQueue_array, &tagHTTP, portMAX_DELAY) == pdPASS)
        {
			// melakukan pengiriman data ketika memasuki state SEND
			if (*state == STATE_SEND)
			{
				uint8_t tx_buf[TX_BUF_SIZE];
				size_t tx_length = TX_BUF_SIZE;
				uint8_t variabel_kirim[TX_BUF_SIZE];

				printf("tag count:%d \r\n",tagHTTP.tag_count); // DEBUG

				for (int k = 0;k<tagHTTP.tag_count;k++)
				{
					cyhal_uart_write(&cy_retarget_io_uart_obj, (void*)tagHTTP.epc_array[k], &epc_length);
					printf("\r\n");
				}

				// TO-DO LIST : memasukkan kode pengolahan array epc untuk mengirim ke back-end kak ikbar
				if (epc != NULL)
				{
				   // Copy the contents of the array to variabel_kirim
					memcpy(variabel_kirim, epc, TX_BUF_SIZE * sizeof(uint8_t));
				}

				// menyalin dari variabel kirim ke tx_buf
				for (size_t k = 0;k<TX_BUF_SIZE;k++)
				{
					tx_buf[k] = variabel_kirim[k];
				}

				// menuliskan tx_buf ke terminal
				printf("tag yang dikirim: ");
				cyhal_uart_write(&cy_retarget_io_uart_obj, (void*)tx_buf, &tx_length);
				printf("\r\n");

				//membuat directory indbound
				char REQUEST_BODY[59];
				variabel_kirim[TX_BUF_SIZE] = '\0';

				snprintf(REQUEST_BODY, sizeof(REQUEST_BODY), "{\"data\": [{\"kode_stok\":\"%s\",\"scanned\": true}]}", (char*)variabel_kirim);

				size_t REQUEST_BODY_LENGTH= sizeof(REQUEST_BODY)-1;

				printf("%s\r\n",REQUEST_BODY);

				// terhubung ke access point wifi
				result = connect_to_wifi_ap();
				CY_ASSERT(result == CY_RSLT_SUCCESS);

				// inisiasi sebagai HTTP client
				result = cy_http_client_init();
				if(result != CY_RSLT_SUCCESS)
				{
					printf("HTTP Client Library Initialization Failed!\r\n");
					CY_ASSERT(0);
				}

				// Server Info
				cy_awsport_server_info_t serverInfo;
				(void) memset(&serverInfo, 0, sizeof(serverInfo));
				serverInfo.host_name = SERVERHOSTNAME;
				serverInfo.port = SERVERPORT;

				// Disconnection Callback
				cy_http_disconnect_callback_t disconnectCallback = (void*)disconnect_callback;

				// Client Handle
				cy_http_client_t clientHandle;

				// Create the HTTP Client
				result = cy_http_client_create(NULL, &serverInfo, disconnectCallback, NULL, &clientHandle);
				if(result != CY_RSLT_SUCCESS){
					printf("HTTP Client Creation Failed!\r\n");
					CY_ASSERT(0);
				}

				// Connect to the HTTP Server
				result = cy_http_client_connect(clientHandle, SENDRECEIVETIMEOUT, SENDRECEIVETIMEOUT);
				if(result != CY_RSLT_SUCCESS)
				{
					printf("HTTP Client Connection Failed!\r\n");
					CY_ASSERT(0);
				}
				else
				{
					printf("\r\nConnected to HTTP Server Successfully\r\n\r\n");
					connected = true;
				}

				// Create Request
				uint8_t buffer[BUFFERSIZE];
				cy_http_client_request_header_t request;
				request.buffer = buffer;
				request.buffer_len = BUFFERSIZE;

				// Pick the correct method for a POST from the enumeration
				request.method = CY_HTTP_CLIENT_METHOD_POST;
				request.range_start = -1;
				request.range_end = -1;
				request.resource_path = ANYTHINGRESOURCE;

				// Create Headers
				uint32_t num_header = 2;
				cy_http_client_header_t header[num_header];
				header[0].field = "Host";
				header[0].field_len = strlen("Host");
				header[0].value = SERVERHOSTNAME;
				header[0].value_len = strlen(SERVERHOSTNAME);

				// populate header[1]'s field with "Content-Type" and value "application/json"
				header[1].field = "Content-Type";
				header[1].field_len = strlen("Content-Type");
				header[1].value = "application/json";
				header[1].value_len = strlen("application/json");

				// Content length header is automatically added!
				result = cy_http_client_write_header(clientHandle, &request, header, num_header);
				if(result != CY_RSLT_SUCCESS){
					printf("HTTP Client Header Write Failed!\r\n");
					CY_ASSERT(0);
				}

				// Var to hold the servers responses
				cy_http_client_response_t response;

				// Send get request to /html resource
				if(connected)
				{
					result = cy_http_client_send(clientHandle, &request, (uint8_t *)REQUEST_BODY, REQUEST_BODY_LENGTH, &response);
					if(result != CY_RSLT_SUCCESS){
						printf("HTTP Client Send Failed!\r\n");
						CY_ASSERT(0);
					}
				}
				else
				{
					// Connect to the HTTP Server
					result = cy_http_client_connect(clientHandle, SENDRECEIVETIMEOUT, SENDRECEIVETIMEOUT);
					if(result != CY_RSLT_SUCCESS){
						printf("HTTP Client Connection Failed!\r\n");
						CY_ASSERT(0);
					}
					else{
						printf("\r\nConnected to HTTP Server Successfully\r\n\r\n");
						connected = true;
					}
					// Send get request to /html resource
					result = cy_http_client_send(clientHandle, &request, (uint8_t *)REQUEST_BODY, REQUEST_BODY_LENGTH, &response);
					if(result != CY_RSLT_SUCCESS){
						printf("HTTP Client Send Failed!\r\n");
						CY_ASSERT(0);
					}
				}

				// Print response message
				printf("Response received:\r\n");
				for(int i = 0; i < response.body_len; i++){
					printf("%c", response.body[i]);
				}
				printf("\n");

				taskYIELD(); // Yield to other tasks
			}
		}
	}
}

/*******************************************************************************
 * Function Name: connect_to_wifi_ap()
 *******************************************************************************
 * Summary:
 *  Connects to Wi-Fi AP using the user-configured credentials, retries up to a
 *  configured number of times until the connection succeeds.
 *
 *******************************************************************************/
cy_rslt_t connect_to_wifi_ap(void)
{

    /* Variables used by Wi-Fi connection manager.*/
    cy_wcm_connect_params_t wifi_conn_param;

    cy_wcm_config_t wifi_config = { .interface = CY_WCM_INTERFACE_TYPE_STA };

    cy_wcm_ip_address_t ip_address;

     /* Initialize Wi-Fi connection manager. */
    result = cy_wcm_init(&wifi_config);

    if (result != CY_RSLT_SUCCESS)
    {
        printf("Wi-Fi Connection Manager initialization failed!\r\n");
        return result;
    }
    printf("Wi-Fi Connection Manager initialized.\r\n");

     /* Set the Wi-Fi SSID, password and security type. */
    memset(&wifi_conn_param, 0, sizeof(cy_wcm_connect_params_t));
    memcpy(wifi_conn_param.ap_credentials.SSID, WIFI_SSID, sizeof(WIFI_SSID));
    memcpy(wifi_conn_param.ap_credentials.password, WIFI_PASSWORD, sizeof(WIFI_PASSWORD));
    wifi_conn_param.ap_credentials.security = WIFI_SECURITY_TYPE;

    /* Join the Wi-Fi AP. */
    for(uint32_t conn_retries = 0; conn_retries < MAX_WIFI_CONN_RETRIES; conn_retries++ )
    {
        result = cy_wcm_connect_ap(&wifi_conn_param, &ip_address);

        if(result == CY_RSLT_SUCCESS)
        {
            printf("Successfully connected to Wi-Fi network '%s'.\n",
                                wifi_conn_param.ap_credentials.SSID);
            printf("IP Address Assigned: %d.%d.%d.%d\n", (uint8)ip_address.ip.v4,
                    (uint8)(ip_address.ip.v4 >> 8), (uint8)(ip_address.ip.v4 >> 16),
                    (uint8)(ip_address.ip.v4 >> 24));
            return result;
        }

        printf("Connection to Wi-Fi network failed with error code %d."
               "Retrying in %d ms...\n", (int)result, WIFI_CONN_RETRY_INTERVAL_MSEC);

        vTaskDelay(pdMS_TO_TICKS(WIFI_CONN_RETRY_INTERVAL_MSEC));
    }

    /* Stop retrying after maximum retry attempts. */
    printf("Exceeded maximum Wi-Fi connection attempts\n");

    return result;
}

/*******************************************************************************
 * Function Name: disconnect_callback
 *******************************************************************************
 * Summary:
 *  Invoked when the server disconnects
 *
 * Parameters:
 *  void *arg : unused
 *
 * Return:
 *  void
 *
 *******************************************************************************/
void disconnect_callback(void *arg)
{
    printf("Disconnected from HTTP Server\n");
    connected = false;
}

#endif /* HTTP_CLIENT_H_ */

/* [] END OF FILE */
