/*
 *  hash -- A generic hash table implementation.
 *
 *  Created by liuxin <6835021@qq.com>
 *  2021/03/05
 *
 *  Modified hash_iter_hasnext
 *  2021/4/8
 *  Modified hash_equal and hash_value
 *  2021/4/13
 */

#include "hash.h"

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#define HASH_BUCKET_COUNT_INIT    1024

struct hash_node {
	const void *key;
	void *value;
	uint32_t hv;                 /* hash value of key */
	struct hash_node *next;
};

struct hash {
	struct hash_node **buckets;  /* bucket array in hash table */
	size_t bucket_count;         /* bucket count in hash table */
	size_t element_count;        /* element count in hash table */
	
	uint32_t (*hash_value)(const void *key);
	bool (*hash_equal)(const void *key1, const void *key2);    /* return 0 if equal */
};

/* tool function 
 * mode == 1 : fast mode
 * mode == 0 : normal mode
 */
static struct hash_node *
hash_get_node(struct hash *hash, 
              const void *key, 
              uint32_t hv, 
              size_t hi, 
              int mode);

struct hash *
hash_create_bucket_count(size_t bucket_count, 
                         uint32_t (*hash_value)(const void *key),
                         bool (*hash_equal)(const void *key1, const void *key2))
{
	struct hash *hash = NULL;
	
	assert(bucket_count != 0);
	assert(hash_value != NULL);
	assert(hash_equal != NULL);
	
	hash = (struct hash *)malloc(sizeof(struct hash));
	if (hash == NULL) return NULL;
	
	assert(hash);
	hash->buckets = NULL;
	hash->bucket_count = 0;
	hash->element_count = 0;
	hash->hash_value = hash_value;
	hash->hash_equal = hash_equal;
	
	hash->buckets = (struct hash_node **)calloc(bucket_count, sizeof(struct hash_node *));
	if (hash->buckets == NULL) {
		free(hash);
		return NULL;
	}
	
	assert(hash->buckets);
	hash->bucket_count = bucket_count;
	
	return hash;
}
							  
struct hash *
hash_create(uint32_t (*hash_value)(const void *key),
            bool (*hash_equal)(const void *key1, const void *key2))
{
	assert(hash_value != NULL);
	assert(hash_equal != NULL);
	
	return hash_create_bucket_count(HASH_BUCKET_COUNT_INIT, hash_value, hash_equal);
}
			
void
hash_destroy(struct hash *hash)
{
	assert(hash != NULL);
	assert(hash->buckets != NULL);
	
	free(hash->buckets);
	free(hash);
	
	return;
}

void
hash_clear(struct hash *hash)
{
	struct hash_node *node = NULL;
	size_t i;
	
	assert(hash != NULL);
	assert(hash->buckets != NULL);
	assert(hash->bucket_count> 0);

	for (i = 0; i < hash->bucket_count; i++) {
		if (hash->buckets[i]) {
			node = hash->buckets[i];
			hash->buckets[i] = node->next;
			free(node);
			hash->element_count--;
		}
	}

	assert(hash->element_count == 0);

    return; 
}

size_t
hash_count(const struct hash *hash)
{
	assert(hash != NULL);
	assert(hash->buckets != NULL);
	assert(hash->element_count >= 0);
	
	return hash->element_count;
}

bool
hash_isempty(const struct hash *hash)
{
	assert(hash != NULL);
	assert(hash->buckets != NULL);
	assert(hash->element_count >= 0);
	
	return hash->element_count == 0;	
}

void *
hash_get_fast(struct hash *hash, const void *key)
{
	struct hash_node *found = NULL;
	uint32_t hv;  /* hash value */
	size_t hi;    /* hash index */

	assert(key != NULL);
	assert(hash != NULL);
	assert(hash->buckets != NULL);
	
	hv = (*hash->hash_value)(key);
	hi = hv % hash->bucket_count;
	
    found = hash_get_node(hash, key, hv, hi, 1); /* 1 : fast mode */
	
    if (found) {
		return found->value;
	}
	
	return NULL;
}

void *
hash_get(const struct hash *hash, const void *key)
{
	struct hash_node *found = NULL;
	uint32_t hv;  /* hash value */
	size_t hi;    /* hash index */

	assert(hash != NULL);
	assert(hash->buckets != NULL);
	assert(key != NULL);
	
	hv = (*hash->hash_value)(key);
	hi = hv % hash->bucket_count;
	
	/* 0 : nomal mode */
    found = hash_get_node((struct hash *)hash, key, hv, hi, 0);  
    
    if (found) {
       return found->value;
    }
	
	return NULL;   /* if not found */
}

void *
hash_put(struct hash *hash, const void *key, const void *value)
{
	struct hash_node *find = NULL;
	struct hash_node *node = NULL;
	void *ret = NULL;
	uint32_t hv = 0;
	size_t hi = 0;
	
	assert(hash != NULL);
	assert(hash->buckets != NULL);
	assert(key != NULL);
	
	hv = (*hash->hash_value)(key);
	hi = hv % hash->bucket_count;
	
	find = hash_get_node(hash, key, hv, hi, 1);
	
	if (find) {
		ret = find->value;
		find->value = (void *)value;
		return ret;
	} 
	
	/* not found */
	assert(find == NULL);
	node = (struct hash_node *)malloc(sizeof(struct hash_node));
	if (node == NULL) {
	    perror("malloc() failed in hash_put()");
		exit(EXIT_FAILURE);
	} 
	
	assert(node);
	node->key = key;
	node->value = (void *)value;
	node->hv = hv;  /* !!! */
	node->next = NULL;
	
	node->next = hash->buckets[hi];
	hash->buckets[hi] = node;
	hash->element_count++;
	return NULL;
}

void *
hash_remove(struct hash *hash, const void *key)
{
	struct hash_node *p = NULL;
	struct hash_node *prev = NULL;
	void *ret = NULL;
	uint32_t hv = 0;
	size_t hi = 0;
	
	assert(hash != NULL);
	assert(hash->buckets != NULL);
	assert(key != NULL);
	
	hv = (*hash->hash_value)(key);
	hi = hv % hash->bucket_count;
	
	p = hash->buckets[hi];
	while (p) {
		if ((p->hv == hv) && (*hash->hash_equal)(p->key, key) == true) {
			if (prev) {
				prev->next = p->next;
			} else {
				hash->buckets[hi] = p->next;
			}
			ret = p->value;
			free(p);
			hash->element_count--;
			return ret;
		}
		
		prev = p;  /**/
		p = p->next;
	} 
	
	return NULL;
}

static struct hash_node *
hash_get_node(struct hash *hash, 
              const void *key, 
              uint32_t hv, 
              size_t hi, 
              int mode)
{
	struct hash_node *p = NULL;
	struct hash_node *prev = NULL;
	
	assert(hash != NULL);
	assert(hash->buckets != NULL);
	assert(key != NULL);
	assert(mode == 0 || mode == 1);
	
	p = hash->buckets[hi];
	while (p) {
		if ((p->hv == hv) && (*hash->hash_equal)(p->key, key) == true) {
            /* move to the head of list */
            if (mode && prev) {    
				prev->next = p->next;
				p->next = hash->buckets[hi];
				hash->buckets[hi] = p;
			} 
			return p;   /* if found */
        }

        prev = p;
		p = p->next;
	}

    return NULL;  /* if not found */
}

/* functions for the iterator of the hash table */

struct hash_iter {
    const struct hash *hash;   /* const - once be created, can't be modified */
    size_t bi;                 /* current bucket index */
    struct hash_node *ptr;     /* current hash node */       
}; 

struct hash_iter *
hash_make_iter(const struct hash *hash)
{
    struct hash_iter *iter = NULL;
    
    assert(hash != NULL);
    iter = (struct hash_iter *)malloc(sizeof(struct hash_iter));
    if (iter == NULL) return NULL;
    
    assert(iter != NULL);
    iter->hash = hash;
    iter->bi = 0;
    iter->ptr = NULL;
    
    return iter;  
}

void
hash_iter_destroy(struct hash_iter *iter)
{
    assert(iter != NULL);
    assert(iter->hash != NULL);
    
    free(iter);
    
    return;     
}

bool
hash_iter_hasnext(struct hash_iter *iter)
{
    assert(iter != NULL);
    
    /* lookup in one bucket */
    if (iter->ptr && iter->ptr->next) {
        iter->ptr = iter->ptr->next; 
        return true;          
    }
    
    /* 2021/4/6 modified */
    if (iter->ptr && !iter->ptr->next) {
        iter->bi++;              
    }
    
    /* lookup next bucket */
    while (iter->bi < iter->hash->bucket_count) {
       iter->ptr = iter->hash->buckets[iter->bi];
       if (iter->ptr) return true;
       iter->bi++;
    }
    
    return false;           
}

struct hash_pair 
hash_iter_next(struct hash_iter *iter)
{
    struct hash_pair pair = {NULL, NULL};
    
    assert(iter != NULL);
    assert(iter->ptr != NULL);
    
    pair.key = iter->ptr->key;
    pair.value = iter->ptr->value;
    
    return pair;                     
}

/*
struct hash_pair 
hash_iter_remove(struct hash_iter *iter)
{
                
}
*/

void hash_iter_rewind(struct hash_iter *iter)
{
    assert(iter != NULL);
    assert(iter->hash != NULL);
    
    iter->bi = 0;
    iter->ptr = NULL;
    
    return;
}

