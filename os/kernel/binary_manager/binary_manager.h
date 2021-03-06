/****************************************************************************
 *
 * Copyright 2019 Samsung Electronics All Rights Reserved.
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

#ifndef __SCHED_BINARY_MANAGER_BINARY_MANAGER_H
#define __SCHED_BINARY_MANAGER_BINARY_MANAGER_H

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <tinyara/config.h>
#include <queue.h>
#ifdef CONFIG_BINMGR_RECOVERY
#include <mqueue.h>
#endif

#include <tinyara/sched.h>
#include <tinyara/binary_manager.h>

#ifdef CONFIG_BINARY_MANAGER

/****************************************************************************
 * Pre-Processor Definitions
 ****************************************************************************/
/* Priority Range of Binary Manager Modules */
#define BM_PRIORITY_MAX            205                        /* The maximum priority of BM module */
#define BM_PRIORITY_MIN            200                        /* The minimum priority of BM module */

/* Fault Message Sender Thread information */
#define FAULTMSGSENDER_NAME        "bm_faultmsgsender"        /* Fault Message Sender thread name */
#define FAULTMSGSENDER_STACKSIZE   1024                       /* Fault Message Sender thread stack size */
#define FAULTMSGSENDER_PRIORITY    BM_PRIORITY_MAX            /* Fault Message Sender thread priority */

/* Binary Manager Core Thread information */
#define BINARY_MANAGER_NAME        "binary_manager"           /* Binary manager thread name */
#define BINARY_MANAGER_STACKSIZE   2048                       /* Binary manager thread stack size */
#define BINARY_MANAGER_PRIORITY    203                        /* Binary manager thread priority */

/* Loading Thread information */
#define LOADINGTHD_NAME            "bm_loader"                 /* Loading thread name */
#define LOADINGTHD_STACKSIZE       4096                        /* Loading thread stack size */
#define LOADINGTHD_PRIORITY        200                         /* Loading thread priority */

/* Supported binary types */
#define BIN_TYPE_BIN               0                          /* 'bin' type for kernel binary */
#define BIN_TYPE_ELF               1                          /* 'elf' type for user binary */

#define FILES_PER_BIN              2                          /* The number of files per binary */

#define CHECKSUM_SIZE              4
#define CRC_BUFFER_SIZE            512

#ifdef CONFIG_SUPPORT_COMMON_BINARY
/* bin id value of zero will indicate the library */
#define BM_BINID_LIBRARY	0
#endif

/* Index of 'Common Library' data in binary table. */
#define COMMLIB_IDX                0

/* The number of arguments for loading thread */
#define LOADTHD_ARGC               2

#define BINMGR_DEVNAME_FMT         "/dev/mtdblock%d"

#if (defined(CONFIG_BUILD_PROTECTED) || defined(CONFIG_BUILD_KERNEL)) && defined(CONFIG_MM_KERNEL_HEAP)
#define MAX_WAIT_COUNT             3                          /* The maximum number of times you can wait to process a delayed free */
#endif
#define BINMGR_LOADING_TRYCNT      2

/* Loading thread cmd types */
enum loading_thread_cmd {
	LOADCMD_LOAD = 0,
	LOADCMD_LOAD_ALL = 1,
	LOADCMD_UPDATE = 2,          /* Reload on update request */
#ifdef CONFIG_BINMGR_RECOVERY
	LOADCMD_RELOAD = 3,          /* Reload on recovery request */
#endif
	LOADCMD_LOAD_MAX,
};

/* Binary states */
enum binary_state_e {
	BINARY_UNREGISTERED = 0,     /* Partition is unregistered */
	BINARY_INACTIVE = 1,         /* Partition is registered, but binary is not loaded yet */
	BINARY_LOADING_DONE = 2,     /* Loading binary is done */
	BINARY_RUNNING = 3,          /* Loaded binary gets scheduling */
	BINARY_WAITUNLOAD = 4,       /* Loaded binary would be unloaded */
	BINARY_FAULT = 5,            /* Binary is excluded from scheduling and would be reloaded */
	BINARY_STATE_MAX,
};

/* Binary types */
enum binary_type_e {
	BINARY_TYPE_REALTIME = 0,
	BINARY_TYPE_NONREALTIME = 1,
	BINARY_TYPE_MAX,
};

#ifdef CONFIG_BINMGR_RECOVERY
struct faultmsg_s {
	struct faultmsg_s *flink;	/* Implements singly linked list */
	int binid;
};
typedef struct faultmsg_s faultmsg_t;
#endif

/* User binary data type in binary table */
struct binmgr_uinfo_s {
	pid_t bin_id;
	uint8_t state;
	uint8_t rttype;
	uint8_t rtcount;
	load_attr_t load_attr;
	char bin_ver[BIN_VER_MAX];
	char kernel_ver[KERNEL_VER_MAX];
	sq_queue_t cb_list; // list node type : statecb_node_t
#ifdef CONFIG_OPTIMIZE_APP_RELOAD_TIME
	struct binary_s *binp;
#endif
};
typedef struct binmgr_uinfo_s binmgr_uinfo_t;

/* Kernel binary data type in kernel table */
struct binmgr_kinfo_s {
	char name[BIN_NAME_MAX];
	uint8_t inuse_idx;
	uint32_t part_count;
	part_info_t part_info[KERNEL_BIN_COUNT];
	char version[KERNEL_VER_MAX];
};
typedef struct binmgr_kinfo_s binmgr_kinfo_t;

struct statecb_node_s {
	struct statecb_node_s *flink;
	int pid;
	binmgr_cb_t *cb_info;
};
typedef struct statecb_node_s statecb_node_t;

binmgr_uinfo_t *binary_manager_get_udata(uint32_t bin_idx);
#define BIN_ID(bin_idx)                                 binary_manager_get_udata(bin_idx)->bin_id
#define BIN_STATE(bin_idx)                              binary_manager_get_udata(bin_idx)->state
#define BIN_RTTYPE(bin_idx)                             binary_manager_get_udata(bin_idx)->rttype
#define BIN_RTCOUNT(bin_idx)                            binary_manager_get_udata(bin_idx)->rtcount

#define BIN_VER(bin_idx)                                binary_manager_get_udata(bin_idx)->bin_ver
#define BIN_KERNEL_VER(bin_idx)                         binary_manager_get_udata(bin_idx)->kernel_ver
#define BIN_CBLIST(bin_idx)                             binary_manager_get_udata(bin_idx)->cb_list

#define BIN_LOAD_ATTR(bin_idx)                          binary_manager_get_udata(bin_idx)->load_attr
#define BIN_NAME(bin_idx)                               binary_manager_get_udata(bin_idx)->load_attr.bin_name
#define BIN_SIZE(bin_idx)                               binary_manager_get_udata(bin_idx)->load_attr.bin_size
#define BIN_RAMSIZE(bin_idx)                            binary_manager_get_udata(bin_idx)->load_attr.ram_size
#define BIN_OFFSET(bin_idx)                             binary_manager_get_udata(bin_idx)->load_attr.offset
#define BIN_STACKSIZE(bin_idx)                          binary_manager_get_udata(bin_idx)->load_attr.stack_size
#define BIN_PRIORITY(bin_idx)                           binary_manager_get_udata(bin_idx)->load_attr.priority
#define BIN_COMPRESSION_TYPE(bin_idx)                   binary_manager_get_udata(bin_idx)->load_attr.compression_type
#ifdef CONFIG_OPTIMIZE_APP_RELOAD_TIME
#define BIN_LOADINFO(bin_idx)                           binary_manager_get_udata(bin_idx)->binp
#endif

/****************************************************************************
 * Function Prototypes
 ****************************************************************************/
#ifdef CONFIG_BINMGR_RECOVERY
/****************************************************************************
 * Name: binary_manager_recovery
 *
 * Description:
 *   This function will receive the faulty pid and check if its binary id is one
 *   of the registered binary with binary manager.
 *   If the binary is registered, it excludes its children from scheduling
 *   and creates loading thread which will terminate them and load binary again.
 *   Otherwise, board will be rebooted.
 *
 * Input parameters:
 *   pid   -   The pid of recovery message
 *
 * Returned Value:
 *   None
 *
 ****************************************************************************/
void binary_manager_recovery(int pid);
void binary_manager_deactivate_rtthreads(struct tcb_s *tcb);
void binary_manager_set_faultmsg_sender(pid_t pid);
int binary_manager_faultmsg_sender(int argc, char *argv[]);
mqd_t binary_manager_get_mqfd(void);
void binary_manager_recover_userfault(uint32_t assert_pc);
#endif

void binary_manager_add_binlist(FAR struct tcb_s *tcb);
void binary_manager_remove_binlist(FAR struct tcb_s *tcb);
void binary_manager_register_statecb(int pid, binmgr_cb_t *cb_info);
void binary_manager_unregister_statecb(int pid);
void binary_manager_clear_bin_statecb(int bin_idx);
int binary_manager_send_statecb_msg(int recv_binidx, char *bin_name, uint8_t state, bool need_response);
void binary_manager_notify_state_changed(int bin_idx, uint8_t state);
int binary_manager_loading(char *loading_data[]);
uint32_t binary_manager_get_ucount(void);
uint32_t binary_manager_get_kcount(void);
binmgr_kinfo_t *binary_manager_get_kdata(void);
int binary_manager_get_index_with_binid(int bin_id);
void binary_manager_get_info_with_name(int request_pid, char *bin_name);
void binary_manager_get_info_all(int request_pid);
void binary_manager_send_response(char *q_name, void *response_msg, int msg_size);
int binary_manager_register_ubin(char *name);
void binary_manager_scan_ubin(void);
int binary_manager_read_header(char *path, binary_header_t *header_data);
int binary_manager_create_entry(int requester_pid, char *bin_name, int version);
void binary_manager_release_binary_sem(int binid);

/****************************************************************************
 * Binary Manager Main Thread
 ****************************************************************************/
int binary_manager(int argc, char *argv[]);

#endif							/* CONFIG_BINARY_MANAGER */
#endif							/* __SCHED_BINARY_MANAGER_BINARY_MANAGER_H */
