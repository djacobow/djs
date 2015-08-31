#include "djs.h"
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>

#ifndef DJS_DEBUG
#define DJS_DEBUG (0)
#endif

#ifndef DJS_STRICT
#define DJS_STRICT (0)
#endif

#if DJS_DEBUG
#define DBG(...) printf(__VA_ARGS__)
#else
#define DBG(...)
#endif


// private to djs
bool       djs_trim(djs_tok_t *vt);
const char *djs_type2str(djs_tok_t t);
djs_tok_t  djs_createNull();
bool       djs_strEql(const djs_tok_t kt, const char *ks);
int        djs_fA2(const djs_tok_t st, bool find, const int idx,  djs_tok_t *vt);
bool       djs_getBlob(const djs_tok_t st, djs_tok_t *left, djs_tok_t *right);


djs_type_t
djs_detType(djs_tok_t it) {
    if (it.t == _djs_na) return _djs_na;
    djs_type_t rt = djs_naked;
    const char *ns = it.bs;
    bool done = false;
    while (!done) {
        // printf("i %d s %llx ns %llx it.s[i] %llx\n",i,it.s,ns,&it.s[i]);
        if      (*ns  == '[') {
            rt = djs_array;
            done = true;
        } else if (*ns  == '{') {
            rt = djs_hash;
            done = true;
        } else if (*ns  == '"') {
            rt = djs_string;
            done = true;
        } else if (isgraph(*ns)) done = true;
        ns++;
        done |= (ns == it.es);
    }
    it.t = rt;
    DBG("type %s\n",djs_type2str(it));
    return rt;
};

djs_tok_t djs_createFrom(const char *src) {
    djs_tok_t ot = djs_createNull();
    ot.bs = src;
    ot.es = ot.bs + strlen(src)-1;
    ot.t = _djs_valid_untyped;
    ot.t = djs_detType(ot);
    return ot;
};

djs_tok_t djs_createNull() {
    djs_tok_t ot;
    memset(&ot,0,sizeof(ot));
    return ot;
}

void
showRange(const char *bs, const char *es) {
    for (const char *is=bs; is<=es; is++) {
        DBG("%c",*is);
    }
};

bool djs_strEql(const djs_tok_t kt, const char *ks) {
    if (kt.t == _djs_na) return false;
    int ksl = strlen(ks);
    int ktl = (kt.es - kt.bs) + 1;
    if (!strlen(ks)) return false;
    if (ksl != ktl)  return false;
    const char *ts = kt.bs;
    for (int i=0; i<ksl; i++) {
        if (ks[i] != *ts++) return false;
    }
    return true;
};

const char *djs_type2str(djs_tok_t t) {
    return ((t.t == djs_hash)   ? "hash" :
            (t.t == djs_string) ? "str " :
            (t.t == djs_array)  ? "arry" :
            (t.t == djs_naked)  ? "nakd" : "????");
}

void djs_showTok(const djs_tok_t t, bool show) {
    if (!show) return;
    if (t.t != _djs_na) {
        printf("tok [%llx,%llx,%d] (%s) >>",
               t.bs,t.es,t.es-t.bs, djs_type2str(t));
        for (const char *is=t.bs; is<=t.es; is++) {
            printf("%c",*is);
        }
        printf("<<\n");
    } else {
        printf("tok [invalid]\n");
    }
};

bool djs_trim(djs_tok_t *vt) {
    int fix_count = 0;
    if (vt->t != _djs_na) {
        const char *is = vt->bs;
        bool done = false;
        int qcount = 0;
        while (!done) {
            bool trimmable = !isgraph(*is);
            if (vt->t == djs_string) {
                if (*is == '"') {
                    qcount += 1;
                    if (qcount == 1) trimmable = true;
                }
            }
            if (trimmable) vt->bs++;
            done |= (is == vt->es);
            done |= !trimmable;
            is++;
        }
        const char *js = vt->es;
        done = false;
        qcount = 0;
        while (!done) {
            bool trimmable = !isgraph(*js);
            if (vt->t == djs_string) {
                if (*js  == '"') {
                    qcount += 1;
                    if (qcount == 1) {
                        trimmable = true;
                    }
                }
            }
            if (trimmable) vt->es--;
            done |= (js == vt->bs);
            done |= !trimmable;
            js--;
        }
    }
    return (fix_count > 0);
}

bool djs_findIndex(const djs_tok_t st, const int idx, djs_tok_t *vt) {
    return djs_fA2(st, true, idx, vt);
}
int djs_getLength(const djs_tok_t st) {
    djs_tok_t dummy;
    return djs_fA2(st, false, 0, &dummy);
}

bool
djs_getBlob(const djs_tok_t st, djs_tok_t *left, djs_tok_t *right) {
    *left = st;
    *right = st;

    if (st.t != _djs_na) {
        char lastch = 0;
        bool in_string = false;
        int depth = 0;
        int last_depth = 0;
        for (const char *is=st.bs; is<=st.es; is++) {
            char inch = *is;
            if ((inch == '"') && (lastch != '\\'))  in_string = !in_string;
            if (!in_string) {
                if      (inch == '{') depth++;
                else if (inch == '}') depth--;
                else if (inch == '[') depth++;
                else if (inch == ']') depth--;
            }
            if ((!in_string && ((inch == ',') && (depth == 0))) ||
                    ((depth == -1) && (last_depth == 0)))  {
                left->es = is-1;
                right->bs = is+1;
                if (left->es >= left->bs) {
                    left->t = djs_detType(*left);
                    //DBG("blob before trim\n");
                    // djs_showTok(left,DJS_DEBUG);
                    djs_trim(left);
                    DBG("blob after trim\n");
                    djs_showTok(*left,DJS_DEBUG);
                    return true;
                } else {

                }
            }
            lastch = inch;
            last_depth = depth;
        }
        left->t = _djs_na;
        right->t = _djs_na;
        return false;
    }
    return false;
};


int djs_fA2(const djs_tok_t st, bool find, const int idx , djs_tok_t *left) {
    int vcount = 0;
    djs_tok_t stc = st;
    *left = st;
    left->t = _djs_na;
    if (st.t == djs_array) {
        while ((*stc.bs != '[') && (stc.bs <= st.es)) stc.bs++;
        stc.bs++;
        djs_tok_t right;
        while (djs_getBlob(stc,left,&right)) {
            if (left->t != _djs_na) {
                DBG("incrementing vcount\n");
                vcount++;
                djs_showTok(*left,DJS_DEBUG);
            }
            if (find && ((vcount-1) == idx)) {
                DBG("we found:\n");
                djs_showTok(*left,DJS_DEBUG);
                return 1;
            }
            stc.bs = right.bs;
        }
    }
    return find ? 0 : vcount;
}






bool djs_findNamed(const djs_tok_t st, const char *ss, djs_tok_t *vt) {
    DBG("findNamed\n");
    int depth = 0;
    bool in_string = 0;
    djs_tok_t str = st;
    djs_state_t cs = djs_st_start;
    djs_state_t ns = cs;
    djs_tok_t   kt = st;
    djs_tok_t   left, right;
    *vt = st;
    if (str.t == djs_hash) {
        char lastch = 0;
        for (const char *is=str.bs; is<str.es; is++) {
            char inch = *is;
            if (lastch == '\\') continue;
            if (!in_string) {
                if      (inch == '{') depth++;
                else if (inch == '}') depth--;
                else if (inch == '[') depth++;
                else if (inch == ']') depth--;
            }
            switch (cs) {
            case djs_st_start :
            case djs_st_value_done :
                if (!in_string && isgraph(inch) && (inch != '{')) {
                    if (inch == '"') in_string = true;
                    ns = djs_st_in_key;
                    const char *bs = in_string ? is+1 : is;
                    if (bs <= str.es) {
                        kt.bs     = bs;
                        kt.es     = bs;
                        vt->bs    = 0;
                        vt->es    = 0;
                    } else {
                        return false;
                    }
                }
                break;
            case djs_st_in_key :
                if (in_string && (inch == '"')) {
                    in_string = false;
                    ns = djs_st_key_done;
                    kt.es = is-1;
                    DBG("X key is: ");
                    showRange(kt.bs,kt.es);
                    DBG("\n");
                } else if (!in_string && (inch == ':')) {
                    ns = djs_st_in_value;
                    kt.es = is-1;
                    if (is+1 <= str.es) str.bs = is+1;
                    else return false;
                    DBG("Y key is: ");
                    showRange(kt.bs,kt.es);
                    DBG("\n");
                }
                break;
            case djs_st_key_done :
                if (inch == ':') {
                    ns = djs_st_in_value;
                    if (is+1 <= str.es) str.bs = is+1;
                    else return false; // malformed json
                };
                break;
            case djs_st_in_value :
                if (djs_getBlob(str,&left,&right)) {
                    DBG("got blob");
                    DBG("LEFT");
                    djs_showTok(left,DJS_DEBUG);
                    DBG("RIGHT");
                    djs_showTok(right,DJS_DEBUG);

                    ns = djs_st_value_done;
                    str = right;
                    is  = right.bs -1; // -1 because loop incrs
                    if (djs_strEql(kt,ss)) {
                        *vt = left;
                        vt->t = _djs_valid_untyped;
                        vt->t = djs_detType(*vt);
                        djs_trim(vt);
                        return true;
                    }
                };
                break;
            default :
                break;
            }
            if (0) DBG("depth %d is %llx cs %d ns %d ch %c\n",depth,is,cs,ns,inch);
            cs = ns;
            lastch = inch;
        }
    }
    return false;
};


bool djs_getStr(const djs_tok_t t, char *s, int sl) {
    int len = t.es-t.bs+1;
    if (len > (sl-1)) return false;
    int i = 0;
    while (i<len) {
        s[i] = t.bs[i];
        i++;
    }
    s[i] = 0;
    return true;
};

bool djs_getBool(const djs_tok_t t, bool *v) {
// note this functon will try to convert a naked value
//  or a string. Obviously, the string still needs to look
//  like true or false.
    if ((t.t == djs_naked) || (t.t == djs_string)) {
        char temp[20];
        djs_getStr(t,temp,20);
        if (strnlen(temp,20) < 3) return false;
        DBG("getBool temp is \"%s\"\n",temp);
        if ((tolower(temp[0] == 't')) &&
                (tolower(temp[1] == 'r')) &&
                (tolower(temp[2] == 'u'))
           ) {
            *v = true;
        } else {
            *v = false;
        }
        return true;
    }
    return false;
};

bool djs_getInt(const djs_tok_t t, int *v) {
// note this functon will try to convert a naked value
//  or a string. Obviously, the string still needs to look
//  like a number.
    if ((t.t == djs_naked) || (t.t == djs_string)) {
        char temp[20];
        char *eptr;
        djs_getStr(t,temp,20);
        DBG("temp is \"%s\"\n",temp);
        *v = strtol(temp,&eptr,10);
        if (temp == eptr) return false; // checks if some of input was consumed
#if DJS_STRICT
        if (*eptr == 0)   return false; // checks if all of input was consumed
#endif
        return true;
    }
    return false;
}

bool djs_valid(const djs_tok_t st) {
    return (st.t != _djs_na);
}
