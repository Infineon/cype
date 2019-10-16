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
#include "stdint.h"
#ifdef __cplusplus
extern "C" {
#endif

#if !defined(__ICCARM__) && !defined(__ARMCC_VERSION)
/*! \brief Signed 8-bit value. */
typedef signed char int8_t;
/*! \brief Unsigned 8-bit value. */
typedef unsigned char uint8_t;
/*! \brief Signed 16-bit value. */
typedef signed short int16_t;
/*! \brief Unsigned 16-bit value. */
typedef unsigned short uint16_t;

/*! \brief Signed 32-bit value. */
typedef signed long int32_t;
/*! \brief Unsigned 32-bit value. */
typedef unsigned long uint32_t;

/*! \brief Unsigned 64-bit value. */
typedef unsigned long long uint64_t;
#endif

/**
 * Boolean values
 */
typedef enum
{
    CYPE_FALSE = 0,
    CYPE_TRUE  = 1
} cype_bool_t;

typedef uint32_t  cype_time_t;        /**< Time value in milliseconds */
typedef uint32_t  cype_utc_time_t;    /**< UTC Time in seconds        */
typedef uint64_t  cype_utc_time_ms_t; /**< UTC Time in milliseconds   */

#ifdef __cplusplus
} /* extern "C" */
#endif

