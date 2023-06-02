/* SPDX-License-Identifier: BSD-2-Clause */

DEF_3(wrap, assert_failed, void, const char *, const char *, int)
DEF_2(wrap, not_reached, void, const char *, int)
DEF_2(wrap, not_implemented, void, const char *, int)
DEF_3(wrap, tilck_vprintk, void, u32, const char *, va_list)
DEF_1(wrap, kmutex_lock, void, struct kmutex *)
DEF_1(wrap, kmutex_unlock, void, struct kmutex *)
DEF_2(wrap, fat_ramdisk_prepare_for_mmap, int, void *, size_t)
DEF_1(wrap, wth_create_thread_for, int, void *)
DEF_1(wrap, wth_wakeup, void, void *)
DEF_0(wrap, check_in_irq_handler, void)
DEF_2(wrap, general_kmalloc, void *, size_t *, u32)
DEF_3(wrap, general_kfree, void, void *, size_t *, u32)
DEF_1(wrap, kmalloc_get_first_heap, void *, size_t *)
DEF_0(real, experiment_bar, bool)
DEF_1(real, experiment_foo, int, int)
DEF_2(real, vfs_dup, int, fs_handle, fs_handle *)
DEF_1(real, vfs_close, void, fs_handle)
DEF_1(real, handle_sys_trace_arg, int , const char *);
DEF_4(real, copy_str_from_user, int, void *, const void *, size_t, size_t *);
DEF_3(real, copy_from_user, int, void *, const void *, size_t);
