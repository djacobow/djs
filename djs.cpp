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
bool       djs_trim(djs_tok_t &vt);
const char *djs_type2str(djs_tok_t t);
djs_tok_t  djs_createNull();
bool       djs_strEql(const djs_tok_t kt, const char *ks);
int        djs_fA2(const djs_tok_t st, bool find, const int idx,  djs_tok_t &vt);
bool       djs_getBlob(const djs_tok_t st, djs_tok_t &left, djs_tok_t &right);


djs_type_t 
djs_detType(djs_tok_t it) {
 if (!it.v) return _djs_na; 
 djs_type_t rt = djs_naked;
 int i = it.b;
 bool done = false;
 while (!done) {
  if      (it.s[i] == '[') { rt = djs_array;  done = true; }
  else if (it.s[i] == '{') { rt = djs_hash;   done = true; }
  else if (it.s[i] == '"') { rt = djs_string; done = true; }
  else if (isgraph(it.s[i])) done = true;
  i++;
  done |= (i == it.e);
 }
 it.t = rt;
 // DBG("type %s\n",djs_type2str(it));
 return rt;
};

djs_tok_t djs_createFrom(const char *src) {
 djs_tok_t ot = djs_createNull();
 ot.s = src;
 ot.e = strlen(src)-1;
 ot.v = true;
 ot.t = djs_detType(ot);
 return ot;
};

djs_tok_t djs_createNull() {
 djs_tok_t ot;
 memset(&ot,0,sizeof(ot));
 return ot;
}

void
showRange(const char *s,int b, int e) {
 for (int i=b;i<=e;i++) {
  DBG("%c",s[i]);
 }
};

bool djs_strEql(const djs_tok_t kt, const char *ks) {
 if (!kt.v)       return false;
 int ksl = strlen(ks);
 int ktl = kt.e - kt.b + 1;
 if (!strlen(ks)) return false; 
 if (ksl != ktl)  return false;
 for (int i=0;i<ksl;i++) {
  if (ks[i] != kt.s[kt.b+i]) return false;
 } 
 return true;
};

const char *djs_type2str(djs_tok_t t) {
 return ((t.t == djs_hash)   ? "hash" :
         (t.t == djs_string) ? "str " :
	 (t.t == djs_array)  ? "arry" :
	 (t.t == djs_naked)  ? "nakd" : "????");
}

void djs_showTok(const djs_tok_t t) {
 if (t.v) {
  DBG("tok [%d,%d] (%s) >>",
		  t.b,t.e,djs_type2str(t));
  for (int i=t.b;i<=t.e;i++) {
   DBG("%c",t.s[i]);
  }
  DBG("<<\n");
 } else {
  DBG("tok [invalid]\n");
 }
};

bool djs_trim(djs_tok_t &vt) {
 int fix_count = 0;
 if (vt.v) {
  int i = vt.b;	  
  bool done = false;
  int qcount = 0;
  while (!done) {
   bool trimmable = !isgraph(vt.s[i]);
   if (vt.t == djs_string) {
    if (vt.s[i] == '"') {
     qcount += 1;
     if (qcount == 1) trimmable = true;
    }
   }
   if (trimmable) vt.b++;
   done |= (i == vt.e);
   done |= !trimmable;
   i++;
  }
  int j = vt.e;	  
  done = false;
  qcount = 0;
  while (!done) {
   bool trimmable = !isgraph(vt.s[j]);
   // if (vt.t == djs_string) g &= (vt.s[j] != '"');
   if (vt.t == djs_string) {
    if (vt.s[j] == '"') {
     qcount += 1;
     // DBG("qcount %d\n",qcount);
     if (qcount == 1) { trimmable = true; }
    }
   }
   if (trimmable) vt.e--;
   done |= (j == vt.b);
   done |= !trimmable;
   j--;
  }
 }
 return (fix_count > 0);
}

bool djs_findIndex(const djs_tok_t st, const int idx, djs_tok_t &vt) {
 return djs_fA2(st, true, idx, vt);
}
int djs_getLength(const djs_tok_t st) {
 djs_tok_t dummy;
 return djs_fA2(st, false, 0, dummy);
}

bool
djs_getBlob(const djs_tok_t st, djs_tok_t &left, djs_tok_t &right) {
 left = st;
 right = st;

 if (st.v) {
  char lastch = 0;
  bool in_string = false;
  int depth = 0;
  int last_depth = 0;
  for (int i=st.b;i<=st.e;i++) {
   char inch = st.s[i];
   //  if (lastch == '\\') continue;
   if ((inch == '"') && (lastch != '\\'))  in_string = !in_string;
   if (!in_string) {
    if      (inch == '{') depth++;
    else if (inch == '}') depth--;
    else if (inch == '[') depth++;
    else if (inch == ']') depth--;
   }
   if ((!in_string && ((inch == ',') && (depth == 0))) ||
       ((depth == -1) && (last_depth == 0)))  {
    left.e = i-1;
    right.b = i+1;
    if (left.e >= left.b) {
     left.t = djs_detType(left);
     //DBG("blob before trim\n");
     // djs_showTok(left);
     djs_trim(left);
     DBG("blob after trim\n");
     djs_showTok(left);
     return true;
    } else {

    }
   }
   lastch = inch;
   last_depth = depth;
  }
  left.v = false;
  right.v = false;
  return false;
 }
 return false;
};


// this routine does double duty to find an element at 
// an index location, and also to return the number of 
// elements in an array. The find parameters determines
// what the return value is: true/false for found at index
// or the number of elems in the array
int djs_fA2(const djs_tok_t st, bool find, const int idx , djs_tok_t &left) {
 int vcount = 0;
 djs_tok_t stc = st;
 left = st;
 left.v = false;
 if (st.v && (st.t == djs_array)) {
  while ((stc.s[stc.b] != '[') && (stc.b <= st.e))  stc.b++;
  stc.b++;
  djs_tok_t right;
  while (djs_getBlob(stc,left,right)) {
   //if (left.b != left.e) {
   if (left.v) {
    DBG("incrementing vcount\n");
    vcount++;
    left.v = true;
    djs_showTok(left);
   }
   if (find && ((vcount-1) == idx)) {
    // left.t = djs_detType(left);
    DBG("we found:\n");
    djs_showTok(left);
    return 1;
   }
   stc.b = right.b;
  }
 }
 return find ? 0 : vcount;
}






bool djs_findNamed(const djs_tok_t st, const char *ss, djs_tok_t &vt) {
 int depth = 0;
 bool in_string = 0;
 djs_tok_t str = st;
 djs_state_t cs = djs_st_start;
 djs_state_t ns = cs;
 djs_tok_t   kt = st;
 djs_tok_t   left, right;
 vt = st;
 if (str.v && (str.t == djs_hash)) {
  char lastch = 0;	  
  for (int i=str.b;i<str.e;i++) {
   char inch = str.s[i];
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
      int b = in_string ? (i+1) : i;
      if (b <= str.e) {
       kt.b      = b;
       kt.e      = b;
       vt.b      = 0;
       vt.e      = 0;
      } else {
       return false;
      }
     }
     break;
    case djs_st_in_key :
     if (in_string && (inch == '"')) {
      in_string = false;
      ns = djs_st_key_done;
      kt.e = i-1;
      DBG("key is: ");
      showRange(kt.s,kt.b,kt.e);
      DBG("\n");
     } else if (!in_string && (inch == ':')) {
      ns = djs_st_in_value; 
      kt.e = i-1;
      if (i+1 <= str.e) str.b = i+1;
      else return false;
      DBG("key is: ");
      showRange(kt.s,kt.b,kt.e);
      DBG("\n");
     }
     break;
    case djs_st_key_done :
     if (inch == ':') {
      ns = djs_st_in_value;
      if (i+1 <= str.e) str.b = i+1;
      else return false; // malformed json
     };
     break;
    case djs_st_in_value :
     if (djs_getBlob(str,left,right)) {
      ns = djs_st_value_done;
      str = right;
      i   = right.b;
      if (djs_strEql(kt,ss)) {
       vt = left;
       vt.v = true;
       vt.t = djs_detType(vt);
       djs_trim(vt);
       return true;
      }
     };
     break;
    default :
     break;
   }
   if (0) DBG("depth %d i %d cs %d ns %d ch %c\n",depth,i,cs,ns,inch);
   cs = ns;
   lastch = inch;
  }
 }
 return false;
};


bool djs_getStr(const djs_tok_t t, char *s, int sl) {
 int len = t.e-t.b+1;
 if (len > (sl-1)) return false;
 int i = 0;
 while (i<len) {
  s[i] = t.s[t.b+i];
  i++;
 }
 s[i] = 0;
 return true;
};

bool djs_getInt(const djs_tok_t t, int &v) {
 // note this functon will try to convert a naked value
 //  or a string. Obviously, the string still needs to look
 //  like a number.
 if ((t.t == djs_naked) || (t.t == djs_string)) {
   char temp[20];
   char *eptr;
   djs_getStr(t,temp,20);
   DBG("temp is \"%s\"\n",temp);
   v = strtol(temp,&eptr,10);
   if (temp == eptr) return false; // checks if some of input was consumed
#if DJS_STRICT
   if (*eptr == 0)   return false; // checks if all of input was consumed
#endif
   return true;
 }
 return false;
}

