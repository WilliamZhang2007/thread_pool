/*  Implement a simple doubly circular linked list for use, 
      I will optimus it soon :) */ 

#ifndef __RA_LIST_H__
#define __RA_LIST_H__

#define LIST_ADD_HEAD(list, item)  do {  \
    if ((list) == NULL)   \
    {   \
        (item)->prev = item;    \
        (item)->next = item;    \
    }   \
    else    \
    {   \
        (list)->prev->next = item;  \
        (item)->prev = (list)->prev;    \
        (list)->prev = item;    \
        (item)->next = list;    \
    }   \
    list = item;    \
}while(0);

#define LIST_ADD_TAIL(list, item)  do {  \
    if ((list) == NULL)   \
    {   \
        (item)->prev = item;    \
        (item)->next = item;    \
        list = item;    \
    }   \
    else    \
    {   \
        (item)->prev = (list)->prev;    \
        (list)->prev->next = item;  \
        (list)->prev = item;    \
        (item)->next = list;    \
    }   \
}while(0);

#define LIST_REMOVE(list, item)  do {   \
    if ((item)->prev == (item) && (item)->next == (item))   \
        (list) = NULL;  \
    (item)->next->prev = (item)->prev;  \
    (item)->prev->next = (item)->next;  \
    if ((list) == (item))   \
        list = (item)->next;    \
    (item)->prev = (item)->next = NULL; \
}while(0);

#endif