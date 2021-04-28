/*
 *  hash -- A generic hash table implementation.
 *
 *  Created by liuxin <6835021@qq.com>
 *  2021/03/05
 *
 *  Modified hash_iter_hasnext
 *  2021/4/8
 */

#ifndef __HASH_H__
#define __HASH_H__

#include <stddef.h>       /* use type size_t */
#include <stdint.h>       /* use type uint32_t */
#include <stdbool.h>      /* use type bool, true and false in C99 */  

struct hash;              /* incomplete type for information hiding */

struct hash *hash_create_bucket_count(size_t bucket_count, 
                              uint32_t (*hash_value)(const void *key),
                              bool (*hash_equal)(const void *key1, const void *key2));
							  
struct hash *hash_create(uint32_t (*hash_value)(const void *key),
                     bool (*hash_equal)(const void *key1, const void *key2));
			
void hash_destroy(struct hash *hash);

void hash_clear(struct hash *hash);
size_t hash_count(const struct hash *hash);
bool hash_isempty(const struct hash *hash);

void *hash_get_fast(struct hash *hash, const void *key);
void *hash_get(const struct hash *hash, const void *key);

void *hash_put(struct hash *hash, const void *key, const void *value);
void *hash_remove(struct hash *hash, const void *key);

/* only used to record the result of hash_iter_next and hash_iter_remove.
 * can't be modified. 
 */
struct hash_pair {
    const void *key;
    const void *value;
};

/* functions for the iterator of the hash table */

struct hash_iter;

struct hash_iter *hash_make_iter(const struct hash *hash);
void hash_iter_destroy(struct hash_iter *iter);

bool hash_iter_hasnext(struct hash_iter *iter);
struct hash_pair hash_iter_next(struct hash_iter *iter);
//struct hash_pair hash_iter_remove(struct hash_iter *iter);

void hash_iter_rewind(struct hash_iter *iter);

#endif /* __HASH_H__ */
