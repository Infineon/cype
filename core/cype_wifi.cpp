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

#include "cype.h"
#include "whd_wifi_api.h"
#include "whd_emac.h"
#include "whd_wlioctl.h"
#include "cy_power_estimator.h"
#include "WhdSTAInterface.h"
#include "whd_wifi_api.h"

#ifdef __cplusplus
extern "C" {
#endif

extern void cpl_log_update( uint8_t proc_id, uint8_t event_id, uint8_t event_state, uint32_t event_data );


#define WL_CHANSPEC_BW_MASK		0x3800
#define WL_CHANSPEC_BW_SHIFT		11
#define WL_CHANSPEC_BW_5		0x0000
#define WL_CHANSPEC_BW_10		0x0800
#define WL_CHANSPEC_BW_20		0x1000
#define WL_CHANSPEC_BW_40		0x1800
#define WL_CHANSPEC_BW_80		0x2000
#define WL_CHANSPEC_BW_160		0x2800
#define WL_CHANSPEC_BW_8080		0x3000
#define WL_CHANSPEC_BW_2P5		0x3800

extern whd_interface_t my_ifp;


/******************************************************
 *                      Macros
 ******************************************************/
#define AMPDU_DUMP_SIZE             1500
#define AMPDU_NO_DUMP_STRING_SIZE    100
#define CYPE_MAX_RATE_COUNT           (EVENT_DESC_WIFI_MAX - EVENT_DESC_WIFI_RATE0)


WiFiInterface *cype_wifi_if = NULL; 
WhdSTAInterface *cype_sta_wifi_if = NULL;
whd_interface_t cype_ifp;

cype_result_t cype_ampdu_clear_dump();

extern "C" void cype_wifi_init(void)
{
	if (cype_wifi_if != NULL)
		return;
	cype_wifi_if = cype_wifi_if->get_default_instance();
	cype_sta_wifi_if = (WhdSTAInterface *)cype_wifi_if;

	cype_sta_wifi_if->wifi_get_ifp(&cype_ifp);
	
	if(cype_ifp == NULL)
		printf("cype wifi init failed");
	
}

uint32_t cype_wifi_parse_ampdu_dump( uint8_t *ampdu_dump_buf, uint32_t *tx_rx_rate_data )
{
    char *tx_mcs_rate_str, *rx_mcs_rate_str;
    char *tx_vhtrate_str, *rx_vhtrate_str;
    char *tx_rate_str, *rx_rate_str;
    char *tempPtr, *tempPtr1;
    char tempPtr2[10];
    uint32_t tx_rate_count = 0, rx_rate_count = 0;

    tx_mcs_rate_str = strstr( ( char * )ampdu_dump_buf, "TX MCS" );
    rx_mcs_rate_str = strstr( ( char * )ampdu_dump_buf, "RX MCS" );

    tx_vhtrate_str = strstr( ( char * )ampdu_dump_buf, "TX VHT" );
    rx_vhtrate_str = strstr( ( char * )ampdu_dump_buf, "RX VHT" );

    sscanf( tx_mcs_rate_str,"%[^\n]", tx_mcs_rate_str );
    sscanf( rx_mcs_rate_str,"%[^\n]", rx_mcs_rate_str );
    sscanf( tx_vhtrate_str,"%[^\n]", tx_vhtrate_str );
    sscanf( rx_vhtrate_str,"%[^\n]", rx_vhtrate_str );

    if ( strlen(tx_vhtrate_str)  > (strlen(tx_mcs_rate_str) ) )
    {
        tx_rate_str = tx_vhtrate_str;
        rx_rate_str = rx_vhtrate_str;
    }
    else
    {
        tx_rate_str = tx_mcs_rate_str;
        rx_rate_str = rx_mcs_rate_str;
    }

    //Parse the ampdu dump to extract the TX data
    tx_rate_str = strstr( tx_rate_str, "  ");

    tempPtr = strtok( tx_rate_str, "  " );
    while ( tempPtr != NULL ) {
        tempPtr = strtok( NULL, "  " );

        if ( tempPtr != NULL )
        {
            int idx = 0;
            tempPtr1 = tempPtr;
            while ( strncmp( tempPtr1, "(", 1 ) ) {
                tempPtr2[idx] = *tempPtr1;
                idx++;
                tempPtr1++;
            }
            tempPtr2[idx] = 0;

            tx_rx_rate_data[tx_rate_count] = ( uint32_t )atoi( tempPtr2 ) << 16;
            tx_rate_count++;
        }
    }

    //Parse the ampdu dump to extract the RX data
    rx_rate_str = strstr( rx_rate_str,"  " );

    tempPtr = strtok( rx_rate_str, "  " );
    while ( tempPtr != NULL ) {
        tempPtr = strtok( NULL, "  " );

        if ( tempPtr != NULL )
        {
            int idx = 0;
            tempPtr1 = tempPtr;
            while(strncmp( tempPtr1, "(", 1 ) ) {
                tempPtr2[idx] = *tempPtr1;
                idx++;
                tempPtr1++;
            }
            tempPtr2[idx] = 0;

            tx_rx_rate_data[rx_rate_count] |= ( uint32_t )atoi( tempPtr2 );
            rx_rate_count++;
        }
    }

    return tx_rate_count;
}

cype_result_t cype_get_dump( uint8_t *dump_buf, uint16_t len, char *cmd, uint16_t cmd_len )
{
	 whd_wifi_get_iovar_buffer_with_param( cype_ifp, "dump", cmd, cmd_len,
                   (uint8_t *)dump_buf, len );
				   
	 return CYPE_SUCCESS;
}

cype_result_t cype_ampdu_clear_dump()
{

	whd_wifi_set_iovar_buffer( cype_ifp, "dump_clear", (char *)"ampdu", strlen("ampdu"));
	

	return CYPE_SUCCESS;
}

uint32_t cype_wifi_get_data_from_ampdu_dump( uint32_t *tx_rx_rate_data )
{
    uint8_t *ampdu_dump_buf;
    char *ampdu_cmd = (char *)"ampdu";
    uint32_t ret, rate_count;


    rate_count = 0;

    //Get number of packets per each data rate
     ampdu_dump_buf = (uint8_t *)calloc( AMPDU_DUMP_SIZE, sizeof(char) );
     if ( ampdu_dump_buf == NULL ) {
         printf ( "Failed to allocate dump buffer of %d bytes\n", AMPDU_DUMP_SIZE ) ;
         goto exit;
     }

    ret = cype_get_dump( ampdu_dump_buf, AMPDU_DUMP_SIZE, ampdu_cmd, (uint16_t)strlen(ampdu_cmd) );

    if ( ret ) {
        printf ( "dump failed with error : %d, make sure that FW supports this particular dump feature\n", ( int16_t )ret ) ;
        goto exit;
    }

    if ( strlen( (char *)ampdu_dump_buf ) < AMPDU_NO_DUMP_STRING_SIZE)
        goto exit;

   //Dump is collected, now clear it
   if ( cype_ampdu_clear_dump( ) )
   {
       printf ( "CyPE: Failed to clear ampdu dump\n" ) ;
       goto exit;
   }

    rate_count = cype_wifi_parse_ampdu_dump( ampdu_dump_buf, tx_rx_rate_data );

exit:
    free( ampdu_dump_buf );
    return rate_count;
}


cype_result_t cype_wifi_get_current_band( uint32_t *current_band )
{
    uint32_t band;
    uint32_t bandlist[3];

    /* Get band */
    whd_wifi_get_ioctl_value( cype_ifp, WLC_GET_BAND, &band );

    /* Get supported Band List */
    whd_wifi_get_ioctl_buffer( cype_ifp, WLC_GET_BANDLIST, ( uint8_t * )bandlist, sizeof(bandlist) );

    /* only support a maximum of 2 bands */
    if (!bandlist[0])
        return CYPE_ERROR;
    else if (bandlist[0] > 2)
        bandlist[0] = 2;

    switch (band) {
    /* In case of auto band selection the current associated band will be in the entry 1 */
    case WLC_BAND_AUTO :
        if ( bandlist[1] == WLC_BAND_5G )
            *current_band = WLC_BAND_5G;
        else if ( bandlist[1] == WLC_BAND_2G )
            *current_band = WLC_BAND_2G;
        else
            return CYPE_ERROR;

        break;

    case WLC_BAND_5G :
        *current_band = WLC_BAND_5G;
        break;

    case WLC_BAND_2G :
        *current_band = WLC_BAND_2G;
        break;

    default :
        return CYPE_ERROR;
        break;
    }

    return CYPE_SUCCESS;
}

cype_result_t cype_wifi_get_bw( uint32_t *bwidth )
{
    uint32_t chanspec;

    whd_wifi_get_iovar_value( cype_ifp, IOVAR_STR_CHANSPEC, &chanspec );
    *bwidth = ( ( chanspec & WL_CHANSPEC_BW_MASK ) == WL_CHANSPEC_BW_20 ) ? 20 : ( ( ( chanspec & WL_CHANSPEC_BW_MASK ) == WL_CHANSPEC_BW_40 ) ? 40 : 80 );

    return CYPE_SUCCESS;
}

extern "C" void cype_wifi_update_power_data(void)
{
    uint32_t band = 0xFFFFFFFF;
    uint32_t bandwidth = 0xFFFFFFFF;
    uint32_t cnt, pm_mode, rate_count;
    uint32_t tx_rx_rate_data[CYPE_MAX_RATE_COUNT] = { 0 };
    uint8_t rate_index = EVENT_DESC_WIFI_RATE0;
    uint32_t ampdu_tx_frame_cnt = 0, ampdu_rx_frame_cnt = 0;

   if(cype_sta_wifi_if->is_interface_connected() == CYPE_SUCCESS) {
        cpl_log_update( EVENT_PROC_ID_WIFI, EVENT_ID_WIFI_DATA, EVENT_DESC_WIFI_UP_STATUS,  1);
   	}
    else {
        cpl_log_update( EVENT_PROC_ID_WIFI, EVENT_ID_WIFI_DATA, EVENT_DESC_WIFI_UP_STATUS,  0);
		return;
    }

    //Get Wi-Fi PM mode
    if(whd_wifi_get_ioctl_value( cype_ifp, WLC_GET_PM, &pm_mode))
    {
       printf( "CyPE: Couldn't get PM mode \n");
       return;
    }

    cpl_log_update( EVENT_PROC_ID_WIFI, EVENT_ID_WIFI_DATA, EVENT_DESC_WIFI_PMMODE, pm_mode );

    //Get band
    if ( cype_wifi_get_current_band( &band ) )
    {
        printf(  "CyPE: Couldn't get band \n" ) ;
        return;
    }
    cpl_log_update( EVENT_PROC_ID_WIFI, EVENT_ID_WIFI_DATA, EVENT_DESC_WIFI_BAND, band );

    //Get bandwidth
    if ( cype_wifi_get_bw( &bandwidth ) )
    {
        printf(  "CyPE: Couldn't get Bandwidth \n" ) ;
        return;
    }
	
    cpl_log_update( EVENT_PROC_ID_WIFI, EVENT_ID_WIFI_DATA, EVENT_DESC_WIFI_BW, bandwidth );

    rate_count = cype_wifi_get_data_from_ampdu_dump ( tx_rx_rate_data );
	
    cpl_log_update( EVENT_PROC_ID_WIFI, EVENT_ID_WIFI_DATA, EVENT_DESC_WIFI_RATE_TYPE, rate_count );

    for ( cnt = 0 ;  cnt < rate_count ; cnt++) {
       printf ( "tx_rx_rate_data[%u]:0x%08X\n", (unsigned int)cnt,(unsigned int)tx_rx_rate_data[cnt] ) ;
        ampdu_tx_frame_cnt += tx_rx_rate_data[cnt] >> 16;
        ampdu_rx_frame_cnt += tx_rx_rate_data[cnt] & 0x0000FFFF;
        cpl_log_update( EVENT_PROC_ID_WIFI, EVENT_ID_WIFI_DATA, rate_index++, tx_rx_rate_data[cnt] );
    }

}

/* Reset WiFi data counters */
extern "C" void cype_wifi_reset_power_data(void)
{
	if(cype_sta_wifi_if->is_interface_connected() == CYPE_SUCCESS) {
	    cype_ampdu_clear_dump( );
   	}
    return;
}

#ifdef __cplusplus
}
#endif
