/*
 * Copyright (c) 2013-2019, Huawei Technologies Co., Ltd. All rights reserved.
 * Copyright (c) 2020, Huawei Device Co., Ltd. All rights reserved.
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

#include "los_sys_pri.h"
#include "los_sched_pri.h"

/*!
 * @file  los_sys.c
 * @brief  系统时间转化
 * @details 
 * @link  kernel-small-basic-time http://weharmonyos.com/openharmony/zh-cn/device-dev/kernel/kernel-small-basic-time.html @endlink
 * @verbatim 
	基本概念 
		时间管理以系统时钟为基础，给应用程序提供所有和时间有关的服务。

		系统时钟是由定时器/计数器产生的输出脉冲触发中断产生的，一般定义为整数或长整数。
		输出脉冲的周期叫做一个“时钟滴答”。系统时钟也称为时标或者Tick。

		用户以秒、毫秒为单位计时，而操作系统以Tick为单位计时，当用户需要对系统进行操作时，
		例如任务挂起、延时等，此时需要时间管理模块对Tick和秒/毫秒进行转换。
		时间管理模块提供时间转换、统计、延迟功能

	相关概念	
		Cycle	
		系统最小的计时单位。Cycle的时长由系统主时钟频率决定，系统主时钟频率就是每秒钟的Cycle数。

		Tick	
		Tick是操作系统的基本时间单位，由用户配置的每秒Tick数决定。

	使用场景	
		用户需要了解当前系统运行的时间以及Tick与秒、毫秒之间的转换关系等。 

	时间管理的典型开发流程	
		根据实际需求，在板级配置适配时确认是否使能LOSCFG_BASE_CORE_TICK_HW_TIME宏选择外部定时器，
		并配置系统主时钟频率OS_SYS_CLOCK（单位Hz）。OS_SYS_CLOCK的默认值基于硬件平台配置。
		通过make menuconfig配置LOSCFG_BASE_CORE_TICK_PER_SECOND。

	参考	
		http://weharmonyos.com/openharmony/zh-cn/device-dev/kernel/kernel-small-basic-time.html
	@endverbatim 
 *	@attention 
	 获取系统Tick数需要在系统时钟使能之后。
	 时间管理不是单独的功能模块，依赖于los_config.h中的OS_SYS_CLOCK和LOSCFG_BASE_CORE_TICK_PER_SECOND两个配置选项。
	 系统的Tick数在关中断的情况下不进行计数，故系统Tick数不能作为准确时间计算。
 */

#define OS_MAX_VALUE    0xFFFFFFFFUL

/**
 * @brief 获取自系统启动以来的Tick数
 * 
 * @return LITE_OS_SEC_TEXT_MINOR 
 */
LITE_OS_SEC_TEXT_MINOR UINT64 LOS_TickCountGet(VOID)
{
    return OsGetCurrSchedTimeCycle() / OS_CYCLE_PER_TICK;
}
/**
 * @brief 每个Tick多少Cycle数
 * 
 * @return LITE_OS_SEC_TEXT_MINOR 
 */
LITE_OS_SEC_TEXT_MINOR UINT32 LOS_CyclePerTickGet(VOID)
{
    return g_sysClock / LOSCFG_BASE_CORE_TICK_PER_SECOND;
}
/**
 * @brief 毫秒转换成Tick
 * 
 * @param millisec 
 * @return LITE_OS_SEC_TEXT_MINOR 
 */
LITE_OS_SEC_TEXT_MINOR UINT32 LOS_MS2Tick(UINT32 millisec)
{
    if (millisec == OS_MAX_VALUE) {
        return OS_MAX_VALUE;
    }

    return ((UINT64)millisec * LOSCFG_BASE_CORE_TICK_PER_SECOND) / OS_SYS_MS_PER_SECOND;
}
/**
 * @brief Tick转化为毫秒
 * 
 * @param tick 
 * @return LITE_OS_SEC_TEXT_MINOR 
 */
LITE_OS_SEC_TEXT_MINOR UINT32 LOS_Tick2MS(UINT32 tick)
{
    return ((UINT64)tick * OS_SYS_MS_PER_SECOND) / LOSCFG_BASE_CORE_TICK_PER_SECOND;
}
/**
 * @brief 纳秒转化成 tick
 * 
 * @param nanoseconds 
 * @return LITE_OS_SEC_TEXT_MINOR 
 */
LITE_OS_SEC_TEXT_MINOR UINT32 OsNS2Tick(UINT64 nanoseconds)
{
    const UINT32 nsPerTick = OS_SYS_NS_PER_SECOND / LOSCFG_BASE_CORE_TICK_PER_SECOND;

    UINT64 ticks = (nanoseconds + nsPerTick - 1) / nsPerTick;
    if (ticks > OS_MAX_VALUE) {
        ticks = OS_MAX_VALUE;
    }
    return (UINT32)ticks;
}

