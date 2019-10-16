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

#include "cype_platform_api.h"
#include "us_ticker_api.h"
#include "cy_syspm.h"
#include "cy_scb_uart.h"
#include "platform_cype_ble.h"
#include "cy_power_estimator.h"
#include "cype_mcu_power.h"
#include "cyabs_rtos.h"



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
 *               Function Declarations
 ******************************************************/

/******************************************************
 *               Variables Definitions
 ******************************************************/

/******************************************************
 *               Function Definitions
 ******************************************************/
extern cype_result_t cype_send_power_data_to_host( void );

uint32 deep_sleep_count;

cy_en_syspm_status_t cype_pm_callback(cy_stc_syspm_callback_params_t *params, cy_en_syspm_callback_mode_t mode)
{
    cy_en_syspm_status_t status = CY_SYSPM_FAIL;

    switch (mode) {
        case CY_SYSPM_CHECK_READY:
            status = CY_SYSPM_SUCCESS;
            break;

        case CY_SYSPM_CHECK_FAIL:
			status = CY_SYSPM_SUCCESS;
            break;

        case CY_SYSPM_BEFORE_TRANSITION:
			status = CY_SYSPM_SUCCESS;
            break;

        case CY_SYSPM_AFTER_TRANSITION:
			deep_sleep_count++;
			status = CY_SYSPM_SUCCESS;
            break;

        default:
            break;
    }

	return status;
}


static cy_stc_syspm_callback_params_t cype_pm_callback_params = {
    .base = NULL,
};

static cy_stc_syspm_callback_t cype_pm_callback_handler = {
    .callback = cype_pm_callback,
    .type = CY_SYSPM_DEEPSLEEP,
    .skipMode = 0,
    .callbackParams = &cype_pm_callback_params,
};

/**
 * Gets time in milliseconds
 *
 * @Note: Platform specific, need to be ported accroding to the platform.
 *
 * @Note: In the current platform time stamp is obtained from PMU timer.
 *
 * @returns Time in milliseconds
 */

uint32_t platform_cype_get_time_stamp( void )
{
    uint32_t current_time;
    cy_rtos_get_time(&current_time);
    return current_time;
}

void platform_cype_enable_logging( cype_bool_t enable )
{
    UNUSED_PARAMETER( enable );
}

cype_result_t platform_cype_init(void)
{
	if (!Cy_SysPm_RegisterCallback(&cype_pm_callback_handler)) {
		CYPE_PRINT_ERROR (( "PM callback registration failed!\n" )) ;	
		return CYPE_ERROR;
	}
#ifdef CY_BLE_ESTIMATION_ENABLE
	cype_ble_init();
#endif
    return CYPE_SUCCESS;
}

void platform_cype_reset_power_data(cpl_procid_t processor_id)
{
    if(processor_id == EVENT_PROC_ID_WIFI)
    {
 #ifdef CY_WIFI_ESTIMATION_ENABLE
   	    cype_wifi_init();
        cype_wifi_reset_power_data();
#endif
    }
	if(processor_id == EVENT_PROC_ID_MCU)
    {
        cype_mcu_reset_power_data();
    }
    if(processor_id == EVENT_PROC_ID_BT)
    {
#ifdef CY_BLE_ESTIMATION_ENABLE
        cype_bt_reset_power_data();
#endif
    }
    return;
}

void platform_cype_update_power_data(cpl_procid_t processor_id)
{
    if(processor_id == EVENT_PROC_ID_WIFI)
    {
#ifdef CY_WIFI_ESTIMATION_ENABLE
      cype_wifi_update_power_data();
#endif
    }
	if(processor_id == EVENT_PROC_ID_MCU)
    {
        cype_mcu_update_power_data();
    }
    if(processor_id == EVENT_PROC_ID_BT)
    {
#ifdef CY_BLE_ESTIMATION_ENABLE
        cype_bt_update_power_data();
#endif
    }
    return;
}

cype_bool_t platform_is_warm_booted()
{
	return CYPE_FALSE;
}

void mbed_main(void)
{
	if ( CYPE_START() != CYPE_SUCCESS)
		CYPE_PRINT_ERROR (( "Error: Failed to start CyPE\n" )) ;	
}
