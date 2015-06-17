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


/* 	2012-10  Created by jorya_txj
  *	xxxxxx   please added here
  */

#ifndef RAW_MLOCK_H
#define RAW_MLOCK_H

typedef struct RAW_MLOCK{

	RAW_EVENT event_ptr;
	
} RAW_MLOCK;

RAW_U16 raw_mlock(RAW_MLOCK *mlock, RAW_U8 num, RAW_TICK_TYPE wait_option);
RAW_U16 raw_munlock(RAW_MLOCK *mlock, RAW_U8 num);
RAW_U16 raw_mlock_create(RAW_MLOCK *mlock, RAW_U8 *name_ptr);
RAW_U16 raw_mlock_delete(RAW_MLOCK *mlock);

#endif

