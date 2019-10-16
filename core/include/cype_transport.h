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

#include "cype_packet.h"

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
typedef enum
{
    UART_TRANSPORT = 0x01,
}cype_transport_t;

/******************************************************
 *                 Type Definitions
 ******************************************************/
typedef cype_result_t ( *cype_transport_received_packet_handler_t )( cype_packet_t* packet );

/******************************************************
 *                    Structures
 ******************************************************/

/******************************************************
 *                 Global Variables
 ******************************************************/

/******************************************************
 *               Function Declarations
 ******************************************************/

cype_result_t cype_transport_init( cype_transport_t transport_type,  cype_transport_received_packet_handler_t handler );
cype_result_t cype_transport_deinit( void );
cype_result_t cype_transport_send_packet( cype_packet_t* packet );

#ifdef __cplusplus
} /* extern "C" */
#endif
