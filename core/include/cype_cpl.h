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

/*******************************************************************************
 * All common definitions for Platform specific CPL
 *******************************************************************************/

#pragma once

#include "cype.h"
#include "cy_power_estimator.h"

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************
 *                      Macros
 ******************************************************/
/******************************************************
 *                 Type Definitions
 ******************************************************/
/******************************************************
 *                    Structures
 ******************************************************/
typedef struct
{
    uint8_t proc_id;
    uint8_t event_id;
    uint8_t event_desc;
} event_lookup_table_entry_t;

typedef struct
{
    uint8_t event_id;
    uint8_t *event_desc_list;
    uint8_t lut_event_desc_cnt;
    uint8_t max_event_desc;
    cype_bool_t log_enabled;
    uint8_t current_event_desc;
	uint8_t no_refresh;
    uint32_t previous_time_stamp;
}cpl_event_id_struct_t;

typedef struct
{
    uint8_t proc_id;
    cpl_event_id_struct_t *event_data;
    uint8_t *event_list;
    uint8_t lut_event_id_cnt;
    uint8_t max_event_id;
}cpl_proc_id_struct_t;

typedef struct
{
    cpl_proc_id_struct_t *proc_data;
    uint8_t *proc_list;
    uint8_t lut_proc_id_cnt;
    uint8_t max_proc_id;
}cpl_state_ctrl_t;

//8.4.4    Power Log Event Format (Architecture Document)
//Processor ID    Event ID    Event Descriptor    Time Stamp/Power Data    Extended Data
//8 Bits                  8 Bits    8 Bits                  32 Bits                         Byte0(Length, N Bytes), Byte1, Byte2, …., ByteN
#pragma pack(1)
typedef struct
{
    uint8_t proc_id;
    uint8_t event_id;
    uint8_t event_desc;
    uint32_t event_duration;
}cpl_packet_t;

typedef struct
{
    uint32_t log_count;
    cpl_packet_t *cpl_log_packet;
}cpl_log_buffer_t;
#pragma pack()

/******************************************************
 *               Function Declarations
 ******************************************************/
cype_result_t cype_cpl_init( void );
#ifndef CY_POWER_ESTIMATOR_ENABLE
cype_result_t cype_cpl_init( void )
{
}
#endif

//For individual event updates, where time duration is not considered, used in case of Wi-Fi power data
void cpl_log_update( uint8_t proc_id, uint8_t event_id, uint8_t event_state, uint32_t event_data );
void cpl_log_reset_event( uint8_t proc_id, uint8_t event_id, uint8_t event_state );

void cpl_set_powerstate( uint8_t proc_id, uint8_t event_id, uint8_t event_state );
#ifdef __cplusplus
extern "C" }
#endif
