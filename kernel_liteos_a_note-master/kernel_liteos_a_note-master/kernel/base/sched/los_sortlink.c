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

#include "los_sortlink_pri.h"
/// 排序链表初始化
VOID OsSortLinkInit(SortLinkAttribute *sortLinkHeader)
{
    LOS_ListInit(&sortLinkHeader->sortLink);
    LOS_SpinInit(&sortLinkHeader->spinLock);
    sortLinkHeader->nodeNum = 0;
}

/*!
 * @brief OsAddNode2SortLink 向链表中插入结点,并按时间顺序排列	
 *
 * @param sortLinkHeader 被插入的链表	
 * @param sortList	要插入的结点
 * @return	
 *
 * @see
 */
STATIC INLINE VOID AddNode2SortLink(SortLinkAttribute *sortLinkHeader, SortLinkList *sortList)
{
    LOS_DL_LIST *head = (LOS_DL_LIST *)&sortLinkHeader->sortLink; //获取双向链表 

    if (LOS_ListEmpty(head)) { //空链表,直接插入
        LOS_ListHeadInsert(head, &sortList->sortLinkNode);//插入结点
        sortLinkHeader->nodeNum++;//CPU的工作量增加了
        return;
    }
	//链表不为空时,插入分三种情况, responseTime 大于,等于,小于的处理
    SortLinkList *listSorted = LOS_DL_LIST_ENTRY(head->pstNext, SortLinkList, sortLinkNode);
    if (listSorted->responseTime > sortList->responseTime) {//如果要插入的节点 responseTime 最小 
        LOS_ListAdd(head, &sortList->sortLinkNode);//能跑进来说明是最小的,直接插入到第一的位置
        sortLinkHeader->nodeNum++;//CPU的工作量增加了
        return;//直接返回了
    } else if (listSorted->responseTime == sortList->responseTime) {//相等的情况
        LOS_ListAdd(head->pstNext, &sortList->sortLinkNode);//插到第二的位置
        sortLinkHeader->nodeNum++;
        return;
    }
	//处理大于链表中第一个responseTime的情况,需要遍历链表
    LOS_DL_LIST *prevNode = head->pstPrev;//注意这里用的前一个结点,也就是说前一个结点中的responseTime 是最大的
    do { // @note_good 这里写的有点妙,也是双向链表的魅力所在
        listSorted = LOS_DL_LIST_ENTRY(prevNode, SortLinkList, sortLinkNode);//一个个遍历,先比大的再比小的
        if (listSorted->responseTime <= sortList->responseTime) {//如果时间比你小,就插到后面
            LOS_ListAdd(prevNode, &sortList->sortLinkNode);
            sortLinkHeader->nodeNum++;
            break;
        }

        prevNode = prevNode->pstPrev;//再拿上一个更小的responseTime进行比较
    } while (1);//死循环
}

VOID OsAdd2SortLink(SortLinkAttribute *head, SortLinkList *node, UINT64 responseTime, UINT16 idleCpu)
{
    LOS_SpinLock(&head->spinLock);
    SET_SORTLIST_VALUE(node, responseTime);
    AddNode2SortLink(head, node);
#ifdef LOSCFG_KERNEL_SMP
    node->cpuid = idleCpu;
#endif
    LOS_SpinUnlock(&head->spinLock);
}

VOID OsDeleteFromSortLink(SortLinkAttribute *head, SortLinkList *node)
{
    LOS_SpinLock(&head->spinLock);
    if (node->responseTime != OS_SORT_LINK_INVALID_TIME) {
        OsDeleteNodeSortLink(head, node);
    }
    LOS_SpinUnlock(&head->spinLock);
}

UINT32 OsSortLinkAdjustNodeResponseTime(SortLinkAttribute *head, SortLinkList *node, UINT64 responseTime)
{
    UINT32 ret = LOS_NOK;

    LOS_SpinLock(&head->spinLock);
    if (node->responseTime != OS_SORT_LINK_INVALID_TIME) {
        OsDeleteNodeSortLink(head, node);
        SET_SORTLIST_VALUE(node, responseTime);
        AddNode2SortLink(head, node);
        ret = LOS_OK;
    }
    LOS_SpinUnlock(&head->spinLock);
    return ret;
}

UINT64 OsSortLinkGetTargetExpireTime(UINT64 currTime, const SortLinkList *targetSortList)
{
    if (currTime >= targetSortList->responseTime) {
        return 0;
    }

    return (UINT32)(targetSortList->responseTime - currTime);
}

UINT64 OsSortLinkGetNextExpireTime(UINT64 currTime, const SortLinkAttribute *sortLinkHeader)
{
    LOS_DL_LIST *head = (LOS_DL_LIST *)&sortLinkHeader->sortLink;

    if (LOS_ListEmpty(head)) {
        return OS_SORT_LINK_INVALID_TIME;
    }

    SortLinkList *listSorted = LOS_DL_LIST_ENTRY(head->pstNext, SortLinkList, sortLinkNode);
    return OsSortLinkGetTargetExpireTime(currTime, listSorted);
}

