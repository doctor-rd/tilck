/* SPDX-License-Identifier: BSD-2-Clause */

#include <tilck/common/basic_defs.h>
#include <tilck/common/string_util.h>

#include <tilck/kernel/debug_utils.h>
#include <tilck/kernel/process.h>
#include <tilck/kernel/timer.h>
#include <tilck/kernel/cmdline.h>

#include <elf.h>         // system header
#include <multiboot.h>   // system header in include/system_headers

volatile bool __in_panic;
volatile bool __in_double_fault;

/*
 * NOTE: this flag affect affect sched_alive_thread() only when it is actually
 * running. By default it does *not* run. It gets activated only by the kernel
 * cmdline option -sat.
 */
static volatile bool sched_alive_thread_enabled = true;

#if KERNEL_SELFTESTS
#define MAX_NO_DEADLOCK_SET_ELEMS   144

static int no_deadlock_set_elems;
static int no_deadlock_set[MAX_NO_DEADLOCK_SET_ELEMS];
static u64 no_deadlock_set_progress[MAX_NO_DEADLOCK_SET_ELEMS];
static u64 no_deadlock_set_progress_old[MAX_NO_DEADLOCK_SET_ELEMS];

void debug_reset_no_deadlock_set(void)
{
   disable_preemption();
   {
      no_deadlock_set_elems = 0;
      bzero(no_deadlock_set, sizeof(no_deadlock_set));
      bzero(no_deadlock_set_progress, sizeof(no_deadlock_set_progress));
      bzero(no_deadlock_set_progress_old, sizeof(no_deadlock_set_progress_old));
   }
   enable_preemption();
}

void debug_add_task_to_no_deadlock_set(int tid)
{
   disable_preemption();
   {
      for (int i = 0; i < no_deadlock_set_elems; i++) {
         VERIFY(no_deadlock_set[i] != tid);
      }

      VERIFY(no_deadlock_set_elems < (int)ARRAY_SIZE(no_deadlock_set));
      no_deadlock_set[no_deadlock_set_elems++] = tid;
   }
   enable_preemption();
}

static int nds_find_pos(int tid)
{
   ASSERT(!is_preemption_enabled());

   for (int i = 0; i < no_deadlock_set_elems; i++) {
      if (no_deadlock_set[i] == tid)
         return i;
   }

   return -1;
}

void debug_remove_task_from_no_deadlock_set(int tid)
{
   int pos;

   disable_preemption();
   {
      pos = nds_find_pos(tid);

      if (pos < 0)
         panic("Task %d not found in no_deadlock_set", tid);

      no_deadlock_set[pos] = 0;
   }
   enable_preemption();
}

void debug_no_deadlock_set_report_progress(void)
{
   int tid = get_curr_tid();
   int pos;

   disable_preemption();
   {
      pos = nds_find_pos(tid);
      VERIFY(pos >= 0);
      no_deadlock_set_progress[pos]++;
   }
   enable_preemption();
}

void debug_check_for_deadlock(void)
{
   bool found_runnable = false;
   struct task *ti;
   int tid, candidates = 0;

   disable_preemption();
   {
      for (int i = 0; i < no_deadlock_set_elems; i++) {

         if (!(tid = no_deadlock_set[i]))
            continue;

         if (!(ti = get_task(tid)))
            continue;

         candidates++;

         if (ti->state == TASK_STATE_RUNNABLE) {
            found_runnable = true;
            break;
         }
      }
   }
   enable_preemption();

   if (candidates > 0 && !found_runnable) {
      panic("No runnable task found in no_deadlock_set [%d elems]", candidates);
   }
}

void debug_check_for_any_progress(void)
{
   int tid;
   int counter = 0, candidates = 0;
   struct task *ti;

   disable_preemption();
   {
      for (int i = 0; i < no_deadlock_set_elems; i++) {

         if (!(tid = no_deadlock_set[i]))
            continue;

         if (!(ti = get_task(tid)))
            continue;

         if (ti->state == TASK_STATE_ZOMBIE)
            continue;

         if (!no_deadlock_set_progress[i])
            continue;

         if (no_deadlock_set_progress[i] == no_deadlock_set_progress_old[i])
            panic("[deadlock?] No progress for tid %d", tid);

         candidates++;
      }

      if (candidates) {

         printk("Progress for the first 4 tasks: [\n");

         for (int i = 0; i < no_deadlock_set_elems && counter < 4; i++) {

            if (!(tid = no_deadlock_set[i]))
               continue;

            if (!(ti = get_task(tid)))
               continue;

            if (ti->state == TASK_STATE_ZOMBIE)
               continue;

            if (!no_deadlock_set_progress[i])
               continue;

            if (i == 0)
               printk("%llu ", no_deadlock_set_progress[i]);
            else
               printk(NO_PREFIX "%llu ", no_deadlock_set_progress[i]);

            counter++;
         }

         printk(NO_PREFIX "\n");
         printk("]\n\n");
      }

      memcpy(no_deadlock_set_progress_old,
             no_deadlock_set_progress,
             sizeof(no_deadlock_set_progress));
   }
   enable_preemption();
}

#endif

static void sched_alive_thread()
{
   for (int counter = 0; ; counter++) {

      if (sched_alive_thread_enabled) {
         printk("---- Sched alive thread: %d ----\n", counter);

         if (counter % 2) {
            debug_check_for_deadlock();
            debug_check_for_any_progress();
         }
      }

      kernel_sleep(TIMER_HZ);
   }
}

void init_extra_debug_features()
{
   if (kopt_sched_alive_thread)
      if (kthread_create(&sched_alive_thread, 0, NULL) < 0)
         panic("Unable to create a kthread for sched_alive_thread()");
}

void set_sched_alive_thread_enabled(bool enabled)
{
   sched_alive_thread_enabled = enabled;
}


#if SLOW_DEBUG_REF_COUNT

/*
 * Set here the address of the ref_count to track.
 */
void *debug_refcount_obj = (void *)0;

/* Return the new value */
int __retain_obj(int *ref_count)
{
   int ret;
   ATOMIC(int) *atomic = (ATOMIC(int) *)ref_count;
   ret = atomic_fetch_add_explicit(atomic, 1, mo_relaxed) + 1;

   if (!debug_refcount_obj || ref_count == debug_refcount_obj) {
      printk(COLOR_GREEN "refcount at %p: %d -> %d" RESET_ATTRS "\n",
             ref_count, ret-1, ret);
   }

   return ret;
}

/* Return the new value */
int __release_obj(int *ref_count)
{
   int old, ret;
   ATOMIC(int) *atomic = (ATOMIC(int) *)ref_count;
   old = atomic_fetch_sub_explicit(atomic, 1, mo_relaxed);
   ASSERT(old > 0);
   ret = old - 1;

   if (!debug_refcount_obj || ref_count == debug_refcount_obj) {
      printk(COLOR_RED "refcount at %p: %d -> %d" RESET_ATTRS "\n",
             ref_count, old, ret);
   }

   return ret;
}
#endif
