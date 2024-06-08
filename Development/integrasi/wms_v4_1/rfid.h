/*******************************************************************************
* File Name:   rfid.h (WMS v4.1)
* Description: implementasi RFID sebagai bagian dari sub-sistem data acquisition
* 			   Mengirim perintah baca ke RFID scanner, Menunggu byte pertama (header),
* 			   melakukan pembacaan hingga byte terakhir (checksum),
* 			   melakukan perhitungan checksum, melakukan parsing EPC dari response packet,
* 			   melakukan konversi format.
* Programmer: Bostang
* Tanggal : 31 Mei 2024
*******************************************************************************/
// HEADER GUARD
#ifndef RFID_H
#define RFID_H

/*******************************************************************************
* Header Files
*******************************************************************************/
#include <stdlib.h>
#include "cyhal.h"
#include "cybsp.h"
#include "cy_retarget_io.h"
#include "utils.h"
#include "fsm.h"

/*******************************************************************************
* Macros
*******************************************************************************/
#define RFID_RX 				(P12_0)
#define RFID_TX 				(P12_1)

#define UART_DELAY      		(10u)
#define RFID_RX_BUF_SIZE    	(1)
#define RFID_TX_BUF_SIZE    	(6)

#define RFID_TAG_BYTE_SIZE		(12)
//#define LEN_RESPONSE			(42)
#define HEADER_LENGTH 			(7)
#define LEN_RESPONSE			(HEADER_LENGTH+LEN_EPC)

const uint8_t header_pattern[HEADER_LENGTH] = {0xA0, 0x13, 0x01, 0x89, 0x74, 0x30, 0x00}; // disesuaikan untuk PSoC
const uint8_t empty_tag_response[] = {0xA0, 0x0A,0x01,0x89,0x00,0x00,0x00,0x00,0x00,0x00,0xCC};
const uint8_t empty_tag[LEN_EPC]= {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};


/*******************************************************************************
* Global Variables
*******************************************************************************/
cy_rslt_t result;

cyhal_uart_t uart_rfid; // UART object for RFID scanner
uint32_t actualbaud;
uint8_t rfid_tx_buf[RFID_TX_BUF_SIZE] = {0xA0, 0x04, 0x01, 0x89, 0x01, 0xD1}; // cmd_read byte array

uint8_t rfid_rx_buf[RFID_RX_BUF_SIZE];
size_t rfid_tx_length = RFID_TX_BUF_SIZE;

size_t rfid_rx_length = RFID_RX_BUF_SIZE;

cy_rslt_t rslt;


static bool rfid_initialized = false;

// Flags
// Variabel flag untuk menandakan telah terdeteksi tag
volatile bool flag_tag_detect;
// variabel flag untuk deteksi header (0x0A)
volatile bool flag_header_detect;
// variabel flag untuk menyatakan telah selesai deteksi EPC
volatile bool flag_finish_read;
// Variabel flag untuk menandakan telah terdeteksi tag
volatile bool flag_epc_available;

// indeks tag
int index_response_packet;
int index_tag;

// variabel EPC
uint8_t response_packet[LEN_RESPONSE];
uint8_t epc[LEN_EPC];
uint8_t checksum;


/*******************************************************************************
* Function Prototypes
********************************************************************************/
uint8_t* data_acquisition_rfid_task(); // RFID
void data_acquisition_rfid_multiple_read_task(uint8_t** epc_array);

/*******************************************************************************
* Function Definitions
*******************************************************************************/
/*******************************************************************************
 * Function Name: data_acquisition_rfid_task
 *******************************************************************************
 * Summary:
 *  Task used to retrieve data from rfid scanner.
 *
 * Parameters:
 *  void *args : Task parameter defined during task creation (unused).
 *
 * Return:
 *  void
 *
 * Writer :
 *	Bostang Palaguna
 *
*******************************************************************************/
uint8_t* data_acquisition_rfid_task()
{
	// Inisiasi
    if (!rfid_initialized)
    {
		#if defined(CY_DEVICE_SECURE)
			cyhal_wdt_t wdt_obj;
			/* Clear watchdog timer so that it doesn't trigger a reset */
			result = cyhal_wdt_init(&wdt_obj, cyhal_wdt_get_max_timeout_ms());
			CY_ASSERT(CY_RSLT_SUCCESS == result);
			cyhal_wdt_free(&wdt_obj);
		#endif

		/* Initialize the UART configuration structures */
		const cyhal_uart_cfg_t uart_rfid_config =
		{
			.data_bits = DATA_BITS_8,
			.stop_bits = STOP_BITS_1,
			.parity = CYHAL_UART_PARITY_NONE,
			.rx_buffer = rfid_rx_buf,
			.rx_buffer_size = RFID_RX_BUF_SIZE
		};

		/* Initialize the UART Blocks */
		result = cyhal_uart_init(&uart_rfid, RFID_TX, RFID_RX, NC, NC, NULL, &uart_rfid_config);
		handle_error(result);

		/* Set the baud rates */
		result = cyhal_uart_set_baud(&uart_rfid, BAUD_RATE, &actualbaud);
		handle_error(result);

		rfid_initialized = true;
    }

    while(true) // terus melakukan proses scanning sampai ada data sukses yang diterima
    {
    	flag_tag_detect = false;
		flag_header_detect = false;
		flag_finish_read = false;
		flag_epc_available = false;

		// Membersihkan flag di awal iterasi dan rfid tag
		index_response_packet = 0;
		index_tag = -7;

		uint8_t* epc_processed = malloc(TX_BUF_SIZE * sizeof(uint8_t)); // agar mekanisme pemrosesan serupa dengan barcode
		if (epc_processed == NULL)
		{
			return NULL;
		}

		memset(epc, 0, sizeof(epc));
		memset(response_packet, 0, sizeof(response_packet));

		/* Begin Tx Transfer */
		// Send the command to start reading from the RFID scanner
		cyhal_uart_write(&uart_rfid, (void*)rfid_tx_buf, &rfid_tx_length);

		// Read the response data from the RFID scanner
			// tunggu sampai data pertama terdeteksi
		while (!flag_tag_detect)
		{
			if (CY_RSLT_SUCCESS == cyhal_uart_getc(&uart_rfid, &rfid_rx_buf, 0))
			{
				if (rfid_rx_buf[0] == 0xA0)
				{
					index_tag++; // supaya setelah ini langsung cek indeks ke-dua dan seterusnya

					response_packet[index_response_packet] = rfid_rx_buf[0]; // 0xA0 sebagai elemen pertama response_packet
					index_response_packet++;
					flag_tag_detect = 1;
				}
				else
				{
					printf("byte pertama BUKAN 0xA0\r\n");
					continue;

				}
			}
			if (flag_tag_detect) // jika 0xA0 sudah terdeteksi, terus melakukan akuisisi data dan simpan ke response_packet
			{
				int byte_detected = 0; // indeks untuk menangani kasus tag kosong
				while (index_tag < 0)
				{
					if (CY_RSLT_SUCCESS == cyhal_uart_getc(&uart_rfid, &rfid_rx_buf[0], 0))
					{
						byte_detected++;
						if ((rfid_rx_buf[0] == header_pattern[index_tag + 7]) || (index_tag == -3)) // skip untuk frekuensi antenna karena bisa berbeda-beda
						{
							index_tag++;

							response_packet[index_response_packet] = rfid_rx_buf[0] ;
							index_response_packet++;
						}
					}
					// jika sudah selesai baca tetapi indeks masih -6 berarti tag tidak terdeteksi
					// isi tag kosong = {0xA0, 0x0A,0x01,0x89,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
					if ((byte_detected == 11) && (index_tag < 0))
					{
						printf("tag tidak terdeteksi!\r\n");
						continue;
					}

					if (index_tag == 0) // header terdeteksi dengan sukses
					{
						flag_epc_available = true;
					}
				}
			}
		}

		// terus baca sampai akhir
		if (flag_epc_available)
		{
			while((CY_RSLT_SUCCESS == cyhal_uart_getc(&uart_rfid, &rfid_rx_buf, 0))&&(!flag_finish_read))
			{
				response_packet[index_response_packet] = rfid_rx_buf[0];
				index_response_packet++;

				if (index_response_packet > LEN_RESPONSE-1) // baca 2 byte tambahan setelah epc yaitu RSSI & checksum
				{
					flag_finish_read = true;
				}
			}

			// memeriksa checksum
			checksum = CheckSum(response_packet,LEN_RESPONSE-1);
			if (response_packet[LEN_RESPONSE-1] ==checksum)
			{
				// melakukan ekstraksi epc dari response packet
				for (int k = 0;k<RFID_TAG_BYTE_SIZE;k++)
				{
					// byte ke 7 sampai dengan 19 dari response packet yang utuh adalah epc
					epc[k] = response_packet[k+7];
				}
			}
			else
			{
				printf("Checksum tidak sesuai! paket cacat!\r\n");
				continue;
			}

			// mengubah format array heksadesimal menjadi epc (mengambil 14 dari 24 byte pertama)
				// misal : 0x21 0x05 0x24 0x08 0x58 0x52 0x1D 0x00 0x00 0x00 0x00 0x00 -> 21052408581D
			size_t array_length = sizeof(epc) / sizeof(epc[0]);
			hex_array_to_char_array(epc, array_length, epc_processed);
			return epc_processed;
		}
		else
		{
			continue;
		}
    }
}

#endif // RFID_H
/* [] END OF FILE */
