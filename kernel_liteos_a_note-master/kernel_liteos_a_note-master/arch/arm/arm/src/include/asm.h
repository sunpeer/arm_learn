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

#ifndef __ASM_H
#define __ASM_H

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */

#define FUNCTION(x) .global x; .text; .code 32; x:
#define MPIDR_CPUID_MASK 0xFFU

/*
 *           ------------------------------------------
 * stackTop |   cpu n   |  ... |   cpu 1   |   cpu 0   | stackBottom
 *           ------------------------------------------
 *          | stackSize |  ... | stackSize | stackSize |
 */
.macro EXC_SP_SET stackBottom, stackSize, reg0, reg1
    mrc    p15, 0, \reg0, c0, c0, 5        @获取CPU信息
    and    \reg0, \reg0, #MPIDR_CPUID_MASK /* get cpu id */ @获取当前运行CPUID
    mov    \reg1, #\stackSize			   @reg1 记录 栈大小
    mul    \reg1, \reg1, \reg0             /* calculate current cpu stack offset */ @计算当前CPU栈的偏移位置
    ldr    \reg0, =\stackBottom			   @reg0 记录栈底
    sub    \reg0, \reg0, \reg1             /* calculate current cpu stack bottom */ @相减得到栈顶
    mov    sp, \reg0                       /* set  sp */ @设置SP,将SP移到栈顶
.endm

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */

#endif /* __ASM_H */
