/*******************************************************************************
* Copyright 2017-2019 Cypress Semiconductor Corporation
* SPDX-License-Identifier: Apache-2.0
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*     http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
********************************************************************************/

#include "cype.h"
#include "cype_transport_uart.h"
#include "cype_packet.h"
#include "cype_core.h"
#include "cy_power_estimator.h"
#include "cyabs_rtos.h"

#include "cype_platform_api.h"
#include "platform_cype_uart.h"
#include "mbed_power_mgmt.h"

#ifdef MBED_COMMAND_CONSOLE_MW_EXISTS
#include "command_console.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************
 *                      Macros
 ******************************************************/

/******************************************************
 *                    Constants
 ******************************************************/

/******************************************************
 *                   Enumerations
 ******************************************************/

/******************************************************
 *                 Type Definitions
 ******************************************************/

/******************************************************
 *                    Structures
 ******************************************************/

/******************************************************
 *               Static Function Declarations
 ******************************************************/
static cype_result_t cype_transport_uart_read_handler( cype_packet_t** packet );
extern bool cy_command_console_status() ;
/******************************************************
 *               Variable Definitions
 ******************************************************/

static cype_bool_t                                 cype_transport_uart_thread_initialised = CYPE_FALSE;
static cype_transport_received_packet_handler_t     cype_transport_received_packet_handler = NULL;

static volatile cype_bool_t                  uart_thread_running = CYPE_FALSE;

extern cy_semaphore_t  cype_stdio_uart_rx_semaphore;

/******************************************************
 *               CMSIS Definitions
 ******************************************************/
//Thread
static cy_thread_t uart_thread;

/******************************************************
 *               Function Definitions
 ******************************************************/
void cype_transport_uart_thread_main( uint32_t argument )
{
    cype_packet_t* packet = NULL;
    CYPE_PRINT_ERROR( ( "cype_transport_uart_thread_main: enter\n" ) );
	sleep_manager_lock_deep_sleep();

    /* turn off buffers, so IO occurs immediately */
#ifdef _IONBF
    setvbuf( stdin, NULL, _IONBF, 0 );
    setvbuf( stdout, NULL, _IONBF, 0 );
    setvbuf( stderr, NULL, _IONBF, 0 );
#endif

    while( 1 )
    {
        if ( ( cype_get_mode( ) != CYPE_MODE_NO_CONSOLE ) && platform_is_warm_booted()) {
            CYPE_PRINT_DEBUG( ( "cype_transport_uart_thread_main: Already in console mode before warmboot..\n" ) );
            break;
        }
        /* Wait for some time to give command console the opportunity to come up and take UART control */
        osDelay( CYPE_TIME_TO_WAIT_FOR_CONSOLE );

#ifdef MBED_COMMAND_CONSOLE_MW_EXISTS		
		if(cy_command_console_status() == CYPE_SUCCESS) {
			cype_uart_deinit();
			break;
		}
#endif

        /* Set to No Console mode */
        cype_set_mode( CYPE_MODE_NO_CONSOLE );

        while ( 1 )
        {
            if ( cype_get_mode( ) != CYPE_MODE_NO_CONSOLE )
            {
                /* PAD mode is not enabled, release the semaphore and wait for the mode to be enabled */
				cy_rtos_set_semaphore(&cype_stdio_uart_rx_semaphore, CYPE_FALSE);
                break;
            }
            CYPE_PRINT_DEBUG( ( "cype_transport_uart_thread_main: Got semaphore to read\n" ) );

            if ( cype_transport_uart_read_handler( &packet ) != CYPE_SUCCESS )
            {
                continue;
            }
            /* Read successful. Notify upper layer via callback that a new packet is available */
            cype_transport_received_packet_handler( packet );
        }
    }
}

cype_result_t cype_uart_transport_init( cype_transport_received_packet_handler_t handler )
{
    cype_result_t result = CYPE_ERROR;

	if ( handler == NULL )
    {
        return CYPE_BADARG;
    }

    if ( cype_transport_uart_thread_initialised == CYPE_TRUE )
    {
        return CYPE_SUCCESS;
    }

	result = cype_uart_init();
	
	if ( result != CYPE_SUCCESS )
		return result;
	
    /* Create UART thread to receive the packets.
     */
    uart_thread_running = CYPE_TRUE;
    cype_transport_received_packet_handler = handler;

    /* Set the semaphore for Command Console or CYPE */
	cy_rtos_set_semaphore(&cype_stdio_uart_rx_semaphore, CYPE_FALSE);
 
	result = (cype_result_t)cy_rtos_create_thread(&uart_thread, cype_transport_uart_thread_main,
											   "CYPE UART THREAD",(char *) NULL,
											   CYPE_UART_STACK_SIZE,
											   (cy_thread_priority_t)osPriorityHigh,(uint32_t)NULL);

	if (result != CYPE_SUCCESS)
	{
	 	/* Could not start CyPE UART main thread */
	 	CYPE_PRINT_ERROR( ("Could not start CYPE thread\n") );
		goto error;
	}
	cype_transport_uart_thread_initialised = CYPE_TRUE;

    return CYPE_SUCCESS;

    error:
    cype_uart_transport_deinit( );
    return result;
}

cype_result_t cype_uart_transport_deinit( void )
{
    if ( cype_transport_uart_thread_initialised == CYPE_FALSE )
    	{
        return CYPE_SUCCESS;
    	}
	cype_uart_deinit();
	cy_rtos_terminate_thread(&uart_thread);

    uart_thread_running = CYPE_FALSE;

    cype_transport_received_packet_handler = NULL;
    cype_transport_uart_thread_initialised = CYPE_FALSE;
    return CYPE_SUCCESS;
}

cype_result_t cype_uart_transport_send_packet( cype_packet_t* packet )
{
    cype_result_t result = CYPE_ERROR;

    if ( cype_get_console_prints_status( ) )
    {
        uint32_t i = 0;
        for( i = 0; i < packet->packet_size; i++  )
        {
            CYPE_PRINT_INFO( ( "0x%x ", *( packet->packet_start+i ) ) );
        }
        CYPE_PRINT_INFO( ( "\n" ) );
    }
    else
    {
        result = (cype_result_t) cype_uart_transmit_bytes(packet->packet_start, packet->packet_size );
    }
    if ( result != CYPE_SUCCESS )
    {
        cype_free_packet( packet );
        return result;
    }

    /* Destroy packet */
    return cype_free_packet( packet );
}

cype_result_t cype_transport_uart_read_handler( cype_packet_t** packet )
{
    uint8_t packet_byte=0;
    cype_result_t    result = CYPE_ERROR;
    uint8_t payload_len =  0;
    int expected_transfer_size = 1;

    /* Get the SYN byte */
    result = (cype_result_t) cype_uart_receive_bytes(&packet_byte, expected_transfer_size);

    if ( ( result != CYPE_SUCCESS ) || ( packet_byte != CYPE_SYN_BYTE1 ) )
    {
        return CYPE_PACKET_BUFFER_CORRUPT;
    }

	result = (cype_result_t) cype_uart_receive_bytes(&packet_byte, expected_transfer_size);

    if ( ( result != CYPE_SUCCESS ) || ( packet_byte != CYPE_SYN_BYTE2 ) )
    {
        return CYPE_PACKET_BUFFER_CORRUPT;
    }

    /* Get the packet type */
    result = (cype_result_t) cype_uart_receive_bytes(&packet_byte, expected_transfer_size);
    if ( result != CYPE_SUCCESS ) {
        return result;
    }

    /* Read the header and determine the   */
    switch ( packet_byte )
    {
        case CMD_TARGET_DETECTION:
        case CMD_GET_TARGET_STATUS:
        case CMD_GET_TARGET_CYPE_VERSION:
        case CMD_GET_PROCESSOR_LIST:
        case CMD_LOG_REQUEST:
        case 'L':
        {
            payload_len = 0;
            break;
        }
        case CMD_GET_EVENTS_LIST:
        {
            payload_len = 1;
            break;
        }
        case CMD_GET_EVENT_DESCRIPTOR_LIST:
        {
            payload_len = 2;
            break;
        }
        case CMD_START_LOGGING:
        case CMD_STOP_LOGGING:
        {
            uint8_t events;
            /* Get number of events */
            result = (cype_result_t) cype_uart_receive_bytes(&events, expected_transfer_size);
            if ( result != CYPE_SUCCESS )
                return result;
            payload_len = events * 2;
            break;
        }
        case CMD_LOG_POLL_PERIOD:
            payload_len = 1;
            break;
        default:
            return CYPE_UNSUPPORTED;
    }
    result = cype_dynamic_allocate_packet( packet, CYPE_PAD_REQUEST, packet_byte, payload_len );
    if ( result != CYPE_SUCCESS )
    {
        	CYPE_PRINT_DEBUG(("Failed to allocate request packet\n"));
        return result;
    }

    /* Retrieve the payload data */
    if ( payload_len )
    {
        expected_transfer_size = payload_len;
        //result = platform_cype_uart_receive_bytes(( *packet )->payload_start, &expected_transfer_size, NEVER_TIMEOUT);
        result = (cype_result_t) cype_uart_receive_bytes(( *packet )->payload_start, expected_transfer_size);
        if ( result != CYPE_SUCCESS )
        {
            CYPE_PRINT_DEBUG(("Failed to read the request packet\n"));
            cype_free_packet( *packet );
            return result;
        }
    }
    return CYPE_SUCCESS;
}
#ifdef __cplusplus
} /*extern "C" */
#endif

