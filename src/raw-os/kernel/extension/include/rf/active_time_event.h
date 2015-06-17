/*
     raw os - Copyright (C)  Lingjun Chen(jorya_txj).

    This file is part of raw os.

    raw os is free software; you can redistribute it it under the terms of the 
    GNU General Public License as published by the Free Software Foundation; 
    either version 3 of the License, or  (at your option) any later version.

    raw os is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; 
    without even the implied warranty of  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  
    See the GNU General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with this program. if not, write email to jorya.txj@gmail.com
                                      ---

    A special exception to the LGPL can be applied should you wish to distribute
    a combined work that includes raw os, without being obliged to provide
    the source code for any proprietary components. See the file exception.txt
    for full details of how and when the exception can be applied.
*/


/* 	2012-9  Created by jorya_txj
  *	xxxxxx   please added here
  */



#ifndef ACTIVE_TIME_EVENT_H
#define ACTIVE_TIME_EVENT_H

typedef struct time_event_struct {
  
    STATE_EVENT          father;
	
    ACTIVE_OBJECT_STRUCT *active_object;
	RAW_TIMER             timer;

} TIME_EVENT_STRUCT;


void active_time_event_create(TIME_EVENT_STRUCT *me, RAW_U16 sig);
void active_time_event_fire(TIME_EVENT_STRUCT *me, ACTIVE_OBJECT_STRUCT *active_object, RAW_TICK_TYPE ticks, RAW_U8 once);
RAW_U16 active_time_event_deactivate(TIME_EVENT_STRUCT *me);


#endif


