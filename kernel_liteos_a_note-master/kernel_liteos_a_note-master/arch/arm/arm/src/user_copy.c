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

#include "user_copy.h"
#include "arm_user_copy.h"
#include "arm_user_clear.h"
#include "securec.h"
#include "los_memory.h"
#include "los_vm_map.h"



/**
 * @brief 
 * @verbatim
    从用户空间拷贝到内核空间
    请思考个问题,系统调用时为什么一定要copy_from_user ? 为什么不直接用memcpy？
    https://mp.weixin.qq.com/s/H3nXlOpP_XyF7M-1B4qreQ
 * @endverbatim
 * @param dst 
 * @param src 
 * @param len 
 * @return size_t 
 */
size_t arch_copy_from_user(void *dst, const void *src, size_t len)
{
    return LOS_ArchCopyFromUser(dst, src, len);
}

size_t LOS_ArchCopyFromUser(void *dst, const void *src, size_t len)
{
    if (!LOS_IsUserAddressRange((VADDR_T)(UINTPTR)src, len)) {//[src,src+len]在内核空间
        return len;
    }

    return _arm_user_copy(dst, src, len);//完成从用户空间到内核空间的拷贝
}
///拷贝到用户空间
size_t arch_copy_to_user(void *dst, const void *src, size_t len)
{
    return LOS_ArchCopyToUser(dst, src, len);//
}

/**
 * @brief 从内核空间拷贝到用户空间
 * @param dst 必须在用户空间
 * @param src 必须在内核空间
 * @param len 
 * @return size_t 
 */
size_t LOS_ArchCopyToUser(void *dst, const void *src, size_t len)
{//先判断地址是不是在用户空间
    if (!LOS_IsUserAddressRange((VADDR_T)(UINTPTR)dst, len)) {//[dest,dest+count] 不在用户空间
        return len;//必须在用户空间
    }

    return _arm_user_copy(dst, src, len);//完成从内核空间到用户空间的拷贝
}
///将内核数据拷贝到用户空间
INT32 LOS_CopyFromKernel(VOID *dest, UINT32 max, const VOID *src, UINT32 count)
{
    INT32 ret;

    if (!LOS_IsUserAddressRange((VADDR_T)(UINTPTR)dest, count)) {//[dest,dest+count] 不在用户空间
        ret = memcpy_s(dest, max, src, count);
    } else {//[dest,dest+count] 在用户空间
        ret = ((max >= count) ? _arm_user_copy(dest, src, count) : ERANGE_AND_RESET);//用户空间copy
    }

    return ret;
}
///将用户空间的数据拷贝到内核空间
INT32 LOS_CopyToKernel(VOID *dest, UINT32 max, const VOID *src, UINT32 count)
{
    INT32 ret;

    if (!LOS_IsUserAddressRange((vaddr_t)(UINTPTR)src, count)) {//[src,src+count] 在内核空间的情况
        ret = memcpy_s(dest, max, src, count);
    } else {//[src,src+count] 在内核空间的情况
        ret = ((max >= count) ? _arm_user_copy(dest, src, count) : ERANGE_AND_RESET);
    }

    return ret;
}
///清除用户空间数据
INT32 LOS_UserMemClear(unsigned char *buf, UINT32 len)
{
    INT32 ret = 0;
    if (!LOS_IsUserAddressRange((vaddr_t)(UINTPTR)buf, len)) {//[buf,buf+len] 不在用户空间
        (VOID)memset_s(buf, len, 0, len);//清0
    } else {//在用户空间
        if (_arm_clear_user(buf, len)) {
            return -EFAULT;
        }
    }
    return ret;
}


