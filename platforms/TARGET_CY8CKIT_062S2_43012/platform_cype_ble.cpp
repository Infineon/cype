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
#ifdef CY_BLE_ESTIMATION_ENABLE

#include <events/mbed_events.h>
#include "mbed.h"
#include "cype.h"
#include "wsf_types.h"
#include "cype_platform_api.h"
#include "CyVscCmd.h"
#include "util/bstream.h"
#include "cy_power_estimator.h"
#include "platform_cype_ble.h"
#include "ble/BLE.h"

/******************************************************
 *                      Macros
 ******************************************************/

/******************************************************
 *               Class Variable Definitions
 ******************************************************/


/******************************************************
 *               CMSIS Definitions
 ******************************************************/
//Semaphore
cy_semaphore_t cype_ble_txrx_semaphore;


uint32_t prev_timestamp;

/******************************************************
 *               Variable Definitions
 ******************************************************/

/******************************************************
 *               Function Definitions
 ******************************************************/

extern "C" cype_result_t cype_ble_init()
{
	cype_result_t result;
	
   	result = cy_rtos_init_semaphore(&cype_ble_txrx_semaphore,1,0);
	if ( result != CYPE_SUCCESS )
	{
	    CYPE_PRINT_ERROR( ( "Error creating Semaphore\n" ) );
		return CYPE_ERROR;
	}

    cype_bt_reset_power_data();
    return CYPE_SUCCESS;
}

void cype_vsc_callback(uint16_t opcode, uint8_t status, uint16_t data_len, uint8_t* data)
{
	uint32_t tx_value, rx_value;
	uint32_t curr_timestamp;

	BSTREAM_TO_UINT32( tx_value, data ); // Get Tx
	BSTREAM_TO_UINT32( rx_value, data ); // Get Rx

	curr_timestamp = platform_cype_get_time_stamp();
	CY_POWER_ESTIMATOR_DATA( EVENT_PROC_ID_BT, EVENT_ID_BT_DATA, EVENT_DESC_BT_POWER_TX,  tx_value);
	CY_POWER_ESTIMATOR_DATA( EVENT_PROC_ID_BT, EVENT_ID_BT_DATA, EVENT_DESC_BT_POWER_RX,   rx_value);
	CY_POWER_ESTIMATOR_DATA( EVENT_PROC_ID_BT, EVENT_ID_BT_DATA, EVENT_DESC_BT_POWER_IDLE,  (curr_timestamp - prev_timestamp) - (tx_value+rx_value) );
	CY_POWER_ESTIMATOR_DATA( EVENT_PROC_ID_BT, EVENT_ID_BT_DATA, EVENT_DESC_BT_POWER_OFF,	0);

	prev_timestamp = curr_timestamp;
	cy_rtos_set_semaphore(&cype_ble_txrx_semaphore, CYPE_FALSE);
}

extern "C" void cype_bt_update_power_data(void)
{
	BLE &ble = BLE::Instance();

    if(!ble.hasInitialized())
    {
		uint32_t curr_timestamp;
		curr_timestamp = platform_cype_get_time_stamp();

		CY_POWER_ESTIMATOR_DATA( EVENT_PROC_ID_BT, EVENT_ID_BT_DATA, EVENT_DESC_BT_POWER_TX,  0);
		CY_POWER_ESTIMATOR_DATA( EVENT_PROC_ID_BT, EVENT_ID_BT_DATA, EVENT_DESC_BT_POWER_RX,   0);
		CY_POWER_ESTIMATOR_DATA( EVENT_PROC_ID_BT, EVENT_ID_BT_DATA, EVENT_DESC_BT_POWER_IDLE,	0);
        CY_POWER_ESTIMATOR_DATA( EVENT_PROC_ID_BT, EVENT_ID_BT_DATA, EVENT_DESC_BT_POWER_OFF,	curr_timestamp-prev_timestamp);
		prev_timestamp = curr_timestamp;
        return;
    }
	cy_send_vendor_specific_cmd( 0x159, 0, NULL, cype_vsc_callback);
	cy_rtos_get_semaphore(&cype_ble_txrx_semaphore, CY_RTOS_NEVER_TIMEOUT, CYPE_FALSE);

}

extern "C" void cype_bt_reset_power_data( void )
{
	BLE &ble = BLE::Instance();

    if(!ble.hasInitialized())
    {
        return;
    }
	cy_send_vendor_specific_cmd( 0x159, 0, NULL, cype_vsc_callback);
	cy_rtos_get_semaphore(&cype_ble_txrx_semaphore, CY_RTOS_NEVER_TIMEOUT, CYPE_FALSE);
}
#endif
