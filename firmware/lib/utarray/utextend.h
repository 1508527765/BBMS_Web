/*****************************************************************************
 * @copyright       2024-2024, . POWER SUPPLY CO., LTD. 
 * @file            kit_utextend.h
 * @brief           utarray拓展定义
 * @author          Gary
 * @date            2025/01/10
 * @remark          初修订
 *****************************************************************************/

#ifndef _KIT_UT_EXTEND_H_
#define _KIT_UT_EXTEND_H_

#include <stdlib.h>

#include "utarray.h"
#include "uthash.h"
#include "utlist.h"

static inline UT_array *utarray_clone(UT_array *array)
{
    UT_array *result = (UT_array *) calloc(1, sizeof(UT_array));

    result->i   = array->i;
    result->n   = array->n;
    result->icd = array->icd;
    if (array->n > 0) {
        result->d = (char *) calloc(1, array->n * array->icd.sz);
        memcpy(result->d, array->d, array->n * array->icd.sz);
    } else {
        result->d = NULL;
    }

    return result;
}

static inline void *utarray_steal(UT_array *array)
{
    void *data = array->d;
    array->i   = 0;
    array->n   = 0;
    array->d   = NULL;
    return data;
}

#define utarray_foreach(array, type, elem)                      \
    for (type elem = (type) utarray_front(array); elem != NULL; \
         elem      = (type) utarray_next(array, elem))

#define utarray_reverse_foreach(array, type, elem)             \
    for (type elem = (type) utarray_back(array); elem != NULL; \
         elem      = (type) utarray_prev(array, elem))

#define DL_LAST(head) ((head) != NULL) ? (head)->prev : NULL;
#define DL_PREV(el) ((el) != NULL) ? (el)->prev : NULL;

#define DL_REVERSE_FOREACH(head, tail, el) \
    (tail) = DL_LAST(head);                \
    (el)   = (tail);                       \
    do

#define DL_REVERSE_FOREACH_END(head, tail, el) \
    while (((el) = (el)->prev) != (head))

#ifdef NO_DECLTYPE
#define HASH_LOOP(hh, head, el)                       \
    for ((el)                 = (head); (el) != NULL; \
         (*(char **) (&(el))) = (char *) (el)->hh.next)
#else
#define HASH_LOOP(hh, head, el) \
    for ((el) = (head); (el) != NULL; (el) = DECLTYPE(el)((el)->hh.next))
#endif

static inline UT_array *utarray_deep_clone(UT_array *array)
{
    UT_array *result = NULL;

    utarray_new(result, &array->icd);

    utarray_foreach(array, void *, elem) { utarray_push_back(result, elem); }

    return result;
}

#endif /* _KIT_UT_EXTEND_H_ */
