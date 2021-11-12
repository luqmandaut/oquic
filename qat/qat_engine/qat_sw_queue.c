/* ====================================================================
 *
 *
 *   BSD LICENSE
 *
 *   Copyright(c) 2020-2021 Intel Corporation.
 *   All rights reserved.
 *
 *   Redistribution and use in source and binary forms, with or without
 *   modification, are permitted provided that the following conditions
 *   are met:
 *
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in
 *       the documentation and/or other materials provided with the
 *       distribution.
 *     * Neither the name of Intel Corporation nor the names of its
 *       contributors may be used to endorse or promote products derived
 *       from this software without specific prior written permission.
 *
 *   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *   "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *   LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 *   A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 *   OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 *   SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 *   LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 *   DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 *   THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *   (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 *   OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *
 * ====================================================================
 */

/*****************************************************************************
 * @file qat_sw_queue.c
 *
 * This file provides multibuff implementations of a queue
 *
 *****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

/* Local Includes */
#include "e_qat.h"
#include "qat_sw_queue.h"
#include "qat_sw_request.h"

/* OpenSSL Includes */
#include <openssl/err.h>

int mb_queue_rsa2k_priv_create(mb_queue_rsa2k_priv *queue)
{
    if (queue == NULL)
        return 1;

    if (0 == enable_external_polling) {
        pthread_mutex_init(&queue->mb_queue_mutex, NULL);
        pthread_mutex_lock(&queue->mb_queue_mutex);
    }
    queue->head = NULL;
    queue->tail = NULL;
    queue->disabled = 0;
    queue->num_items = 0;
    if (0 == enable_external_polling) {
        pthread_mutex_unlock(&queue->mb_queue_mutex);
    }

    return 0;
}

int mb_queue_rsa2k_priv_disable(mb_queue_rsa2k_priv *queue)
{
    if (queue == NULL)
        return 1;

    if (0 == enable_external_polling) {
        pthread_mutex_lock(&queue->mb_queue_mutex);
    }
    queue->disabled = 1;
    if (0 == enable_external_polling) {
        pthread_mutex_unlock(&queue->mb_queue_mutex);
    }

    return 0;
}

int mb_queue_rsa2k_priv_cleanup(mb_queue_rsa2k_priv *queue)
{
    if (queue == NULL)
        return 1;

    if (0 == enable_external_polling) {
        pthread_mutex_destroy(&queue->mb_queue_mutex);
    }
    return 0;
}

int mb_queue_rsa2k_priv_enqueue(mb_queue_rsa2k_priv *queue,
                                rsa_priv_op_data *item)
{
    if (queue == NULL || item == NULL)
        return 1;

    if (0 == enable_external_polling) {
        pthread_mutex_lock(&queue->mb_queue_mutex);
    }

    if (queue->disabled == 1) {
        if (0 == enable_external_polling) {
            pthread_mutex_unlock(&queue->mb_queue_mutex);
        }
        return 1;
    }

    if (queue->num_items == 0) {
        queue->tail = item;
        queue->head = item;
    } else {
        queue->tail->next = item;
        queue->tail = item;
    }
    queue->tail->next = NULL;
    queue->num_items++;

    if (enable_heuristic_polling) {
        QAT_ATOMIC_INC(num_asym_mb_items_in_queue);
    }

    if (0 == enable_external_polling) {
        pthread_mutex_unlock(&queue->mb_queue_mutex);
    }
    return 0;
}

rsa_priv_op_data * mb_queue_rsa2k_priv_dequeue(mb_queue_rsa2k_priv *queue)
{
    rsa_priv_op_data *item = NULL;

    if (queue == NULL)
        return NULL;

    if (0 == enable_external_polling) {
        pthread_mutex_lock(&queue->mb_queue_mutex);
    }

    if (queue->head == NULL) {
        if (0 == enable_external_polling) {
            pthread_mutex_unlock(&queue->mb_queue_mutex);
        }
        return NULL;
    }

    item = queue->head;
    queue->head = item->next;
    queue->num_items--;

    if (enable_heuristic_polling) {
        QAT_ATOMIC_DEC(num_asym_mb_items_in_queue);
    }

    if (queue->num_items == 0)
        queue->tail = NULL;

    if (0 == enable_external_polling) {
        pthread_mutex_unlock(&queue->mb_queue_mutex);
    }

    return item;
}

int mb_queue_rsa2k_priv_get_size(mb_queue_rsa2k_priv *queue)
{
    if (queue == NULL)
        return 0;

    return queue->num_items;
}

int mb_queue_rsa2k_pub_create(mb_queue_rsa2k_pub *queue)
{
    if (queue == NULL)
        return 1;

    if (0 == enable_external_polling) {
        pthread_mutex_init(&queue->mb_queue_mutex, NULL);
        pthread_mutex_lock(&queue->mb_queue_mutex);
    }
    queue->head = NULL;
    queue->tail = NULL;
    queue->disabled = 0;
    queue->num_items = 0;
    if (0 == enable_external_polling) {
        pthread_mutex_unlock(&queue->mb_queue_mutex);
    }

    return 0;
}

int mb_queue_rsa2k_pub_disable(mb_queue_rsa2k_pub *queue)
{
    if (queue == NULL)
        return 1;

    if (0 == enable_external_polling) {
        pthread_mutex_lock(&queue->mb_queue_mutex);
    }
    queue->disabled = 1;
    if (0 == enable_external_polling) {
        pthread_mutex_unlock(&queue->mb_queue_mutex);
    }

    return 0;
}

int mb_queue_rsa2k_pub_cleanup(mb_queue_rsa2k_pub *queue)
{
    if (queue == NULL)
        return 1;

    if (0 == enable_external_polling) {
        pthread_mutex_destroy(&queue->mb_queue_mutex);
    }
    return 0;
}

int mb_queue_rsa2k_pub_enqueue(mb_queue_rsa2k_pub *queue,
                               rsa_pub_op_data *item)
{
    if (queue == NULL || item == NULL)
        return 1;

    if (0 == enable_external_polling) {
        pthread_mutex_lock(&queue->mb_queue_mutex);
    }

    if (queue->disabled == 1) {
        if (0 == enable_external_polling) {
            pthread_mutex_unlock(&queue->mb_queue_mutex);
        }
        return 1;
    }

    if (queue->num_items == 0) {
        queue->tail = item;
        queue->head = item;
    } else {
        queue->tail->next = item;
        queue->tail = item;
    }
    queue->tail->next = NULL;
    queue->num_items++;

    if (enable_heuristic_polling) {
        QAT_ATOMIC_INC(num_asym_mb_items_in_queue);
    }

    if (0 == enable_external_polling) {
        pthread_mutex_unlock(&queue->mb_queue_mutex);
    }
    return 0;
}

rsa_pub_op_data * mb_queue_rsa2k_pub_dequeue(mb_queue_rsa2k_pub *queue)
{
    rsa_pub_op_data *item = NULL;

    if (queue == NULL)
        return NULL;

    if (0 == enable_external_polling) {
        pthread_mutex_lock(&queue->mb_queue_mutex);
    }

    if (queue->head == NULL) {
        if (0 == enable_external_polling) {
            pthread_mutex_unlock(&queue->mb_queue_mutex);
        }
        return NULL;
    }

    item = queue->head;
    queue->head = item->next;
    queue->num_items--;

    if (enable_heuristic_polling) {
        QAT_ATOMIC_DEC(num_asym_mb_items_in_queue);
    }

    if (queue->num_items == 0)
        queue->tail = NULL;

    if (0 == enable_external_polling) {
        pthread_mutex_unlock(&queue->mb_queue_mutex);
    }

    return item;
}

int mb_queue_rsa2k_pub_get_size(mb_queue_rsa2k_pub *queue)
{
    if (queue == NULL)
        return 0;

    return queue->num_items;
}

int mb_queue_rsa3k_priv_create(mb_queue_rsa3k_priv *queue)
{
    if (queue == NULL)
        return 1;

    if (0 == enable_external_polling) {
        pthread_mutex_init(&queue->mb_queue_mutex, NULL);
        pthread_mutex_lock(&queue->mb_queue_mutex);
    }
    queue->head = NULL;
    queue->tail = NULL;
    queue->disabled = 0;
    queue->num_items = 0;
    if (0 == enable_external_polling) {
        pthread_mutex_unlock(&queue->mb_queue_mutex);
    }

    return 0;
}

int mb_queue_rsa3k_priv_disable(mb_queue_rsa3k_priv *queue)
{
    if (queue == NULL)
        return 1;

    if (0 == enable_external_polling) {
        pthread_mutex_lock(&queue->mb_queue_mutex);
    }
    queue->disabled = 1;
    if (0 == enable_external_polling) {
        pthread_mutex_unlock(&queue->mb_queue_mutex);
    }

    return 0;
}

int mb_queue_rsa3k_priv_cleanup(mb_queue_rsa3k_priv *queue)
{
    if (queue == NULL)
        return 1;

    if (0 == enable_external_polling) {
        pthread_mutex_destroy(&queue->mb_queue_mutex);
    }
    return 0;
}

int mb_queue_rsa3k_priv_enqueue(mb_queue_rsa3k_priv *queue,
                                rsa_priv_op_data *item)
{
    if (queue == NULL || item == NULL)
        return 1;

    if (0 == enable_external_polling) {
        pthread_mutex_lock(&queue->mb_queue_mutex);
    }

    if (queue->disabled == 1) {
        if (0 == enable_external_polling) {
            pthread_mutex_unlock(&queue->mb_queue_mutex);
        }
        return 1;
    }

    if (queue->num_items == 0) {
        queue->tail = item;
        queue->head = item;
    } else {
        queue->tail->next = item;
        queue->tail = item;
    }
    queue->tail->next = NULL;
    queue->num_items++;

    if (enable_heuristic_polling) {
        QAT_ATOMIC_INC(num_asym_mb_items_in_queue);
    }

    if (0 == enable_external_polling) {
        pthread_mutex_unlock(&queue->mb_queue_mutex);
    }
    return 0;
}

rsa_priv_op_data * mb_queue_rsa3k_priv_dequeue(mb_queue_rsa3k_priv *queue)
{
    rsa_priv_op_data *item = NULL;

    if (queue == NULL)
        return NULL;

    if (0 == enable_external_polling) {
        pthread_mutex_lock(&queue->mb_queue_mutex);
    }

    if (queue->head == NULL) {
        if (0 == enable_external_polling) {
            pthread_mutex_unlock(&queue->mb_queue_mutex);
        }
        return NULL;
    }

    item = queue->head;
    queue->head = item->next;
    queue->num_items--;

    if (enable_heuristic_polling) {
        QAT_ATOMIC_DEC(num_asym_mb_items_in_queue);
    }

    if (queue->num_items == 0)
        queue->tail = NULL;

    if (0 == enable_external_polling) {
        pthread_mutex_unlock(&queue->mb_queue_mutex);
    }

    return item;
}

int mb_queue_rsa3k_priv_get_size(mb_queue_rsa3k_priv *queue)
{
    if (queue == NULL)
        return 0;

    return queue->num_items;
}

int mb_queue_rsa3k_pub_create(mb_queue_rsa3k_pub *queue)
{
    if (queue == NULL)
        return 1;

    if (0 == enable_external_polling) {
        pthread_mutex_init(&queue->mb_queue_mutex, NULL);
        pthread_mutex_lock(&queue->mb_queue_mutex);
    }
    queue->head = NULL;
    queue->tail = NULL;
    queue->disabled = 0;
    queue->num_items = 0;
    if (0 == enable_external_polling) {
        pthread_mutex_unlock(&queue->mb_queue_mutex);
    }

    return 0;
}

int mb_queue_rsa3k_pub_disable(mb_queue_rsa3k_pub *queue)
{
    if (queue == NULL)
        return 1;

    if (0 == enable_external_polling) {
        pthread_mutex_lock(&queue->mb_queue_mutex);
    }
    queue->disabled = 1;
    if (0 == enable_external_polling) {
        pthread_mutex_unlock(&queue->mb_queue_mutex);
    }

    return 0;
}

int mb_queue_rsa3k_pub_cleanup(mb_queue_rsa3k_pub *queue)
{
    if (queue == NULL)
        return 1;

    if (0 == enable_external_polling) {
        pthread_mutex_destroy(&queue->mb_queue_mutex);
    }
    return 0;
}

int mb_queue_rsa3k_pub_enqueue(mb_queue_rsa3k_pub *queue,
                               rsa_pub_op_data *item)
{
    if (queue == NULL || item == NULL)
        return 1;

    if (0 == enable_external_polling) {
        pthread_mutex_lock(&queue->mb_queue_mutex);
    }

    if (queue->disabled == 1) {
        if (0 == enable_external_polling) {
            pthread_mutex_unlock(&queue->mb_queue_mutex);
        }
        return 1;
    }

    if (queue->num_items == 0) {
        queue->tail = item;
        queue->head = item;
    } else {
        queue->tail->next = item;
        queue->tail = item;
    }
    queue->tail->next = NULL;
    queue->num_items++;

    if (enable_heuristic_polling) {
        QAT_ATOMIC_INC(num_asym_mb_items_in_queue);
    }

    if (0 == enable_external_polling) {
        pthread_mutex_unlock(&queue->mb_queue_mutex);
    }
    return 0;
}

rsa_pub_op_data * mb_queue_rsa3k_pub_dequeue(mb_queue_rsa3k_pub *queue)
{
    rsa_pub_op_data *item = NULL;

    if (queue == NULL)
        return NULL;

    if (0 == enable_external_polling) {
        pthread_mutex_lock(&queue->mb_queue_mutex);
    }

    if (queue->head == NULL) {
        if (0 == enable_external_polling) {
            pthread_mutex_unlock(&queue->mb_queue_mutex);
        }
        return NULL;
    }

    item = queue->head;
    queue->head = item->next;
    queue->num_items--;

    if (enable_heuristic_polling) {
        QAT_ATOMIC_DEC(num_asym_mb_items_in_queue);
    }

    if (queue->num_items == 0)
        queue->tail = NULL;

    if (0 == enable_external_polling) {
        pthread_mutex_unlock(&queue->mb_queue_mutex);
    }

    return item;
}

int mb_queue_rsa3k_pub_get_size(mb_queue_rsa3k_pub *queue)
{
    if (queue == NULL)
        return 0;

    return queue->num_items;
}

int mb_queue_rsa4k_priv_create(mb_queue_rsa4k_priv *queue)
{
    if (queue == NULL)
        return 1;

    if (0 == enable_external_polling) {
        pthread_mutex_init(&queue->mb_queue_mutex, NULL);
        pthread_mutex_lock(&queue->mb_queue_mutex);
    }
    queue->head = NULL;
    queue->tail = NULL;
    queue->disabled = 0;
    queue->num_items = 0;
    if (0 == enable_external_polling) {
        pthread_mutex_unlock(&queue->mb_queue_mutex);
    }

    return 0;
}

int mb_queue_rsa4k_priv_disable(mb_queue_rsa4k_priv *queue)
{
    if (queue == NULL)
        return 1;

    if (0 == enable_external_polling) {
        pthread_mutex_lock(&queue->mb_queue_mutex);
    }
    queue->disabled = 1;
    if (0 == enable_external_polling) {
        pthread_mutex_unlock(&queue->mb_queue_mutex);
    }

    return 0;
}

int mb_queue_rsa4k_priv_cleanup(mb_queue_rsa4k_priv *queue)
{
    if (queue == NULL)
        return 1;

    if (0 == enable_external_polling) {
        pthread_mutex_destroy(&queue->mb_queue_mutex);
    }
    return 0;
}

int mb_queue_rsa4k_priv_enqueue(mb_queue_rsa4k_priv *queue,
                                rsa_priv_op_data *item)
{
    if (queue == NULL || item == NULL)
        return 1;

    if (0 == enable_external_polling) {
        pthread_mutex_lock(&queue->mb_queue_mutex);
    }

    if (queue->disabled == 1) {
        if (0 == enable_external_polling) {
            pthread_mutex_unlock(&queue->mb_queue_mutex);
        }
        return 1;
    }

    if (queue->num_items == 0) {
        queue->tail = item;
        queue->head = item;
    } else {
        queue->tail->next = item;
        queue->tail = item;
    }
    queue->tail->next = NULL;
    queue->num_items++;

    if (enable_heuristic_polling) {
        QAT_ATOMIC_INC(num_asym_mb_items_in_queue);
    }

    if (0 == enable_external_polling) {
        pthread_mutex_unlock(&queue->mb_queue_mutex);
    }
    return 0;
}

rsa_priv_op_data * mb_queue_rsa4k_priv_dequeue(mb_queue_rsa4k_priv *queue)
{
    rsa_priv_op_data *item = NULL;

    if (queue == NULL)
        return NULL;

    if (0 == enable_external_polling) {
        pthread_mutex_lock(&queue->mb_queue_mutex);
    }

    if (queue->head == NULL) {
        if (0 == enable_external_polling) {
            pthread_mutex_unlock(&queue->mb_queue_mutex);
        }
        return NULL;
    }

    item = queue->head;
    queue->head = item->next;
    queue->num_items--;

    if (enable_heuristic_polling) {
        QAT_ATOMIC_DEC(num_asym_mb_items_in_queue);
    }

    if (queue->num_items == 0)
        queue->tail = NULL;

    if (0 == enable_external_polling) {
        pthread_mutex_unlock(&queue->mb_queue_mutex);
    }

    return item;
}

int mb_queue_rsa4k_priv_get_size(mb_queue_rsa4k_priv *queue)
{
    if (queue == NULL)
        return 0;

    return queue->num_items;
}

int mb_queue_rsa4k_pub_create(mb_queue_rsa4k_pub *queue)
{
    if (queue == NULL)
        return 1;

    if (0 == enable_external_polling) {
        pthread_mutex_init(&queue->mb_queue_mutex, NULL);
        pthread_mutex_lock(&queue->mb_queue_mutex);
    }
    queue->head = NULL;
    queue->tail = NULL;
    queue->disabled = 0;
    queue->num_items = 0;
    if (0 == enable_external_polling) {
        pthread_mutex_unlock(&queue->mb_queue_mutex);
    }

    return 0;
}

int mb_queue_rsa4k_pub_disable(mb_queue_rsa4k_pub *queue)
{
    if (queue == NULL)
        return 1;

    if (0 == enable_external_polling) {
        pthread_mutex_lock(&queue->mb_queue_mutex);
    }
    queue->disabled = 1;
    if (0 == enable_external_polling) {
        pthread_mutex_unlock(&queue->mb_queue_mutex);
    }

    return 0;
}

int mb_queue_rsa4k_pub_cleanup(mb_queue_rsa4k_pub *queue)
{
    if (queue == NULL)
        return 1;

    if (0 == enable_external_polling) {
        pthread_mutex_destroy(&queue->mb_queue_mutex);
    }
    return 0;
}

int mb_queue_rsa4k_pub_enqueue(mb_queue_rsa4k_pub *queue,
                               rsa_pub_op_data *item)
{
    if (queue == NULL || item == NULL)
        return 1;

    if (0 == enable_external_polling) {
        pthread_mutex_lock(&queue->mb_queue_mutex);
    }

    if (queue->disabled == 1) {
        if (0 == enable_external_polling) {
            pthread_mutex_unlock(&queue->mb_queue_mutex);
        }
        return 1;
    }

    if (queue->num_items == 0) {
        queue->tail = item;
        queue->head = item;
    } else {
        queue->tail->next = item;
        queue->tail = item;
    }
    queue->tail->next = NULL;
    queue->num_items++;

    if (enable_heuristic_polling) {
        QAT_ATOMIC_INC(num_asym_mb_items_in_queue);
    }

    if (0 == enable_external_polling) {
        pthread_mutex_unlock(&queue->mb_queue_mutex);
    }
    return 0;
}

rsa_pub_op_data * mb_queue_rsa4k_pub_dequeue(mb_queue_rsa4k_pub *queue)
{
    rsa_pub_op_data *item = NULL;

    if (queue == NULL)
        return NULL;

    if (0 == enable_external_polling) {
        pthread_mutex_lock(&queue->mb_queue_mutex);
    }

    if (queue->head == NULL) {
        if (0 == enable_external_polling) {
            pthread_mutex_unlock(&queue->mb_queue_mutex);
        }
        return NULL;
    }

    item = queue->head;
    queue->head = item->next;
    queue->num_items--;

    if (enable_heuristic_polling) {
        QAT_ATOMIC_DEC(num_asym_mb_items_in_queue);
    }

    if (queue->num_items == 0)
        queue->tail = NULL;

    if (0 == enable_external_polling) {
        pthread_mutex_unlock(&queue->mb_queue_mutex);
    }

    return item;
}

int mb_queue_rsa4k_pub_get_size(mb_queue_rsa4k_pub *queue)
{
    if (queue == NULL)
        return 0;

    return queue->num_items;
}

int mb_queue_x25519_keygen_create(mb_queue_x25519_keygen *queue)
{
    if (queue == NULL)
        return 1;

    if (0 == enable_external_polling) {
        pthread_mutex_init(&queue->mb_queue_mutex, NULL);
        pthread_mutex_lock(&queue->mb_queue_mutex);
    }
    queue->head = NULL;
    queue->tail = NULL;
    queue->disabled = 0;
    queue->num_items = 0;
    if (0 == enable_external_polling) {
        pthread_mutex_unlock(&queue->mb_queue_mutex);
    }

    return 0;
}

int mb_queue_x25519_keygen_disable(mb_queue_x25519_keygen *queue)
{
    if (queue == NULL)
        return 1;

    if (0 == enable_external_polling) {
        pthread_mutex_lock(&queue->mb_queue_mutex);
    }
    queue->disabled = 1;
    if (0 == enable_external_polling) {
        pthread_mutex_unlock(&queue->mb_queue_mutex);
    }

    return 0;
}

int mb_queue_x25519_keygen_cleanup(mb_queue_x25519_keygen *queue)
{
    if (queue == NULL)
        return 1;

    if (0 == enable_external_polling) {
        pthread_mutex_destroy(&queue->mb_queue_mutex);
    }
    return 0;
}

int mb_queue_x25519_keygen_enqueue(mb_queue_x25519_keygen *queue,
                                   x25519_keygen_op_data *item)
{
    if (queue == NULL || item == NULL)
        return 1;

    if (0 == enable_external_polling) {
        pthread_mutex_lock(&queue->mb_queue_mutex);
    }

    if (queue->disabled == 1) {
        if (0 == enable_external_polling) {
            pthread_mutex_unlock(&queue->mb_queue_mutex);
        }
        return 1;
    }

    if (queue->num_items == 0) {
        queue->tail = item;
        queue->head = item;
    } else {
        queue->tail->next = item;
        queue->tail = item;
    }
    queue->tail->next = NULL;
    queue->num_items++;

    if (enable_heuristic_polling) {
        QAT_ATOMIC_INC(num_asym_mb_items_in_queue);
    }

    if (0 == enable_external_polling) {
        pthread_mutex_unlock(&queue->mb_queue_mutex);
    }
    return 0;
}

x25519_keygen_op_data * mb_queue_x25519_keygen_dequeue(mb_queue_x25519_keygen *queue)
{
    x25519_keygen_op_data *item = NULL;

    if (queue == NULL) {
        return NULL;
    }

    if (0 == enable_external_polling) {
        pthread_mutex_lock(&queue->mb_queue_mutex);
    }

    if (queue->head == NULL) {
        if (0 == enable_external_polling) {
            pthread_mutex_unlock(&queue->mb_queue_mutex);
        }
        return NULL;
    }

    item = queue->head;
    queue->head = item->next;
    queue->num_items--;

    if (enable_heuristic_polling) {
        QAT_ATOMIC_DEC(num_asym_mb_items_in_queue);
    }

    if (queue->num_items == 0)
        queue->tail = NULL;

    if (0 == enable_external_polling) {
        pthread_mutex_unlock(&queue->mb_queue_mutex);
    }

    return item;
}

int mb_queue_x25519_keygen_get_size(mb_queue_x25519_keygen *queue)
{
    if (queue == NULL)
        return 0;

    return queue->num_items;
}

int mb_queue_x25519_derive_create(mb_queue_x25519_derive *queue)
{
    if (queue == NULL)
        return 1;

    if (0 == enable_external_polling) {
        pthread_mutex_init(&queue->mb_queue_mutex, NULL);
        pthread_mutex_lock(&queue->mb_queue_mutex);
    }
    queue->head = NULL;
    queue->tail = NULL;
    queue->disabled = 0;
    queue->num_items = 0;
    if (0 == enable_external_polling) {
        pthread_mutex_unlock(&queue->mb_queue_mutex);
    }

    return 0;
}

int mb_queue_x25519_derive_disable(mb_queue_x25519_derive *queue)
{
    if (queue == NULL)
        return 1;

    if (0 == enable_external_polling) {
        pthread_mutex_lock(&queue->mb_queue_mutex);
    }
    queue->disabled = 1;
    if (0 == enable_external_polling) {
        pthread_mutex_unlock(&queue->mb_queue_mutex);
    }

    return 0;
}

int mb_queue_x25519_derive_cleanup(mb_queue_x25519_derive *queue)
{
    if (queue == NULL)
        return 1;

    if (0 == enable_external_polling) {
        pthread_mutex_destroy(&queue->mb_queue_mutex);
    }
    return 0;
}

int mb_queue_x25519_derive_enqueue(mb_queue_x25519_derive *queue,
                                   x25519_derive_op_data *item)
{
    if (queue == NULL || item == NULL)
        return 1;

    if (0 == enable_external_polling) {
        pthread_mutex_lock(&queue->mb_queue_mutex);
    }

    if (queue->disabled == 1) {
        if (0 == enable_external_polling) {
            pthread_mutex_unlock(&queue->mb_queue_mutex);
        }
        return 1;
    }

    if (queue->num_items == 0) {
        queue->tail = item;
        queue->head = item;
    } else {
        queue->tail->next = item;
        queue->tail = item;
    }
    queue->tail->next = NULL;
    queue->num_items++;

    if (enable_heuristic_polling) {
        QAT_ATOMIC_INC(num_asym_mb_items_in_queue);
    }

    if (0 == enable_external_polling) {
        pthread_mutex_unlock(&queue->mb_queue_mutex);
    }
    return 0;
}

x25519_derive_op_data * mb_queue_x25519_derive_dequeue(mb_queue_x25519_derive *queue)
{
    x25519_derive_op_data *item = NULL;

    if (queue == NULL)
        return NULL;

    if (0 == enable_external_polling) {
        pthread_mutex_lock(&queue->mb_queue_mutex);
    }

    if (queue->head == NULL) {
        if (0 == enable_external_polling) {
            pthread_mutex_unlock(&queue->mb_queue_mutex);
        }
        return NULL;
    }

    item = queue->head;
    queue->head = item->next;
    queue->num_items--;

    if (enable_heuristic_polling) {
        QAT_ATOMIC_DEC(num_asym_mb_items_in_queue);
    }

    if (queue->num_items == 0)
        queue->tail = NULL;

    if (0 == enable_external_polling) {
        pthread_mutex_unlock(&queue->mb_queue_mutex);
    }

    return item;
}

int mb_queue_x25519_derive_get_size(mb_queue_x25519_derive *queue)
{
    if (queue == NULL)
        return 0;

    return queue->num_items;
}

int mb_queue_ecdsap256_sign_create(mb_queue_ecdsap256_sign *queue)
{
    if (queue == NULL)
        return 1;

    if (0 == enable_external_polling) {
        pthread_mutex_init(&queue->mb_queue_mutex, NULL);
        pthread_mutex_lock(&queue->mb_queue_mutex);
    }
    queue->head = NULL;
    queue->tail = NULL;
    queue->disabled = 0;
    queue->num_items = 0;
    if (0 == enable_external_polling) {
        pthread_mutex_unlock(&queue->mb_queue_mutex);
    }

    return 0;
}

int mb_queue_ecdsap256_sign_disable(mb_queue_ecdsap256_sign *queue)
{
    if (queue == NULL)
        return 1;

    if (0 == enable_external_polling) {
        pthread_mutex_lock(&queue->mb_queue_mutex);
    }
    queue->disabled = 1;
    if (0 == enable_external_polling) {
        pthread_mutex_unlock(&queue->mb_queue_mutex);
    }

    return 0;
}

int mb_queue_ecdsap256_sign_cleanup(mb_queue_ecdsap256_sign *queue)
{
    if (queue == NULL)
        return 1;

    if (0 == enable_external_polling) {
        pthread_mutex_destroy(&queue->mb_queue_mutex);
    }
    return 0;
}

int mb_queue_ecdsap256_sign_enqueue(mb_queue_ecdsap256_sign *queue,
                                    ecdsa_sign_op_data *item)
{
    if (queue == NULL || item == NULL)
        return 1;

    if (0 == enable_external_polling) {
        pthread_mutex_lock(&queue->mb_queue_mutex);
    }

    if (queue->disabled == 1) {
        if (0 == enable_external_polling) {
            pthread_mutex_unlock(&queue->mb_queue_mutex);
        }
        return 1;
    }

    if (queue->num_items == 0) {
        queue->tail = item;
        queue->head = item;
    } else {
        queue->tail->next = item;
        queue->tail = item;
    }
    queue->tail->next = NULL;
    queue->num_items++;

    if (enable_heuristic_polling) {
        QAT_ATOMIC_INC(num_asym_mb_items_in_queue);
    }

    if (0 == enable_external_polling) {
        pthread_mutex_unlock(&queue->mb_queue_mutex);
    }
    return 0;
}

ecdsa_sign_op_data
    *mb_queue_ecdsap256_sign_dequeue(mb_queue_ecdsap256_sign *queue)
{
    ecdsa_sign_op_data *item = NULL;

    if (queue == NULL)
        return NULL;

    if (0 == enable_external_polling) {
        pthread_mutex_lock(&queue->mb_queue_mutex);
    }

    if (queue->head == NULL) {
        if (0 == enable_external_polling) {
            pthread_mutex_unlock(&queue->mb_queue_mutex);
        }
        return NULL;
    }

    item = queue->head;
    queue->head = item->next;
    queue->num_items--;

    if (enable_heuristic_polling) {
        QAT_ATOMIC_DEC(num_asym_mb_items_in_queue);
    }

    if (queue->num_items == 0)
        queue->tail = NULL;

    if (0 == enable_external_polling) {
        pthread_mutex_unlock(&queue->mb_queue_mutex);
    }

    return item;
}

int mb_queue_ecdsap256_sign_get_size(mb_queue_ecdsap256_sign *queue)
{
    if (queue == NULL)
        return 0;

    return queue->num_items;
}

int mb_queue_ecdsap256_sign_setup_create(mb_queue_ecdsap256_sign_setup *queue)
{
    if (queue == NULL)
        return 1;

    if (0 == enable_external_polling) {
        pthread_mutex_init(&queue->mb_queue_mutex, NULL);
        pthread_mutex_lock(&queue->mb_queue_mutex);
    }
    queue->head = NULL;
    queue->tail = NULL;
    queue->disabled = 0;
    queue->num_items = 0;
    if (0 == enable_external_polling) {
        pthread_mutex_unlock(&queue->mb_queue_mutex);
    }

    return 0;
}

int mb_queue_ecdsap256_sign_setup_disable(mb_queue_ecdsap256_sign_setup *queue)
{
    if (queue == NULL)
        return 1;

    if (0 == enable_external_polling) {
        pthread_mutex_lock(&queue->mb_queue_mutex);
    }
    queue->disabled = 1;
    if (0 == enable_external_polling) {
        pthread_mutex_unlock(&queue->mb_queue_mutex);
    }

    return 0;
}

int mb_queue_ecdsap256_sign_setup_cleanup(mb_queue_ecdsap256_sign_setup *queue)
{
    if (queue == NULL)
        return 1;

    if (0 == enable_external_polling) {
        pthread_mutex_destroy(&queue->mb_queue_mutex);
    }
    return 0;
}

int mb_queue_ecdsap256_sign_setup_enqueue(mb_queue_ecdsap256_sign_setup *queue,
                                          ecdsa_sign_setup_op_data *item)
{
    if (queue == NULL || item == NULL)
        return 1;

    if (0 == enable_external_polling) {
        pthread_mutex_lock(&queue->mb_queue_mutex);
    }

    if (queue->disabled == 1) {
        if (0 == enable_external_polling) {
            pthread_mutex_unlock(&queue->mb_queue_mutex);
        }
        return 1;
    }

    if (queue->num_items == 0) {
        queue->tail = item;
        queue->head = item;
    } else {
        queue->tail->next = item;
        queue->tail = item;
    }
    queue->tail->next = NULL;
    queue->num_items++;

    if (enable_heuristic_polling) {
        QAT_ATOMIC_INC(num_asym_mb_items_in_queue);
    }

    if (0 == enable_external_polling) {
        pthread_mutex_unlock(&queue->mb_queue_mutex);
    }
    return 0;
}

ecdsa_sign_setup_op_data
    *mb_queue_ecdsap256_sign_setup_dequeue(mb_queue_ecdsap256_sign_setup *queue)
{
    ecdsa_sign_setup_op_data *item = NULL;

    if (queue == NULL)
        return NULL;

    if (0 == enable_external_polling) {
        pthread_mutex_lock(&queue->mb_queue_mutex);
    }

    if (queue->head == NULL) {
        if (0 == enable_external_polling) {
            pthread_mutex_unlock(&queue->mb_queue_mutex);
        }
        return NULL;
    }

    item = queue->head;
    queue->head = item->next;
    queue->num_items--;

    if (enable_heuristic_polling) {
        QAT_ATOMIC_DEC(num_asym_mb_items_in_queue);
    }

    if (queue->num_items == 0)
        queue->tail = NULL;

    if (0 == enable_external_polling) {
        pthread_mutex_unlock(&queue->mb_queue_mutex);
    }

    return item;
}

int mb_queue_ecdsap256_sign_setup_get_size(mb_queue_ecdsap256_sign_setup *queue)
{
    if (queue == NULL)
        return 0;

    return queue->num_items;
}

int mb_queue_ecdsap256_sign_sig_create(mb_queue_ecdsap256_sign_sig *queue)
{
    if (queue == NULL)
        return 1;

    if (0 == enable_external_polling) {
        pthread_mutex_init(&queue->mb_queue_mutex, NULL);
        pthread_mutex_lock(&queue->mb_queue_mutex);
    }
    queue->head = NULL;
    queue->tail = NULL;
    queue->disabled = 0;
    queue->num_items = 0;
    if (0 == enable_external_polling) {
        pthread_mutex_unlock(&queue->mb_queue_mutex);
    }

    return 0;
}

int mb_queue_ecdsap256_sign_sig_disable(mb_queue_ecdsap256_sign_sig *queue)
{
    if (queue == NULL)
        return 1;

    if (0 == enable_external_polling) {
        pthread_mutex_lock(&queue->mb_queue_mutex);
    }
    queue->disabled = 1;
    if (0 == enable_external_polling) {
        pthread_mutex_unlock(&queue->mb_queue_mutex);
    }

    return 0;
}

int mb_queue_ecdsap256_sign_sig_cleanup(mb_queue_ecdsap256_sign_sig *queue)
{
    if (queue == NULL)
        return 1;

    if (0 == enable_external_polling) {
        pthread_mutex_destroy(&queue->mb_queue_mutex);
    }
    return 0;
}

int mb_queue_ecdsap256_sign_sig_enqueue(mb_queue_ecdsap256_sign_sig *queue,
                                        ecdsa_sign_sig_op_data *item)
{
    if (queue == NULL || item == NULL)
        return 1;

    if (0 == enable_external_polling) {
        pthread_mutex_lock(&queue->mb_queue_mutex);
    }

    if (queue->disabled == 1) {
        if (0 == enable_external_polling) {
            pthread_mutex_unlock(&queue->mb_queue_mutex);
        }
        return 1;
    }

    if (queue->num_items == 0) {
        queue->tail = item;
        queue->head = item;
    } else {
        queue->tail->next = item;
        queue->tail = item;
    }
    queue->tail->next = NULL;
    queue->num_items++;

    if (enable_heuristic_polling) {
        QAT_ATOMIC_INC(num_asym_mb_items_in_queue);
    }

    if (0 == enable_external_polling) {
        pthread_mutex_unlock(&queue->mb_queue_mutex);
    }
    return 0;
}

ecdsa_sign_sig_op_data
    *mb_queue_ecdsap256_sign_sig_dequeue(mb_queue_ecdsap256_sign_sig *queue)
{
    ecdsa_sign_sig_op_data *item = NULL;

    if (queue == NULL)
        return NULL;

    if (0 == enable_external_polling) {
        pthread_mutex_lock(&queue->mb_queue_mutex);
    }

    if (queue->head == NULL) {
        if (0 == enable_external_polling) {
            pthread_mutex_unlock(&queue->mb_queue_mutex);
        }
        return NULL;
    }

    item = queue->head;
    queue->head = item->next;
    queue->num_items--;

    if (enable_heuristic_polling) {
        QAT_ATOMIC_DEC(num_asym_mb_items_in_queue);
    }

    if (queue->num_items == 0)
        queue->tail = NULL;

    if (0 == enable_external_polling) {
        pthread_mutex_unlock(&queue->mb_queue_mutex);
    }

    return item;
}

int mb_queue_ecdsap256_sign_sig_get_size(mb_queue_ecdsap256_sign_sig *queue)
{
    if (queue == NULL)
        return 0;

    return queue->num_items;
}

int mb_queue_ecdhp256_keygen_create(mb_queue_ecdhp256_keygen *queue)
{
    if (queue == NULL)
        return 1;

    if (0 == enable_external_polling) {
        pthread_mutex_init(&queue->mb_queue_mutex, NULL);
        pthread_mutex_lock(&queue->mb_queue_mutex);
    }
    queue->head = NULL;
    queue->tail = NULL;
    queue->disabled = 0;
    queue->num_items = 0;
    if (0 == enable_external_polling) {
        pthread_mutex_unlock(&queue->mb_queue_mutex);
    }

    return 0;
}

int mb_queue_ecdhp256_keygen_disable(mb_queue_ecdhp256_keygen *queue)
{
    if (queue == NULL)
        return 1;

    if (0 == enable_external_polling) {
        pthread_mutex_lock(&queue->mb_queue_mutex);
    }
    queue->disabled = 1;
    if (0 == enable_external_polling) {
        pthread_mutex_unlock(&queue->mb_queue_mutex);
    }

    return 0;
}

int mb_queue_ecdhp256_keygen_cleanup(mb_queue_ecdhp256_keygen *queue)
{
    if (queue == NULL)
        return 1;

    if (0 == enable_external_polling) {
        pthread_mutex_destroy(&queue->mb_queue_mutex);
    }
    return 0;
}

int mb_queue_ecdhp256_keygen_enqueue(mb_queue_ecdhp256_keygen *queue,
                                     ecdh_keygen_op_data *item)
{
    if (queue == NULL || item == NULL)
        return 1;

    if (0 == enable_external_polling) {
        pthread_mutex_lock(&queue->mb_queue_mutex);
    }

    if (queue->disabled == 1) {
        if (0 == enable_external_polling) {
            pthread_mutex_unlock(&queue->mb_queue_mutex);
        }
        return 1;
    }

    if (queue->num_items == 0) {
        queue->tail = item;
        queue->head = item;
    } else {
        queue->tail->next = item;
        queue->tail = item;
    }
    queue->tail->next = NULL;
    queue->num_items++;

    if (enable_heuristic_polling) {
        QAT_ATOMIC_INC(num_asym_mb_items_in_queue);
    }

    if (0 == enable_external_polling) {
        pthread_mutex_unlock(&queue->mb_queue_mutex);
    }
    return 0;
}

ecdh_keygen_op_data
    *mb_queue_ecdhp256_keygen_dequeue(mb_queue_ecdhp256_keygen *queue)
{
    ecdh_keygen_op_data *item = NULL;

    if (queue == NULL) {
        return NULL;
    }

    if (0 == enable_external_polling) {
        pthread_mutex_lock(&queue->mb_queue_mutex);
    }

    if (queue->head == NULL) {
        if (0 == enable_external_polling) {
            pthread_mutex_unlock(&queue->mb_queue_mutex);
        }
        return NULL;
    }

    item = queue->head;
    queue->head = item->next;
    queue->num_items--;

    if (enable_heuristic_polling) {
        QAT_ATOMIC_DEC(num_asym_mb_items_in_queue);
    }

    if (queue->num_items == 0)
        queue->tail = NULL;

    if (0 == enable_external_polling) {
        pthread_mutex_unlock(&queue->mb_queue_mutex);
    }

    return item;
}

int mb_queue_ecdhp256_keygen_get_size(mb_queue_ecdhp256_keygen *queue)
{
    if (queue == NULL)
        return 0;

    return queue->num_items;
}

int mb_queue_ecdhp256_compute_create(mb_queue_ecdhp256_compute *queue)
{
    if (queue == NULL)
        return 1;

    if (0 == enable_external_polling) {
        pthread_mutex_init(&queue->mb_queue_mutex, NULL);
        pthread_mutex_lock(&queue->mb_queue_mutex);
    }
    queue->head = NULL;
    queue->tail = NULL;
    queue->disabled = 0;
    queue->num_items = 0;
    if (0 == enable_external_polling) {
        pthread_mutex_unlock(&queue->mb_queue_mutex);
    }

    return 0;
}

int mb_queue_ecdhp256_compute_disable(mb_queue_ecdhp256_compute *queue)
{
    if (queue == NULL)
        return 1;

    if (0 == enable_external_polling) {
        pthread_mutex_lock(&queue->mb_queue_mutex);
    }
    queue->disabled = 1;
    if (0 == enable_external_polling) {
        pthread_mutex_unlock(&queue->mb_queue_mutex);
    }

    return 0;
}

int mb_queue_ecdhp256_compute_cleanup(mb_queue_ecdhp256_compute *queue)
{
    if (queue == NULL)
        return 1;

    if (0 == enable_external_polling) {
        pthread_mutex_destroy(&queue->mb_queue_mutex);
    }
    return 0;
}

int mb_queue_ecdhp256_compute_enqueue(mb_queue_ecdhp256_compute *queue,
                                      ecdh_compute_op_data *item)
{
    if (queue == NULL || item == NULL)
        return 1;

    if (0 == enable_external_polling) {
        pthread_mutex_lock(&queue->mb_queue_mutex);
    }

    if (queue->disabled == 1) {
        if (0 == enable_external_polling) {
            pthread_mutex_unlock(&queue->mb_queue_mutex);
        }
        return 1;
    }

    if (queue->num_items == 0) {
        queue->tail = item;
        queue->head = item;
    } else {
        queue->tail->next = item;
        queue->tail = item;
    }
    queue->tail->next = NULL;
    queue->num_items++;

    if (enable_heuristic_polling) {
        QAT_ATOMIC_INC(num_asym_mb_items_in_queue);
    }

    if (0 == enable_external_polling) {
        pthread_mutex_unlock(&queue->mb_queue_mutex);
    }
    return 0;
}

ecdh_compute_op_data
    *mb_queue_ecdhp256_compute_dequeue(mb_queue_ecdhp256_compute *queue)
{
    ecdh_compute_op_data *item = NULL;

    if (queue == NULL)
        return NULL;

    if (0 == enable_external_polling) {
        pthread_mutex_lock(&queue->mb_queue_mutex);
    }

    if (queue->head == NULL) {
        if (0 == enable_external_polling) {
            pthread_mutex_unlock(&queue->mb_queue_mutex);
        }
        return NULL;
    }

    item = queue->head;
    queue->head = item->next;
    queue->num_items--;

    if (enable_heuristic_polling) {
        QAT_ATOMIC_DEC(num_asym_mb_items_in_queue);
    }

    if (queue->num_items == 0)
        queue->tail = NULL;

    if (0 == enable_external_polling) {
        pthread_mutex_unlock(&queue->mb_queue_mutex);
    }

    return item;
}

int mb_queue_ecdhp256_compute_get_size(mb_queue_ecdhp256_compute *queue)
{
    if (queue == NULL)
        return 0;

    return queue->num_items;
}

int mb_queue_ecdhp384_keygen_create(mb_queue_ecdhp384_keygen *queue)
{
    if (queue == NULL)
        return 1;

    if (0 == enable_external_polling) {
        pthread_mutex_init(&queue->mb_queue_mutex, NULL);
        pthread_mutex_lock(&queue->mb_queue_mutex);
    }
    queue->head = NULL;
    queue->tail = NULL;
    queue->disabled = 0;
    queue->num_items = 0;
    if (0 == enable_external_polling) {
        pthread_mutex_unlock(&queue->mb_queue_mutex);
    }
    return 0;
}

int mb_queue_ecdhp384_keygen_disable(mb_queue_ecdhp384_keygen *queue)
{
    if (queue == NULL)
        return 1;

    if (0 == enable_external_polling) {
        pthread_mutex_lock(&queue->mb_queue_mutex);
    }
    queue->disabled = 1;
    if (0 == enable_external_polling) {
        pthread_mutex_unlock(&queue->mb_queue_mutex);
    }
    return 0;
}

int mb_queue_ecdhp384_keygen_cleanup(mb_queue_ecdhp384_keygen *queue)
{
    if (queue == NULL)
        return 1;

    if (0 == enable_external_polling) {
        pthread_mutex_destroy(&queue->mb_queue_mutex);
    }
    return 0;
}

int mb_queue_ecdhp384_keygen_enqueue(mb_queue_ecdhp384_keygen *queue,
        ecdh_keygen_op_data *item)
{
    if (queue == NULL || item == NULL)
        return 1;

    if (0 == enable_external_polling) {
        pthread_mutex_lock(&queue->mb_queue_mutex);
    }

    if (queue->disabled == 1) {
        if (0 == enable_external_polling) {
            pthread_mutex_unlock(&queue->mb_queue_mutex);
        }
        return 1;
    }

    if (queue->num_items == 0) {
        queue->tail = item;
        queue->head = item;
    } else {
        queue->tail->next = item;
        queue->tail = item;
    }
    queue->tail->next = NULL;
    queue->num_items++;

    if (enable_heuristic_polling) {
        QAT_ATOMIC_INC(num_asym_mb_items_in_queue);
    }

    if (0 == enable_external_polling) {
        pthread_mutex_unlock(&queue->mb_queue_mutex);
    }
    return 0;
}

ecdh_keygen_op_data
    *mb_queue_ecdhp384_keygen_dequeue(mb_queue_ecdhp384_keygen *queue)
{
    ecdh_keygen_op_data *item = NULL;

    if (queue == NULL) {
        return NULL;
    }
    if (0 == enable_external_polling) {
        pthread_mutex_lock(&queue->mb_queue_mutex);
    }
    if (queue->head == NULL) {
        if (0 == enable_external_polling) {
            pthread_mutex_unlock(&queue->mb_queue_mutex);
        }
        return NULL;
    }
    item = queue->head;
    queue->head = item->next;
    queue->num_items--;

    if (enable_heuristic_polling) {
        QAT_ATOMIC_DEC(num_asym_mb_items_in_queue);
    }
    if (queue->num_items == 0)
        queue->tail = NULL;

    if (0 == enable_external_polling) {
        pthread_mutex_unlock(&queue->mb_queue_mutex);
    }
    return item;
}

int mb_queue_ecdhp384_keygen_get_size(mb_queue_ecdhp384_keygen *queue)
{
    if (queue == NULL)
        return 0;

    return queue->num_items;
}

int mb_queue_ecdhp384_compute_create(mb_queue_ecdhp384_compute *queue)
{
    if (queue == NULL)
        return 1;

    if (0 == enable_external_polling) {
        pthread_mutex_init(&queue->mb_queue_mutex, NULL);
        pthread_mutex_lock(&queue->mb_queue_mutex);
    }
    queue->head = NULL;
    queue->tail = NULL;
    queue->disabled = 0;
    queue->num_items = 0;

    if (0 == enable_external_polling) {
        pthread_mutex_unlock(&queue->mb_queue_mutex);
    }
    return 0;
}

int mb_queue_ecdhp384_compute_disable(mb_queue_ecdhp384_compute *queue)
{
    if (queue == NULL)
        return 1;

    if (0 == enable_external_polling) {
        pthread_mutex_lock(&queue->mb_queue_mutex);
    }
    queue->disabled = 1;
    if (0 == enable_external_polling) {
        pthread_mutex_unlock(&queue->mb_queue_mutex);
    }
    return 0;
}

int mb_queue_ecdhp384_compute_cleanup(mb_queue_ecdhp384_compute *queue)
{
    if (queue == NULL)
        return 1;

    if (0 == enable_external_polling) {
        pthread_mutex_destroy(&queue->mb_queue_mutex);
    }
    return 0;
}

int mb_queue_ecdhp384_compute_enqueue(mb_queue_ecdhp384_compute *queue,
        ecdh_compute_op_data *item)
{
    if (queue == NULL || item == NULL)
        return 1;

    if (0 == enable_external_polling) {
        pthread_mutex_lock(&queue->mb_queue_mutex);
    }
    if (queue->disabled == 1) {
        if (0 == enable_external_polling) {
            pthread_mutex_unlock(&queue->mb_queue_mutex);
        }
        return 1;
    }
    if (queue->num_items == 0) {
        queue->tail = item;
        queue->head = item;
    } else {
        queue->tail->next = item;
        queue->tail = item;
    }
    queue->tail->next = NULL;
    queue->num_items++;

    if (enable_heuristic_polling) {
        QAT_ATOMIC_INC(num_asym_mb_items_in_queue);
    }
    if (0 == enable_external_polling) {
        pthread_mutex_unlock(&queue->mb_queue_mutex);
    }
    return 0;
}

ecdh_compute_op_data * mb_queue_ecdhp384_compute_dequeue(mb_queue_ecdhp384_compute *queue)
{
    ecdh_compute_op_data *item = NULL;

    if (queue == NULL)
        return NULL;

    if (0 == enable_external_polling) {
        pthread_mutex_lock(&queue->mb_queue_mutex);
    }
    if (queue->head == NULL) {
        if (0 == enable_external_polling) {
            pthread_mutex_unlock(&queue->mb_queue_mutex);
        }
        return NULL;
    }
    item = queue->head;
    queue->head = item->next;
    queue->num_items--;

    if (enable_heuristic_polling) {
        QAT_ATOMIC_DEC(num_asym_mb_items_in_queue);
    }
    if (queue->num_items == 0)
        queue->tail = NULL;
    if (0 == enable_external_polling) {
        pthread_mutex_unlock(&queue->mb_queue_mutex);
    }
    return item;
}

int mb_queue_ecdhp384_compute_get_size(mb_queue_ecdhp384_compute *queue)
{
    if (queue == NULL)
        return 0;

    return queue->num_items;
}

int mb_queue_ecdsap384_sign_create(mb_queue_ecdsap384_sign *queue)
{
    if (queue == NULL)
        return 1;

    if (0 == enable_external_polling) {
        pthread_mutex_init(&queue->mb_queue_mutex, NULL);
        pthread_mutex_lock(&queue->mb_queue_mutex);
    }
    queue->head = NULL;
    queue->tail = NULL;
    queue->disabled = 0;
    queue->num_items = 0;
    if (0 == enable_external_polling) {
        pthread_mutex_unlock(&queue->mb_queue_mutex);
    }

    return 0;
}

int mb_queue_ecdsap384_sign_disable(mb_queue_ecdsap384_sign *queue)
{
    if (queue == NULL)
        return 1;

    if (0 == enable_external_polling) {
        pthread_mutex_lock(&queue->mb_queue_mutex);
    }
    queue->disabled = 1;
    if (0 == enable_external_polling) {
        pthread_mutex_unlock(&queue->mb_queue_mutex);
   }

    return 0;
}

int mb_queue_ecdsap384_sign_cleanup(mb_queue_ecdsap384_sign *queue)
{
    if (queue == NULL)
        return 1;

    if (0 == enable_external_polling) {
        pthread_mutex_destroy(&queue->mb_queue_mutex);
    }
    return 0;
}

int mb_queue_ecdsap384_sign_enqueue(mb_queue_ecdsap384_sign *queue,
                                    ecdsa_sign_op_data *item)
{
    if (queue == NULL || item == NULL)
        return 1;

    if (0 == enable_external_polling) {
        pthread_mutex_lock(&queue->mb_queue_mutex);
    }

    if (queue->disabled == 1) {
        if (0 == enable_external_polling) {
            pthread_mutex_unlock(&queue->mb_queue_mutex);
        }
        return 1;
    }

    if (queue->num_items == 0) {
        queue->tail = item;
        queue->head = item;
    } else {
        queue->tail->next = item;
        queue->tail = item;
    }
    queue->tail->next = NULL;
    queue->num_items++;

    if (enable_heuristic_polling) {
        QAT_ATOMIC_INC(num_asym_mb_items_in_queue);
    }

    if (0 == enable_external_polling) {
        pthread_mutex_unlock(&queue->mb_queue_mutex);
    }
    return 0;
}

ecdsa_sign_op_data * mb_queue_ecdsap384_sign_dequeue(mb_queue_ecdsap384_sign *queue)
{
    ecdsa_sign_op_data *item = NULL;

    if (queue == NULL)
        return NULL;

    if (0 == enable_external_polling) {
        pthread_mutex_lock(&queue->mb_queue_mutex);
    }

    if (queue->head == NULL) {
        if (0 == enable_external_polling) {
            pthread_mutex_unlock(&queue->mb_queue_mutex);
        }
        return NULL;
    }

    item = queue->head;
    queue->head = item->next;
    queue->num_items--;

    if (enable_heuristic_polling) {
        QAT_ATOMIC_DEC(num_asym_mb_items_in_queue);
    }

    if (queue->num_items == 0)
        queue->tail = NULL;

    if (0 == enable_external_polling) {
        pthread_mutex_unlock(&queue->mb_queue_mutex);
    }

    return item;
}

int mb_queue_ecdsap384_sign_get_size(mb_queue_ecdsap384_sign *queue)
{
    if (queue == NULL)
        return 0;

    return queue->num_items;
}

int mb_queue_ecdsap384_sign_setup_create(mb_queue_ecdsap384_sign_setup *queue)
{
    if (queue == NULL)
        return 1;

    if (0 == enable_external_polling) {
        pthread_mutex_init(&queue->mb_queue_mutex, NULL);
        pthread_mutex_lock(&queue->mb_queue_mutex);
    }
    queue->head = NULL;
    queue->tail = NULL;
    queue->disabled = 0;
    queue->num_items = 0;
    if (0 == enable_external_polling) {
        pthread_mutex_unlock(&queue->mb_queue_mutex);
    }

    return 0;
}

int mb_queue_ecdsap384_sign_setup_disable(mb_queue_ecdsap384_sign_setup *queue)
{
    if (queue == NULL)
        return 1;

    if (0 == enable_external_polling) {
        pthread_mutex_lock(&queue->mb_queue_mutex);
    }
    queue->disabled = 1;
    if (0 == enable_external_polling) {
        pthread_mutex_unlock(&queue->mb_queue_mutex);
    }

    return 0;
}

int mb_queue_ecdsap384_sign_setup_cleanup(mb_queue_ecdsap384_sign_setup *queue)
{
    if (queue == NULL)
        return 1;

    if (0 == enable_external_polling) {
        pthread_mutex_destroy(&queue->mb_queue_mutex);
    }
    return 0;
}

int mb_queue_ecdsap384_sign_setup_enqueue(mb_queue_ecdsap384_sign_setup *queue,
                                          ecdsa_sign_setup_op_data *item)
{
    if (queue == NULL || item == NULL)
        return 1;

    if (0 == enable_external_polling) {
        pthread_mutex_lock(&queue->mb_queue_mutex);
    }

    if (queue->disabled == 1) {
        if (0 == enable_external_polling) {
            pthread_mutex_unlock(&queue->mb_queue_mutex);
        }
        return 1;
    }

    if (queue->num_items == 0) {
        queue->tail = item;
        queue->head = item;
    } else {
        queue->tail->next = item;
        queue->tail = item;
    }
    queue->tail->next = NULL;
    queue->num_items++;

    if (enable_heuristic_polling) {
        QAT_ATOMIC_INC(num_asym_mb_items_in_queue);
    }

    if (0 == enable_external_polling) {
        pthread_mutex_unlock(&queue->mb_queue_mutex);
    }
    return 0;
}

ecdsa_sign_setup_op_data
    *mb_queue_ecdsap384_sign_setup_dequeue(mb_queue_ecdsap384_sign_setup *queue)
{
    ecdsa_sign_setup_op_data *item = NULL;

    if (queue == NULL)
        return NULL;

    if (0 == enable_external_polling) {
        pthread_mutex_lock(&queue->mb_queue_mutex);
    }

    if (queue->head == NULL) {
        if (0 == enable_external_polling) {
            pthread_mutex_unlock(&queue->mb_queue_mutex);
        }
        return NULL;
    }
    item = queue->head;
    queue->head = item->next;
    queue->num_items--;

    if (enable_heuristic_polling) {
        QAT_ATOMIC_DEC(num_asym_mb_items_in_queue);
    }

    if (queue->num_items == 0)
        queue->tail = NULL;

    if (0 == enable_external_polling) {
        pthread_mutex_unlock(&queue->mb_queue_mutex);
    }

    return item;
}

int mb_queue_ecdsap384_sign_setup_get_size(mb_queue_ecdsap384_sign_setup *queue)
{
    if (queue == NULL)
        return 0;

    return queue->num_items;
}

int mb_queue_ecdsap384_sign_sig_create(mb_queue_ecdsap384_sign_sig *queue)
{
    if (queue == NULL)
        return 1;

    if (0 == enable_external_polling) {
        pthread_mutex_init(&queue->mb_queue_mutex, NULL);
        pthread_mutex_lock(&queue->mb_queue_mutex);
    }
    queue->head = NULL;
    queue->tail = NULL;
    queue->disabled = 0;
    queue->num_items = 0;
    if (0 == enable_external_polling) {
        pthread_mutex_unlock(&queue->mb_queue_mutex);
    }

    return 0;
}

int mb_queue_ecdsap384_sign_sig_disable(mb_queue_ecdsap384_sign_sig *queue)
{
    if (queue == NULL)
        return 1;

    if (0 == enable_external_polling) {
        pthread_mutex_lock(&queue->mb_queue_mutex);
    }
    queue->disabled = 1;
    if (0 == enable_external_polling) {
        pthread_mutex_unlock(&queue->mb_queue_mutex);
    }

    return 0;
}

int mb_queue_ecdsap384_sign_sig_cleanup(mb_queue_ecdsap384_sign_sig *queue)
{
    if (queue == NULL)
        return 1;

    if (0 == enable_external_polling) {
        pthread_mutex_destroy(&queue->mb_queue_mutex);
    }
    return 0;
}

int mb_queue_ecdsap384_sign_sig_enqueue(mb_queue_ecdsap384_sign_sig *queue,
                                        ecdsa_sign_sig_op_data *item)
{
    if (queue == NULL || item == NULL)
        return 1;

    if (0 == enable_external_polling) {
        pthread_mutex_lock(&queue->mb_queue_mutex);
    }

    if (queue->disabled == 1) {
        if (0 == enable_external_polling) {
            pthread_mutex_unlock(&queue->mb_queue_mutex);
        }
        return 1;
    }

    if (queue->num_items == 0) {
        queue->tail = item;
        queue->head = item;
    } else {
        queue->tail->next = item;
        queue->tail = item;
    }
    queue->tail->next = NULL;
    queue->num_items++;

    if (enable_heuristic_polling) {
        QAT_ATOMIC_INC(num_asym_mb_items_in_queue);
    }

    if (0 == enable_external_polling) {
        pthread_mutex_unlock(&queue->mb_queue_mutex);
    }
    return 0;
}

ecdsa_sign_sig_op_data
    *mb_queue_ecdsap384_sign_sig_dequeue(mb_queue_ecdsap384_sign_sig *queue)
{
    ecdsa_sign_sig_op_data *item = NULL;

    if (queue == NULL)
        return NULL;

    if (0 == enable_external_polling) {
        pthread_mutex_lock(&queue->mb_queue_mutex);
    }

    if (queue->head == NULL) {
        if (0 == enable_external_polling) {
            pthread_mutex_unlock(&queue->mb_queue_mutex);
        }
        return NULL;
    }

    item = queue->head;
    queue->head = item->next;
    queue->num_items--;

    if (enable_heuristic_polling) {
        QAT_ATOMIC_DEC(num_asym_mb_items_in_queue);
    }

    if (queue->num_items == 0)
        queue->tail = NULL;

    if (0 == enable_external_polling) {
        pthread_mutex_unlock(&queue->mb_queue_mutex);
    }

    return item;

}

int mb_queue_ecdsap384_sign_sig_get_size(mb_queue_ecdsap384_sign_sig *queue)
{
    if (queue == NULL)
        return 0;

    return queue->num_items;
}
