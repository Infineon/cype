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
#include "cype.h"

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
    CYPE_PAD_REQUEST,
    CYPE_PAD_RESPONSE
}cype_packet_group_t;

/******************************************************
 *                 Type Definitions
 ******************************************************/
typedef struct cype_packet      cype_packet_t;

/******************************************************
 *                    Structures
 ******************************************************/
#pragma pack( 1 )
struct cype_packet
{
    uint8_t             packet_group;
    uint8_t             packet_id;
    uint32_t            packet_size;
    uint8_t*            payload_start;
    uint8_t             packet_start[1];
};
#pragma pack()

/******************************************************
 *               Function Declarations
 ******************************************************/
cype_result_t cype_dynamic_allocate_packet( cype_packet_t** packet, uint8_t pkt_group, uint8_t pkt_id, uint16_t data_size );

cype_result_t cype_free_packet( cype_packet_t* packet );

#ifdef __cplusplus
} /* extern "C" */
#endif
