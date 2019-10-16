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

#ifdef __cplusplus
extern "C" {
#endif

event_lookup_table_entry_t cype_event_list_table[] =
{
    /* { proc-id, event-id, event-desc } */
    //MCU Events
    { EVENT_PROC_ID_MCU,        EVENT_ID_POWERSTATE,      EVENT_DESC_POWER_ACTIVE1                },
    { EVENT_PROC_ID_MCU,        EVENT_ID_POWERSTATE,      EVENT_DESC_POWER_ACTIVE11               }, // LP, BUCK
    { EVENT_PROC_ID_MCU,        EVENT_ID_POWERSTATE,      EVENT_DESC_POWER_ACTIVE2                },
    { EVENT_PROC_ID_MCU,        EVENT_ID_POWERSTATE,      EVENT_DESC_POWER_ACTIVE21               }, // ULP, BUCK
    { EVENT_PROC_ID_MCU,        EVENT_ID_POWERSTATE,      EVENT_DESC_POWER_SLEEP1                 },
    { EVENT_PROC_ID_MCU,        EVENT_ID_POWERSTATE,      EVENT_DESC_POWER_SLEEP11                }, // LP, BUCK
    { EVENT_PROC_ID_MCU,        EVENT_ID_POWERSTATE,      EVENT_DESC_POWER_SLEEP2                 },
    { EVENT_PROC_ID_MCU,        EVENT_ID_POWERSTATE,      EVENT_DESC_POWER_SLEEP21                }, // ULP, BUCK
    { EVENT_PROC_ID_MCU,        EVENT_ID_POWERSTATE,      EVENT_DESC_POWER_DEEPSLEEP              },


#ifdef CY_WIFI_ESTIMATION_ENABLE
     //Wi-Fi Events
    { EVENT_PROC_ID_WIFI,       EVENT_ID_WIFI_DATA,       EVENT_DESC_WIFI_IDLE                    },
    { EVENT_PROC_ID_WIFI,       EVENT_ID_WIFI_DATA,       EVENT_DESC_WIFI_BAND                    },
    { EVENT_PROC_ID_WIFI,       EVENT_ID_WIFI_DATA,       EVENT_DESC_WIFI_BW                      },
    { EVENT_PROC_ID_WIFI,       EVENT_ID_WIFI_DATA,       EVENT_DESC_WIFI_PMMODE                  },
    { EVENT_PROC_ID_WIFI,       EVENT_ID_WIFI_DATA,       EVENT_DESC_WIFI_RATE_TYPE               },
    { EVENT_PROC_ID_WIFI,       EVENT_ID_WIFI_DATA,       EVENT_DESC_WIFI_UP_STATUS               },
    { EVENT_PROC_ID_WIFI,       EVENT_ID_WIFI_DATA,       EVENT_DESC_WIFI_TXRX_FRAG               },
    { EVENT_PROC_ID_WIFI,       EVENT_ID_WIFI_DATA,       EVENT_DESC_WIFI_SCAN_TIME               },
    { EVENT_PROC_ID_WIFI,       EVENT_ID_WIFI_DATA,       EVENT_DESC_WIFI_JOIN_TIME               },
    { EVENT_PROC_ID_WIFI,       EVENT_ID_WIFI_DATA,       EVENT_DESC_WIFI_RATE0                   },
    { EVENT_PROC_ID_WIFI,       EVENT_ID_WIFI_DATA,       EVENT_DESC_WIFI_RATE1                   },
    { EVENT_PROC_ID_WIFI,       EVENT_ID_WIFI_DATA,       EVENT_DESC_WIFI_RATE2                   },
    { EVENT_PROC_ID_WIFI,       EVENT_ID_WIFI_DATA,       EVENT_DESC_WIFI_RATE3                   },
    { EVENT_PROC_ID_WIFI,       EVENT_ID_WIFI_DATA,       EVENT_DESC_WIFI_RATE4                   },
    { EVENT_PROC_ID_WIFI,       EVENT_ID_WIFI_DATA,       EVENT_DESC_WIFI_RATE5                   },
    { EVENT_PROC_ID_WIFI,       EVENT_ID_WIFI_DATA,       EVENT_DESC_WIFI_RATE6                   },
    { EVENT_PROC_ID_WIFI,       EVENT_ID_WIFI_DATA,       EVENT_DESC_WIFI_RATE7                   },
    { EVENT_PROC_ID_WIFI,       EVENT_ID_WIFI_DATA,       EVENT_DESC_WIFI_RATE8                   },
    { EVENT_PROC_ID_WIFI,       EVENT_ID_WIFI_DATA,       EVENT_DESC_WIFI_RATE9                   },
 #endif 
 
#ifdef CY_BLE_ESTIMATION_ENABLE	
    { EVENT_PROC_ID_BT,        EVENT_ID_BT_DATA,       EVENT_DESC_BT_POWER_OFF                  },    
    { EVENT_PROC_ID_BT,        EVENT_ID_BT_DATA,       EVENT_DESC_BT_POWER_IDLE                  },
    { EVENT_PROC_ID_BT,        EVENT_ID_BT_DATA,       EVENT_DESC_BT_POWER_TX                    },
    { EVENT_PROC_ID_BT,        EVENT_ID_BT_DATA,       EVENT_DESC_BT_POWER_RX                    },
#endif
};

#ifdef __cplusplus
} /* extern "C" */
#endif
