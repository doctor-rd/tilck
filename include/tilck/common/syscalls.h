/* SPDX-License-Identifier: BSD-2-Clause */
#pragma once

#define TILCK_TESTCMD_SYSCALL    499

enum tilck_testcmd_type {
   TILCK_TESTCMD_RUN_SELFTEST       = 0,
   TILCK_TESTCMD_GCOV_GET_NUM_FILES = 1,
   TILCK_TESTCMD_GCOV_FILE_INFO     = 2,
   TILCK_TESTCMD_GCOV_GET_FILE      = 3,
   TILCK_TESTCMD_QEMU_POWEROFF      = 4,
   TILCK_TESTCMD_SET_SAT_ENABLED    = 5
};
