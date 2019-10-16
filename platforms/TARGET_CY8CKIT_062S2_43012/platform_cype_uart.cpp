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

#include "mbed.h"
#include "cype.h"
#include "cype_platform_api.h"
#include "CircularBuffer.h"
#include "cyabs_rtos.h"

/******************************************************
 *                      Macros
 ******************************************************/

/******************************************************
 *               Class Variable Definitions
 ******************************************************/

CircularBuffer<uint8_t, CYPE_UART_CIRCULAR_BUFFER_SIZE> rxbuf;
RawSerial cypetxrx(CYPE_TX, CYPE_RX,CYPE_UART_BAUD_RATE);

/******************************************************
 *               CMSIS Definitions
 ******************************************************/
//Semaphore
cy_semaphore_t cype_uart_txrx_semaphore;


/******************************************************
 *               Variable Definitions
 ******************************************************/

/******************************************************
 *               Function Definitions
 ******************************************************/

void callback_ex() {
	while(cypetxrx.readable()) {
		rxbuf.push((uint8_t)cypetxrx.getc());
		
	}
	cy_rtos_set_semaphore(&cype_uart_txrx_semaphore, CYPE_FALSE);
}


extern "C" cype_result_t cype_uart_init()
{
	cy_rslt_t result = CYPE_ERROR;

    result= cy_rtos_init_semaphore(&cype_uart_txrx_semaphore,1,0);   
	if ( result != CYPE_SUCCESS )
	{
	    CYPE_PRINT_ERROR( ( "Error creating Semaphore\n" ) );
		return CYPE_ERROR;
	}
	cypetxrx.attach(&callback_ex, RawSerial::RxIrq);
	//To reverse the ulocking done inside the attach
	sleep_manager_unlock_deep_sleep();
	return CYPE_SUCCESS;
}

extern "C" cype_result_t cype_uart_deinit()
{
	cypetxrx.attach(NULL, RawSerial::RxIrq);
	return CYPE_SUCCESS;
}

extern "C" int cype_uart_receive_bytes( uint8_t* data, int size)
{
	while(size) {
		if(rxbuf.empty()) {
			cy_rtos_get_semaphore(&cype_uart_txrx_semaphore, CY_RTOS_NEVER_TIMEOUT, CYPE_FALSE);
		}

		while( (!rxbuf.empty()) &&  size )  {
			rxbuf.pop(*(uint8_t *)data++);
			size--;
		}
	}

	return 0;
}

extern "C" int cype_uart_transmit_bytes( uint8_t* data, int size )
{
	int i;

	data = (uint8_t *)data;

	for(i = 0 ; i < size;i++,data++)
		cypetxrx.putc(*data);

	return 0;
}
