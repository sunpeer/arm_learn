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

#include "los_seq_buf.h"
#include "los_typedef.h"
#include <stdlib.h>
#include "securec.h"

static int ExpandSeqBuf(struct SeqBuf *seqBuf, size_t oldCount)//扩展buf
{
    char *newBuf = NULL;
    int ret;

    if ((seqBuf == NULL) || (seqBuf->buf == NULL)) {
        return -LOS_NOK;
    }

    if (seqBuf->size >= SEQBUF_LIMIT_SIZE) {
        goto EXPAND_FAILED;
    }

    newBuf = (char*)malloc(seqBuf->size <<= 1);//将现有buf扩大一倍
    if (newBuf == NULL) {
        goto EXPAND_FAILED;
    }
    (void)memset_s(newBuf + oldCount, seqBuf->size - oldCount, 0, seqBuf->size - oldCount);
	//memset_s 注意 oldCount 位置,因为 newBuf头部要被旧buf覆盖,所以只set后部分
    ret = memcpy_s(newBuf, seqBuf->size, seqBuf->buf, oldCount);//拷贝旧buf数据到新buf,注意 oldCount 位置
    if (ret != LOS_OK) {
        free(newBuf);
        goto EXPAND_FAILED;
    }
    seqBuf->count = oldCount;//目前偏移量,可理解为seek 

    free(seqBuf->buf);//释放原有的buf
    seqBuf->buf = newBuf;//采用新的buf

    return LOS_OK;
EXPAND_FAILED:
    free(seqBuf->buf);
    seqBuf->buf = NULL;
    seqBuf->count = 0;
    seqBuf->size = 0;

    return -LOS_NOK;
}
///创建seq buf
struct SeqBuf *LosBufCreat(void)
{
    struct SeqBuf *seqBuf = NULL;

    seqBuf = (struct SeqBuf *)malloc(sizeof(struct SeqBuf));
    if (seqBuf == NULL) {
        errno = -LOS_ENOMEM;
        return NULL;
    }
    (void)memset_s(seqBuf, sizeof(struct SeqBuf), 0, sizeof(struct SeqBuf));

    return seqBuf;
}
///真正写 buf 函数,调整 size/count的值,count可理解为偏移位
int LosBufVprintf(struct SeqBuf *seqBuf, const char *fmt, va_list argList)
{
    bool needreprintf = FALSE;
    int bufLen;

    if (seqBuf == NULL) {
        return -LOS_EPERM;
    }

    if (seqBuf->buf == NULL) {
        seqBuf->size = SEQBUF_PAGE_SIZE;
        seqBuf->buf = (char *)malloc(seqBuf->size);
        if (seqBuf->buf == NULL) {
            return -LOS_ENOMEM;
        }
        (void)memset_s(seqBuf->buf, seqBuf->size, 0, seqBuf->size);
        seqBuf->count = 0;
    }

    do {
        bufLen = vsnprintf_s(seqBuf->buf + seqBuf->count, seqBuf->size - seqBuf->count,
                             seqBuf->size - seqBuf->count - 1, fmt, argList);
        if (bufLen >= 0) {
            /* succeed write. */
            seqBuf->count += bufLen;//成功写入,count要增长
            return 0;
        }
        if (seqBuf->buf[seqBuf->count] == '\0') {//这里没看懂,为啥要有这个判断, @note_thinking
            free(seqBuf->buf);
            seqBuf->buf = NULL;
            break;
        }

        needreprintf = TRUE;

        if (ExpandSeqBuf(seqBuf, seqBuf->count) != 0) {
            break;
        }
    } while (needreprintf);

    return -LOS_NOK;
}
///支持可变参数 写 buf
int LosBufPrintf(struct SeqBuf *seqBuf, const char *fmt, ...)
{
    va_list argList;
    int ret;

    va_start(argList, fmt);//可变参数的实现,有点意思.
    ret = LosBufVprintf(seqBuf, fmt, argList);
    va_end(argList);

    return ret;
}
///释放 seq buf
int LosBufRelease(struct SeqBuf *seqBuf)
{
    if (seqBuf == NULL) {
        return -LOS_EPERM;
    }

    if (seqBuf->buf != NULL) {
        free(seqBuf->buf);
        seqBuf->buf = NULL;
    }
    free(seqBuf);

    return LOS_OK;
}
