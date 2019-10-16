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


/******************************************************
 *               Variable Definitions
 ******************************************************/
static cype_transport_t transport_type;

/******************************************************
 *               Function Definitions
 ******************************************************/

cype_result_t cype_transport_init( cype_transport_t transport,  cype_transport_received_packet_handler_t handler )
{
    cype_result_t    result = CYPE_ERROR;
    if ( transport == UART_TRANSPORT )
    {
       result = cype_uart_transport_init( handler );
    }
    transport_type = transport;
    return result;
}

cype_result_t cype_transport_deinit( void )
{
    cype_result_t    result = CYPE_ERROR;
    if(transport_type == UART_TRANSPORT)
    {
       result = cype_uart_transport_deinit();
    }
    return result;

}

cype_result_t cype_transport_send_packet( cype_packet_t* packet )
{
    cype_result_t    result = CYPE_ERROR;
    if ( transport_type == UART_TRANSPORT )
    {
       result = cype_uart_transport_send_packet( packet );
    }
    return result;
}
