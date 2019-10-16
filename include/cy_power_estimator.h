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


/*****************************************************************************/
/** @addtogroup cype_event       CyPE Power Event
 *  @ingroup cype
 *
 * A CyPE power event can be logged by power logger. It is a 3-tuple of (Processor, Event, Event State).
 *
 *  @{
 */

/******************************************************
 *                      Macros
 ******************************************************/

/******************************************************
 *                   Enumerations
 ******************************************************/
/**
 * @brief Processors (power consuming component) available on the target platform is identified by this ID
 *
 */
typedef enum {
    EVENT_PROC_ID_MCU,       /**< MCU Processor ID. */
    EVENT_PROC_ID_WIFI,      /**< Wi-Fi Processor ID. */
    EVENT_PROC_ID_BT,        /**< BT Processor ID. */
    EVENT_PROC_ID_MAX,       /**< Processor ID Count. */
} cpl_procid_t;

/**
 * @brief Event ID: Supported Power Events for Processors
 *
 */
typedef enum {
    EVENT_ID_POWERSTATE,     /**< Power State Event ID. */
    EVENT_ID_FLASH,          /**< Flash Event ID. */
    EVENT_ID_UART,           /**< UART Event ID. */
    EVENT_ID_WIFI_DATA,      /**< Wi-Fi Event ID. */
    EVENT_ID_I2S,            /**< I2S Event ID. */
    EVENT_ID_PROFILING,      /**< Function Profiling Event ID. */
    EVENT_ID_BT_DATA,        /**< BT Event ID. */
    EVENT_ID_I2C,            /**< I2C Event ID. */
    EVENT_ID_SPI_SFLASH,     /**< SPI_SFLASH Event ID. */
    EVENT_ID_SDIO,           /**< SDIO Event ID. */
    EVENT_ID_SPI_1,          /**< SPI_1 Event ID. */
    EVENT_ID_MAX,            /**< Event ID Count. */
} cpl_event_id_t;

/**
 * @brief Possible State Descriptors for MCU Processor + Powerstate Event combination
 *
 */
typedef enum {
    EVENT_DESC_POWER_ACTIVE1,     /**< Active State-1 Descriptor ID. */
    EVENT_DESC_POWER_ACTIVE11,    /**< Active State-11 Descriptor ID. */
    EVENT_DESC_POWER_ACTIVE2,     /**< Active State-2 Descriptor ID. */
    EVENT_DESC_POWER_ACTIVE21,    /**< Active State-21 Descriptor ID. */
    EVENT_DESC_POWER_SLEEP1,      /**< Sleep State-1 Descriptor ID. */
    EVENT_DESC_POWER_SLEEP11,     /**< Sleep State-11 Descriptor ID. */
    EVENT_DESC_POWER_SLEEP2,      /**< Sleep State-2 Descriptor ID. */
    EVENT_DESC_POWER_SLEEP21,     /**< Sleep State-21 Descriptor ID. */
    EVENT_DESC_POWER_DEEPSLEEP,   /**< Deep Sleep State Descriptor ID. */
    EVENT_DESC_POWER_OFF,         /**< Off State Descriptor ID. */
    EVENT_DESC_POWER_HIBERNATE,   /**< Hibernate State Descriptor ID. */
    EVENT_DESC_POWER_PDS,         /**< PDS State Descriptor ID. */
    EVENT_DESC_MAX,               /**< Power Descriptor ID Count. */
} cpl_event_power_state_t;

/**
 * @brief Possible State Descriptors for BT Processor + BT Event combination
 *
 */
typedef enum
{
    EVENT_DESC_BT_POWER_OFF,            /**< BT Off State Descriptor ID. */
    EVENT_DESC_BT_POWER_IDLE,           /**< BT Idle State Descriptor ID. */
    EVENT_DESC_BT_POWER_TX,             /**< BT Tx Descriptor ID. */
    EVENT_DESC_BT_POWER_RX,             /**< BT Rx Descriptor ID. */
    EVENT_DESC_BT_POWER_TX_PDS,         /**< BT Tx PDS Descriptor ID. */
    EVENT_DESC_BT_POWER_RX_PDS,         /**< BT Rx PDS Descriptor ID. */
    EVENT_DESC_BT_POWER_DEEP_SLEEP,     /**< BT Deep Sleep Descriptor ID. */
    EVENT_DESC_BT_MAX,                  /**< BT Descriptor ID Count. */
} cpl_event_bt_power_state_t;

/**
 * @brief Possible State Descriptors for MCU Processor + UART Event combination
 *
 */
typedef enum {
    EVENT_DESC_UART_IDLE,            /**< UART Idle State Descriptor ID. */
    EVENT_DESC_UART_TX,              /**< UART tx State Descriptor ID. */
    EVENT_DESC_UART_RX,              /**< UART rx State Descriptor ID. */
    EVENT_DESC_UART_MAX,             /**< UART Descriptor ID Count. */
} cpl_event_uart_state_t;

/**
 * @brief Possible State Descriptors for MCU Processor + I2C Event combination
 *
 */
typedef enum {
    EVENT_DESC_I2C_IDLE,            /**< I2C Idle State Descriptor ID. */
    EVENT_DESC_I2C_TX,              /**< I2C tx State Descriptor ID. */
    EVENT_DESC_I2C_RX,              /**< I2C rx State Descriptor ID. */
    EVENT_DESC_I2C_MAX,             /**< I2C Descriptor ID Count. */
} cpl_event_i2c_state_t;

/**
 * @brief Possible State Descriptors for MCU Processor + SPI_SFLASH Event combination
 *
 */
typedef enum {
    EVENT_DESC_SPI_SFLASH_IDLE,     /**< SPI-SFLASH Idle State Descriptor ID. */
    EVENT_DESC_SPI_SFLASH_READ,     /**< SPI-SFLASH Read State Descriptor ID. */
    EVENT_DESC_SPI_SFLASH_WRITE,    /**< SPI-SFLASH Write State Descriptor ID. */
    EVENT_DESC_SPI_SFLASH_ERASE,    /**< SPI-SFLASH Erase State Descriptor ID. */
    EVENT_DESC_SPI_SFLASH_MAX,      /**< I2C Descriptor ID Count. */
} cpl_event_spi_sflash_state_t;

/**
 * @brief Possible State Descriptors for MCU Processor + SDIO Event combination
 *
 */
typedef enum {
    EVENT_DESC_SDIO_IDLE,     /**< SDIO Idle State Descriptor ID. */
    EVENT_DESC_SDIO_READ,     /**< SDIO Read State Descriptor ID. */
    EVENT_DESC_SDIO_WRITE,    /**< SDIO Write State Descriptor ID. */
    EVENT_DESC_SDIO_MAX,      /**< SDIO Descriptor ID Count. */
} cpl_event_sdio_state_t;

/**
 * @brief Possible State Descriptors for MCU Processor + SPI Event combination
 *
 */
typedef enum {
    EVENT_DESC_SPI_OFF,      /**< SPI OFF State Descriptor ID. */
    EVENT_DESC_SPI_IDLE,     /**< SPI Idle State Descriptor ID. */
    EVENT_DESC_SPI_READ,     /**< SPI Read State Descriptor ID. */
    EVENT_DESC_SPI_WRITE,    /**< SPI Write State Descriptor ID. */
    EVENT_DESC_SPI_MAX,      /**< SPI Descriptor ID Count. */
} cpl_event_spi_state_t;

/**
 * @brief Possible State Descriptors for MCU Processor + Function Profiling combination
 *
 */
typedef enum {
    EVENT_DESC_FUNC_IDLE,            /**< Function Idle State Descriptor ID. */
    EVENT_DESC_FUNC_TIME,            /**< Function Time State Descriptor ID. */
} cpl_event_profiling_state_t;

/**
 * @brief Possible State Descriptors for WiFi Processor + WiFi Data Event combination
 *
 */
typedef enum {
    EVENT_DESC_WIFI_IDLE,            /**< Wi-Fi idle State Descriptor ID. */
    EVENT_DESC_WIFI_BAND,            /**< Wi-Fi Band Descriptor ID. */
    EVENT_DESC_WIFI_BW,              /**< Wi-Fi Bandwidth Descriptor ID. */
    EVENT_DESC_WIFI_PMMODE,          /**< Wi-Fi PM mode Descriptor ID. */
    EVENT_DESC_WIFI_RATE_TYPE,       /**< Wi-Fi Rate Type Descriptor ID. */
    EVENT_DESC_WIFI_UP_STATUS,       /**< Wi-Fi Join Status Descriptor ID. */
    EVENT_DESC_WIFI_TXRX_FRAG,       /**< Wi-Fi Management/Control frames count Descriptor ID. */
    EVENT_DESC_WIFI_SCAN_TIME,       /**< Wi-Fi Scan Time Descriptor ID. */
    EVENT_DESC_WIFI_JOIN_TIME,       /**< Wi-Fi Join Time Descriptor ID. */
    EVENT_DESC_WIFI_RATE0,           /**< Wi-Fi Rate0 tx/rx counters data Descriptor ID. */
    EVENT_DESC_WIFI_RATE1,           /**< Wi-Fi Rate1 tx/rx counters data Descriptor ID. */
    EVENT_DESC_WIFI_RATE2,           /**< Wi-Fi Rate2 tx/rx counters data Descriptor ID. */
    EVENT_DESC_WIFI_RATE3,           /**< Wi-Fi Rate3 tx/rx counters data Descriptor ID. */
    EVENT_DESC_WIFI_RATE4,           /**< Wi-Fi Rate4 tx/rx counters data Descriptor ID. */
    EVENT_DESC_WIFI_RATE5,           /**< Wi-Fi Rate5 tx/rx counters data Descriptor ID. */
    EVENT_DESC_WIFI_RATE6,           /**< Wi-Fi Rate6 tx/rx counters data Descriptor ID. */
    EVENT_DESC_WIFI_RATE7,           /**< Wi-Fi Rate7 tx/rx counters data Descriptor ID. */
    EVENT_DESC_WIFI_RATE8,           /**< Wi-Fi Rate8 tx/rx counters data Descriptor ID. */
    EVENT_DESC_WIFI_RATE9,           /**< Wi-Fi Rate9 tx/rx counters data Descriptor ID. */
    EVENT_DESC_WIFI_MAX,             /**< Wi-Fi Descriptor ID Count. */
} cpl_event_wifi_state_t;

/**
 * @brief Possible State Descriptor Values for WiFi Processor + WiFi Data Event + WiFi Rate Type combination
 *
 */
typedef enum {
    EVENT_DESC_WIFI_MCS_RATE = 1,    /**< Wi-Fi MCS Rate Type ID. */
    EVENT_DESC_WIFI_VHT_RATE,        /**< Wi-Fi VHT Rate Type ID. */
} cpl_event_wifi_rate_type_t;

/** @} */

/******************************************************
 *                 Type Definitions
 ******************************************************/
/******************************************************
 *                    Structures
 ******************************************************/

/******************************************************
 *               Function Prototypes
 ******************************************************/

/** @addtogroup cype_api       CyPE Functions
 *  @ingroup cype
 *
 * CyPE provides power event logging APIs in the platform.
 *  @{
 */

/**
 * Logs time for a supported power event. Current time is logged as the start time for the specified event_state.
 * The time spent from previous logging is added into the previous event_state
 *
 * @param           proc_id : Processor ID
 * @param           event_id : Event ID
 * @param           event_state : Descriptor ID, represents different states assumed by the power events (for example, an MCU Power event can have event descriptors like: Active, Sleep etc.)
 * @return          NO return value.
 */

#ifdef CY_POWER_ESTIMATOR_ENABLE
extern void cpl_event_state_update( uint8_t proc_id, uint8_t event_id, uint8_t event_state );
#define CY_POWER_ESTIMATOR( proc_id, event_id, event_state ) cpl_event_state_update( proc_id, event_id, event_state )
#else
#define CY_POWER_ESTIMATOR( proc_id, event_id, event_state )
#endif

/**
 * Logs data value for a supported power event. Resets the current data value of event_state with the specified data
 *
 * @param           proc_id : Processor ID
 * @param           event_id : Event ID
 * @param           event_state : Descriptor ID, represents different states assumed by the power events (for example, an MCU Power event can have event descriptors like: Active, Sleep etc.)
 * @param           data : Data value to be put for event_state.
 * @return          NO return value.
 */

#ifdef CY_POWER_ESTIMATOR_ENABLE
void cpl_log_reset_event_data( uint8_t proc_id, uint8_t event_id, uint8_t event_state, uint32_t data );
#define CY_POWER_ESTIMATOR_DATA( proc_id, event_id, event_state, data ) cpl_log_reset_event_data( proc_id, event_id, event_state, data )
#else
#define CY_POWER_ESTIMATOR_DATA( proc_id, event_id, event_state, data )
#endif


/**
 * Initializes and starts CyPE's power logging module.
 *
 * @param           NO parameters required.
 * @return          NO return value.
 */
#ifdef CY_POWER_ESTIMATOR_ENABLE
int cype_start(void);
#define CYPE_START() cype_start()
#else
#define CYPE_START()
#endif
/** @} */

#ifdef __cplusplus
}
#endif
