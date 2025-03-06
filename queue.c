#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "queue.h"

/* Print all elements starting from the head for debugging. */
static inline void print_list(struct list_head *head, struct list_head *tail)
{
    for (struct list_head *node = head; node != tail; node = node->next) {
        printf("\"%s\", ", list_entry(node, element_t, list)->value);
    }
    printf("\n");
}

/* Merge two list from the first element*/
struct list_head *merge_two_lists(struct list_head *L1,
                                  struct list_head *L2,
                                  bool descend)
{
    /* Indirect pointer method */
    struct list_head *head = NULL, **ptr = &head, **node;

    for (node = &L1; L1 && L2; *node = (*node)->next) {
        node = ((strcmp(list_entry(L1, element_t, list)->value,
                        list_entry(L2, element_t, list)->value) <= 0) ^
                descend)
                   ? &L1
                   : &L2;
        *ptr = *node;
        ptr = &(*ptr)->next;
    }
    *ptr = (struct list_head *) ((uintptr_t) L1 | (uintptr_t) L2);
    return head;
}

/* Perform merge sort on the list. */
struct list_head *mergesort_list(struct list_head *head, bool descend)
{
    if (!head || !head->next)
        return head;

    struct list_head *slow = head, *mid, *left, *right;
    for (const struct list_head *fast = head->next; fast && fast->next;
         fast = fast->next->next)
        slow = slow->next;
    mid = slow->next;
    slow->next = NULL;

    left = mergesort_list(head, descend);
    right = mergesort_list(mid, descend);
    return merge_two_lists(left, right, descend);
}

/*
 * Insert an element at the head or tail of the queue, depending on the caller.
 * This function simplifies the implementation of q_insert_head and
 * q_insert_tail.
 */
static inline bool q_insert(struct list_head *head,
                            char *s,
                            void (*insert_func)(struct list_head *,
                                                struct list_head *))
{
    if (!head)
        return false;

    element_t *new = malloc(sizeof(element_t));
    if (!new)
        return false;

    size_t len = strlen(s) + 1;
    new->value = malloc(len * sizeof(char));
    if (!new->value) {
        free(new);
        return false;
    }
    strncpy(new->value, s, len);

    insert_func(&new->list, head);
    return true;
}

/*
 * Remove an element from the head or tail of the queue, depending on the
 * caller. This function simplifies the implementation of q_remove_head and
 * q_remove_tail.
 */
static inline element_t *q_remove(struct list_head *head,
                                  struct list_head *node,
                                  char *sp,
                                  size_t bufsize)
{
    if (!head || list_empty(head))
        return NULL;

    element_t *entry = list_entry(node, element_t, list);
    list_del(&entry->list);

    if (sp) {
        strncpy(sp, entry->value, bufsize - 1);
        sp[bufsize - 1] = '\0';
    }
    return entry;
}

/* Create an empty queue */
struct list_head *q_new()
{
    struct list_head *head = malloc(sizeof(struct list_head));
    if (!head)
        return NULL;

    INIT_LIST_HEAD(head);
    return head;
}

/* Free all storage used by queue */
void q_free(struct list_head *head)
{
    if (!head)
        return;

    element_t *curr = NULL, *next = NULL;
    list_for_each_entry_safe (curr, next, head, list) {
        free(curr->value);
        free(curr);
    }
    free(head);
}

/* Insert an element at head of queue */
bool q_insert_head(struct list_head *head, char *s)
{
    return q_insert(head, s, list_add);
}

/* Insert an element at tail of queue */
bool q_insert_tail(struct list_head *head, char *s)
{
    return q_insert(head, s, list_add_tail);
}

/* Remove an element from head of queue */
element_t *q_remove_head(struct list_head *head, char *sp, size_t bufsize)
{
    return q_remove(head, head->next, sp, bufsize);
}

/* Remove an element from tail of queue */
element_t *q_remove_tail(struct list_head *head, char *sp, size_t bufsize)
{
    return q_remove(head, head->prev, sp, bufsize);
}

/* Return number of elements in queue */
int q_size(struct list_head *head)
{
    if (!head)
        return 0;

    int len = 0;
    struct list_head *it;

    list_for_each (it, head)
        len++;

    return len;
}

/* Delete the middle node in queue */
bool q_delete_mid(struct list_head *head)
{
    // https://leetcode.com/problems/delete-the-middle-node-of-a-linked-list/

    if (!head || list_empty(head))
        return false;

    /* Find the middle node in queue */
    struct list_head *del_node = head->next;
    for (const struct list_head *ptr = head->next;
         ptr != head && ptr->next != head; ptr = ptr->next->next) {
        del_node = del_node->next;
    }

    /* Delete the middle node */
    element_t *del = list_entry(del_node, element_t, list);
    list_del(del_node);
    free(del->value);
    free(del);
    return true;
}

/* Delete all nodes that have duplicate string */
bool q_delete_dup(struct list_head *head)
{
    // https://leetcode.com/problems/remove-duplicates-from-sorted-list-ii/
    if (!head || list_empty(head))
        return false;

    struct list_head *q = head, *p = head->next;
    while (p != head) {
        const char *value = list_entry(p, element_t, list)->value;
        if (p->next != head &&
            strcmp(list_entry(p->next, element_t, list)->value, value) != 0) {
            q = p;
            p = p->next;
        } else {
            struct list_head *del_node = p;
            p = p->next;
            while (p != head &&
                   strcmp(list_entry(p, element_t, list)->value, value) == 0) {
                p = p->next;
                free(list_entry(p->prev, element_t, list)->value);
                free(list_entry(p->prev, element_t, list));
            }
            free(list_entry(del_node, element_t, list)->value);
            free(list_entry(del_node, element_t, list));
            q->next = p;
            p->prev = q;
        }
    }
    return true;
}

/* Swap every two adjacent nodes */
void q_swap(struct list_head *head)
{
    // https://leetcode.com/problems/swap-nodes-in-pairs/
    if (!head || list_empty(head))
        return;

    struct list_head *L = head->next, *R = L->next;
    while (L != head && R != head) {
        /* swap L and R node */
        L->prev->next = R;
        R->next->prev = L;
        R->prev = L->prev;
        L->next = R->next;
        L->prev = R;
        R->next = L;
        L = L->next;
        R = L->next;
    }
    return;
}

/* Reverse elements in queue */
void q_reverse(struct list_head *head)
{
    if (!head || list_empty(head))
        return;
    struct list_head *node, *safe = NULL;
    list_for_each_safe (node, safe, head) {
        node->next = node->prev;
        node->prev = safe;
    }
    safe = head->next;
    head->next = head->prev;
    head->prev = safe;
}

/* Reverse the nodes of the list k at a time */
void q_reverseK(struct list_head *head, int k)
{
    if (!head || list_empty(head))
        return;

    struct list_head *L = head;
    int size = q_size(head);

    while (size >= k) {
        struct list_head *node = L->next;
        struct list_head *safe = L->next;
        for (int i = 0; i < k; ++i) {
            node = safe;
            safe = node->next;
            node->next = node->prev;
            node->prev = safe;
        }
        L->next->next = safe;
        safe->prev = L->next;
        node->prev = L;
        L->next = node;
        L = safe->prev;
        size -= k;
    }
    // https://leetcode.com/problems/reverse-nodes-in-k-group/
}

/* Sort elements of queue in ascending/descending order */
void q_sort(struct list_head *head, bool descend)
{
    if (!head || list_empty(head))
        return;
    head->prev->next = NULL;
    head->next = mergesort_list(head->next, descend);
    /* Fix the prev pointer */
    struct list_head *p = head;
    while (p->next != NULL) {
        p->next->prev = p;
        p = p->next;
    }
    p->next = head;
    head->prev = p;
}

/* Remove every node which has a node with a strictly less value anywhere to
 * the right side of it */
int q_ascend(struct list_head *head)
{
    // https://leetcode.com/problems/remove-nodes-from-linked-list/
    if (!head || list_empty(head))
        return 0;
    struct list_head *q = head->prev, *p = q->prev;
    const char *value = list_entry(q, element_t, list)->value;
    int size = 1;
    while (p != head) {
        if (strcmp(list_entry(p, element_t, list)->value, value) >= 0) {
            p = p->prev;
            q->prev = p;
            free(list_entry(p->next, element_t, list)->value);
            free(list_entry(p->next, element_t, list));
            p->next = q;
        } else {
            q = p;
            value = list_entry(p, element_t, list)->value;
            size++;
            p = p->prev;
        }
    }

    return size;
}

/* Remove every node which has a node with a strictly greater value anywhere to
 * the right side of it */
int q_descend(struct list_head *head)
{
    // https://leetcode.com/problems/remove-nodes-from-linked-list/
    if (!head || list_empty(head))
        return 0;
    struct list_head *q = head->prev, *p = q->prev;
    const char *value = list_entry(q, element_t, list)->value;
    int size = 1;
    while (p != head) {
        if (strcmp(list_entry(p, element_t, list)->value, value) < 0) {
            p = p->prev;
            q->prev = p;
            free(list_entry(p->next, element_t, list)->value);
            free(list_entry(p->next, element_t, list));
            p->next = q;
        } else {
            q = p;
            value = list_entry(p, element_t, list)->value;
            size++;
            p = p->prev;
        }
    }

    return size;
}

/* Merge all the queues into one sorted queue, which is in ascending/descending
 * order */
int q_merge(struct list_head *head, bool descend)
{
    // https://leetcode.com/problems/merge-k-sorted-lists/
    if (!head || list_empty(head))
        return 0;

    int chain_size = q_size(head);

    // Convert each queue to a non-circular linked list.
    for (struct list_head *p = head->next; p != head; p = p->next) {
        list_entry(p, queue_contex_t, chain)->q->prev->next = NULL;
    }

    // Merge all the queues into one sorted queue
    struct list_head *L, *R = head->prev;
    while (chain_size > 1) {
        L = head->next;
        while (L != R) {
            queue_contex_t *contex_L = list_entry(L, queue_contex_t, chain);
            queue_contex_t *contex_R = list_entry(R, queue_contex_t, chain);
            contex_L->q->next =
                merge_two_lists(contex_L->q->next, contex_R->q->next, descend);
            contex_L->size += contex_R->size;
            contex_R->q->prev = contex_R->q->next = contex_R->q;
            R = R->prev;
            if (L == R)
                break;
            L = L->next;
        }
        chain_size = (chain_size + 1) >> 1;
    }
    /* Restore the first queue to circular doubly Linked List*/
    struct list_head *q_head = list_entry(head->next, queue_contex_t, chain)->q;
    L = q_head;
    while (L->next != NULL) {
        L->next->prev = L;
        L = L->next;
    }
    L->next = q_head;
    q_head->prev = L;

    return list_first_entry(head, queue_contex_t, chain)->size;
}
