/*
 * Copyright : (C) 2022 Phytium Information Technology, Inc. 
 * All Rights Reserved.
 *  
 * This program is OPEN SOURCE software: you can redistribute it and/or modify it  
 * under the terms of the Phytium Public License as published by the Phytium Technology Co.,Ltd,  
 * either version 1.0 of the License, or (at your option) any later version. 
 *  
 * This program is distributed in the hope that it will be useful,but WITHOUT ANY WARRANTY;  
 * without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the Phytium Public License for more details. 
 *  
 * 
 * FilePath: fslink_list.h
 * Date: 2021-12-02 13:18:02
 * LastEditTime: 2022-02-17 18:02:59
 * Description:  This file is for singal link list definition
 * 
 * Modify History: 
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0   zhugengyu  2021/12/2    init
 */

#ifndef FSLINK_LIST_H
#define FSLINK_LIST_H

#include "ftypes.h"
#include "fkernel.h"

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * Single List structure
 */
struct FSListNode_
{
    struct FSListNode_ *next;                         /**< point to next node. */
};
typedef struct FSListNode_ FSListNode;                /**< Type for single list. */

/**
 * @name: FSListInit
 * @msg: initialize a single list
 * @return {*}
 * @param {FSListNode} *l the single list to be initialized
 */
static void FSListInit(FSListNode *l)
{
    l->next = NULL;
}

/**
 * @name: FSListAppend
 * @msg: append node at the tail of list
 * @return {*}
 * @param {FSListNode} *l the single list
 * @param {FSListNode} *n the node to append
 */
static void FSListAppend(FSListNode *l, FSListNode *n)
{
    FSListNode *node;

    node = l;
    while (node->next) node = node->next;

    /* append the node to the tail */
    node->next = n;
    n->next = NULL;
}

/**
 * @name: FSListInsert
 * @msg: insert node at list after l node
 * @return {*}
 * @param {FSListNode} *l the single list node at list
 * @param {FSListNode} *n the node to append
 */
static void FSListInsert(FSListNode *l, FSListNode *n)
{
    n->next = l->next;
    l->next = n;
}

/**
 * @name: FSListLen
 * @msg: get length of single list
 * @return {*}
 * @param {FSListNode} *l the single list
 */
static unsigned int FSListLen(const FSListNode *l)
{
    unsigned int len = 0;
    const FSListNode *list = l->next;
    while (list != NULL)
    {
        list = list->next;
        len ++;
    }

    return len;
}

/**
 * @name: FSListFirst
 * @msg: get first node of single list
 * @return {*}
 * @param {FSListNode} *l the single list
 */
static FSListNode *FSListFirst(FSListNode *l)
{
    return l->next;
}

/**
 * @name: FSListFirst
 * @msg: get last node of single list
 * @return {*}
 * @param {FSListNode} *l the single list
 */
static FSListNode *FSListTail(FSListNode *l)
{
    while (l->next) l = l->next;

    return l;
}

/**
 * @name: FSListFirst
 * @msg: get next node of single list
 * @return {*}
 * @param {FSListNode} *l the single list
 */
static FSListNode *FSListNext(FSListNode *n)
{
    return n->next;
}

/**
 * @name: FSListIsEmpty
 * @msg: check if single list is empty
 * @return {*}
 * @param {FSListNode} *l the single list
 */
static boolean FSListIsEmpty(FSListNode *l)
{
    return l->next == NULL;
}

/**
 * @name: FSListRemove
 * @msg: remove node from single list
 * @return {*}
 * @param {FSListNode} *l the single list
 * @param {FSListNode} *n node to remove
 */
static FSListNode *FSListRemove(FSListNode *l, FSListNode *n)
{
    /* remove slist head */
    FSListNode *node = l;
    while (node->next && node->next != n) node = node->next;

    /* remove node */
    if (node->next != (FSListNode *)0) node->next = node->next->next;

    return l;
}

/**
 * @brief get the struct for this single list node
 * @param node the entry point
 * @param type the type of structure
 * @param member the name of list in structure
 */
#define FSLIST_ENTRY(node, type, member) \
        CONTAINER_OF(node, type, member)

/**
 * FSLIST_FOR_EACH - iterate over a single list
 * @pos:    the FSListNode * to use as a loop cursor.
 * @head:   the head for your single list.
 */
#define FSLIST_FOR_EACH(pos, head) \
    for (pos = (head)->next; pos != NULL; pos = pos->next)

/**
 * FSLIST_FOR_EACH_ENTRY  -   iterate over single list of given type
 * @pos:    the type * to use as a loop cursor.
 * @head:   the head for your single list.
 * @member: the name of the list_struct within the struct.
 */
#define FSLIST_FOR_EACH_ENTRY(pos, head, member) \
    for (pos = FSLIST_ENTRY((head)->next, typeof(*pos), member); \
         &pos->member != (NULL); \
         pos = FSLIST_ENTRY(pos->member.next, typeof(*pos), member))

/**
 * FSLIST_FOR_FIRST_ENTRY - get the first element from a slist
 * @ptr:    the slist head to take the element from.
 * @type:   the type of the struct this is embedded in.
 * @member: the name of the slist_struct within the struct.
 *
 * Note, that slist is expected to be not empty.
 */
#define FSLIST_FOR_FIRST_ENTRY(ptr, type, member) \
    FSLIST_ENTRY((ptr)->next, type, member)

/**
 * FSLIST_TAIL_ENTRY - get the tail element from a slist
 * @ptr:    the slist head to take the element from.
 * @type:   the type of the struct this is embedded in.
 * @member: the name of the slist_struct within the struct.
 *
 * Note, that slist is expected to be not empty.
 */
#define FSLIST_TAIL_ENTRY(ptr, type, member) \
    FSLIST_ENTRY(FSListTail(ptr), type, member)

#ifdef __cplusplus
}
#endif

#endif // !