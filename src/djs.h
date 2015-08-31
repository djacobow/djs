#ifndef DJS_H
#define DJS_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum djs_type_t {
    _djs_na = 0,
    _djs_valid_untyped,  // this is a state for a token that as been
    // initialized by that detType has not been
    // called on yet
    djs_array,
    djs_hash,
    djs_string,
    djs_naked,
}
djs_type_t;

typedef enum djs_state_t {
    djs_st_start,
    djs_st_in_key,
    djs_st_key_done,
    djs_st_in_value,
    djs_st_value_done,
    djs_all_done,
} djs_state_t;

typedef struct djs_tok_t {
    const char *bs;
    const char *es;
    djs_type_t t;
} djs_tok_t;



djs_tok_t djs_createFrom(const char *src);
bool      djs_findNamed(const djs_tok_t st, const char *ss, djs_tok_t *vt);
void      djs_showTok(const djs_tok_t t, bool show = true);
bool      djs_getInt(const djs_tok_t t, int *v);
bool      djs_getBool(const djs_tok_t t, bool *b);
bool      djs_getStr(const djs_tok_t t, char *s, int sl);
bool      djs_findIndex(const djs_tok_t st, const int idx, djs_tok_t *vt);
int       djs_getLength(const djs_tok_t st);
bool      djs_valid(const djs_tok_t st);
#ifdef __cplusplus
}
#endif

#endif

