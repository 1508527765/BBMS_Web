/*****************************************************************************
 * @copyright       1997-2050, Gary. POWER SUPPLY CO., LTD.
 * @file            kit_queue.h
 * @brief           队列结构体
 * @author          gary
 * @date            2026-03-27
 * @remark
 *****************************************************************************/
#ifndef __KIT_QUEUE_H__
#define __KIT_QUEUE_H__

#include <stddef.h>
#include <pthread.h>

/* 队列结构体 */
typedef struct
{
    void *buffer;           /* 缓冲区起始地址 */
    unsigned int head;      /* 读索引（出队位置） */
    unsigned int tail;      /* 写索引（入队位置） */
    unsigned int count;     /* 当前元素个数 */
    unsigned int capacity;  /* 缓冲区容量（最大元素个数） */
    unsigned int elem_size; /* 每个元素的大小（字节） */
    pthread_mutex_t lock;   /* 互斥锁 */
    int dynamic;            /* 缓冲区是否动态分配（用于销毁时释放） */
} kit_queue_t;

/**
 * @brief 初始化队列
 * @param q         队列指针
 * @param buffer    缓冲区指针（若为NULL，则内部动态分配）
 * @param capacity  最大元素个数
 * @param elem_size 每个元素的大小（字节）
 * @return 0成功，-1失败（如参数无效或内存不足）
 */
int kit_queue_init(kit_queue_t *q, void *buffer, unsigned int capacity, unsigned int elem_size);

/**
 * @brief 销毁队列（释放内部动态分配的缓冲区）
 * @param q 队列指针
 */
void kit_queue_destroy(kit_queue_t *q);

/**
 * @brief 入队
 * @param q    队列指针
 * @param data 要插入的元素数据（大小为elem_size字节）
 * @return 0成功，-1失败（队列已满）
 */
int kit_queue_enqueue(kit_queue_t *q, const void *data);

/**
 * @brief 出队
 * @param q    队列指针
 * @param data 用于存放取出元素的数据缓冲区（大小为elem_size字节）
 * @return 0成功，-1失败（队列为空）
 */
int kit_queue_dequeue(kit_queue_t *q, void *data);

/**
 * @brief 检查队列是否为空
 * @param q 队列指针
 * @return 1为空，0非空
 */
int kit_queue_is_empty(kit_queue_t *q);

/**
 * @brief 检查队列是否已满
 * @param q 队列指针
 * @return 1已满，0未满
 */
int kit_queue_is_full(kit_queue_t *q);

/**
 * @brief 获取队列中当前元素个数
 * @param q 队列指针
 * @return 元素个数
 */
unsigned int kit_queue_count(kit_queue_t *q);

#endif // __KIT_QUEUE_H__
