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



/* 	
 *	2012-9  Slab Memory Managment 
 *          by Hu Zhigang <huzhigang.rawos@gmail.com>
 */

#include <raw_api.h>

#include <mm/slab/config.h>
#include <mm/slab/misc.h>
#include <mm/slab/spinlock.h>
#include <mm/slab/arch.h>
#include <mm/slab/list.h>
#include <mm/slab/buddy.h>
#include <mm/slab/frame.h>
#include <mm/slab/slab.h>



zones_t zones;

/** Global configuration structure. */
config_t config;
/** Initial user-space tasks */
init_t init = {
	0,
};

/** Boot allocations. */
ballocs_t ballocs = {
	(uintptr_t) NULL,
	0
};

/********************/
/* Helper functions */
/********************/

RAW_INLINE size_t frame_index(zone_t *zone, frame_t *frame)
{
	return (size_t) (frame - zone->frames);
}

RAW_INLINE size_t frame_index_abs(zone_t *zone, frame_t *frame)
{
	return (size_t) (frame - zone->frames) + zone->base;
}

RAW_INLINE bool frame_index_valid(zone_t *zone, size_t index)
{
	return (index < zone->count);
}

RAW_INLINE size_t make_frame_index(zone_t *zone, frame_t *frame)
{
	return (frame - zone->frames);
}

/** Initialize frame structure.
 *
 * @param frame Frame structure to be initialized.
 *
 */
 static void frame_initialize(frame_t *frame)
{
	frame->refcount = 1;
	frame->buddy_order = 0;
}

/*******************/
/* Zones functions */
/*******************/

/** Insert-sort zone into zones list.
 *
 * Assume interrupts are disabled and zones lock is
 * locked.
 *
 * @param base  Base frame of the newly inserted zone.
 * @param count Number of frames of the newly inserted zone.
 *
 * @return Zone number on success, -1 on error.
 *
 */
 static size_t zones_insert_zone(pfn_t base, size_t count,
    zone_flags_t flags)
{
	size_t i;
	size_t j;

	if (zones.count + 1 == ZONES_MAX) {
		raw_printk("Maximum zone count %u exceeded!\n", ZONES_MAX);
		return (size_t) -1;
	}
	

	for (i = 0; i < zones.count; i++) {
		/* Check for overlap */
		if (overlaps(zones.info[i].base, zones.info[i].count,
		    base, count)) {
			
			/*
			 * If the overlaping zones are of the same type
			 * and the new zone is completely within the previous
			 * one, then quietly ignore the new zone.
			 *
			 */
			
			if ((zones.info[i].flags != flags) ||
			    (!iswithin(zones.info[i].base, zones.info[i].count,
			    base, count))) {
				raw_printk("Zone (%p, %p) overlaps "
				    "with previous zone (%p %p)!\n",
				    (void *) PFN2ADDR(base), (void *) PFN2ADDR(count),
				    (void *) PFN2ADDR(zones.info[i].base),
				    (void *) PFN2ADDR(zones.info[i].count));
			}
			
			return (size_t) -1;
		}
		if (base < zones.info[i].base)
			break;
	}
	
	/* Move other zones up */

	for (j = zones.count; j > i; j--) {
		zones.info[j] = zones.info[j - 1];
		if (zones.info[j].buddy_system != NULL)
			zones.info[j].buddy_system->data =
			    (void *) &zones.info[j];
	}
	
	zones.count++;
	
	return i;
}

/** Get total available frames.
 *
 * Assume interrupts are disabled and zones lock is
 * locked.
 *
 * @return Total number of available frames.
 *
 */
#ifdef CONFIG_DEBUG
 static size_t total_frames_free(void)
{
	size_t total = 0;
	size_t i;
	for (i = 0; i < zones.count; i++)
		total += zones.info[i].free_count;
	
	return total;
}
#endif /* CONFIG_DEBUG */

/** Find a zone with a given frames.
 *
 * Assume interrupts are disabled and zones lock is
 * locked.
 *
 * @param frame Frame number contained in zone.
 * @param count Number of frames to look for.
 * @param hint  Used as zone hint.
 *
 * @return Zone index or -1 if not found.
 *
 */
 size_t find_zone(pfn_t frame, size_t count, size_t hint)
{
	size_t i;
	if (hint >= zones.count)
		hint = 0;
	
	i = hint;
	do {
		if ((zones.info[i].base <= frame)
		    && (zones.info[i].base + zones.info[i].count >= frame + count))
			return i;
		
		i++;
		if (i >= zones.count)
			i = 0;
		
	} while (i != hint);
	
	return (size_t) -1;
}

/** @return True if zone can allocate specified order */
 static bool zone_can_alloc(zone_t *zone, RAW_U8 order)
{
	return (zone_flags_available(zone->flags)
	    && buddy_system_can_alloc(zone->buddy_system, order));
}

/** Find a zone that can allocate order frames.
 *
 * Assume interrupts are disabled and zones lock is
 * locked.
 *
 * @param order Size (2^order) of free space we are trying to find.
 * @param flags Required flags of the target zone.
 * @param hind  Preferred zone.
 *
 */
 static size_t find_free_zone(RAW_U8 order, zone_flags_t flags,
    size_t hint)
{
	size_t i;
	if (hint >= zones.count)
		hint = 0;
	
	i = hint;

	do {
		/*
		 * Check whether the zone meets the search criteria.
		 */
		if ((zones.info[i].flags & flags) == flags) {
			/*
			 * Check if the zone has 2^order frames area available.
			 */
			if (zone_can_alloc(&zones.info[i], order))
				return i;
		}
		
		i++;
		if (i >= zones.count)
			i = 0;
		
	} while (i != hint);
	
	return (size_t) -1;
}

/**************************/
/* Buddy system functions */
/**************************/

/** Buddy system find_block implementation.
 *
 * Find block that is parent of current list.
 * That means go to lower addresses, until such block is found
 *
 * @param order Order of parent must be different then this
 *              parameter!!
 *
 */
 static link_t *zone_buddy_find_block(buddy_system_t *buddy,
    link_t *child, RAW_U8 order)
{
	frame_t *frame = list_get_instance(child, frame_t, buddy_link);
	zone_t *zone = (zone_t *) buddy->data;
	
	size_t index = frame_index(zone, frame);
	do {
		if (zone->frames[index].buddy_order != order)
			return &zone->frames[index].buddy_link;
	} while (index-- > 0);
	
	return NULL;
}

/** Buddy system find_buddy implementation.
 *
 * @param buddy Buddy system.
 * @param block Block for which buddy should be found.
 *
 * @return Buddy for given block if found.
 *
 */
 static link_t *zone_buddy_find_buddy(buddy_system_t *buddy,
    link_t *block)
{
	bool is_left;
	size_t index;

	frame_t *frame = list_get_instance(block, frame_t, buddy_link);
	zone_t *zone = (zone_t *) buddy->data;
	ASSERT(IS_BUDDY_ORDER_OK(frame_index_abs(zone, frame),
	    frame->buddy_order));
	
	is_left = IS_BUDDY_LEFT_BLOCK_ABS(zone, frame);
	
	if (is_left) {
		index = (frame_index(zone, frame)) +
		    (1 << frame->buddy_order);
	} else {  /* is_right */
		index = (frame_index(zone, frame)) -
		    (1 << frame->buddy_order);
	}
	
	if (frame_index_valid(zone, index)) {
		if ((zone->frames[index].buddy_order == frame->buddy_order) &&
		    (zone->frames[index].refcount == 0)) {
			return &zone->frames[index].buddy_link;
		}
	}
	
	return NULL;
}

/** Buddy system bisect implementation.
 *
 * @param buddy Buddy system.
 * @param block Block to bisect.
 *
 * @return Right block.
 *
 */
 static link_t *zone_buddy_bisect(buddy_system_t *buddy, link_t *block)
{
	frame_t *frame_l = list_get_instance(block, frame_t, buddy_link);
	frame_t *frame_r = (frame_l + (1 << (frame_l->buddy_order - 1)));
	
	return &frame_r->buddy_link;
}

/** Buddy system coalesce implementation.
 *
 * @param buddy   Buddy system.
 * @param block_1 First block.
 * @param block_2 First block's buddy.
 *
 * @return Coalesced block (actually block that represents lower
 *         address).
 *
 */
 static link_t *zone_buddy_coalesce(buddy_system_t *buddy,
    link_t *block_1, link_t *block_2)
{
	frame_t *frame1 = list_get_instance(block_1, frame_t, buddy_link);
	frame_t *frame2 = list_get_instance(block_2, frame_t, buddy_link);
	
	return ((frame1 < frame2) ? block_1 : block_2);
}

/** Buddy system set_order implementation.
 *
 * @param buddy Buddy system.
 * @param block Buddy system block.
 * @param order Order to set.
 *
 */
 static void zone_buddy_set_order(buddy_system_t *buddy, link_t *block,
  RAW_U8 order)
{
	list_get_instance(block, frame_t, buddy_link)->buddy_order = order;
}

/** Buddy system get_order implementation.
 *
 * @param buddy Buddy system.
 * @param block Buddy system block.
 *
 * @return Order of block.
 *
 */
 static RAW_U8 zone_buddy_get_order(buddy_system_t *buddy,
    link_t *block)
{
	return list_get_instance(block, frame_t, buddy_link)->buddy_order;
}

/** Buddy system mark_busy implementation.
 *
 * @param buddy Buddy system.
 * @param block Buddy system block.
 *
 */
 static void zone_buddy_mark_busy(buddy_system_t *buddy, link_t *block)
{
	list_get_instance(block, frame_t, buddy_link)->refcount = 1;
}

/** Buddy system mark_available implementation.
 *
 * @param buddy Buddy system.
 * @param block Buddy system block.
 *
 */
 static void zone_buddy_mark_available(buddy_system_t *buddy,
    link_t *block)
{
	list_get_instance(block, frame_t, buddy_link)->refcount = 0;
}


static buddy_system_operations_t zone_buddy_system_operations = {
	zone_buddy_find_buddy,
	zone_buddy_bisect,
	zone_buddy_coalesce,
	zone_buddy_set_order,
	zone_buddy_get_order,
	zone_buddy_mark_busy,
	zone_buddy_mark_available,
	zone_buddy_find_block
};

/******************/
/* Zone functions */
/******************/

/** Allocate frame in particular zone.
 *
 * Assume zone is locked and is available for allocation.
 * Panics if allocation is impossible.
 *
 * @param zone  Zone to allocate from.
 * @param order Allocate exactly 2^order frames.
 *
 * @return Frame index in zone.
 *
 */
 static pfn_t zone_frame_alloc(zone_t *zone, RAW_U8 order)
{
	frame_t *frame;
	link_t *link;

	ASSERT(zone_flags_available(zone->flags));
	
	/* Allocate frames from zone buddy system */
	link = buddy_system_alloc(zone->buddy_system, order);
	
	ASSERT(link);
	
	/* Update zone information. */
	zone->free_count -= (1 << order);
	zone->busy_count += (1 << order);
	
	/* Frame will be actually a first frame of the block. */
	frame = list_get_instance(link, frame_t, buddy_link);
	
	/* Get frame address */
	return make_frame_index(zone, frame);
}

/** Free frame from zone.
 *
 * Assume zone is locked and is available for deallocation.
 *
 * @param zone      Pointer to zone from which the frame is to be freed.
 * @param frame_idx Frame index relative to zone.
 *
 */
 static void zone_frame_free(zone_t *zone, size_t frame_idx)
{
	RAW_U8 order;
	frame_t *frame;
	ASSERT(zone_flags_available(zone->flags));
	
	frame = &zone->frames[frame_idx];
	
	/* Remember frame order */
	order = frame->buddy_order;
	
	ASSERT(frame->refcount);
	
	if (!--frame->refcount) {
		buddy_system_free(zone->buddy_system, &frame->buddy_link);
		
		/* Update zone information. */
		zone->free_count += (1 << order);
		zone->busy_count -= (1 << order);
	}
}

/** Return frame from zone. */
 static frame_t *zone_get_frame(zone_t *zone, size_t frame_idx)
{
	ASSERT(frame_idx < zone->count);
	return &zone->frames[frame_idx];
}

/** Mark frame in zone unavailable to allocation. */
 static void zone_mark_unavailable(zone_t *zone, size_t frame_idx)
{
	link_t *link;
	frame_t *frame;
	ASSERT(zone_flags_available(zone->flags));
	
	frame = zone_get_frame(zone, frame_idx);
	if (frame->refcount)
		return;
	
	link = buddy_system_alloc_block(zone->buddy_system,
	    &frame->buddy_link);
	
	ASSERT(link);
	zone->free_count--;
}

/** Merge two zones.
 *
 * Expect buddy to point to space at least zone_conf_size large.
 * Assume z1 & z2 are locked and compatible and zones lock is
 * locked.
 *
 * @param z1     First zone to merge.
 * @param z2     Second zone to merge.
 * @param old_z1 Original date of the first zone.
 * @param buddy  Merged zone buddy.
 *
 */
 static void zone_merge_internal(size_t z1, size_t z2, zone_t *old_z1,
    buddy_system_t *buddy)
{
	size_t i;
	pfn_t base_diff;
	RAW_U8 order;
	pfn_t frame_idx;
	frame_t *frame;

	ASSERT(zone_flags_available(zones.info[z1].flags));
	ASSERT(zone_flags_available(zones.info[z2].flags));
	ASSERT(zones.info[z1].flags == zones.info[z2].flags);
	ASSERT(zones.info[z1].base < zones.info[z2].base);
	ASSERT(!overlaps(zones.info[z1].base, zones.info[z1].count,
	    zones.info[z2].base, zones.info[z2].count));
	
	/* Difference between zone bases */
	base_diff = zones.info[z2].base - zones.info[z1].base;
	
	zones.info[z1].count = base_diff + zones.info[z2].count;
	zones.info[z1].free_count += zones.info[z2].free_count;
	zones.info[z1].busy_count += zones.info[z2].busy_count;
	zones.info[z1].buddy_system = buddy;
	
	order = fnzb(zones.info[z1].count);
	buddy_system_create(zones.info[z1].buddy_system, order,
	    &zone_buddy_system_operations, (void *) &zones.info[z1]);
	
	zones.info[z1].frames =
	    (frame_t *) ((RAW_U8 *) zones.info[z1].buddy_system
	    + buddy_conf_size(order));
	
	/* This marks all frames busy */

	for (i = 0; i < zones.info[z1].count; i++)
		frame_initialize(&zones.info[z1].frames[i]);
	
	/* Copy frames from both zones to preserve full frame orders,
	 * parents etc. Set all free frames with refcount = 0 to 1, because
	 * we add all free frames to buddy allocator later again, clearing
	 * order to 0. Don't set busy frames with refcount = 0, as they
	 * will not be reallocated during merge and it would make later
	 * problems with allocation/free.
	 */
	for (i = 0; i < old_z1->count; i++)
		zones.info[z1].frames[i] = old_z1->frames[i];
	
	for (i = 0; i < zones.info[z2].count; i++)
		zones.info[z1].frames[base_diff + i]
		    = zones.info[z2].frames[i];
	
	i = 0;
	while (i < zones.info[z1].count) {
		if (zones.info[z1].frames[i].refcount) {
			/* Skip busy frames */
			i += 1 << zones.info[z1].frames[i].buddy_order;
		} else {
			/* Free frames, set refcount = 1
			 * (all free frames have refcount == 0, we need not
			 * to check the order)
			 */
			zones.info[z1].frames[i].refcount = 1;
			zones.info[z1].frames[i].buddy_order = 0;
			i++;
		}
	}
	
	/* Add free blocks from the original zone z1 */
	while (zone_can_alloc(old_z1, 0)) {
		/* Allocate from the original zone */
		pfn_t frame_idx = zone_frame_alloc(old_z1, 0);
		
		/* Free the frame from the merged zone */
		frame_t *frame = &zones.info[z1].frames[frame_idx];
		frame->refcount = 0;
		buddy_system_free(zones.info[z1].buddy_system, &frame->buddy_link);
	}
	
	/* Add free blocks from the original zone z2 */
	while (zone_can_alloc(&zones.info[z2], 0)) {
		/* Allocate from the original zone */
		frame_idx = zone_frame_alloc(&zones.info[z2], 0);
		
		/* Free the frame from the merged zone */
		frame = &zones.info[z1].frames[base_diff + frame_idx];
		frame->refcount = 0;
		buddy_system_free(zones.info[z1].buddy_system, &frame->buddy_link);
	}
}

/** Return old configuration frames into the zone.
 *
 * We have two cases:
 * - The configuration data is outside the zone
 *   -> do nothing (perhaps call frame_free?)
 * - The configuration data was created by zone_create
 *   or updated by reduce_region -> free every frame
 *
 * @param znum  The actual zone where freeing should occur.
 * @param pfn   Old zone configuration frame.
 * @param count Old zone frame count.
 *
 */
 static void return_config_frames(size_t znum, pfn_t pfn, size_t count)
{
	frame_t *frame;
	size_t i;
	size_t cframes;

	ASSERT(zone_flags_available(zones.info[znum].flags));
	
	cframes = SIZE2FRAMES(zone_conf_size(count));
	
	if ((pfn < zones.info[znum].base)
	    || (pfn >= zones.info[znum].base + zones.info[znum].count))
		return;
	


	frame = &zones.info[znum].frames[pfn - zones.info[znum].base];
	ASSERT(!frame->buddy_order);
	
	for (i = 0; i < cframes; i++) {
		zones.info[znum].busy_count++;
		zone_frame_free(&zones.info[znum],
		    pfn - zones.info[znum].base + i);
	}
}

/** Reduce allocated block to count of order 0 frames.
 *
 * The allocated block needs 2^order frames. Reduce all frames
 * in the block to order 0 and free the unneeded frames. This means that
 * when freeing the previously allocated block starting with frame_idx,
 * you have to free every frame.
 *
 * @param znum      Zone.
 * @param frame_idx Index the first frame of the block.
 * @param count     Allocated frames in block.
 *
 */
 static void zone_reduce_region(size_t znum, pfn_t frame_idx,
    size_t count)
{
	size_t i;
	RAW_U8 order;

	ASSERT(zone_flags_available(zones.info[znum].flags));
	ASSERT(frame_idx + count < zones.info[znum].count);
	
	order = zones.info[znum].frames[frame_idx].buddy_order;
	ASSERT((size_t) (1 << order) >= count);
	
	/* Reduce all blocks to order 0 */

	for (i = 0; i < (size_t) (1 << order); i++) {
		frame_t *frame = &zones.info[znum].frames[i + frame_idx];
		frame->buddy_order = 0;
		if (!frame->refcount)
			frame->refcount = 1;
		ASSERT(frame->refcount == 1);
	}
	
	/* Free unneeded frames */
	for (i = count; i < (size_t) (1 << order); i++)
		zone_frame_free(&zones.info[znum], i + frame_idx);
}

/** Merge zones z1 and z2.
 *
 * The merged zones must be 2 zones with no zone existing in between
 * (which means that z2 = z1 + 1). Both zones must be available zones
 * with the same flags.
 *
 * When you create a new zone, the frame allocator configuration does
 * not to be 2^order size. Once the allocator is running it is no longer
 * possible, merged configuration data occupies more space :-/
 *
 * The function uses
 *
 */
bool zone_merge(size_t z1, size_t z2)
{
	pfn_t cframes;
	bool ret = true;
	RAW_U8 order;
	pfn_t pfn;
	size_t i;
	zone_t old_z1;
	buddy_system_t *buddy;
	
	irq_spinlock_lock(&zones.lock, true);
	/* We can join only 2 zones with none existing inbetween,
	 * the zones have to be available and with the same
	 * set of flags
	 */
	if ((z1 >= zones.count) || (z2 >= zones.count)
	    || (z2 - z1 != 1)
	    || (!zone_flags_available(zones.info[z1].flags))
	    || (!zone_flags_available(zones.info[z2].flags))
	    || (zones.info[z1].flags != zones.info[z2].flags)) {
		ret = false;
		goto errout;
	}
	
	cframes = SIZE2FRAMES(zone_conf_size(
	    zones.info[z2].base - zones.info[z1].base
	    + zones.info[z2].count));
	                        
	if (cframes == 1)
		order = 0;
	else
		order = fnzb(cframes - 1) + 1;
	
	/* Allocate merged zone data inside one of the zones */
	if (zone_can_alloc(&zones.info[z1], order)) {
		pfn = zones.info[z1].base + zone_frame_alloc(&zones.info[z1], order);
	} else if (zone_can_alloc(&zones.info[z2], order)) {
		pfn = zones.info[z2].base + zone_frame_alloc(&zones.info[z2], order);
	} else {
		ret = false;
		goto errout;
	}
	
	/* Preserve original data from z1 */
	old_z1 = zones.info[z1];
	old_z1.buddy_system->data = (void *) &old_z1;
	
	/* Do zone merging */
	buddy = (buddy_system_t *)PFN2ADDR(pfn);
	zone_merge_internal(z1, z2, &old_z1, buddy);
	
	/* Free unneeded config frames */
	zone_reduce_region(z1, pfn - zones.info[z1].base, cframes);
	
	/* Subtract zone information from busy frames */
	zones.info[z1].busy_count -= cframes;
	
	/* Free old zone information */
	return_config_frames(z1,
	    ADDR2PFN((uintptr_t) old_z1.frames), old_z1.count);
	return_config_frames(z1,
	    ADDR2PFN((uintptr_t) zones.info[z2].frames),
	    zones.info[z2].count);
	
	/* Move zones down */

	for (i = z2 + 1; i < zones.count; i++) {
		zones.info[i - 1] = zones.info[i];
		if (zones.info[i - 1].buddy_system != NULL)
			zones.info[i - 1].buddy_system->data =
			    (void *) &zones.info[i - 1];
	}
	
	zones.count--;
	
errout:
	irq_spinlock_unlock(&zones.lock, true);
	
	return ret;
}

/** Merge all mergeable zones into one big zone.
 *
 * It is reasonable to do this on systems where
 * BIOS reports parts in chunks, so that we could
 * have 1 zone (it's faster).
 *
 */
void zone_merge_all(void)
{
	size_t i = 0;
	while (i < zones.count) {
		if (!zone_merge(i, i + 1))
			i++;
	}
}

/** Create new frame zone.
 *
 * @param zone  Zone to construct.
 * @param buddy Address of buddy system configuration information.
 * @param start Physical address of the first frame within the zone.
 * @param count Count of frames in zone.
 * @param flags Zone flags.
 *
 * @return Initialized zone.
 *
 */
 static void zone_construct(zone_t *zone, buddy_system_t *buddy,
    pfn_t start, size_t count, zone_flags_t flags)
{
	size_t i;
	RAW_U8 order;
	zone->base = start;
	zone->count = count;
	zone->flags = flags;
	zone->free_count = count;
	zone->busy_count = 0;
	zone->buddy_system = buddy;

	
	if (zone_flags_available(flags)) {
		/*
		 * Compute order for buddy system and initialize
		 */
		order = fnzb(count);
		buddy_system_create(zone->buddy_system, order,
		    &zone_buddy_system_operations, (void *) zone);
		
		/* Allocate frames _after_ the confframe */
		
		/* Check sizes */
		zone->frames = (frame_t *) ((RAW_U8 *) zone->buddy_system +
		    buddy_conf_size(order));
		
		for (i = 0; i < count; i++)
			frame_initialize(&zone->frames[i]);
		
		/* Stuffing frames */
		for (i = 0; i < count; i++) {
			zone->frames[i].refcount = 0;
			buddy_system_free(zone->buddy_system, &zone->frames[i].buddy_link);
		}
	} else
		zone->frames = NULL;
}

/** Compute configuration data size for zone.
 *
 * @param count Size of zone in frames.
 *
 * @return Size of zone configuration info (in bytes).
 *
 */
size_t zone_conf_size(size_t count)
{
	return (count * sizeof(frame_t) + buddy_conf_size(fnzb(count)));
}

/** Create and add zone to system.
 *
 * @param start     First frame number (absolute).
 * @param count     Size of zone in frames.
 * @param confframe Where configuration frames are supposed to be.
 *                  Automatically checks, that we will not disturb the
 *                  kernel and possibly init. If confframe is given
 *                  _outside_ this zone, it is expected, that the area is
 *                  already marked BUSY and big enough to contain
 *                  zone_conf_size() amount of data. If the confframe is
 *                  inside the area, the zone free frame information is
 *                  modified not to include it.
 *
 * @return Zone number or -1 on error.
 *
 */
size_t zone_create(pfn_t start, size_t count, pfn_t confframe,
    zone_flags_t flags)
{
	size_t confcount;
//	bool overlap;
//	size_t i;
	size_t znum;
	buddy_system_t *buddy;

	irq_spinlock_lock(&zones.lock, true);

	if (zone_flags_available(flags)) {  /* Create available zone */
		/* Theoretically we could have NULL here, practically make sure
		 * nobody tries to do that. If some platform requires, remove
		 * the assert
		 */
		ASSERT(confframe != ADDR2PFN((uintptr_t ) NULL));
		
		/* If confframe is supposed to be inside our zone, then make sure
		 * it does not span kernel & init
		 */
		confcount = SIZE2FRAMES(zone_conf_size(count));
#if 0
		if ((confframe >= start) && (confframe < start + count)) {
			for (; confframe < start + count; confframe++) {
				uintptr_t addr = PFN2ADDR(confframe);
				if (overlaps(addr, PFN2ADDR(confcount),
				    config.base, config.kernel_size))
					continue;
				
				if (overlaps(addr, PFN2ADDR(confcount),
				    config.stack_base, config.stack_size))
					continue;
				
				overlap = false;

				for (i = 0; i < init.cnt; i++)
					if (overlaps(addr, PFN2ADDR(confcount),
					    init.tasks[i].addr,
					    init.tasks[i].size)) {
						overlap = true;
						break;
					}
				if (overlap)
					continue;
				
				break;
			}
			
			if (confframe >= start + count)
				panic("Cannot find configuration data for zone.");
		}
#endif		
		znum = zones_insert_zone(start, count, flags);
		if (znum == (size_t) -1) {
			irq_spinlock_unlock(&zones.lock, true);
			return (size_t) -1;
		}
		
		buddy = (buddy_system_t *)PFN2ADDR(confframe);
		zone_construct(&zones.info[znum], buddy, start, count, flags);
		
		/* If confdata in zone, mark as unavailable */
		if ((confframe >= start) && (confframe < start + count)) {
			size_t i;
			for (i = confframe; i < confframe + confcount; i++)
				zone_mark_unavailable(&zones.info[znum],
				    i - zones.info[znum].base);
		}
		
		irq_spinlock_unlock(&zones.lock, true);
		
		return znum;
	}
	
	/* Non-available zone */
	znum = zones_insert_zone(start, count, flags);
	if (znum == (size_t) -1) {
		irq_spinlock_unlock(&zones.lock, true);
		return (size_t) -1;
	}
	zone_construct(&zones.info[znum], NULL, start, count, flags);
	
	irq_spinlock_unlock(&zones.lock, true);
	
	return znum;
}

/*******************/
/* Frame functions */
/*******************/

/** Set parent of frame. */
void frame_set_parent(pfn_t pfn, void *data, size_t hint)
{
	size_t znum;

	irq_spinlock_lock(&zones.lock, true);
	znum = find_zone(pfn, 1, hint);
	
	ASSERT(znum != (size_t) -1);
	
	zone_get_frame(&zones.info[znum],
	    pfn - zones.info[znum].base)->parent = data;
	
	irq_spinlock_unlock(&zones.lock, true);
}

void *frame_get_parent(pfn_t pfn, size_t hint)
{
	void *res;
	size_t znum;

	irq_spinlock_lock(&zones.lock, true);

	znum = find_zone(pfn, 1, hint);
	
	ASSERT(znum != (size_t) -1);
	
	res = zone_get_frame(&zones.info[znum],
	    pfn - zones.info[znum].base)->parent;
	
	irq_spinlock_unlock(&zones.lock, true);
	
	return res;
}

/** Allocate power-of-two frames of physical memory.
 *
 * @param order Allocate exactly 2^order frames.
 * @param flags Flags for host zone selection and address processing.
 * @param pzone Preferred zone.
 *
 * @return Physical address of the allocated frame.
 *
 */
void *frame_alloc_generic(RAW_U8 order, frame_flags_t flags, size_t *pzone)
{
	size_t freed;
	pfn_t pfn;
	size_t znum;
#ifdef CONFIG_DEBUG
	size_t avail;
#endif
	size_t size = ((size_t) 1) << order;
	size_t hint = pzone ? (*pzone) : 0;
	
loop:
	irq_spinlock_lock(&zones.lock, true);

	/*
	 * First, find suitable frame zone.
	 */
	znum = find_free_zone(order,
	    FRAME_TO_ZONE_FLAGS(flags), hint);

	/* If no memory, reclaim some slab memory,
	   if it does not help, reclaim all */
	if ((znum == (size_t) -1) && (!(flags & FRAME_NO_RECLAIM))) {
		irq_spinlock_unlock(&zones.lock, true);
		freed = slab_reclaim(0);
		irq_spinlock_lock(&zones.lock, true);
		
		if (freed > 0)
			znum = find_free_zone(order,
			    FRAME_TO_ZONE_FLAGS(flags), hint);
		
		if (znum == (size_t) -1) {
			irq_spinlock_unlock(&zones.lock, true);
			freed = slab_reclaim(SLAB_RECLAIM_ALL);
			irq_spinlock_lock(&zones.lock, true);
			
			if (freed > 0)
				znum = find_free_zone(order,
				    FRAME_TO_ZONE_FLAGS(flags), hint);
		}
	}
	
	if (znum == (size_t) -1) {
		if (flags & FRAME_ATOMIC) {
			irq_spinlock_unlock(&zones.lock, true);
			return NULL;
		}
		
#ifdef CONFIG_DEBUG
		size_t avail = total_frames_free();
#endif
		
		irq_spinlock_unlock(&zones.lock, true);
		
		goto loop;
	}
	
	pfn = zone_frame_alloc(&zones.info[znum], order)
	    + zones.info[znum].base;
	
	irq_spinlock_unlock(&zones.lock, true);
	
	if (pzone)
		*pzone = znum;
	
	if (flags & FRAME_KA)
		return (void *)PFN2ADDR(pfn);
	
	return (void *) PFN2ADDR(pfn);
}

/** Free a frame.
 *
 * Find respective frame structure for supplied physical frame address.
 * Decrement frame reference count. If it drops to zero, move the frame
 * structure to free list.
 *
 * @param frame Physical Address of of the frame to be freed.
 *
 */
void frame_free(uintptr_t frame)
{
	int msg = 0;
	pfn_t pfn;
	size_t znum;

	irq_spinlock_lock(&zones.lock, true);
	
	/*
	 * First, find host frame zone for addr.
	 */
	pfn = ADDR2PFN(frame);
	znum = find_zone(pfn, 1, 0);
	
	ASSERT(znum != (size_t) -1);
	
	zone_frame_free(&zones.info[znum], pfn - zones.info[znum].base);
	
	irq_spinlock_unlock(&zones.lock, true);
}

/** Add reference to frame.
 *
 * Find respective frame structure for supplied PFN and
 * increment frame reference count.
 *
 * @param pfn Frame number of the frame to be freed.
 *
 */
 void frame_reference_add(pfn_t pfn)
{
	size_t znum;

	irq_spinlock_lock(&zones.lock, true);
	/*
	 * First, find host frame zone for addr.
	 */
	znum = find_zone(pfn, 1, 0);
	
	ASSERT(znum != (size_t) -1);
	
	zones.info[znum].frames[pfn - zones.info[znum].base].refcount++;
	
	irq_spinlock_unlock(&zones.lock, true);
}

/** Mark given range unavailable in frame zones.
 *
 */
 void frame_mark_unavailable(pfn_t start, size_t count)
{
	size_t i;
	size_t znum;
	irq_spinlock_lock(&zones.lock, true);

	for (i = 0; i < count; i++) {
		znum = find_zone(start + i, 1, 0);
		if (znum == (size_t) -1)  /* PFN not found */
			continue;
		
		zone_mark_unavailable(&zones.info[znum],
		    start + i - zones.info[znum].base);
	}
	
	irq_spinlock_unlock(&zones.lock, true);
}


void frame_arch_init(RAW_U32 start, RAW_U32 size_frame)
{
	RAW_U32 base;
	RAW_U32 size;
	RAW_U32 new_base;
	RAW_U32 new_size;
	pfn_t pfn;
	pfn_t conf;
	size_t count;
	pfn_t minconf = 1;

	base =  start;
	size = size_frame;

	new_base = ALIGN_UP(base, FRAME_SIZE);
	new_size = ALIGN_DOWN(size - (new_base - base), FRAME_SIZE);
		
	pfn = ADDR2PFN(new_base);
	count = SIZE2FRAMES(new_size);

	if ((minconf < pfn) || (minconf >= (pfn + count)))
		conf = pfn;
	else
		conf = minconf;

	raw_printk("pfn = 0x%x; count = %d; conf = 0x%x\n", pfn, count, conf);
	zone_create(pfn, count, conf, ZONE_AVAILABLE);

	return;
}


/** Initialize physical memory management.
 *
 */
void frame_init(RAW_U32 start, RAW_U32 size)
{
//	size_t i;

	zones.count = 0;
	irq_spinlock_initialize(&zones.lock, "frame.zones.lock");
	
	/* Tell the architecture to create some memory */
	frame_arch_init(start, size);
#if 0
	frame_mark_unavailable(ADDR2PFN(config.base),
		    SIZE2FRAMES(config.kernel_size));
	frame_mark_unavailable(ADDR2PFN(config.stack_base),
		    SIZE2FRAMES(config.stack_size));
		

	for (i = 0; i < init.cnt; i++) {
		pfn_t pfn = ADDR2PFN(init.tasks[i].addr);
		frame_mark_unavailable(pfn,
			    SIZE2FRAMES(init.tasks[i].size));
	}
		
	if (ballocs.size)
		frame_mark_unavailable(ADDR2PFN(ballocs.base),
			    SIZE2FRAMES(ballocs.size));
		
	/* Black list first frame, as allocating NULL would
	* fail in some places
	*/
	frame_mark_unavailable(0, 1);
#endif
}

/** Return total size of all zones.
 *
 */
RAW_U64 zones_total_size(void)
{
	RAW_U64 total = 0;
	size_t i;

	irq_spinlock_lock(&zones.lock, true);
	for (i = 0; i < zones.count; i++)
		total += (RAW_U64) FRAMES2SIZE(zones.info[i].count);
	
	irq_spinlock_unlock(&zones.lock, true);
	
	return total;
}

void zones_stats(RAW_U64 *total, RAW_U64 *unavail, RAW_U64 *busy,
    RAW_U64 *free)
{
	size_t i;
	ASSERT(total != NULL);
	ASSERT(unavail != NULL);
	ASSERT(busy != NULL);
	ASSERT(free != NULL);
	
	irq_spinlock_lock(&zones.lock, true);
	
	*total = 0;
	*unavail = 0;
	*busy = 0;
	*free = 0;
	

	for (i = 0; i < zones.count; i++) {
		*total += (RAW_U64) FRAMES2SIZE(zones.info[i].count);
		
		if (zone_flags_available(zones.info[i].flags)) {
			*busy += (RAW_U64) FRAMES2SIZE(zones.info[i].busy_count);
			*free += (RAW_U64) FRAMES2SIZE(zones.info[i].free_count);
		} else
			*unavail += (RAW_U64) FRAMES2SIZE(zones.info[i].count);
	}
	
	irq_spinlock_unlock(&zones.lock, true);
}

/** Prints list of zones.
 *
 */
void zones_print_list(void)
{
	size_t i;
	bool available;
	uintptr_t base;
	size_t count;
	zone_flags_t flags;
	size_t free_count;
	size_t busy_count;
#ifdef __32_BITS__
	raw_printk("[nr] [base addr] [frames    ] [flags ] [free frames ] [busy frames ]\n");
#endif

#ifdef __64_BITS__
	raw_printk("[nr] [base address    ] [frames    ] [flags ] [free frames ] [busy frames ]\n");
#endif
	
	/*
	 * Because printing may require allocation of memory, we may not hold
	 * the frame allocator locks when printing zone statistics.  Therefore,
	 * we simply gather the statistics under the protection of the locks and
	 * print the statistics when the locks have been released.
	 *
	 * When someone adds/removes zones while we are printing the statistics,
	 * we may end up with inaccurate output (e.g. a zone being skipped from
	 * the listing).
	 */

	for (i = 0;; i++) {
		irq_spinlock_lock(&zones.lock, true);
		
		if (i >= zones.count) {
			irq_spinlock_unlock(&zones.lock, true);
			break;
		}
		
		base = PFN2ADDR(zones.info[i].base);
		count = zones.info[i].count;
		flags = zones.info[i].flags;
		free_count = zones.info[i].free_count;
		busy_count = zones.info[i].busy_count;
		
		irq_spinlock_unlock(&zones.lock, true);
		
		available = zone_flags_available(flags);
		
		raw_printk("%-4lu", i);
		
#ifdef __32_BITS__
		raw_printk("  %p", (void *) base);
#endif
		
#ifdef __64_BITS__
		raw_printk(" %p", (void *) base);
#endif
		
		raw_printk(" %12lu %c%c%c      ", count,
		    available ? 'A' : ' ',
		    (flags & ZONE_RESERVED) ? 'R' : ' ',
		    (flags & ZONE_FIRMWARE) ? 'F' : ' ');
		
		if (available)
			raw_printk("%14lu %14lu",
			    free_count, busy_count);
		
		raw_printk("\n");
	}
}

/** Prints zone details.
 *
 * @param num Zone base address or zone number.
 *
 */
void zone_print_one(size_t num)
{
	bool available;
	size_t znum = (size_t) -1;
	size_t i;
	uintptr_t base;
	zone_flags_t flags;
	size_t count;
	size_t free_count;
	size_t busy_count;

	irq_spinlock_lock(&zones.lock, true);
	for (i = 0; i < zones.count; i++) {
		if ((i == num) || (PFN2ADDR(zones.info[i].base) == num)) {
			znum = i;
			break;
		}
	}
	
	if (znum == (size_t) -1) {
		irq_spinlock_unlock(&zones.lock, true);
		raw_printk("Zone not found.\n");
		return;
	}
	
	base = PFN2ADDR(zones.info[i].base);
	flags = zones.info[i].flags;
	count = zones.info[i].count;
	free_count = zones.info[i].free_count;
	busy_count = zones.info[i].busy_count;
	
	irq_spinlock_unlock(&zones.lock, true);
	
	available = zone_flags_available(flags);
	
	raw_printk("Zone number:       %lu\n", znum);
	raw_printk("Zone base address: %p\n", (void *) base);
	raw_printk("Zone size:         %lu frames (%lu KiB)\n", count,
	    SIZE2KB(FRAMES2SIZE(count)));
	raw_printk("Zone flags:        %c%c%c\n",
	    available ? 'A' : ' ',
	    (flags & ZONE_RESERVED) ? 'R' : ' ',
	    (flags & ZONE_FIRMWARE) ? 'F' : ' ');
	
	if (available) {
		raw_printk("Allocated space:   %lu frames (%lu KiB)\n",
		    busy_count, SIZE2KB(FRAMES2SIZE(busy_count)));
		raw_printk("Available space:   %lu frames (%lu KiB)\n",
		    free_count, SIZE2KB(FRAMES2SIZE(free_count)));
	}
}

/** @}
 */
