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

#pragma once

#include <stdint.h>
#include <string.h>
#include "cype.h"
#include "cype_packet.h"

#ifdef __cplusplus
extern "C"
{
#endif

/******************************************************
 *                      Macros
 ******************************************************/

/******************************************************
 *                      CYPE-Macros
 ******************************************************/
#define CYPE_SYN_DATA                                   0x1616
#define CYPE_SYN_DATA_SIZE                              2

#define CYPE_SYN_BYTE1                                  0x16
#define CYPE_SYN_BYTE2                                  0x16

#define POWER_LOGGER_HEADER_SIZE                       3

#define TARGET_CYPE_VERSION                             "v1"

#define CYPE_POLL_MULTIPLY_FACTOR                       10


//PAD-CYPE Commands
typedef enum {
    CMD_LOG_POLL_PERIOD,
    CMD_TARGET_DETECTION,
    CMD_GET_TARGET_CYPE_VERSION,
    CMD_GET_PROCESSOR_LIST,
    CMD_GET_EVENTS_LIST,
    CMD_GET_EVENT_DESCRIPTOR_LIST,
    CMD_START_LOGGING,
    CMD_STOP_LOGGING,
    CMD_GET_TARGET_STATUS,
    CMD_STOP_PAD = 'L',
    CMD_LOG_REQUEST = 0x56,
} pad_commands_t;


//CPL callback commands
typedef enum {
    CPL_GET_EVENTS_LIST,
    CPL_GET_EVENTS_DESC_LIST,
    CPL_START_LOG,
    CPL_STOP_LOG,
    CPL_SEND_LOG_REQUEST,
} cpl_callback_commands_t;

/******************************************************
 *                    Constants
 ******************************************************/

/******************************************************
 *                   Enumerations
 ******************************************************/

/******************************************************
 *                 Type Definitions
 ******************************************************/
typedef cype_result_t ( *cpl_callback_handler_t )( uint8_t cmd_id, uint32_t* len, uint8_t* in_data, uint8_t** out_data );
typedef uint32_t ( *cpl_get_timestamp_handler_t )( void );

/******************************************************
 *                    Structures
 ******************************************************/
typedef struct
{
        uint8_t proc_count;                     /* Number of processor ids */
        uint8_t* proc_id;                       /* List of processor ids */
        uint32_t log_size;                      /* Size of log data */
        cpl_callback_handler_t cpl_callback;
}cpl_data_t;

/******************************************************
 *                 Global Variables
 ******************************************************/

/******************************************************
 *               Function Declarations
 ******************************************************/

cype_result_t cype_register_cpl( cpl_data_t* cpl );
cype_result_t cype_init( void );
cype_result_t packets_from_cype_host_handler( cype_packet_t* packet );
cype_result_t cype_send_pad_cmd( uint8_t packet_byte );
uint8_t cype_get_console_prints_status(void);
#ifdef __cplusplus
} /* extern "C" */
#endif
