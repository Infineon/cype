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
#include "cy_power_estimator.h"

#ifdef __cplusplus
extern "C"
{
#endif

/* platform_cype.h is part of CYPE platform implementation and expected to define CYPE Macros listed below */
#define PLATFORM_CYPE_HEADER_INCLUDED
#include "platform_cype.h"

/******************************************************
 *                      Macros
 ******************************************************/
/* platform_cype.h must set this MACRO */
#ifndef PLATFORM_CYPE_MACROS_DEFINED
#error "platform_cype.h (CYPE Platform implementation) header file must define mandatory CYPE MACROS "
#endif

/* Following are the mandatory MACROs that need to be defined in platform CYPE header file */
//#define CYPE_UART_STACK_SIZE
//#define CYPE_TIME_TO_WAIT_FOR_CONSOLE

/******************************************************
 *                    Constants
 ******************************************************/

/******************************************************
 *                   Enumerations
 ******************************************************/
/**
 * @brief CYPE Mode: CYPE Core receives commands either in Console Mode or Direct Mode (No Console). The Console Mode is enabled
 * when application is using Wiced Command Console and CYPE commands are sent to CYPE Core through the Command Console
 * framework. In Direct mode, CYPE Host directly communicates with CYPE core using CYPE Protocol
 *
 */
typedef enum {
    CYPE_MODE_CONSOLE, /**< CYPE Console Mode */
    CYPE_MODE_NO_CONSOLE, /**< CYPE Direct Mode (No Console) */
} cype_mode_t;

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

/*****************************************************************************/

/**
 * CYPE's platform module implements this API. CYPE Core calls it to initialize CYPE platform implementation.
 *
 * @param           none
 * @return          WICED_SUCCESS : on success.
 * @return          WICED_ERROR   : if an error occurred with any step
 */
cype_result_t platform_cype_init(void);

/**
 * CYPE's platform module implements this API. CYPE Core calls it to enable/disable CYPE logging for all the components
 * supported by platform
 *
 * @param[in]  enable      : If true, enables CYPE logging in platform else disables
 * @return     none
 */
void platform_cype_enable_logging( cype_bool_t enable );

/**
 * CYPE's platform module implements this API. CYPE Core calls it to reset power data stored for a CYPE processor component
 *
 * @param[in]  processor_id      : CYPE processor component like EVENT_PROC_ID_BT
 * @return     none
 */
void platform_cype_reset_power_data(cpl_procid_t processor_id);

/**
 * CYPE's platform module implements this API. CYPE Core calls it to update power data for a CYPE processor component.
 * CYPE platform implementation retrieves the power data from components and logs against relevant power state using
 * CY_POWER_ESTIMATOR_DATA()
 *
 * @param[in]  processor_id      : CYPE processor component like EVENT_PROC_ID_BT
 * @return     none
 */
void platform_cype_update_power_data(cpl_procid_t processor_id);

/**
 * CYPE's platform module implements this API. CYPE components call it to get the current timestamp in milliseconds
 *
 * @param  none
 * @return The current timestamp in milliseconds
 */
uint32_t platform_cype_get_time_stamp( void );

/**
 * CYPE Core implements this API. CYPE Platform implementation calls this API to push power data to CYPE host before going
 * into sleep.
 *
 * @param           none
 * @return          WICED_SUCCESS : on success.
 * @return          WICED_ERROR   : if an error occurred with any step
 */
cype_result_t cype_send_power_data_to_host( void );

/**
 * CYPE Core implements this API to return the current CYPE logging status
 *
 * @param           none
 * @return          WICED_TRUE : CYPE logging is enabled
 * @return          WICED_FALSE: CYPE logging is disabled
 */
cype_bool_t cype_logging_status( void );

/**
 * CYPE core implements this API to initiate deep-sleep state on its data structures. CYPE Platform
 * implementation calls this when platform triggers deep sleep
 *
 * @param[in]  state      : If true, enables deep sleep state on CYPE.
 * @return     none
 */
void cype_set_deep_sleep_state( cype_bool_t state );

/**
 * CYPE core's Wi-Fi module implements this API to reset Wi-Fi's power data. If CYPE platform implementation supports
 * Wi-Fi power logging, will call this API when platform CYPE power reset is called
 *
 * @param      none
 * @return     none
 */
void cype_wifi_reset_power_data(void);

/**
 * CYPE core's Wi-Fi module implements this API to init Wi-Fi and gets the handlers
 *
 * @param      none
 * @return     none
 */
void cype_wifi_init(void);

/**
 * CYPE core's Wi-Fi module implements this API to trigger update in Wi-Fi's power data. CYPE uses WWD's API to retrieve
 * power data. If CYPE platform implementation supports Wi-Fi power logging, will call this API when platform CYPE power
 * update is called
 *
 * @param      none
 * @return     none
 */
void cype_wifi_update_power_data(void);

/* CYPE APIs to support CYPE console commands */

/**
 * Sets the command mode of CYPE to Console Mode or Direct Mode
 *
 * @param           mode: CYPE_MODE_CONSOLE or CYPE_MODE_NO_CONSOLE
 * @return          NO return value.
 */
void cype_set_mode(cype_mode_t mode);

/**
 * Gets the command mode of CYPE
 *
 * @param           none
 * @return          CYPE_MODE_CONSOLE or CYPE_MODE_NO_CONSOLE
 */
cype_mode_t cype_get_mode(void);

/**
 * Process the command received on Console mode using command string "cype_cmd <parameters>"
 *
 * @param           argc: Number of arguments in console command string
 * @param           argv: console command arguments
 * @return          WICED_SUCCESS or WICED_ERROR
 */
cype_result_t cype_process_console_command( int argc, char* argv[] );

/**
 * Process the command received on Console mode using command string "cype_debug <parameters>" to enable/disable
 * CYPE prints on console
 *
 * @param           argc: Number of arguments in console command string
 * @param           argv: console command arguments
 * @return          none
 */
void cype_process_enable_console_prints( int argc, char* argv[] );

#ifdef __cplusplus
} /* extern "C" */
#endif

