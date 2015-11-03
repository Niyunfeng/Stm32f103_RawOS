/*
     raw os - Copyright (C)  Lingjun Chen(jorya_txj).

    This file is part of raw os.

    raw os is free software; you can redistribute it it under the terms of the 
    GNU General Public License as published by the Free Software Foundation; 
    either version 3 of the License, or  (at your option) any later version.

    raw os is distributed in the hope that it will be useful, but WITHOUT ANY 
WARRANTY; 
    without even the implied warranty of  MERCHANTABILITY or FITNESS FOR A 
PARTICULAR PURPOSE.  
    See the GNU General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with this program. if not, write email to jorya.txj@gmail.com
                                      ---

    A special exception to the LGPL can be applied should you wish to 
distribute
    a combined work that includes raw os, without being obliged to provide
    the source code for any proprietary components. See the file exception.txt
    for full details of how and when the exception can be applied.
*/


/* 	2014-1  Created by jorya_txj
  *	xxxxxx   please added here
  */

#ifndef PORT_EXTENSION_H

#define PORT_EXTENSION_H

typedef  RAW_U32  PORT_TIMER_TYPE;


PORT_TIMER_TYPE user_cpu_time_get(void);
RAW_HARD_TIME_TYPE user_sys_time_get(void);


#if (RAW_CONFIG_CPU_TIME > 0)

#define  RAW_CPU_TIME_GET()         user_cpu_time_get()

#else

#define  RAW_CPU_TIME_GET()         0u

#endif

#if (CONFIG_RAW_SYSTEM_STATISTICS > 0)

#define  RAW_SYSTEM_TIME_GET()         user_sys_time_get()

#else

#define  RAW_SYSTEM_TIME_GET()         0u

#endif


#endif

