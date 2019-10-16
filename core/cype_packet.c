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
#include "cype_platform_api.h"
#include <stdlib.h>
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

/******************************************************
 *               Function Definitions
 ******************************************************/
cype_result_t cype_dynamic_allocate_packet( cype_packet_t** packet, uint8_t pkt_group, uint8_t pkt_id, uint16_t data_size )
{
    uint16_t syn_byte = CYPE_SYN_DATA;
    uint16_t header_size = 0;

    if ( packet == NULL )
     {
         return CYPE_BADARG;
     }

    if(pkt_group == CYPE_PAD_RESPONSE)
    {
        header_size = POWER_LOGGER_HEADER_SIZE + 2; // Add 2 for sending payload size after SYN bytes
    }

    *packet = ( cype_packet_t* )malloc( header_size + data_size + sizeof(cype_packet_t) - 1 );
    if ( *packet == NULL )
     {
         return CYPE_OUT_OF_HEAP_SPACE;
     }

    ( *packet )->packet_group        = pkt_group;
    ( *packet )->packet_id           = pkt_id;
    ( *packet )->packet_size         = header_size + data_size;
    ( *packet )->payload_start       = NULL;
     if(data_size)
         ( *packet )->payload_start   = ( *packet )->packet_start + header_size;

     /* Copy Header data (SYN + Id) into the packet */
     if(pkt_group == CYPE_PAD_RESPONSE)
     {
         uint16_t size = data_size + 1; // Add 1 for the packet id
         // Copy SYN Bytes
         memcpy( ( *packet )->packet_start, ( uint8_t* )&syn_byte, CYPE_SYN_DATA_SIZE );
         // Copy size of the packet data
         memcpy( ( *packet )->packet_start + CYPE_SYN_DATA_SIZE, ( uint16_t* )&size, 2 );
         // Copy packet id
         memcpy( ( *packet )->packet_start + CYPE_SYN_DATA_SIZE + 2, ( uint8_t* )&pkt_id, 1 );
     }
     return CYPE_SUCCESS;
}

cype_result_t cype_free_packet( cype_packet_t* packet )
{
    if ( packet == NULL )
     {
         return CYPE_BADARG;
     }
    free( packet );
    return CYPE_SUCCESS;
}
