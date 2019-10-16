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

/** @file
 * Header file that includes all API & helper functions
 */

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************
 * @cond      Macros
 ******************************************************/

#ifndef RESULT_ENUM
#define RESULT_ENUM( prefix, name, value )  prefix ## name = (value)
#endif /* ifndef RESULT_ENUM */

/*
 * Values: 0 - 999
 */
#define CYPE_RESULT_LIST( prefix ) \
    RESULT_ENUM( prefix, SUCCESS,                        0 ),   /**< Success */                        \
    RESULT_ENUM( prefix, ERROR,                          1 ),   /**< Error */                          \
    RESULT_ENUM( prefix, BADARG,                         2 ),   /**< Bad Arguments */                  \
    RESULT_ENUM( prefix, UNSUPPORTED,                    3 ),   /**< Unsupported function */           \
    RESULT_ENUM( prefix, OUT_OF_HEAP_SPACE,              4 ),   /**< Dynamic memory space exhausted */ \
    RESULT_ENUM( prefix, NOTUP,                          5 ),   /**< Interface is not currently Up */  \
    RESULT_ENUM( prefix, ABORTED,                        6 ),   /**< Operation aborted */              \
    RESULT_ENUM( prefix, PACKET_BUFFER_CORRUPT,          7 ),   /**< Packet buffer is corrupted */     \
    RESULT_ENUM( prefix, THREAD_CREATE_FAILED,           8 ),   /**< Thread Creation Failed */         \
	RESULT_ENUM( prefix, THREAD_FINISH_FAIL,             9 ),   /**< Thread Finish Failed */           \
	RESULT_ENUM( prefix, THREAD_DELETE_FAIL,             10 ),  /**< Thread Delete Failed */           \
	RESULT_ENUM( prefix, INVALID_JOIN_STATUS,			 11 ),	/**< Thread Join Failed */		       \
	RESULT_ENUM( prefix, SEMAPHORE_ERROR,			     12 ),	/**< Semaphore Error */      		   \
	RESULT_ENUM( prefix, TIMEOUT,        			     13 ),	/**< Timeout */             		   \
	RESULT_ENUM( prefix, WAIT_ABORTED,  				 14 ),	/**< Wait Aborted Error */ 			   \
	RESULT_ENUM( prefix, RTOS_ERROR,					 15 ),	/**< RTOS Error */ 					   \
	RESULT_ENUM( prefix, RTOS_STATIC_MEM_LIMIT,			 16 ),	/**< RTOS STATIC MEM LIMIT Error */ 					   \

/******************************************************
 * @endcond    Enumerations
 ******************************************************/

/**
 * CYPE Result Type
 */
typedef enum
{
    CYPE_RESULT_LIST     ( CYPE_            )  /*     0 -   999 */
} cype_result_t;

/******************************************************
 *            Structures
 ******************************************************/

/******************************************************
 *            Function Declarations
 ******************************************************/

#ifdef __cplusplus
} /*extern "C" */
#endif
