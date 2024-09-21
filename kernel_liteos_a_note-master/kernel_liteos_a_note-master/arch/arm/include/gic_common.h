/*!
 * @file    gic_common.h
 * @brief GIC（Generic Interrupt Controller）是ARM公司提供的一个通用的中断控制器
 * @link http://weharmonyos.com/blog/44.html
 * @verbatim
 * @endverbatim
 * @version 
 * @author  weharmonyos.com | 鸿蒙研究站 | 每天死磕一点点
 * @date    2021-11-18
 *
 * @history
 *
 */
 
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

#ifndef _GIC_COMMON_H
#define _GIC_COMMON_H

#include "stdint.h"
#include "target_config.h"
#include "los_config.h"

/* gic arch revision | GIC架构版本*/
enum {
    GICV1 = 1,
    GICV2,
    GICV3,
    GICV4
};

#define GIC_REV_MASK                    0xF0
#define GIC_REV_OFFSET                  0x4
/// 前往  http://weharmonyos.com 下载 对应datasheet 
#ifdef LOSCFG_ARCH_GIC_V2
#define GICC_CTLR                       (GICC_OFFSET + 0x00)            /* CPU Interface Control Register | 控制寄存器，控制是否上报中断到处理器 */
#define GICC_PMR                        (GICC_OFFSET + 0x04)            /* Interrupt Priority Mask Register | 中断优先级屏蔽寄存器*/
#define GICC_BPR                        (GICC_OFFSET + 0x08)            /* Binary Point Register */
#define GICC_IAR                        (GICC_OFFSET + 0x0c)            /* Interrupt Acknowledge Register | 中断回应寄存器，处理器会读取此寄存器，用来获取上报的中断号*/
#define GICC_EOIR                       (GICC_OFFSET + 0x10)            /* End of Interrupt Register | 处理器会写这个寄存器来告知CPU interface，此中断已经处理完成*/
#define GICC_RPR                        (GICC_OFFSET + 0x14)            /* Running Priority Register | 运行优先级寄存器，此寄存器的值表示当前CPU interface的运行优先级*/
#define GICC_HPPIR                      (GICC_OFFSET + 0x18)            /* Highest Priority Pending Interrupt Register | 当前pending状态的最高优先级中断号，当在允许中断抢占的情况下，如果此中断的优先级值大于运行优先级寄存器的值的话，就会发生中断抢占*/
#endif

#define GICD_CTLR                       (GICD_OFFSET + 0x000)           /* Distributor Control Register */
#define GICD_TYPER                      (GICD_OFFSET + 0x004)           /* Interrupt Controller Type Register */
#define GICD_IIDR                       (GICD_OFFSET + 0x008)           /* Distributor Implementer Identification Register */
#define GICD_IGROUPR(n)                 (GICD_OFFSET + 0x080 + (n) * 4) /* Interrupt Group Registers */
#define GICD_ISENABLER(n)               (GICD_OFFSET + 0x100 + (n) * 4) /* Interrupt Set-Enable Registers */
#define GICD_ICENABLER(n)               (GICD_OFFSET + 0x180 + (n) * 4) /* Interrupt Clear-Enable Registers */
#define GICD_ISPENDR(n)                 (GICD_OFFSET + 0x200 + (n) * 4) /* Interrupt Set-Pending Registers */
#define GICD_ICPENDR(n)                 (GICD_OFFSET + 0x280 + (n) * 4) /* Interrupt Clear-Pending Registers */
#define GICD_ISACTIVER(n)               (GICD_OFFSET + 0x300 + (n) * 4) /* GICv2 Interrupt Set-Active Registers */
#define GICD_ICACTIVER(n)               (GICD_OFFSET + 0x380 + (n) * 4) /* Interrupt Clear-Active Registers */
#define GICD_IPRIORITYR(n)              (GICD_OFFSET + 0x400 + (n) * 4) /* Interrupt Priority Registers */
#define GICD_ITARGETSR(n)               (GICD_OFFSET + 0x800 + (n) * 4) /* Interrupt Processor Targets Registers */
#define GICD_ICFGR(n)                   (GICD_OFFSET + 0xc00 + (n) * 4) /* Interrupt Configuration Registers */
#define GICD_SGIR                       (GICD_OFFSET + 0xf00)           /* Software Generated Interrupt Register */
#define GICD_CPENDSGIR(n)               (GICD_OFFSET + 0xf10 + (n) * 4) /* SGI Clear-Pending Registers; NOT available on cortex-a9 */
#define GICD_SPENDSGIR(n)               (GICD_OFFSET + 0xf20 + (n) * 4) /* SGI Set-Pending Registers; NOT available on cortex-a9 */
#define GICD_PIDR2V2                    (GICD_OFFSET + 0xfe8)
#define GICD_PIDR2V3                    (GICD_OFFSET + 0xffe8)

#ifdef LOSCFG_ARCH_GIC_V3
#define GICD_IGRPMODR(n)                (GICD_OFFSET + 0x0d00 + (n) * 4) /* Interrupt Group Mode Reisters */
#define GICD_IROUTER(n)                 (GICD_OFFSET + 0x6000 + (n) * 8) /* Interrupt Rounter Reisters */
#endif

#define GIC_REG_8(reg)                  (*(volatile UINT8 *)((UINTPTR)(GIC_BASE_ADDR + (reg))))
#define GIC_REG_32(reg)                 (*(volatile UINT32 *)((UINTPTR)(GIC_BASE_ADDR + (reg))))
#define GIC_REG_64(reg)                 (*(volatile UINT64 *)((UINTPTR)(GIC_BASE_ADDR + (reg))))

#define GICD_INT_DEF_PRI                0xa0U
#define GICD_INT_DEF_PRI_X4             (((UINT32)GICD_INT_DEF_PRI << 24) | \
                                         ((UINT32)GICD_INT_DEF_PRI << 16) | \
                                         ((UINT32)GICD_INT_DEF_PRI << 8)  | \
                                         (UINT32)GICD_INT_DEF_PRI)

#define GIC_MIN_SPI_NUM                 32

/* Interrupt preemption config */
#define GIC_PRIORITY_MASK               0xFFU
#define GIC_PRIORITY_OFFSET             8

/*
 * The number of bits to shift for an interrupt priority is dependent
 * on the number of bits implemented by the interrupt controller.
 * If the MAX_BINARY_POINT_VALUE is 7,
 * it means that interrupt preemption is not supported.
 */
#ifndef LOSCFG_ARCH_INTERRUPT_PREEMPTION
#define MAX_BINARY_POINT_VALUE              7
#define PRIORITY_SHIFT                      0
#define GIC_MAX_INTERRUPT_PREEMPTION_LEVEL  0U ///< 中断最高优先级
#else
#define PRIORITY_SHIFT                      ((MAX_BINARY_POINT_VALUE + 1) % GIC_PRIORITY_OFFSET)
#define GIC_MAX_INTERRUPT_PREEMPTION_LEVEL  ((UINT8)((GIC_PRIORITY_MASK + 1) >> PRIORITY_SHIFT))
#endif

#define GIC_INTR_PRIO_MASK                  ((UINT8)(0xFFFFFFFFU << PRIORITY_SHIFT))

/*
 * The preemption level is up to 128, and the maximum value corresponding to the interrupt priority is 254 [7:1].
 * If the GIC_MAX_INTERRUPT_PREEMPTION_LEVEL is 0, the minimum priority is 0xff.
 */
#define MIN_INTERRUPT_PRIORITY              ((UINT8)((GIC_MAX_INTERRUPT_PREEMPTION_LEVEL - 1) << PRIORITY_SHIFT)) ///< 中断最低优先级

#endif
