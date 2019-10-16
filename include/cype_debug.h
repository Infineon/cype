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
 * All common definitions for enabling prints in CYPE
 *******************************************************************************/

#pragma once

#include <stdio.h>
#include "cype_defaults.h"

#ifdef __cplusplus
extern "C"
{
#endif

/******************************************************
 *                      Macros
 ******************************************************/

/******************************************************
 *             Print declarations
 ******************************************************/

#define CPRINT_MACRO(args) do {printf args;} while(0==1)

/* WICED printing macros for general SDK/Library functions*/
#ifdef CYPE_PRINT_ENABLE_INFO
    #define CYPE_PRINT_INFO(args) CPRINT_MACRO(args)
#else
    #define CYPE_PRINT_INFO(args)
#endif
#ifdef CYPE_PRINT_ENABLE_DEBUG
    #define CYPE_PRINT_DEBUG(args) CPRINT_MACRO(args)
#else
    #define CYPE_PRINT_DEBUG(args)
#endif
#ifdef CYPE_PRINT_ENABLE_ERROR
    #define CYPE_PRINT_ERROR(args) { CPRINT_MACRO(args);}
#else
    #define CYPE_PRINT_ERROR(args)
#endif

#ifdef __cplusplus
} /* extern "C" */
#endif
