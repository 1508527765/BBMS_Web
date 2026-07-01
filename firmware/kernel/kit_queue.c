/*****************************************************************************
 * @copyright       2000-2050,Copyright (c) 2025, Gary. All rights reserved.
 * @file            kit_queue.c
 * @brief           队列实现
 * @author          qiang
 * @date            2026-03-27
 * @remark
 *****************************************************************************/
#include "kit_queue.h"
#include <stdlib.h>
#include <string.h>
#include <errno.h>

int kit_queue_init(kit_queue_t *q, void *buffer, unsigned int capacity, unsigned int elem_size)
{
    if (!q || capacity == 0 || elem_size == 0)
        return -1;

    q->head = 0;
    q->tail = 0;
    q->count = 0;
    q->capacity = capacity;
    q->elem_size = elem_size;

    if (buffer)
    {
        q->buffer = buffer;
        q->dynamic = 0;
    }
    else
    {
        q->buffer = malloc(capacity * elem_size);
        if (!q->buffer)
            return -1;
        q->dynamic = 1;
    }

    if (pthread_mutex_init(&q->lock, NULL) != 0)
    {
        if (q->dynamic)
            free(q->buffer);
        return -1;
    }

    return 0;
}

void kit_queue_destroy(kit_queue_t *q)
{
    if (!q)
        return;
    pthread_mutex_destroy(&q->lock);
    if (q->dynamic && q->buffer)
        free(q->buffer);
    memset(q, 0, sizeof(*q)); /* 安全清理 */
}

/**
 * @brief 入队
 * @param q    队列指针
 * @param data 待入队元素的指针（大小为elem_size字节）
 * @return 0成功，-1失败（队列已满）
 */
int kit_queue_enqueue(kit_queue_t *q, const void *data)
{
    int ret = 0;
    pthread_mutex_lock(&q->lock);

    if (q->count >= q->capacity)
    {
        ret = -1;
        goto unlock;
    }

    /* 拷贝数据到缓冲区尾部 */
    memcpy((char *)q->buffer + q->tail * q->elem_size, data, q->elem_size);

    /* 更新尾指针和计数 */
    q->tail = (q->tail + 1) % q->capacity;
    q->count++;

unlock:
    pthread_mutex_unlock(&q->lock);
    return ret;
}

/**
 * @brief 出队
 * @param q    队列指针
 * @param data 用于存放取出元素的数据缓冲区（大小为elem_size字节）
 * @return 0成功，-1失败（队列为空）
 */
int kit_queue_dequeue(kit_queue_t *q, void *data)
{
    int ret = 0;

    pthread_mutex_lock(&q->lock);

    if (q->count == 0)
    {
        ret = -1;
        goto unlock;
    }

    /* 从缓冲区头部拷贝数据 */
    memcpy(data, (char *)q->buffer + q->head * q->elem_size, q->elem_size);

    /* 更新头指针和计数 */
    q->head = (q->head + 1) % q->capacity;
    q->count--;

unlock:
    pthread_mutex_unlock(&q->lock);
    return ret;
}

/**
 * @brief 检查队列是否为空
 * @param q 队列指针
 * @return 1为空，0非空
 */
int kit_queue_is_empty(kit_queue_t *q)
{
    int empty;

    pthread_mutex_lock(&q->lock);
    empty = (q->count == 0);
    pthread_mutex_unlock(&q->lock);

    return empty;
}

int kit_queue_is_full(kit_queue_t *q)
{
    int full;

    pthread_mutex_lock(&q->lock);
    full = (q->count == q->capacity);
    pthread_mutex_unlock(&q->lock);

    return full;
}

unsigned int kit_queue_count(kit_queue_t *q)
{
    unsigned int cnt;

    pthread_mutex_lock(&q->lock);
    cnt = q->count;
    pthread_mutex_unlock(&q->lock);

    return cnt;
}