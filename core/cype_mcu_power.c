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

#include "mbed_stats.h"
#include "cype_cpl.h"
#if LPA_INTEGRATION
#include "cycfg_system.h"
#endif

#if LPA_INTEGRATION
#define CY_CFG_PWR_MODE_LP 0x01UL
#define CY_CFG_PWR_MODE_ULP 0x02UL
#define CY_CFG_PWR_SYS_ACTIVE_MODE CY_CFG_PWR_MODE_LP
#define CY_CFG_PWR_USING_LDO 1
#endif

/******************************************************
 *                      Variables
 ******************************************************/
mbed_stats_cpu_t backup_stats;

/******************************************************
 *                      Function Definitions
 ******************************************************/
void cype_mcu_reset_power_data( void )
{
		mbed_stats_cpu_get(&backup_stats);
}

void cype_mcu_update_backup_stats(mbed_stats_cpu_t stats)
{
	backup_stats.uptime = stats.uptime;
	backup_stats.sleep_time = stats.sleep_time;
	backup_stats.deep_sleep_time = stats.deep_sleep_time;
	backup_stats.idle_time = stats.idle_time;
}

extern uint32 deep_sleep_count;
void cype_mcu_update_power_data(void)
{
	mbed_stats_cpu_t stats;

	mbed_stats_cpu_get(&stats);
#if LPA_INTEGRATION
#if (CY_CFG_PWR_SYS_ACTIVE_MODE == CY_CFG_PWR_MODE_ULP) /* ULP */
#if CY_CFG_PWR_USING_LDO /* LDO is used */
	CY_POWER_ESTIMATOR_DATA( EVENT_PROC_ID_MCU, EVENT_ID_POWERSTATE, EVENT_DESC_POWER_ACTIVE2, ((stats.uptime - backup_stats.uptime)-(stats.idle_time - backup_stats.idle_time))/1000 );
	CY_POWER_ESTIMATOR_DATA( EVENT_PROC_ID_MCU, EVENT_ID_POWERSTATE, EVENT_DESC_POWER_SLEEP2,  (stats.sleep_time - backup_stats.sleep_time)/1000 );
	CY_POWER_ESTIMATOR_DATA( EVENT_PROC_ID_MCU, EVENT_ID_POWERSTATE, EVENT_DESC_POWER_DEEPSLEEP, (stats.deep_sleep_time - backup_stats.deep_sleep_time)/1000 );
#else 	/* Buck is used */
	CY_POWER_ESTIMATOR_DATA( EVENT_PROC_ID_MCU, EVENT_ID_POWERSTATE, EVENT_DESC_POWER_ACTIVE21, ((stats.uptime - backup_stats.uptime)-(stats.idle_time - backup_stats.idle_time))/1000 );
	CY_POWER_ESTIMATOR_DATA( EVENT_PROC_ID_MCU, EVENT_ID_POWERSTATE, EVENT_DESC_POWER_SLEEP21,	(stats.sleep_time - backup_stats.sleep_time)/1000 );
	CY_POWER_ESTIMATOR_DATA( EVENT_PROC_ID_MCU, EVENT_ID_POWERSTATE, EVENT_DESC_POWER_DEEPSLEEP, (stats.deep_sleep_time - backup_stats.deep_sleep_time)/1000 );
#endif
#else /* LP */
#if CY_CFG_PWR_USING_LDO /* LDO is used */
	CY_POWER_ESTIMATOR_DATA( EVENT_PROC_ID_MCU, EVENT_ID_POWERSTATE, EVENT_DESC_POWER_ACTIVE1, ((stats.uptime - backup_stats.uptime)-(stats.idle_time - backup_stats.idle_time))/1000 );
	CY_POWER_ESTIMATOR_DATA( EVENT_PROC_ID_MCU, EVENT_ID_POWERSTATE, EVENT_DESC_POWER_SLEEP1,  (stats.sleep_time - backup_stats.sleep_time)/1000 );
	CY_POWER_ESTIMATOR_DATA( EVENT_PROC_ID_MCU, EVENT_ID_POWERSTATE, EVENT_DESC_POWER_DEEPSLEEP, (stats.deep_sleep_time - backup_stats.deep_sleep_time)/1000 );
#else 	/* Buck is used */
	CY_POWER_ESTIMATOR_DATA( EVENT_PROC_ID_MCU, EVENT_ID_POWERSTATE, EVENT_DESC_POWER_ACTIVE11, ((stats.uptime - backup_stats.uptime)-(stats.idle_time - backup_stats.idle_time))/1000 );
	CY_POWER_ESTIMATOR_DATA( EVENT_PROC_ID_MCU, EVENT_ID_POWERSTATE, EVENT_DESC_POWER_SLEEP11,	(stats.sleep_time - backup_stats.sleep_time)/1000 );
	CY_POWER_ESTIMATOR_DATA( EVENT_PROC_ID_MCU, EVENT_ID_POWERSTATE, EVENT_DESC_POWER_DEEPSLEEP, (stats.deep_sleep_time - backup_stats.deep_sleep_time)/1000 );
#endif
#endif
#else
	CY_POWER_ESTIMATOR_DATA( EVENT_PROC_ID_MCU, EVENT_ID_POWERSTATE, EVENT_DESC_POWER_ACTIVE1, (((stats.uptime - backup_stats.uptime)-(stats.idle_time - backup_stats.idle_time))/1000) + (deep_sleep_count*50) );
	CY_POWER_ESTIMATOR_DATA( EVENT_PROC_ID_MCU, EVENT_ID_POWERSTATE, EVENT_DESC_POWER_DEEPSLEEP, ((stats.deep_sleep_time - backup_stats.deep_sleep_time)/1000) - (deep_sleep_count*50));
	CY_POWER_ESTIMATOR_DATA( EVENT_PROC_ID_MCU, EVENT_ID_POWERSTATE, EVENT_DESC_POWER_SLEEP1,  (stats.sleep_time - backup_stats.sleep_time)/1000 );
	deep_sleep_count = 0;
#endif	
	cype_mcu_update_backup_stats(stats);
}
