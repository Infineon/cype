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
#include "cype_cpl.h"
#include "cype_core.h"
#include "cype_power_events.h"
#include "cype_platform_api.h"
#include <stdlib.h>
#include "cyabs_rtos.h"

/******************************************************
 *                      Macros
 ******************************************************/
#define ALL_EVENTS_LOG_ENABLED    0xFFFFFFFF

/******************************************************
 *               Variables Definitions
 ******************************************************/
cpl_data_t cpl_data;
cpl_log_buffer_t  *cpl_log;

uint8_t cpl_init_status;
uint8_t cpl_start_log;

/* CPL state machine maintainer */
static cpl_state_ctrl_t *cpl_state;

static uint32_t cpl_deep_sleep_save_data;
uint32_t cpl_last_log_time;

/******************************************************
 *               CMSIS Definitions
 ******************************************************/
//Semaphore
cy_semaphore_t cpl_update_lock;

/******************************************************
 *               Function Declarations
 ******************************************************/
cype_result_t cpl_callback_function( uint8_t cmd_id, uint32_t* len, uint8_t* in_data, uint8_t** out_data );
static uint8_t cpl_cmd_get_events_list( uint8_t proc_id, uint8_t **out_data );
static uint8_t cpl_cmd_get_event_desc_list( uint8_t proc_id, uint8_t event_id, uint8_t **out_data );
static uint8_t cpl_lut_get_proc_data( uint8_t *proc_id );
static uint8_t cpl_lut_get_events_list( uint8_t proc_id, uint8_t *out_data );
static uint8_t cpl_lut_get_event_desc_list( uint8_t proc_id, uint8_t event_id, uint8_t *out_data );
static cype_result_t cpl_cmd_log_enable( uint8_t* in_data, cype_bool_t enable );
static void cpl_log_enable_event( uint8_t proc_id, uint8_t event_id, cype_bool_t enable );
static void cpl_log_enable_all_events( cype_bool_t enable );
static cype_result_t cpl_cmd_log_request( uint8_t* in_data, uint32_t* count );
static void cpl_log_buffer_init( void );
static void cpl_state_init( void );
static void cpl_reset_timestamp( uint32_t adjustment );
static void cpl_reset_event_duration( void );
static uint8_t cpl_proc_index_from_lut( uint8_t proc_id );
static uint8_t cpl_event_index_from_lut( uint8_t proc_id, uint8_t event_id );
static void cpl_refresh_log_buffer( void );
#ifdef CYPE_PRINT_ENABLE_DEBUG
static void cpl_state_print( void );
#endif
static uint8_t  cpl_search_for_duplicate( uint8_t *data, uint32_t data_len, uint8_t event_id );
static void cpl_deep_sleep_log_state_store( uint8_t pindex, uint8_t eindex );
static void cpl_print_log_buffer( uint32_t line );
static void cpl_memory_init( void );
static uint8_t cpl_lut_get_max_proc_id( void );
static uint8_t cpl_lut_get_max_events_list( uint8_t proc_id );
static uint8_t cpl_lut_get_max_event_desc_list( uint8_t proc_id, uint8_t event_id );

/******************************************************
 *               Function Definitions
 ******************************************************/

cype_result_t cype_cpl_init( void )
{
	cy_rslt_t result = CYPE_ERROR;
	
   	result = cy_rtos_init_semaphore(&cpl_update_lock,1,1);
	if ( result != CYPE_SUCCESS )
	{
	    CYPE_PRINT_ERROR( ( "Error creating Semaphore\n" ) );
		return CYPE_ERROR;
	}

    cpl_memory_init( );

    cpl_state_init( );

    cpl_log_buffer_init( );

    cpl_data.proc_count     =  cpl_state->lut_proc_id_cnt;
    cpl_data.proc_id        = cpl_state->proc_list;
    cpl_data.cpl_callback     = cpl_callback_function;
    cpl_data.log_size         = cpl_log->log_count;

#ifdef CYPE_PRINT_ENABLE_DEBUG
    cpl_state_print( );
#endif

    cype_register_cpl( &cpl_data );

    cpl_init_status = 1;
    cpl_start_log = 1;
    return CYPE_SUCCESS;
}

static void cpl_memory_init( void )
{
    cpl_state = ( cpl_state_ctrl_t * )malloc( sizeof( cpl_state_ctrl_t ) );
    if ( cpl_state == NULL )
    {
        CYPE_PRINT_DEBUG( ( "%s: malloc failed to allocate at %d\n", __func__, __LINE__ ) );
    }
}

static void cpl_reset_timestamp( uint32_t adjustment )
{
    uint8_t pindex, eindex;
    uint8_t pentries, eentries;

    pentries =  cpl_state->max_proc_id;

    for ( pindex = 0; pindex < pentries; pindex++ )
    {
            eentries = cpl_state->proc_data[pindex].max_event_id;
            for ( eindex = 0; eindex < eentries; eindex++ )
            {
                cpl_state->proc_data[pindex].event_data[eindex].previous_time_stamp = adjustment;
            }
    }
}

static void cpl_reset_event_duration( void )
{
    uint8_t table_index;
    uint8_t table_entries = ( uint8_t ) cpl_log->log_count;

    for ( table_index = 0; table_index < table_entries; table_index++ )
    {
        cpl_log->cpl_log_packet[table_index].event_duration = 0;
    }
}

static void cpl_state_init( void )
{
    uint8_t pindex, eindex, dindex;
    uint8_t pentries, eentries, dentries;

    //Fill the cpl_state structure
    cpl_state->max_proc_id = EVENT_PROC_ID_MAX;
    cpl_state->lut_proc_id_cnt = cpl_lut_get_max_proc_id( );
    cpl_state->proc_data = ( cpl_proc_id_struct_t * )malloc( cpl_state->max_proc_id * sizeof( cpl_proc_id_struct_t ) );
    if ( cpl_state->proc_data == NULL )
    {
        CYPE_PRINT_DEBUG( ( "%s: malloc failed to allocate at %d\n", __func__, __LINE__ ) );
        return;
    }

    cpl_state->proc_list = ( uint8_t * )malloc( cpl_state->lut_proc_id_cnt );
    if ( cpl_state->proc_list == NULL )
    {
        CYPE_PRINT_DEBUG( ( "%s: malloc failed to allocate at %d\n", __func__, __LINE__ ) );
        return;
    }

    cpl_lut_get_proc_data( cpl_state->proc_list );

    CYPE_PRINT_DEBUG( ( "cpl_state->lut_proc_id_cnt:%u\n", cpl_state->lut_proc_id_cnt ) );
    pentries = cpl_state->max_proc_id;

    for ( pindex = 0; pindex < pentries; pindex++ )
    {

        cpl_state->proc_data[pindex].proc_id = pindex;
        CYPE_PRINT_DEBUG( ( "pindex, cpl_state->proc_data[%u].proc_id:0x%u\n", pindex, cpl_state->proc_data[pindex].proc_id ) );


        //Get Events list for each processor id
        cpl_state->proc_data[pindex].max_event_id = EVENT_ID_MAX;
        cpl_state->proc_data[pindex].lut_event_id_cnt = cpl_lut_get_max_events_list( cpl_state->proc_data[pindex].proc_id );
        cpl_state->proc_data[pindex].event_data = ( cpl_event_id_struct_t * )malloc( cpl_state->proc_data[pindex].max_event_id * sizeof( cpl_event_id_struct_t ) );
        if ( cpl_state->proc_data[pindex].event_data == NULL )
        {
            CYPE_PRINT_DEBUG( ( "%s: malloc failed to allocate at %d\n", __func__, __LINE__ ) );
            return;
        }

        if( !cpl_state->proc_data[pindex].lut_event_id_cnt )
        {
            cpl_state->proc_data[pindex].event_list = NULL;
        }
        else
        {
            cpl_state->proc_data[pindex].event_list = ( uint8_t * )malloc( cpl_state->proc_data[pindex].lut_event_id_cnt );
            if ( cpl_state->proc_data[pindex].event_list == NULL )
            {
                CYPE_PRINT_ERROR( ( "%s: malloc failed to allocate at %d\n", __func__, __LINE__ ) );
                return;
            }
        }

        cpl_lut_get_events_list( cpl_state->proc_data[pindex].proc_id, cpl_state->proc_data[pindex].event_list );

        CYPE_PRINT_DEBUG( ( "cpl_state->proc_data[%u].lut_event_id_cnt:%u\n", pindex, cpl_state->proc_data[pindex].lut_event_id_cnt ) );
        eentries = cpl_state->proc_data[pindex].max_event_id;

        for ( eindex = 0; eindex < eentries; eindex++ )
        {
            cpl_state->proc_data[pindex].event_data[eindex].event_id = eindex;
            CYPE_PRINT_DEBUG( ( "pindex, cpl_state->proc_data[%u].event_data[%u].event_id:0x%u\n", pindex, eindex, cpl_state->proc_data[pindex].event_data[eindex].event_id ) );


            //Get Event descriptor list for each processor id
            cpl_state->proc_data[pindex].event_data[eindex].lut_event_desc_cnt = cpl_lut_get_max_event_desc_list( cpl_state->proc_data[pindex].proc_id, cpl_state->proc_data[pindex].event_data[eindex].event_id );
            if(!cpl_state->proc_data[pindex].event_data[eindex].lut_event_desc_cnt)
            {
                CYPE_PRINT_DEBUG( ( "No descriptor found for proc %d event:%d\n", pindex, eindex) );
            }
            cpl_state->proc_data[pindex].event_data[eindex].max_event_desc = cpl_state->proc_data[pindex].event_data[eindex].lut_event_desc_cnt;
            CYPE_PRINT_DEBUG( ( "Malloc Requested size %d\n", cpl_state->proc_data[pindex].event_data[eindex].max_event_desc) );

            if( !cpl_state->proc_data[pindex].event_data[eindex].max_event_desc )
            {
                cpl_state->proc_data[pindex].event_data[eindex].event_desc_list = NULL;
            }
            else
            {
                cpl_state->proc_data[pindex].event_data[eindex].event_desc_list = ( uint8_t * )malloc( cpl_state->proc_data[pindex].event_data[eindex].max_event_desc );
                if ( cpl_state->proc_data[pindex].event_data[eindex].event_desc_list == NULL )
                {
                    CYPE_PRINT_ERROR( ( "%s: malloc failed to allocate at %d\n", __func__, __LINE__ ) );
                    return;
                }
            }

            cpl_lut_get_event_desc_list( cpl_state->proc_data[pindex].proc_id, cpl_state->proc_data[pindex].event_data[eindex].event_id, cpl_state->proc_data[pindex].event_data[eindex].event_desc_list );
            CYPE_PRINT_DEBUG( ( "cpl_state->proc_data[%u].event_data[%u].lut_event_desc_cnt:%u\n", pindex, eindex, cpl_state->proc_data[pindex].event_data[eindex].lut_event_desc_cnt ) );
            dentries = cpl_state->proc_data[pindex].event_data[eindex].max_event_desc;

            cpl_state->proc_data[pindex].event_data[eindex].previous_time_stamp = 0;
            cpl_state->proc_data[pindex].event_data[eindex].current_event_desc = 0;
            cpl_state->proc_data[pindex].event_data[eindex].log_enabled = (cype_bool_t)0;
			cpl_state->proc_data[pindex].event_data[eindex].no_refresh = 0;

            for ( dindex = 0; dindex < dentries; dindex++ )
                CYPE_PRINT_DEBUG( ( "pindex, pindex, eindex, cpl_state->proc_data[%u].event_data[%u].event_desc_list[%u]:0x%u\n", pindex, eindex, dindex, cpl_state->proc_data[pindex].event_data[eindex].event_desc_list[dindex] ) );
        }
    }
}
#ifdef CYPE_PRINT_ENABLE_DEBUG
static void cpl_state_print( void )
{
    uint8_t pindex, eindex, dindex;
    uint8_t pentries, eentries, dentries;


    CYPE_PRINT_DEBUG( ( "cpl_state->lut_proc_id_cnt:%u\n", cpl_state->lut_proc_id_cnt ) );
    pentries = cpl_state->max_proc_id;

    for ( pindex = 0; pindex < pentries; pindex++ )
    {
        CYPE_PRINT_DEBUG( ( "cpl_state->proc_list[%u]:0x%u\n", pindex, cpl_state->proc_list[pindex] ) );

        CYPE_PRINT_DEBUG( ( "cpl_state->proc_data[%u].proc_id:%u\n", pindex, cpl_state->proc_data[pindex].proc_id ) );
        CYPE_PRINT_DEBUG( ( "cpl_state->proc_data[%u].lut_event_id_cnt:%u\n", pindex, cpl_state->proc_data[pindex].lut_event_id_cnt ) );
        eentries = cpl_state->proc_data[pindex].max_event_id;

        for ( eindex = 0; eindex < eentries; eindex++ )
        {
            CYPE_PRINT_DEBUG( ( "cpl_state->proc_data[%u].event_list[%u]:0x%u\n", pindex, eindex, cpl_state->proc_data[pindex].event_list[eindex] ) );
            CYPE_PRINT_DEBUG( ( "cpl_state->proc_data[%u].event_data[%u].event_id:%u\n", pindex, eindex, cpl_state->proc_data[pindex].event_data[eindex].event_id ) );
            CYPE_PRINT_DEBUG( ( "cpl_state->proc_data[%u].event_data[%u].lut_event_desc_cnt:%u\n", pindex, eindex, cpl_state->proc_data[pindex].event_data[eindex].lut_event_desc_cnt ) );
            CYPE_PRINT_DEBUG( ( "cpl_state->proc_data[%u].event_data[%u].log_enabled:%u\n", pindex, eindex, cpl_state->proc_data[pindex].event_data[eindex].log_enabled ) );
            CYPE_PRINT_DEBUG( ( "cpl_state->proc_data[%u].event_data[%u].current_event_desc:%u\n", pindex, eindex, cpl_state->proc_data[pindex].event_data[eindex].current_event_desc ) );
            CYPE_PRINT_DEBUG( ( "cpl_state->proc_data[%u].event_data[%u].previous_time_stamp:%lu\n", pindex, eindex, cpl_state->proc_data[pindex].event_data[eindex].previous_time_stamp ) );
            dentries = cpl_state->proc_data[pindex].event_data[eindex].max_event_desc;

            for ( dindex = 0; dindex < dentries; dindex++ )
                CYPE_PRINT_DEBUG( ( "cpl_state->proc_data[%u].event_data[%u].event_desc_list[%u]:0x%u\n", pindex, eindex, dindex, cpl_state->proc_data[pindex].event_data[eindex].event_desc_list[dindex] ) );
        }
    }
}
#endif
static void cpl_log_buffer_init( void )
{
    uint8_t table_index;
    uint8_t table_entries = sizeof( cype_event_list_table )/sizeof( event_lookup_table_entry_t );

    cpl_log = ( cpl_log_buffer_t * ) malloc ( sizeof( cpl_log_buffer_t ) );
    if ( cpl_log == NULL )
    {
        CYPE_PRINT_DEBUG( ( "%s: malloc failed to allocate at %d\n", __func__, __LINE__ ) );
        return;
    }

    cpl_log->log_count = table_entries;

    cpl_log->cpl_log_packet = ( cpl_packet_t * ) malloc( cpl_log->log_count * sizeof( cpl_packet_t ) );
    if ( cpl_log->cpl_log_packet == NULL )
    {
        CYPE_PRINT_DEBUG( ( "%s: malloc failed to allocate at %d\n", __func__, __LINE__ ) );
        return;
    }

    for ( table_index = 0; table_index < table_entries; table_index++ )
    {
        cpl_log->cpl_log_packet[table_index].proc_id = cype_event_list_table[table_index].proc_id;
        cpl_log->cpl_log_packet[table_index].event_id = cype_event_list_table[table_index].event_id;
        cpl_log->cpl_log_packet[table_index].event_desc = cype_event_list_table[table_index].event_desc;
        cpl_log->cpl_log_packet[table_index].event_duration = 0;
    }
    cpl_print_log_buffer( __LINE__ );
}

static uint8_t  cpl_search_for_duplicate( uint8_t *data, uint32_t data_len, uint8_t eventid )
{
    uint8_t index;
    for ( index = 0; index < data_len; index++ )
    {
        if ( data[index] == eventid )
            return 1;
    }
    return 0;
}

static uint8_t cpl_lut_get_max_proc_id( void )
{
    uint8_t table_index;
    uint8_t table_entries = sizeof( cype_event_list_table )/sizeof( event_lookup_table_entry_t );
    uint8_t *proc_id;
    uint8_t count = 0;

    proc_id = ( uint8_t * ) malloc( table_entries );
    if ( proc_id == NULL )
    {
        CYPE_PRINT_DEBUG( ( "%s: malloc failed to allocate at %d\n", __func__, __LINE__ ) );
        return 0;
    }
    *proc_id = 0;
    for ( table_index = 0; table_index < table_entries; table_index++ )
    {
        if ( !cpl_search_for_duplicate( proc_id, count, cype_event_list_table[table_index].proc_id ) )
        {
            proc_id[count] = cype_event_list_table[table_index].proc_id;
            count++;
        }
    }
    free( proc_id );
    return count;
}

static uint8_t cpl_lut_get_proc_data( uint8_t *proc_id )
{
    uint8_t table_index;
    uint8_t table_entries = sizeof( cype_event_list_table )/sizeof( event_lookup_table_entry_t );
    uint8_t count = 0;

    for ( table_index = 0; table_index < table_entries; table_index++ )
    {
        if ( !cpl_search_for_duplicate( proc_id, count, cype_event_list_table[table_index].proc_id ) )
        {
            proc_id[count] = cype_event_list_table[table_index].proc_id;
            count++;
        }
    }
    return count;
}

static uint8_t cpl_proc_index_from_lut( uint8_t proc_id )
{
    uint8_t pindex;
    uint8_t pentries;

    pentries = cpl_state->max_proc_id;

    for ( pindex = 0; pindex < pentries; pindex++ )
    {
        if ( proc_id == cpl_state->proc_data[pindex].proc_id )
        {
            return pindex;
        }
    }
    return 0xFF;
}

static uint8_t cpl_lut_get_max_events_list( uint8_t proc_id )
{
    uint8_t table_index;
    uint8_t table_entries = sizeof( cype_event_list_table )/sizeof( event_lookup_table_entry_t );
    uint8_t *event_id;
    uint8_t count = 0;

    event_id = ( uint8_t * ) malloc( table_entries );
    if ( event_id == NULL )
    {
        CYPE_PRINT_DEBUG( ( "%s: malloc failed to allocate at %d\n", __func__, __LINE__ ) );
        return 0;
    }
    *event_id = 0;
    for ( table_index = 0; table_index < table_entries; table_index++ )
    {
        if ( cype_event_list_table[table_index].proc_id == proc_id )
        {
            if ( !cpl_search_for_duplicate( event_id, count, cype_event_list_table[table_index].event_id ) )
            {
                event_id[count] = cype_event_list_table[table_index].event_id;
                count++;
            }
        }
    }
    free( event_id );
    return count;
}

static uint8_t cpl_lut_get_events_list( uint8_t proc_id, uint8_t *out_data )
{
    uint8_t table_index;
    uint8_t table_entries = sizeof( cype_event_list_table )/sizeof( event_lookup_table_entry_t );
    uint8_t count = 0;

    for ( table_index = 0; table_index < table_entries; table_index++ )
    {
        if ( cype_event_list_table[table_index].proc_id == proc_id )
        {
            if ( !cpl_search_for_duplicate( out_data, count, cype_event_list_table[table_index].event_id ) )
            {
                out_data[count] = cype_event_list_table[table_index].event_id;
                count++;
            }
        }
    }
    return count;
}

static uint8_t cpl_cmd_get_events_list( uint8_t proc_id, uint8_t **out_data )
{
    uint8_t pindex = cpl_proc_index_from_lut( proc_id );

    *out_data = cpl_state->proc_data[pindex].event_list;

    return cpl_state->proc_data[pindex].lut_event_id_cnt;

}

static uint8_t cpl_event_index_from_lut( uint8_t proc_id, uint8_t event_id )
{
    uint8_t pindex, eindex;
    uint8_t pentries, eentries;

    pentries =  cpl_state->max_proc_id;

    for ( pindex = 0; pindex < pentries; pindex++ )
    {
        if ( proc_id == cpl_state->proc_data[pindex].proc_id )
        {
             eentries = cpl_state->proc_data[pindex].max_event_id;
             for ( eindex = 0; eindex < eentries; eindex++ )
             {
                 if ( event_id == cpl_state->proc_data[pindex].event_data[eindex].event_id )
                        return eindex;
             }
        }
    }
    return 0xFF;
}

static uint8_t cpl_lut_get_max_event_desc_list( uint8_t proc_id, uint8_t event_id )
{
    uint8_t table_index;
    uint8_t table_entries = sizeof( cype_event_list_table )/sizeof( event_lookup_table_entry_t );
    uint8_t *event_dec_list;
    uint8_t count = 0;

    event_dec_list = ( uint8_t * ) malloc( table_entries );
    if ( event_dec_list == NULL )
    {
        CYPE_PRINT_DEBUG( ( "%s: malloc failed to allocate at %d\n", __func__, __LINE__ ) );
        return 0;
    }

    for ( table_index = 0; table_index < table_entries; table_index++ )
    {
        if ( cype_event_list_table[table_index].proc_id == proc_id && cype_event_list_table[table_index].event_id == event_id )
        {
            event_dec_list[count] = cype_event_list_table[table_index].event_desc;
            count++;
        }
    }
    free( event_dec_list );
    return count;
}

static uint8_t cpl_lut_get_event_desc_list( uint8_t proc_id, uint8_t event_id, uint8_t *out_data )
{
    uint8_t table_index;
    uint8_t table_entries = sizeof( cype_event_list_table )/sizeof( event_lookup_table_entry_t );
    uint8_t count = 0;

    for ( table_index = 0; table_index < table_entries; table_index++ )
    {
        if ( cype_event_list_table[table_index].proc_id == proc_id && cype_event_list_table[table_index].event_id == event_id )
        {
            out_data[count] = cype_event_list_table[table_index].event_desc;
            count++;
        }
    }
    return count;
}

static uint8_t cpl_cmd_get_event_desc_list( uint8_t proc_id, uint8_t event_id, uint8_t **out_data )
{
    uint8_t pindex, eindex;

    pindex = cpl_proc_index_from_lut( proc_id );
    eindex = cpl_event_index_from_lut( proc_id, event_id );

    *out_data = cpl_state->proc_data[pindex].event_data[eindex].event_desc_list;

    return cpl_state->proc_data[pindex].event_data[eindex].lut_event_desc_cnt;
}

static void cpl_log_enable_all_events( cype_bool_t enable )
{
    uint8_t pindex, eindex;
    uint8_t pentries, eentries;

    pentries = cpl_state->max_proc_id;

    for ( pindex = 0; pindex < pentries; pindex++ )
    {
            eentries = cpl_state->proc_data[pindex].max_event_id;
            for ( eindex = 0; eindex < eentries; eindex++ )
            {
                cpl_state->proc_data[pindex].event_data[eindex].log_enabled = enable;
            }

    }

    cpl_deep_sleep_save_data = ALL_EVENTS_LOG_ENABLED;
}

static void cpl_log_enable_event( uint8_t proc_id, uint8_t event_id, cype_bool_t enable )
{
    uint8_t pindex, eindex;
    uint8_t pentries, eentries;

    pentries =  cpl_state->max_proc_id;

    for ( pindex = 0; pindex < pentries; pindex++ )
    {
        if ( proc_id == cpl_state->proc_data[pindex].proc_id )
        {
             eentries = cpl_state->proc_data[pindex].max_event_id;
             for ( eindex = 0; eindex < eentries; eindex++ )
             {
                 if ( event_id == cpl_state->proc_data[pindex].event_data[eindex].event_id )
                 {
                     cpl_state->proc_data[pindex].event_data[eindex].log_enabled = enable;
                     cpl_deep_sleep_log_state_store( pindex, eindex );
                 }
             }
        }
    }
}

static cype_result_t cpl_cmd_log_enable( uint8_t* in_data, cype_bool_t enable )
{
    uint8_t pindex, pentries;

    cpl_reset_timestamp( platform_cype_get_time_stamp( ) );
    platform_cype_enable_logging( enable );

    pentries = cpl_state->max_proc_id;
    for ( pindex = 0; pindex < pentries; pindex++ )
    {
        platform_cype_reset_power_data((cpl_procid_t)cpl_state->proc_data[pindex].proc_id);
    }

    if ( in_data == NULL )
        cpl_log_enable_all_events( enable );
    else
        cpl_log_enable_event( in_data[0], in_data[1], enable );
    return CYPE_SUCCESS;
}

static cype_result_t cpl_cmd_log_request( uint8_t* in_data, uint32_t* count )
{
    uint8_t pindex, pentries;

    if( !cpl_start_log ) {
        *count = 0;
        return CYPE_ERROR;
    }

    cpl_refresh_log_buffer( );

    pentries = cpl_state->max_proc_id;
    for ( pindex = 0; pindex < pentries; pindex++ )
    {
        platform_cype_update_power_data((cpl_procid_t)cpl_state->proc_data[pindex].proc_id);
    }

    cpl_print_log_buffer( __LINE__ );

    memcpy( in_data, cpl_log->cpl_log_packet, cpl_log->log_count * sizeof( cpl_packet_t ) );

    cpl_reset_event_duration( );

    /* Let CYPE handler know whether you have sent some logs */
    *count = cpl_log->log_count;

    return CYPE_SUCCESS;
}

cype_result_t cpl_callback_function( uint8_t cmd_id, uint32_t* len, uint8_t* in_data, uint8_t** out_data )
{
    cype_result_t result = CYPE_ERROR;
    uint8_t *cmd_data = ( uint8_t * ) in_data;

    switch( cmd_id )
    {
        case CPL_GET_EVENTS_LIST:
            *len = cpl_cmd_get_events_list( cmd_data[0], out_data );
            if( *len )
                result = CYPE_SUCCESS;
            else
                result = CYPE_ERROR;
            break;
        case CPL_GET_EVENTS_DESC_LIST:
           *len = cpl_cmd_get_event_desc_list( cmd_data[0], cmd_data[1], out_data );

            if( *len )
                result = CYPE_SUCCESS;
            else
                result = CYPE_ERROR;
            break;
        case CPL_START_LOG:
            result = cpl_cmd_log_enable( in_data, CYPE_TRUE );
            break;
        case CPL_STOP_LOG:
            result = cpl_cmd_log_enable( in_data, CYPE_FALSE );
            break;
        case CPL_SEND_LOG_REQUEST:
            result = cpl_cmd_log_request( in_data, len );
            break;

        default:
            result = CYPE_ERROR;
            break;
    }

    return result;
}

void cpl_set_powerstate( uint8_t proc_id, uint8_t event_id, uint8_t event_state )
{
    uint8_t pindex, eindex;
    uint8_t pentries, eentries;

    pentries = cpl_state->max_proc_id;

    for ( pindex = 0; pindex < pentries; pindex++ )
    {
        if ( proc_id == cpl_state->proc_data[pindex].proc_id )
        {
            eentries = cpl_state->proc_data[pindex].max_event_id;
            for ( eindex = 0; eindex < eentries; eindex++ )
            {

                if ( event_id == cpl_state->proc_data[pindex].event_data[eindex].event_id )
                {
                    cpl_state->proc_data[pindex].event_data[eindex].current_event_desc = event_state;
                    break;
                 }
            }
            break;
        }
    }
}

void cpl_log_update( uint8_t proc_id, uint8_t event_id, uint8_t event_state, uint32_t event_data )
{
    uint8_t table_index;
    uint8_t table_entries = sizeof( cype_event_list_table )/sizeof( event_lookup_table_entry_t );

    for ( table_index = 0; table_index < table_entries; table_index++ )
    {

        if ( ( cype_event_list_table[table_index].proc_id == proc_id ) &&
                ( cype_event_list_table[table_index].event_id == event_id ) &&
                    ( cype_event_list_table[table_index].event_desc == event_state ) )
        {
			cy_rtos_get_semaphore(&cpl_update_lock, CY_RTOS_NEVER_TIMEOUT, CYPE_FALSE);
            cpl_log->cpl_log_packet[table_index].event_duration    += event_data;
			cy_rtos_set_semaphore(&cpl_update_lock, CYPE_FALSE);
        }
    }
}

void cpl_log_reset_event( uint8_t proc_id, uint8_t event_id, uint8_t event_state )
{
    uint8_t table_index;
    uint8_t table_entries = sizeof( cype_event_list_table )/sizeof( event_lookup_table_entry_t );

    for ( table_index = 0; table_index < table_entries; table_index++ )
    {

        if ( ( cype_event_list_table[table_index].proc_id == proc_id ) &&
                ( cype_event_list_table[table_index].event_id == event_id ) &&
                    ( cype_event_list_table[table_index].event_desc == event_state ) )
        {
			cy_rtos_get_semaphore(&cpl_update_lock, CY_RTOS_NEVER_TIMEOUT, CYPE_FALSE);
            cpl_log->cpl_log_packet[table_index].event_duration    = 0;
            cy_rtos_set_semaphore(&cpl_update_lock, CYPE_FALSE);
        }
    }
}

void cpl_log_reset_event_data( uint8_t proc_id, uint8_t event_id, uint8_t event_state, uint32_t data )
{
    uint8_t table_index;
    uint8_t table_entries = sizeof( cype_event_list_table )/sizeof( event_lookup_table_entry_t );

    for ( table_index = 0; table_index < table_entries; table_index++ )
    {

        if ( ( cype_event_list_table[table_index].proc_id == proc_id ) &&
                ( cype_event_list_table[table_index].event_id == event_id ) &&
                    ( cype_event_list_table[table_index].event_desc == event_state ) )
        {
		    cy_rtos_get_semaphore(&cpl_update_lock, CY_RTOS_NEVER_TIMEOUT, CYPE_FALSE);
			cpl_log->cpl_log_packet[table_index].event_duration    = data;
            cpl_state->proc_data[proc_id].event_data[event_id].no_refresh = 1;
		    cy_rtos_set_semaphore(&cpl_update_lock, CYPE_FALSE);
        }
    }
}

static void cpl_refresh_log_buffer( void )
{
    uint8_t pindex, eindex;
    uint8_t pentries, eentries;

    pentries = cpl_state->max_proc_id;

    for ( pindex = 0; pindex < pentries; pindex++ )
    {
        eentries = cpl_state->proc_data[pindex].max_event_id;
        for ( eindex = 0; eindex < eentries; eindex++ )
        {
            cpl_last_log_time = platform_cype_get_time_stamp( );

            if( cpl_state->proc_data[pindex].event_data[eindex].log_enabled  && (cpl_state->proc_data[pindex].event_data[eindex].no_refresh == 0))
                cpl_log_update( cpl_state->proc_data[pindex].proc_id, cpl_state->proc_data[pindex].event_data[eindex].event_id, cpl_state->proc_data[pindex].event_data[eindex].current_event_desc, cpl_last_log_time - cpl_state->proc_data[pindex].event_data[eindex].previous_time_stamp );
            cpl_state->proc_data[pindex].event_data[eindex].previous_time_stamp = cpl_last_log_time;
         }
    }
}

void cpl_event_state_update( uint8_t proc_id, uint8_t event_id, uint8_t event_state )
{
    if( cpl_init_status )
    {
        uint32_t time_stamp;
        time_stamp = platform_cype_get_time_stamp( );

        if ( cpl_state->proc_data[proc_id].event_data[event_id].log_enabled )
            cpl_log_update( proc_id, event_id, cpl_state->proc_data[proc_id].event_data[event_id].current_event_desc, time_stamp - cpl_state->proc_data[proc_id].event_data[event_id].previous_time_stamp );

        cpl_state->proc_data[proc_id].event_data[event_id].current_event_desc = event_state;

        cpl_state->proc_data[proc_id].event_data[event_id].previous_time_stamp = time_stamp;
    }
}

static void cpl_print_log_buffer( uint32_t line )
{
    uint32_t pkt_cnt;
    UNUSED_PARAMETER( line );
    CYPE_PRINT_DEBUG( ( "Line: %lu : Total Packets in Buffer : %lu\n", line, cpl_log->log_count ) );
    for( pkt_cnt = 0; pkt_cnt < cpl_log->log_count;pkt_cnt++ )
        CYPE_PRINT_DEBUG( ( "0x%u-0x%u-0x%u-0x%lx\n", cpl_log->cpl_log_packet[pkt_cnt].proc_id, cpl_log->cpl_log_packet[pkt_cnt].event_id, cpl_log->cpl_log_packet[pkt_cnt].event_desc, cpl_log->cpl_log_packet[pkt_cnt].event_duration ) );

    CYPE_PRINT_DEBUG( ( "\n" ) );

}

static void cpl_deep_sleep_log_state_store( uint8_t pindex, uint8_t eindex )
{
    cpl_deep_sleep_save_data ^= ( 1UL << ( eindex  + ( pindex * 4 ) ) );
}
