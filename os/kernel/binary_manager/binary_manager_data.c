/****************************************************************************
 *
 * Copyright 2020 Samsung Electronics All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND,
 * either express or implied. See the License for the specific
 * language governing permissions and limitations under the License.
 *
 ****************************************************************************/
/****************************************************************************
 * Included Files
 ****************************************************************************/
#include <tinyara/config.h>
#include <debug.h>
#include <string.h>
#include <queue.h>
#include <tinyara/sched.h>
#include <tinyara/binary_manager.h>

#include "sched/sched.h"
#include "binary_manager/binary_manager.h"

/****************************************************************************
 * Private Definitions
 ****************************************************************************/
/* Kernel version (not implemented, it will be modified) */
#ifdef CONFIG_VERSION_STRING
#define KERNEL_VER               CONFIG_VERSION_STRING
#else
#define KERNEL_VER               "2.0"
#endif

/* Table for User binaries */
/* Binary table, the first data [0] is for Common Library. */
static binmgr_uinfo_t bin_table[USER_BIN_COUNT + 1];
static uint32_t g_bin_count;

/* Data for Kernel partitions */
static binmgr_kinfo_t kernel_info;

/****************************************************************************
 * Public Functions
 ****************************************************************************/
/****************************************************************************
 * Name: binary_manager_get_ucount
 *
 * Description:
 *	 This function gets the number of user binaries.
 *
 ****************************************************************************/
uint32_t binary_manager_get_ucount(void)
{
	return g_bin_count;
}

/****************************************************************************
 * Name: binary_manager_get_kcount
 *
 * Description:
 *	 This function gets the number of partitions for kernel.
 *
 ****************************************************************************/
uint32_t binary_manager_get_kcount(void)
{
	return kernel_info.part_count;
}

/****************************************************************************
 * Name: binary_manager_get_udata
 *
 * Description:
 *	 This function gets a row of binary table with bin_idx.
 *
 ****************************************************************************/
binmgr_uinfo_t *binary_manager_get_udata(uint32_t bin_idx)
{
	return &bin_table[bin_idx];
}

/****************************************************************************
 * Name: binary_manager_get_kdata
 *
 * Description:
 *	 This function gets a kernel data.
 *
 ****************************************************************************/
binmgr_kinfo_t *binary_manager_get_kdata(void)
{
	return &kernel_info;
}

/****************************************************************************
 * Name: binary_manager_register_ubin
 *
 * Description:
 *	 This function registers user binaries.
 *
 ****************************************************************************/
int binary_manager_register_ubin(char *name)
{
	int bin_idx;

	if (name == NULL || g_bin_count >= USER_BIN_COUNT) {
		bmdbg("ERROR: Invalid parameter\n");
		return ERROR;
	}

	for (bin_idx = 1; bin_idx <= g_bin_count; bin_idx++) {
		/* Already Registered */
		if (!strncmp(BIN_NAME(bin_idx), name, strlen(name) + 1)) {
			bmdbg("Already registered for binary %s\n", BIN_NAME(bin_idx));
			return ERROR;
		}
	}

	/* If partition is not registered, Register it as a new user partition */
	g_bin_count++;
	BIN_ID(g_bin_count) = -1;
	BIN_RTCOUNT(g_bin_count) = 0;
	BIN_STATE(g_bin_count) = BINARY_INACTIVE;
	strncpy(BIN_NAME(g_bin_count), name, BIN_NAME_MAX);
	sq_init(&BIN_CBLIST(g_bin_count));

	bmvdbg("[USER %d] %s\n", g_bin_count, BIN_NAME(g_bin_count));

	return g_bin_count;
}

/****************************************************************************
 * Name: binary_manager_register_kpart
 *
 * Description:
 *	 This function registers partitions of kernel binaries.
 *
 ****************************************************************************/
void binary_manager_register_kpart(int part_num, int part_size)
{
	int part_count;

	if (part_num < 0 || part_size <= 0 || kernel_info.part_count >= KERNEL_BIN_COUNT) {
		bmdbg("ERROR: Invalid part info : num %d, size %d\n", part_num, part_size);
		return;
	}

	part_count = kernel_info.part_count;
	if (part_count == 0) {
		strncpy(kernel_info.name, "kernel", BIN_NAME_MAX);
		strncpy(kernel_info.version, KERNEL_VER, KERNEL_VER_MAX);
	}
	kernel_info.part_info[part_count].part_size = part_size;
	kernel_info.part_info[part_count].part_num = part_num;
	kernel_info.part_count++;

	bmvdbg("[KERNEL %d] part num %d size %d\n", part_count, part_num, part_size);
}

/****************************************************************************
 * Name: binary_manager_update_running_state
 *
 * Description:
 *	 This function update binary state to BINARY_RUNNING state.
 *   And notify other binaries changed state.
 *
 ****************************************************************************/
void binary_manager_update_running_state(int bin_id)
{
	int bin_idx;

	if (bin_id <= 0) {
		bmdbg("Invalid parameter: bin id %d\n", bin_id);
		return;
	}

	bin_idx = binary_manager_get_index_with_binid(bin_id);
	if (bin_idx < 0) {
		bmdbg("Failed to get index of binary %d\n", bin_id);
		return;
	}

	BIN_STATE(bin_idx) = BINARY_RUNNING;
	bmvdbg("binary '%s' state is changed, state = %d.\n", BIN_NAME(bin_idx), BIN_STATE(bin_idx));

	/* Notify that binary is started. */
	binary_manager_notify_state_changed(bin_idx, BINARY_STARTED);
}

/****************************************************************************
 * Name: binary_manager_add_binlist
 *
 * Description:
 *	 This function adds tcb to binary list.
 *
 ****************************************************************************/
void binary_manager_add_binlist(FAR struct tcb_s *tcb)
{
	struct tcb_s *rtcb;
	struct tcb_s *next;

	rtcb = this_task();
	next = rtcb->bin_flink;
	tcb->bin_blink = rtcb;
	tcb->bin_flink = next;
	if (next) next->bin_blink = tcb;
	rtcb->bin_flink = tcb;
}

/****************************************************************************
 * Name: binary_manager_remove_binlist
 *
 * Description:
 *	 This function removes tcb from binary list.
 *
 ****************************************************************************/
void binary_manager_remove_binlist(FAR struct tcb_s *tcb)
{
	struct tcb_s *prev;
	struct tcb_s *next;

	prev = tcb->bin_blink;
	next = tcb->bin_flink;
	if (prev) prev->bin_flink = next;
	if (next) next->bin_blink = prev;
}
