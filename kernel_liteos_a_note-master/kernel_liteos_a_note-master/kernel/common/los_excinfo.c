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

#include "los_base.h"
#include "los_hwi.h"
#ifdef LOSCFG_SHELL
#include "shcmd.h"
#endif
#ifdef LOSCFG_FS_VFS
#include "fs/fs.h"
#include "fs/fs_operation.h"
#endif

#ifdef LOSCFG_SAVE_EXCINFO
STATIC log_read_write_fn g_excInfoRW = NULL; /* the hook of read-writing exception information */	//读写异常信息的钩子函数
STATIC CHAR *g_excInfoBuf = NULL;            /* pointer to the buffer for storing the exception information */	//指向存储异常信息的缓冲区的指针
STATIC UINT32 g_excInfoIndex = 0xFFFFFFFF;   /* the index of the buffer for storing the exception information */	//用于存储异常信息的缓冲区的索引
STATIC UINT32 g_recordAddr = 0;              /* the address of storing the exception information */ //存储异常信息的地址
STATIC UINT32 g_recordSpace = 0;             /* the size of storing the exception information */	//存储异常信息的大小
//设置异常信息的读写函数
VOID SetExcInfoRW(log_read_write_fn func)
{
    g_excInfoRW = func;
}
///获取异常信息读写函数
log_read_write_fn GetExcInfoRW(VOID)
{
    return g_excInfoRW;
}
///设置异常信息的缓存
VOID SetExcInfoBuf(CHAR *buf)
{
    g_excInfoBuf = buf;
}
///获取异常信息的缓存
CHAR *GetExcInfoBuf(VOID)
{
    return g_excInfoBuf;
}
///设置异常信息索引位
VOID SetExcInfoIndex(UINT32 index)
{
    g_excInfoIndex = index;
}
///获取异常信息索引位
UINT32 GetExcInfoIndex(VOID)
{
    return g_excInfoIndex;
}

VOID SetRecordAddr(UINT32 addr)
{
    g_recordAddr = addr;
}

UINT32 GetRecordAddr(VOID)
{
    return g_recordAddr;
}

VOID SetRecordSpace(UINT32 space)
{
    g_recordSpace = space;
}

UINT32 GetRecordSpace(VOID)
{
    return g_recordSpace;
}
///vsnprintf 为C标准库可变参数的实现函数 见于 ..\third_party\musl\kernel\src\stdio\vsnprintf.c
VOID WriteExcBufVa(const CHAR *format, va_list arglist)
{
    errno_t ret;

    if (g_recordSpace > g_excInfoIndex) {
        ret = vsnprintf_s((g_excInfoBuf + g_excInfoIndex), (g_recordSpace - g_excInfoIndex),
                          (g_recordSpace - g_excInfoIndex - 1), format, arglist);
        if (ret == -1) {
            PRINT_ERR("exc info buffer is not enough or vsnprintf_s is error.\n");
            return;
        }
        g_excInfoIndex += ret;
    }
}
///写异常信息到系统异常信息中心
VOID WriteExcInfoToBuf(const CHAR *format, ...)
{
    va_list arglist;//va_arg
    va_start(arglist, format);//从任务栈中取出入栈参数
    WriteExcBufVa(format, arglist);//入栈参数列表作为实参传入交由vsnprintf处理
    va_end(arglist);//释放资源
}
///用于注册记录异常信息函数，并指定位置、空间和大小
VOID LOS_ExcInfoRegHook(UINT32 startAddr, UINT32 space, CHAR *buf, log_read_write_fn hook)
{
    if ((hook == NULL) || (buf == NULL)) {
        PRINT_ERR("Buf or hook is null.\n");
        return;
    }

    g_recordAddr = startAddr;
    g_recordSpace = space;
    g_excInfoBuf = buf;
    g_excInfoRW = hook;

#ifdef LOSCFG_FS_VFS
    los_vfs_init();//初始化虚拟文件系统
#endif
}

/* Be called in the exception. */ //异常发生时回调这里
VOID OsReadWriteExceptionInfo(UINT32 startAddr, UINT32 space, UINT32 flag, CHAR *buf)
{
    if ((buf == NULL) || (space == 0)) {
        PRINT_ERR("buffer is null or space is zero\n");
        return;
    }
    // user can write exception information to files here
}
///记录异常信息产生的时间
VOID OsRecordExcInfoTime(VOID)
{
#ifdef LOSCFG_FS_VFS
#define NOW_TIME_LENGTH 24
    time_t t;
    struct tm *tmTime = NULL;
    CHAR nowTime[NOW_TIME_LENGTH];

    (VOID)time(&t);
    tmTime = localtime(&t);//获取本地时间的标准C库函数
    if (tmTime == NULL) {
        return;
    }
    (VOID)memset_s(nowTime, sizeof(nowTime), 0, sizeof(nowTime));
    (VOID)strftime(nowTime, NOW_TIME_LENGTH, "%Y-%m-%d %H:%M:%S", tmTime);//生成时间格式
#undef NOW_TIME_LENGTH
    WriteExcInfoToBuf("%s \n", nowTime);
#endif
}

#ifdef LOSCFG_SHELL
INT32 OsShellCmdReadExcInfo(INT32 argc, CHAR **argv)
{
#define EXCINFO_ALIGN_SIZE 64
    UINT32 recordSpace = GetRecordSpace();

    (VOID)argc;
    (VOID)argv;

    CHAR *buf = (CHAR *)LOS_MemAllocAlign((VOID *)OS_SYS_MEM_ADDR, recordSpace + 1, EXCINFO_ALIGN_SIZE);
    if (buf == NULL) {
        return LOS_NOK;
    }
    (VOID)memset_s(buf, recordSpace + 1, 0, recordSpace + 1);

    log_read_write_fn hook = GetExcInfoRW();
    if (hook != NULL) {
        hook(GetRecordAddr(), recordSpace, 1, buf);
    }
    PRINTK("%s\n", buf);
    (VOID)LOS_MemFree((void *)OS_SYS_MEM_ADDR, buf);
    buf = NULL;
    return LOS_OK;
}

SHELLCMD_ENTRY(readExcInfo_shellcmd, CMD_TYPE_EX, "excInfo", 0, (CmdCallBackFunc)OsShellCmdReadExcInfo);
#endif

#endif

