/*
 * Copyright (c) 2013-2019 Huawei Technologies Co., Ltd. All rights reserved.
 * Copyright (c) 2020-2021 Huawei Device Co., Ltd. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this list of
 *    conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice, this list
 *    of conditions and the following disclaimer in the documentation and/or other materials
 *    provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its contributors may be used
 *    to endorse or promote products derived from this software without specific prior written
 *    permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "los_tick_pri.h"
#include "los_swtmr_pri.h"
#include "los_sched_pri.h"
#ifdef LOSCFG_KERNEL_VDSO
#include "los_vdso.h"
#endif


LITE_OS_SEC_DATA_INIT UINT32 g_sysClock;		///< 系统时钟,是绝大部分部件工作的时钟源，也是其他所有外设的时钟的来源 
LITE_OS_SEC_DATA_INIT UINT32 g_tickPerSecond;	///< 每秒Tick数,鸿蒙默认是每秒100次,即:10ms
LITE_OS_SEC_BSS DOUBLE g_cycle2NsScale;			///< 周期转纳秒级

/* spinlock for task module */
LITE_OS_SEC_BSS SPIN_LOCK_INIT(g_tickSpin); ///< 节拍器自旋锁

/*
 * Description : Tick interruption handler | 节拍中断处理函数 ,鸿蒙默认1ms触发一次
 */
LITE_OS_SEC_TEXT VOID OsTickHandler(VOID)
{
#ifdef LOSCFG_SCHED_TICK_DEBUG 
    OsSchedDebugRecordData();
#endif

#ifdef LOSCFG_KERNEL_VDSO
    OsVdsoTimevalUpdate();//更新vdso数据页时间,vdso可以直接在用户进程空间绕过系统调用获取系统时间(例如:gettimeofday)
#endif

#ifdef LOSCFG_BASE_CORE_TICK_HW_TIME
    HalClockIrqClear(); /* diff from every platform */
#endif

    OsSchedTick();//由时钟发起的调度
}

