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

#ifdef __cplusplus
extern "C"
{
#endif

#ifndef PLATFORM_CYPE_HEADER_INCLUDED
#error "Platform CYPE header file must not be included directly, Please use cype_platform_api.h instead."
#endif

/******************************************************
 *                      Macros
 ******************************************************/

/******************************************************
 *                      CYPE-Macros
 ******************************************************/

#define PLATFORM	"CY8CKIT_062S2_43012"

#define CYPE_UART_STACK_SIZE      		( 6 * 1024 )
#define CYPE_TIME_TO_WAIT_FOR_CONSOLE    2000 /* msec */

// Inform CYPE core that mandatory Platform MACROs and data structures are defined
#define PLATFORM_CYPE_MACROS_DEFINED
#define PLATFORM_RTOS_API_DEFINED

//Serial
#define CYPE_TX	USBTX
#define CYPE_RX	USBRX
#define CYPE_UART_CIRCULAR_BUFFER_SIZE	1024
#define CYPE_UART_BAUD_RATE				115200


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
 *                 Global Variables
 ******************************************************/

/******************************************************
 *               Function Declarations
 ******************************************************/
cype_bool_t platform_is_warm_booted();

#ifdef __cplusplus
} /* extern "C" */
#endif
