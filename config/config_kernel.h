/* SPDX-License-Identifier: BSD-2-Clause */

/*
 * This is a TEMPLATE. The actual config file is generated by CMake and stored
 * in <BUILD_DIR>/tilck_gen_headers/.
 */

#pragma once
#include <tilck_gen_headers/config_global.h>

/* --------- Boolean config variables --------- */
#cmakedefine01 KRN_NO_SYS_WARN
#cmakedefine01 KERNEL_FORCE_TC_ISYSTEM
#cmakedefine01 TINY_KERNEL
#cmakedefine01 KERNEL_GCOV

/*
 * --------------------------------------------------------------------------
 *                  Hard-coded global & derived constants
 * --------------------------------------------------------------------------
 *
 * Here below there are some pseudo-constants not designed to be easily changed
 * because of the code makes assumptions about them. Because of that, those
 * constants are hard-coded and not available as CMake variables. With time,
 * some of those constants get "promoted" and moved in CMake, others remain
 * here. See the comments and think about the potential implications before
 * promoting a hard-coded constant to a configurable CMake variable.
 */

/* Don't touch this: set KERNEL_STACK_PAGES instead */
#define KERNEL_STACK_SIZE          (KERNEL_STACK_PAGES << PAGE_SHIFT)

/* We careful here */
#define KMALLOC_MAX_ALIGN                   (64 * KB)
#define KMALLOC_MIN_HEAP_SIZE       KMALLOC_MAX_ALIGN

#define PROCESS_CMDLINE_BUF_SIZE                  256
#define MAX_MOUNTPOINTS                            16
#define MAX_NESTED_INTERRUPTS                      32

#define WTH_MAX_THREADS                            64
#define WTH_MAX_PRIO_QUEUE_SIZE                    32
#define WTH_KB_QUEUE_SIZE                          32
#define WTH_SERIAL_QUEUE_SIZE                      32
