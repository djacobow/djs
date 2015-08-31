#ifndef DJS_H
#define DJS_H

// Author: Dave Jacobowitz
//
// djs is "Dave's JSON" -- it's a SUPER minimalist and lightweight JSON
//  parse, aimed at embedded platforms with highly limited resources,
//  particularly, memory.
//
// Pros:
//   - small, simple, c-only
//   - allocates no memory (operates on original string buffer)
//   - no fp stuff linked in
//
// Cons:
//   - not fully json compliant
//   - floating point values not handled
//     (treated as ints up to the decimal point)
//   - not Unicode-aware. This is a plain ascii implementation.
//   - because no tree is created, accessing many members of
//     a json string will require repeated scans of the string;
//     This trades time for space.

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



/// Create a token from a JSON string
djs_tok_t djs_createFrom(const char *src);

/// find something inside an object token by name, return value (by pointer) is
/// another token
bool      djs_findNamed(const djs_tok_t st, const char *ss, djs_tok_t *vt);

/// find something inside an array token by index, return value (by pointer) is
/// another token
bool      djs_findIndex(const djs_tok_t st, const int idx, djs_tok_t *vt);

/// return the length of an array token, in items
int       djs_getLength(const djs_tok_t st);

/// debugging function to print a token
void      djs_showTok(const djs_tok_t t, bool show = true);

/// extract an integer value from a token. Will return true if the
/// token was an interger, or could be converted to one from a string
bool      djs_getInt(const djs_tok_t t, int *v);

/// extract an bool value from a token. Will return true if the
/// token was an bool , or could be converted to one from a string
bool      djs_getBool(const djs_tok_t t, bool *b);

/// extract an string value from a token. Will return true the token
/// was valid anad if the receiving string is long enough. naked values
/// like integers or booleans will be returned as strings
bool      djs_getStr(const djs_tok_t t, char *s, int sl);

/// return true if the token is initialized and valid
bool      djs_valid(const djs_tok_t st);
#ifdef __cplusplus
}
#endif

#endif

