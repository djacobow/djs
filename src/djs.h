#ifndef DJS_H
#define DJS_H

// Author: Dave Jacobowitz
//
// djs is "Dave's JSON" -- it's a SUPER minimalist and lightweight JSON
//  parse, aimed at embedded platforms with highly limited resources,
//  particularly, memory.
//
//  This is a somewhat "loose" "do what I mean" implementation, so 
//  sticklers for matching the specification exactly will not be happy. 
//  For example, if you have JSON like
//
//  { "foo": "true", "bar": "123" }
//
//  ... and request foo as a bool and bar as an integer, you'll get converted
//  values rather than an  error. 
//
//  This implementation also is cool with trailing commas, which 
//   about which real JSON parsers get upset.
//
// Pros:
//   - small, simple, c-only
//   - allocates no memory (operates on original string buffer)
//   - no fp stuff linked in
//   - tolerant of slightly off-kilter json
//
// Cons:
//   - NOT Unicode-aware. This is a plain ascii implementation.
//   - not fully json compliant
//   - floating point values not handled
//     (treated as ints up to the decimal point)
//   - because no tree is created, accessing many members of
//     a json string will require repeated scans of the string;
//     This trades time for space.

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif


typedef enum djs_type_t {
    _djs_na = 0,         // - an invalid or uninitialized tok
    _djs_valid_untyped,  // this is a state for a token that as been
    // initialized by that detType has not been
    // called on yet
    djs_array,
    djs_hash,
    djs_string,          // - a quoted scalar thing
    djs_naked,           // - an unquoted scalar thing (number or bool)
}
djs_type_t;

/// used internally by parsing state machine
typedef enum djs_state_t {
    djs_st_start,
    djs_st_in_key,
    djs_st_key_done,
    djs_st_in_value,
    djs_st_value_done,
    djs_all_done,
} djs_state_t;

/// the token itself. Tokens are very simple and small. They
/// contains pointers to the beginning and end of whatever 
/// that thing is, as well as a type descriptor for convenience.
typedef struct djs_tok_t {
    const char *bs;
    const char *es;
    djs_type_t t;
} djs_tok_t;



/// Create a token from a JSON string
djs_tok_t djs_createFrom(const char *src);

/// find something inside an object token by name, return value (by pointer) is
/// another token
bool      djs_findNamed(const djs_tok_t *st, const char *ss, djs_tok_t *vt);

/// find something inside an array token by index, return value (by pointer) is
/// another token
bool      djs_findIndex(const djs_tok_t *st, const int idx, djs_tok_t *vt);

/// return the length of an array token, in items
int       djs_getLength(const djs_tok_t *st);

/// debugging function to print a token
void      djs_showTok(const djs_tok_t *t, bool show = true);

/// extract an integer value from a token. Will return true if the
/// token was an interger, or could be converted to one from a string
bool      djs_getInt(const djs_tok_t *t, int *v);

/// extract a bool value from a token. Will return true if the
/// token was a bool , or could be converted to one from a string
bool      djs_getBool(const djs_tok_t *t, bool *b);

/// extract a string value from a token. Will return true if 
/// the token was valid and if the receiving string is long 
/// enough to hold it. naked (unquoted) values
/// like integers or booleans will be returned as strings
bool      djs_getStr(const djs_tok_t *t, char *s, int sl);

/// extract a string value from a token without copying 
/// anything. Will return true if the token
/// was valid. A pointer to the first and last char in the string
/// are returned. These are pointers to the original buffer, so
/// they are NOT NULL TERMINATED. Moreover, they are defined as 
/// const. You should not update the string returned.
bool      djs_getStr_noCopy(const djs_tok_t *t, 
                            const char **start, 
                            const char **end);


/// return true if the token is initialized and valid
bool      djs_valid(const djs_tok_t *st);
#ifdef __cplusplus
}
#endif

#endif

