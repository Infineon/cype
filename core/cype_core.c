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
#include "cype_packet.h"
#include "cype_core.h"
#include "cype_transport.h"
#include "cyabs_rtos.h"

#include "cype_cpl.h"
#include "cype_platform_api.h"
#include "mbed_power_mgmt.h"
#include <stdlib.h>

/******************************************************
 *                      Macros
 ******************************************************/
#define MAX_CPL_LIST        10

//Request packet Macros
#define PACKET_SYNC_WORD_SIZE              2 //bytes
#define PACKET_CMD_SIZE                    1 //byte
#define PACKET_COUNT_SIZE                  1 //byte
#define PACKET_START_TIMESTAMP_SIZE        4 //bytes
#define PACKET_STOP_TIMESTAMP_SIZE         4 //bytes
#define PACKET_EACH_LOG_EVENT_SIZE         7 //bytes

#define PID            1 //byte //Processor ID size
#define EID            1 //byte // Event ID Size
#define EDID           1 //byte //Event Descriptor Size

//Response packet Macros
#define PACKET_RESP_ERROR                1
#define PACKET_RESP_SUCCESS              0

//Console command Macros
#define PACKET_CMD_OFFSET                1
#define PACKET_COUNT_OFFSET              2

#define POLL_TIME_MARGIN_PERCENTAGE             10

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
static cype_result_t handle_target_detection_cmd( void );
static cype_result_t handle_get_target_status_cmd( void );
static cype_result_t handle_get_version_cmd( void );
static cype_result_t handle_request_log_cmd( void );
static cype_result_t handle_get_processor_list_cmd( void );
static cype_result_t handle_get_events_cmd( cype_packet_t* request_packet );
static cype_result_t handle_get_event_descriptor_cmd( cype_packet_t* request_packet );
static cype_result_t handle_start_log_cmd( cype_packet_t* request_packet );
static cype_result_t handle_stop_log_cmd( cype_packet_t* request_packet );
static cype_result_t handle_log_poll_period_cmd( cype_packet_t* request_packet );
static cype_bool_t cpl_has_proc_id( cpl_data_t* cpl, uint8_t proc_id );


/******************************************************
 *                Function Declarations
 ******************************************************/
cype_result_t cype_transport_console_handler( cype_packet_t** packet, int argc, char* argv[] );

/******************************************************
 *               Variable Declarations
 ******************************************************/
 
/******************************************************
 *               CMSIS Definitions
 ******************************************************/
cy_semaphore_t cype_stdio_uart_rx_semaphore;
cy_semaphore_t cype_handle_request_lock;


/******************************************************
 *               Variable Definitions
 ******************************************************/
static cype_time_t  log_start_timestamp ;
static uint32_t      log_sent_time_offset ;
static cype_mode_t  cypeCommunicationMode ;
uint16_t  cype_log_poll_time ;


static cpl_data_t* cpl_list[MAX_CPL_LIST];
static uint8_t registered_cpl_count;
static uint8_t registered_processor_count;
static uint8_t max_power_log_size;
static cype_bool_t cypeStarted;
cype_bool_t deep_sleep_enter = CYPE_FALSE;
uint8_t cype_console_print_status;



/******************************************************
 *               Function Definitions
 ******************************************************/
cype_result_t cype_register_cpl( cpl_data_t* cpl )
{
    /* TODO: Check valid processor id and not existing already */
    cpl_list[registered_cpl_count] = cpl;
    registered_cpl_count++;
    /* Each CPL can have multple processor ids */
    registered_processor_count += cpl->proc_count;
    max_power_log_size += cpl->log_size;

    return CYPE_SUCCESS;
}

cype_result_t cype_init( )
{
	cy_rslt_t result = CYPE_ERROR;

   	result = cy_rtos_init_semaphore(&cype_stdio_uart_rx_semaphore,1,0);
	if ( result != CYPE_SUCCESS )
	{
	    CYPE_PRINT_ERROR( ( "Error creating Semaphore\n" ) );
		return CYPE_ERROR;
	}
	
   	result = cy_rtos_init_semaphore(&cype_handle_request_lock,1,1);
	if ( result != CYPE_SUCCESS )
	{
	    CYPE_PRINT_ERROR( ( "Error creating Semaphore\n" ) );
		return CYPE_ERROR;
	}


    CYPE_PRINT_DEBUG( ( "COLDBOOT: Setting to default console mode\n" ) );
    cype_set_mode( CYPE_MODE_CONSOLE );
	
    if( platform_cype_init() != CYPE_SUCCESS )
    {
        CYPE_PRINT_ERROR(("cype_init: Failed to do platform init\n"));
        return CYPE_ERROR;
    }

    return CYPE_SUCCESS;
}

int cype_start( )
{
    /* Protect cype start */
    if ( cypeStarted )
        return CYPE_SUCCESS;

    CYPE_PRINT_ERROR (( "CyPE Started....\n" )) ;

    if( cype_init() != CYPE_SUCCESS )
    {
        CYPE_PRINT_ERROR(("cype_start: Failed to init\n"));
        return CYPE_ERROR;
    }

    cype_cpl_init( );
    CYPE_PRINT_INFO(( "registered_cpl_count : %d\n",registered_cpl_count ));

    for ( uint8_t i = 0; i < registered_cpl_count; i++ )
    {
        uint8_t j = cpl_list[i]->proc_count;
        CYPE_PRINT_DEBUG(( "cpl_list[%d]->proc_count : %d\n", i, cpl_list[i]->proc_count )) ;
        /* Each CPL can have more than one Processors */
        for ( j = 0; j < cpl_list[i]->proc_count; j++ )
        {
            CYPE_PRINT_DEBUG(( "cpl_list[%d]->proc_id[%d] : 0x%x\n", i, j, *( cpl_list[i]->proc_id+j  ) ));
        }
    }

    cype_transport_init( UART_TRANSPORT, packets_from_cype_host_handler );
		
    cypeStarted = CYPE_TRUE;
	return CYPE_SUCCESS;
}

cype_result_t packets_from_cype_host_handler( cype_packet_t* packet )
{
	
    /* Check if packets to be served by */
    cype_result_t    result = CYPE_SUCCESS;
    /* Route the packet to the proper handler */
    if ( deep_sleep_enter )
    {
        cype_free_packet( packet );
        return CYPE_SUCCESS;
    }

	cy_rtos_get_semaphore(&cype_handle_request_lock, CY_RTOS_NEVER_TIMEOUT, CYPE_FALSE);

    switch ( packet->packet_id )
    {
        case CMD_TARGET_DETECTION:
        {
            result = handle_target_detection_cmd( );
            break;
        }
        case CMD_GET_TARGET_STATUS:
        {
            result = handle_get_target_status_cmd( );
            break;
        }
        case CMD_GET_TARGET_CYPE_VERSION:
        {
            result = handle_get_version_cmd( );
            break;
        }
        case CMD_GET_PROCESSOR_LIST:
        {
            result = handle_get_processor_list_cmd( );
            break;
        }
        case CMD_LOG_REQUEST:
        {

            result = handle_request_log_cmd( );
            break;
        }
        case CMD_GET_EVENTS_LIST:
        {
            result = handle_get_events_cmd( packet );
            break;
        }
        case CMD_GET_EVENT_DESCRIPTOR_LIST:
        {
            result = handle_get_event_descriptor_cmd( packet );
            break;
        }
        case CMD_START_LOGGING:
        {
            result = handle_start_log_cmd( packet );
				
            break;
        }
        case CMD_STOP_LOGGING:
        {
            result = handle_stop_log_cmd( packet );
            break;
        }
        case CMD_LOG_POLL_PERIOD:
        {
            result = handle_log_poll_period_cmd( packet );

            break;
        }
        case CMD_STOP_PAD:
        {
            result = handle_stop_log_cmd( packet );
            cype_set_mode( CYPE_MODE_CONSOLE );
            break;
        }
        default:
            result =  CYPE_UNSUPPORTED;
    }

    /* CYPE packet to be freed here */
    cype_free_packet( packet );

	cy_rtos_set_semaphore(&cype_handle_request_lock, CYPE_FALSE);

    return result;
}

cype_result_t handle_target_detection_cmd( void )
{
    cype_result_t      result;
    cype_packet_t*       out_packet;
    uint8_t                payload_len = strlen( PLATFORM );

    /* Create the cype response packet. Add one for the length of the Platfor m name */
    result = cype_dynamic_allocate_packet( &out_packet, CYPE_PAD_RESPONSE, CMD_TARGET_DETECTION, payload_len + PACKET_COUNT_SIZE );
    if ( result != CYPE_SUCCESS )
        return result;
    memcpy( out_packet->payload_start, &payload_len, PACKET_COUNT_SIZE );
    memcpy( out_packet->payload_start + PACKET_COUNT_SIZE, PLATFORM, payload_len );
    cype_transport_send_packet( out_packet );
    return CYPE_SUCCESS;
}

cype_result_t handle_get_target_status_cmd( void )
{
    cype_result_t      result;
    cype_packet_t*       out_packet;
    uint8_t             payload_len = sizeof(uint32_t);
    uint32_t            payload;

    /* Create the cype response packet. Add one for the length of the payload*/
    result = cype_dynamic_allocate_packet( &out_packet, CYPE_PAD_RESPONSE, CMD_GET_TARGET_STATUS, payload_len + PACKET_COUNT_SIZE );
    if ( result != CYPE_SUCCESS )
        return result;

    payload = ( uint32_t )cype_logging_status( );

    memcpy( out_packet->payload_start, &payload_len, PACKET_COUNT_SIZE );
    memcpy( out_packet->payload_start + PACKET_COUNT_SIZE, &payload, payload_len );
    cype_transport_send_packet( out_packet );
    return CYPE_SUCCESS;
}
#define PSCO_TARGET_CYPE_VERSION "v1"
cype_result_t handle_get_version_cmd( void )
{
    cype_result_t      result;
    cype_packet_t*       out_packet;
    uint8_t                payload_len = strlen( PSCO_TARGET_CYPE_VERSION );

    /* Create the cype response packet. Add one for the length of the version string */
    result = cype_dynamic_allocate_packet( &out_packet, CYPE_PAD_RESPONSE, CMD_GET_TARGET_CYPE_VERSION, payload_len + PACKET_COUNT_SIZE );
    if ( result != CYPE_SUCCESS )
        return result;
    memcpy( out_packet->payload_start, &payload_len, PACKET_COUNT_SIZE );
    memcpy( out_packet->payload_start + PACKET_COUNT_SIZE, PSCO_TARGET_CYPE_VERSION, payload_len );
    cype_transport_send_packet( out_packet );
    return CYPE_SUCCESS;
}

cype_result_t handle_get_processor_list_cmd( void )
{
    cype_result_t      result;
    cype_packet_t*       out_packet;
    uint8_t            payload_len = registered_processor_count + PACKET_COUNT_SIZE; /* Added one for count */
    uint8_t             i;
    uint32_t            payload_offset=0;

    /* Create the cype response packet */
    result = cype_dynamic_allocate_packet( &out_packet, CYPE_PAD_RESPONSE, CMD_GET_PROCESSOR_LIST, payload_len );
    if ( result != CYPE_SUCCESS )
        return result;
    memcpy( out_packet->payload_start + payload_offset, &registered_processor_count, PACKET_COUNT_SIZE );
    payload_offset++;

    for ( i = 0; i < registered_cpl_count; i++ )
    {
        uint8_t j = 0;
        for ( j = 0; j < cpl_list[i]->proc_count; j++ )
        {
            memcpy( out_packet->payload_start + payload_offset, cpl_list[i]->proc_id + j, PACKET_COUNT_SIZE );
            payload_offset++;
        }
    }
    cype_transport_send_packet( out_packet );
    return CYPE_SUCCESS;
}

static cype_bool_t time_to_send_log_data(void)
{
    /* Log request can come much earlier than the registered log poll time. Serve it if it is under 10% margin */
    if ( ( platform_cype_get_time_stamp() - ( log_sent_time_offset + log_start_timestamp ) ) < (uint32_t)( cype_log_poll_time - (cype_log_poll_time/POLL_TIME_MARGIN_PERCENTAGE) ) )
    {
        CYPE_PRINT_INFO( ( "time_to_send_log_data: Early to send, refuse\n") );
        return CYPE_FALSE;
    }

    return CYPE_TRUE;
}

int host_connected_flag = 0;
cype_result_t handle_request_log_cmd( void )
{
    uint8_t i;
    uint32_t count=0;
    cype_result_t   result;
    cype_packet_t*    out_packet = NULL;
    uint32_t data_copied = 0;
    uint8_t     event_copied = 0;
    uint8_t* logstart_ptr =NULL;
    cype_time_t current_time_stamp = 0;

    if ( !cypeStarted )
        return CYPE_NOTUP;

    if ( !cype_log_poll_time )
        return CYPE_NOTUP;

    //Wait for the poll time to maintain log consistency
    if ( !time_to_send_log_data() ) {
        return CYPE_ABORTED;
    }
	if(!host_connected_flag) {
		sleep_manager_unlock_deep_sleep();
		host_connected_flag = 1;
    }
	sleep_manager_lock_deep_sleep();


    /* Create cype response packet with log size + 2 timestamps + count of packets */
    result = cype_dynamic_allocate_packet( &out_packet, CYPE_PAD_RESPONSE, CMD_LOG_REQUEST, (uint16_t)( max_power_log_size * PACKET_EACH_LOG_EVENT_SIZE + PACKET_START_TIMESTAMP_SIZE + PACKET_STOP_TIMESTAMP_SIZE + PACKET_COUNT_SIZE ) );

    if ( result != CYPE_SUCCESS )
    {
        CYPE_PRINT_DEBUG( ( "handle_request_log_cmd: Packet allocation problem\n" ) );
        return result;
    }
    CYPE_PRINT_DEBUG( ( "max_power_log_size: %lu\n", max_power_log_size ) );

    /*Copy the start timestamp */
    memcpy( out_packet->payload_start, ( uint8_t* )&log_sent_time_offset, sizeof( log_sent_time_offset ) );
    CYPE_PRINT_DEBUG( ( "StartTimestamp: %lu\n", log_sent_time_offset ) );
    logstart_ptr = out_packet->payload_start + PACKET_START_TIMESTAMP_SIZE + PACKET_STOP_TIMESTAMP_SIZE + PACKET_COUNT_SIZE;

    for ( i = 0; i < registered_cpl_count; i++ )
    {
        if ( cpl_list[i]->cpl_callback )
        {
            /* Infor m CPL to send the event log data */
            result = cpl_list[i]->cpl_callback( CPL_SEND_LOG_REQUEST, &count, logstart_ptr+data_copied, NULL );
            if ( ( result != CYPE_SUCCESS ) )
                continue;
            CYPE_PRINT_DEBUG( ( "count:%lu data_copied:%lu\n",count, data_copied ) );
            /* CPL would send the event log count pushed by it */
            if ( count )
            {
                data_copied += count*PACKET_EACH_LOG_EVENT_SIZE;
            }
        }
    }
    current_time_stamp = platform_cype_get_time_stamp( );
    log_sent_time_offset = current_time_stamp - log_start_timestamp;

    /*Copy the end timestamp */
    memcpy( out_packet->payload_start+ PACKET_STOP_TIMESTAMP_SIZE, ( uint8_t* )&log_sent_time_offset, sizeof( log_sent_time_offset ) );
    CYPE_PRINT_DEBUG( ( "StopTimestamp: %lu\n", log_sent_time_offset ) );

    /* Copy the number of events */
    event_copied = data_copied / PACKET_EACH_LOG_EVENT_SIZE;
    memcpy( out_packet->payload_start+PACKET_START_TIMESTAMP_SIZE + PACKET_STOP_TIMESTAMP_SIZE, ( uint8_t* )&event_copied, sizeof( event_copied ) );
    CYPE_PRINT_DEBUG( ( "Events: %d\n", event_copied ) );
    if ( event_copied )
        cype_transport_send_packet( out_packet );
    else
        cype_free_packet( out_packet );

	sleep_manager_unlock_deep_sleep();
    return CYPE_SUCCESS;
}

cype_result_t handle_get_events_cmd( cype_packet_t* request_packet )
{
    uint8_t i;
    cype_result_t    result = CYPE_SUCCESS;
    cype_packet_t*    out_packet = NULL;
    uint32_t count = PID;
    uint8_t proc_id = *( request_packet->payload_start );
    uint8_t* out_data;

    for ( i = 0; i < registered_cpl_count; i++ )
    {
        uint8_t j = 0;

        /* Check if this CPL manages the proc_id in request */
        for ( j = 0; j < cpl_list[i]->proc_count; j++ )
        {
            uint8_t cpl_proc = cpl_list[i]->proc_id[j];
            if ( proc_id != cpl_proc )
                continue;

            /* Get the list of events from CPL */
            result = cpl_list[i]->cpl_callback( CPL_GET_EVENTS_LIST, &count, &cpl_proc, &out_data );

            if ( result != CYPE_SUCCESS )
                return result;
            /* CPL would give the number of events and list of events */
            result = cype_dynamic_allocate_packet( &out_packet, CYPE_PAD_RESPONSE, CMD_GET_EVENTS_LIST, count + PID + EID );
            if ( result != CYPE_SUCCESS )
                return result;
            /* Copy back the PID in response */
            memcpy( out_packet->payload_start, &cpl_list[i]->proc_id[j], PID );//TBD &cpl_proc gives zero always so changed
            
            /* Copy count of events  */
            memcpy( out_packet->payload_start + PID, &count, PACKET_COUNT_SIZE );

            /* Copy the list of events  */
            memcpy( out_packet->payload_start + PID + EID, out_data, count );

            /* Send the packet to PAD. The packet will be freed by the transport */
            cype_transport_send_packet( out_packet );
            return CYPE_SUCCESS;
        }
    }
    /* No event list found, send empty response ( Processor Id and 0 count */
    result = cype_dynamic_allocate_packet( &out_packet, CYPE_PAD_RESPONSE, CMD_GET_EVENTS_LIST, PID + EID );
    if ( result != CYPE_SUCCESS )
        return result;
    memcpy( out_packet->payload_start, &proc_id, PID );
    count = 0;
    memcpy( out_packet->payload_start + PACKET_COUNT_SIZE, &count, PACKET_COUNT_SIZE );
    cype_transport_send_packet( out_packet );
    return CYPE_SUCCESS;
}

cype_result_t handle_get_event_descriptor_cmd( cype_packet_t* request_packet )
{

    uint8_t i;
    cype_result_t    result = CYPE_SUCCESS;
    cype_packet_t*    out_packet = NULL;
    uint32_t count = PID + EID;
    uint8_t proc_id = *( request_packet->payload_start );
    uint8_t* out_data;

    for ( i = 0; i < registered_cpl_count; i++ )
    {
        uint8_t j = 0;

        /* Check if this CPL manages the proc_id in request */
        for ( j = 0; j < cpl_list[i]->proc_count; j++ )
        {
            uint8_t cpl_proc = cpl_list[i]->proc_id[j];
            if ( proc_id != cpl_proc )
                continue;


            /* Get the list of events descriptors from CPL */
            result = cpl_list[i]->cpl_callback( CPL_GET_EVENTS_DESC_LIST, &count, request_packet->payload_start, &out_data );
            if ( result != CYPE_SUCCESS )
            {
                return result;
            }
            /* CPL would give the number of events descriptors and list */
            result = cype_dynamic_allocate_packet( &out_packet, CYPE_PAD_RESPONSE, CMD_GET_EVENT_DESCRIPTOR_LIST, count + PID + EID + EDID );
            if ( result != CYPE_SUCCESS )
            {
                return result;
            }
            /* Copy back the PID and Event Id in response */
            memcpy( out_packet->payload_start, request_packet->payload_start, PID + EID );
            /* Copy count of events  */
            memcpy( out_packet->payload_start + PID + EID, &count, PACKET_COUNT_SIZE );

            /* Copy the list of events  */
            memcpy( out_packet->payload_start + PID + EID + EDID, out_data, count );

            /* Send the packet to PAD. The packet will be freed by the transport */
            cype_transport_send_packet( out_packet );
            return CYPE_SUCCESS;
        }
    }
    /* No event list found, send empty response ( Processor Id and 0 count */
    result = cype_dynamic_allocate_packet( &out_packet, CYPE_PAD_RESPONSE, CMD_GET_EVENT_DESCRIPTOR_LIST, PID + EID + EDID );
    if ( result != CYPE_SUCCESS )
        return result;
    memcpy( out_packet->payload_start, request_packet->payload_start, PID + EID );
    count = 0;
    memcpy( out_packet->payload_start + PID + EID, &count, PACKET_COUNT_SIZE );
    cype_transport_send_packet( out_packet );
    return CYPE_SUCCESS;
}

cype_bool_t cpl_has_proc_id( cpl_data_t* cpl, uint8_t proc_id )
{
    uint8_t i = 0;
    for ( i = 0; i < cpl->proc_count; i++ )
    {
        /* Check the proc id in the list of CPL's proc_id */
        if ( *( cpl->proc_id+i ) == proc_id )
            return CYPE_TRUE;
    }
    return CYPE_FALSE;
}

cype_result_t handle_start_log_cmd( cype_packet_t* request_packet )
{
    uint8_t i;
    cype_result_t    result = CYPE_SUCCESS;
    cype_packet_t*    out_packet = NULL;
    uint8_t count = *( request_packet->payload_start );
	


    //Already in start state, nothing to do
    if ( log_start_timestamp )
        return CYPE_SUCCESS;

    /* Reset the timestamp if logging is not already going on*/
    log_start_timestamp = platform_cype_get_time_stamp( );
    log_sent_time_offset = 0;

    if ( count == 0 )
    {
        /* Start logging all the events of all the proc id */
        for ( i = 0; i < registered_cpl_count; i++ )
        {
            result = cpl_list[i]->cpl_callback( CPL_START_LOG, NULL, NULL, NULL );
            if ( result != CYPE_SUCCESS )
                goto startlog_error;
        }
    }
    else
    {
        uint8_t j =0;
        /* Start logging selected events of selected proc id */
        for ( j = 0; j < count; j++ )
        {
            /* Get Proc Id */
            uint8_t* pid_loc = ( request_packet->payload_start + PACKET_COUNT_SIZE + j * ( PID + EID ) );
            cype_bool_t pid_found = CYPE_FALSE;
            for ( i = 0; i < registered_cpl_count; i++ )
            {
                if ( cpl_has_proc_id( cpl_list[i], *pid_loc ) )
                {
                    /* CPL supports this proc id, send the start command with proc_id+event id location*/
                    result = cpl_list[i]->cpl_callback( CPL_START_LOG, NULL, pid_loc, NULL );
                    if ( result != CYPE_SUCCESS )
                        goto startlog_error;
                    pid_found = CYPE_TRUE;
                }
            }
            if ( !pid_found )
            {
                /* Send error if any of the sent PIDs not found */
                result = CYPE_ERROR;
                goto startlog_error;
            }
        }
    }

startlog_error:
    if ( result != CYPE_SUCCESS )
        count = PACKET_RESP_ERROR;  // Send Error
    else
        count = PACKET_RESP_SUCCESS;

    result = cype_dynamic_allocate_packet( &out_packet, CYPE_PAD_RESPONSE, CMD_START_LOGGING, PACKET_COUNT_SIZE );
    if ( result != CYPE_SUCCESS )
        return result;

    memcpy( out_packet->payload_start, &count, PACKET_COUNT_SIZE );
    cype_transport_send_packet( out_packet );

	

    return result;
}

cype_result_t handle_log_poll_period_cmd( cype_packet_t* request_packet )
{
    cype_result_t    result = CYPE_SUCCESS;
    cype_packet_t*    out_packet = NULL;
    uint8_t poll_period = *( request_packet->payload_start );
    uint8_t count;

    cype_log_poll_time = poll_period * CYPE_POLL_MULTIPLY_FACTOR;

    count = PACKET_RESP_SUCCESS;

    result = cype_dynamic_allocate_packet( &out_packet, CYPE_PAD_RESPONSE, CMD_LOG_POLL_PERIOD, PACKET_COUNT_SIZE );
    if ( result != CYPE_SUCCESS )
        return result;

    memcpy( out_packet->payload_start, &count, PACKET_COUNT_SIZE );
    cype_transport_send_packet( out_packet );

    return result;
}

cype_result_t handle_stop_log_cmd( cype_packet_t* request_packet )
{
    uint8_t i;
    cype_result_t    result = CYPE_SUCCESS;
    cype_packet_t*    out_packet = NULL;
    uint8_t count = *( request_packet->payload_start );
    CYPE_PRINT_DEBUG( ( "CYPE: STOP command\n" ) );
    if ( count == 0 )
    {
        /* Stop logging all the events of all the proc id */
        for ( i=0; i < registered_cpl_count; i++ )
            cpl_list[i]->cpl_callback( CPL_STOP_LOG, NULL, NULL, NULL );
    }
    else
    {
        uint8_t j = 0;
        /* Stop logging selected events of selected proc id */
        for ( j=0; j < count; j++ )
        {
            /* Get Proc Id */
            uint8_t* pid_loc = ( request_packet->payload_start +  PACKET_COUNT_SIZE + j * ( PID + EID ) );
             cype_bool_t pid_found = CYPE_FALSE;
            for ( i = 0; i < registered_cpl_count; i++ )
            {
                if ( cpl_has_proc_id( cpl_list[i], *pid_loc ) )
                {
                    /* CPL supports this proc id, send the STOP command with event id location*/
                    result = cpl_list[i]->cpl_callback( CPL_STOP_LOG, NULL, pid_loc, NULL );
                    if ( result != CYPE_SUCCESS )
                        goto stoplog_error;
                    pid_found = CYPE_TRUE;
                }
            }
            if ( !pid_found )
            {
                /* Send error if any of the sent PIDs not found */
                result = CYPE_ERROR;
                goto stoplog_error;
            }
        }
        /* TODO: Check with CPLs if all the logging has stopped */
    }

stoplog_error:
    if ( result != CYPE_SUCCESS )
        count = PACKET_RESP_ERROR;  // Send Error
    else
        count = PACKET_RESP_SUCCESS;

    result = cype_dynamic_allocate_packet( &out_packet, CYPE_PAD_RESPONSE, CMD_STOP_LOGGING, PACKET_COUNT_SIZE );
    if ( result != CYPE_SUCCESS )
        return result;

    memcpy( out_packet->payload_start, &count, PACKET_COUNT_SIZE );
    cype_transport_send_packet( out_packet );
    log_start_timestamp = 0;
    cype_log_poll_time = 0;
    return result;
}

void cype_set_mode( cype_mode_t mode )
{
    cypeCommunicationMode = mode;
    return;
}

cype_mode_t cype_get_mode( void )
{
    return cypeCommunicationMode ;
}

uint8_t cype_get_console_prints_status( void )
{
    return cype_console_print_status;
}

void cype_process_enable_console_prints( int argc, char* argv[] )
{
    uint8_t enable;
    enable = atoi( argv[1] );

    if ( enable )
        cype_console_print_status = 1;
    else
        cype_console_print_status = 0;
}

void console_print_version( void )
{
    CYPE_PRINT_INFO( ( "%s\n", TARGET_CYPE_VERSION ) );
    return;
}

void console_print_target( void )
{
    CYPE_PRINT_INFO( ( "%s\n", PLATFORM ) );
    return;
}

cype_result_t cype_process_console_command( int argc, char* argv[] )
{
    cype_packet_t* packet = NULL;

    cype_transport_console_handler( &packet, argc, argv );

    if ( packet )
       packets_from_cype_host_handler( packet );
    return CYPE_SUCCESS;
}

cype_result_t cype_transport_console_handler( cype_packet_t** packet, int argc, char* argv[] )
{
    uint8_t packet_byte=0;
    cype_result_t    result;
    uint8_t payload_len =  0;

    /* Get the packet type */
    packet_byte = atoi( argv[PACKET_CMD_OFFSET] );

    switch ( packet_byte )
    {
        case CMD_TARGET_DETECTION:
        case CMD_GET_TARGET_STATUS:
        case CMD_GET_TARGET_CYPE_VERSION:
        case CMD_GET_PROCESSOR_LIST:
        case CMD_LOG_REQUEST:
        {
            payload_len = 0;
            break;
        }
        case CMD_GET_EVENTS_LIST:
        {
            payload_len = PID;
            break;
        }
        case CMD_GET_EVENT_DESCRIPTOR_LIST:
        {
            payload_len = PID + EID;
            break;
        }
        case CMD_START_LOGGING:
        case CMD_STOP_LOGGING:
        {
            payload_len = argc - PACKET_COUNT_OFFSET; // Variable number of arguments
            break;
        }
        case CMD_LOG_POLL_PERIOD:
            payload_len = PACKET_COUNT_SIZE;
            break;
        default:
            return CYPE_UNSUPPORTED;
    }

    /* Check if number of arguments is proper */
    if ( ( argc - PACKET_COUNT_OFFSET ) != ( payload_len ) )
    {
        CYPE_PRINT_INFO( ( "Argument error\n" ) );
        return CYPE_ERROR;
    }

    result = cype_dynamic_allocate_packet( packet, CYPE_PAD_REQUEST, packet_byte, payload_len );
    if ( result != CYPE_SUCCESS )
        return result;

    /* Retrieve the payload data */
    if ( payload_len )
    {
        uint8_t i = 0;
        for ( i = 0; i < payload_len; i++ )
        {
            uint8_t byte = atoi( argv[i + PACKET_COUNT_OFFSET] );
            memcpy( ( *packet )->payload_start+i, &byte, 1 );
        }
    }
    return CYPE_SUCCESS;
}

cype_result_t cype_send_pad_cmd( uint8_t packet_byte )
{
    cype_result_t      result = CYPE_SUCCESS;
    uint8_t payload_len =    0;
    cype_packet_t* packet;

    if(!cype_logging_status())
        return CYPE_NOTUP;

    /* Check if it is time to send the log request */
    if ( ( packet_byte==CMD_LOG_REQUEST ) && ( !time_to_send_log_data() ) )
        return CYPE_ABORTED;

    result = cype_dynamic_allocate_packet( &packet, CYPE_PAD_REQUEST, packet_byte, payload_len );
    if ( result != CYPE_SUCCESS ) {
           return CYPE_ERROR;
      }

    result = packets_from_cype_host_handler( packet );

    return result;
}

cype_result_t cype_send_power_data_to_host( void )
{
    /* Generate LOG Request command internally */
    return cype_send_pad_cmd( CMD_LOG_REQUEST );
}

cype_bool_t cype_logging_status( void )
{
    return  cypeStarted ? CYPE_TRUE : CYPE_FALSE;
}

void cype_set_deep_sleep_state( cype_bool_t state )
{
    deep_sleep_enter = state;
    if(state == CYPE_TRUE)
        cype_transport_deinit();
}
