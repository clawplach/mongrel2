#include <stdlib.h>
#include "tst.h"
#include <stdio.h>
#include <assert.h>
#include <dbg.h>

typedef struct tst_collect_t {
    list_t *values;
    tst_collect_test_cb tester;
    void *data;
} tst_collect_t;

static void tst_collect_build(void *value, tst_collect_t *results)
{
    if(!results->tester || results->tester(value, results->data)) {
        lnode_t *node = lnode_create(value);
        list_append(results->values, node);
    }
}

list_t *tst_collect(tst_t *root, const char *s, int len, tst_collect_test_cb tester, void *data)
{
    tst_collect_t results = {.values = NULL, .tester = tester, .data = data};
    tst_t *p = root;
    int i = 0;
    results.values = list_create(LISTCOUNT_T_MAX);

    // first we get to where we match the prefix
    while(i < len && p) {
        if (s[i] < p->splitchar) {
            p = p->low; 
        } else if (s[i] == p->splitchar) {
            i++;
            p = p->equal; 
        } else {
            p = p->high; 
        }
    }

    if(p) {
        // we found node matching this prefix, so traverse and collect
        tst_traverse(p, (tst_traverse_cb)tst_collect_build, &results);
    }

    return results.values;
}

void *tst_search_suffix(tst_t *root, const char *s, int len)
{
    tst_t *p = root;
    int i = len-1;

    debug(">> p: %p, s: %c, splitchar: %c, i: %d", p, s[i], p->splitchar, i);

    while(i > 0 && p) {
        debug("p: %p, s: %c, splitchar: %c, i: %d", p, s[i], p->splitchar, i);
        
        if (s[i] < p->splitchar) {
            p = p->low; 
        } else if (s[i] == p->splitchar) {
            i--;
            if(i < len) p = p->equal; 
        } else {
            p = p->high; 
        }
    }

    debug("<< p: %p, s: %c, i: %d", p, s[i], i);

    if(p) {
        return p->value;
    } else {
        return NULL; 
    }
}

void *tst_search(tst_t *root, const char *s, int len)
{
    tst_t *p = root;
    int i = 0;

    while(i < len && p) {

        if (s[i] < p->splitchar) {
            p = p->low; 
        } else if (s[i] == p->splitchar) {
            i++;
            if(i < len) p = p->equal; 
        } else {
            p = p->high; 
        }
    }

    if(p) {
        return p->value;
    } else {
        return NULL; 
    }
}

tst_t *tst_insert(tst_t *p, const char *s, int len, void *value)
{
    if (p == NULL) { 
        p = (tst_t *) calloc(sizeof(tst_t), 1);
        p->splitchar = *s; 
    }

    if (*s < p->splitchar) {
        p->low = tst_insert(p->low, s, len, value); 
    } else if (*s == p->splitchar) { 
        if (len > 1) {
            // not done yet, keep going but one less
            p->equal = tst_insert(p->equal, s+1, len - 1, value);
        } else {
            p->value = value;
        }
    } else {
        p->high = tst_insert(p->high, s, len, value);
    }

    return p; 
}

void tst_traverse(tst_t *p, tst_traverse_cb cb, void *data)
{
    if (!p) return;

    if(p->low) tst_traverse(p->low, cb, data);

    if (p->equal) {
        tst_traverse(p->equal, cb, data); 
    }

    if(p->high) tst_traverse(p->high, cb, data); 

    if(p->value) cb(p->value, data);
}

