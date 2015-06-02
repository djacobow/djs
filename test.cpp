#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "djs.h"

#define TEST_DEBUG (0)

void loadFile(char *dst, const char *fn) {
 FILE *f = fopen(fn,"r");
 int idx = 0;
 while (!feof(f) && !ferror(f)) {
  char c = getc(f);
  // printf("idx %d c %c %2.2x\n",idx,c,c);
  dst[idx++] = c;
 }
 dst[idx-1] = 0;
 fclose(f);
}

int eI(int a, int b) {
 if (a == b) {
  return 0;
 } else {
  printf("FAIL %d != %d\n",a,b);
 }
 return 1;
}

int eS(const char *a, const char *b) {
 if (!strcmp(a,b)) {
  return 0;
 } else {
  printf("FAIL \"%s\" != \"%s\"\n",a,b);
 }
 return 1;
} 

int eT(int a) {
 if (a) {
  return 0;
 } else {
  printf("FAIL expected true\n");
 }
 return 1;
}

int eF(int a) {
 if (!a) {
  return 0;
 } else {
  printf("FAIL expected false\n");
 }
 return 1;
}

int tests_7() {
 const char ts[] = "{\"lookup\":\"ok\",\"ip\":\"8.8.8.8\"}";
 int fc = 0;
 djs_tok_t w = djs_createFrom(ts);
 djs_tok_t t1, t2;
 fc += eT(w.v);
 fc += eT(djs_findNamed(w,"lookup",t1));
 fc += eT(djs_findNamed(w,"ip",t2)); 
 return fc;
}

int tests_6() {
 const char td1[] = "{aa:{b:{cc:{d:{ee:f}}}}}"; // not json, but we'll take it;
 const char td2[] = "{\"aa\":{\"b\":{\"cc\":{\"d\":{\"ee\":\"f\"}}}}}";

 int fc = 0;
 djs_tok_t w1 = djs_createFrom(td1);
 fc += eT(w1.v);
 djs_tok_t w2 = djs_createFrom(td2);
 fc += eT(w2.v);

 const char *names[] = { "aa", "b", "cc", "d", "ee" };
 djs_tok_t r1 = w1;
 djs_tok_t r2 = w2;
 for (int i=0;i<4;i++) {
  bool ok;
  djs_tok_t nr2;
  fc += eT(ok = djs_findNamed(r2,names[i],nr2));
  r2 = nr2;

  djs_tok_t nr1;
  fc += eT(ok = djs_findNamed(r1,names[i],nr1));
  r1 = nr1;
 }
 return fc;
}

int tests_5() {

 const char td[] = "[[[[[]]]]]";
 djs_tok_t w = djs_createFrom(td);
 int fc = 0;

 fc += eT(w.v);
 djs_tok_t a,b,c,d,e;
 bool ok; 
 fc += eT(ok = djs_findIndex(w,0,a));
 fc += eT(ok = djs_findIndex(a,0,b));
 fc += eT(ok = djs_findIndex(b,0,c));
 fc += eT(ok = djs_findIndex(c,0,d));
 fc += eF(ok = djs_findIndex(d,0,e));
 return fc;
};

int tests_4() {

 const int TSTRL = 128;
 char tdata[2048];
 char tstr[TSTRL];
 int fc = 0;
 bool ok = false;
 djs_tok_t res;

 loadFile(tdata,"inputs/ex3.json");

 djs_tok_t w2 = djs_createFrom(tdata);
 djs_showTok(w2,TEST_DEBUG);

 const char *svals[] = {
  "this",
  "is",
  "a",
  "thing",
  "with,commas,n'stuff,",
  "and single quotes '",
  "and escped double quotes -- singly \\\"",
  "and in pairs \\\"ding\\\""
 };

 fc += eT(w2.v);
 for (int i=0;i<8;i++) {
  fc += eT(ok = djs_findIndex(w2,i,res));
  fc += eT(ok = djs_getStr(res,tstr,TSTRL));
  fc += eS(tstr,svals[i]);
 }
 return fc;
}


int tests_3() {

 const int TSTRL = 128;
 char tdata[2048];
 char tstr[TSTRL];
 int fc = 0;
 bool ok = false;
 djs_tok_t res;

 loadFile(tdata,"inputs/ex2.json");

 djs_tok_t w2 = djs_createFrom(tdata);
 djs_showTok(w2,TEST_DEBUG);

 const char *digits[] = {
  "one", "two", "three", "four", "five", "six", "seven", "eight", "nine", "ten"
 };
 const int ivals[]   = { 1, 2, 3, 4, 5, 6, 7, 8, 9, -10 };
 const char *svals[] = { "1", "2", "3", "4", "5", "6", "7", "8", "9", "[ 10 ]" };


 fc += eT(w2.v);
 for (int i=0;i<10;i++) {
  fc += eT(ok = djs_findNamed(w2,digits[i],res));
  fc += eT(ok = djs_getStr(res,tstr,TSTRL));
  fc += eS(tstr,svals[i]);
  int ival = 42;
  fc += eI(djs_getInt(res,ival),(ivals[i] > 0));
  fc += (ivals[i] > 0) ? eI(ivals[i], ival) : 42;
 }
 return fc;
}


int tests_2() {
 const char td1[] = "{}";
 const char td2[] = "[]";
 int fc = 0;

 djs_tok_t w1 = djs_createFrom(td1);
 djs_tok_t w2 = djs_createFrom(td2);
 djs_tok_t res;
 bool ok = 0;

 fc += eT(w1.v);
 fc += eT(w2.v);
 fc += eI(djs_getLength(w2),0);
 fc += eF(ok = djs_findNamed(w1,"bob",res));
 fc += eF(ok = djs_findNamed(w1,"",res));
 fc += eF(ok = djs_findIndex(w2,0,res));
 fc += eF(ok = djs_findIndex(w2,1,res));

 return fc;
};

int tests_1() {
 const int TSTRL = 128;
 char tdata[2048];
 char tstr[TSTRL];
 int fc = 0;
 bool ok = false;
 djs_tok_t res, r3;
 int aryLen = 0;

 loadFile(tdata,"inputs/example.json");

 djs_tok_t whole = djs_createFrom(tdata);

 fc += eT(ok = djs_findNamed(whole, "this", res));
 fc += eT(ok = djs_getStr(res,tstr,TSTRL));
 fc += eS(tstr,"that");
 fc += eT(ok = djs_findNamed(whole,"other",res));
 fc += eT(ok = djs_findNamed(res,"this",res));
 fc += eT(ok = djs_getStr(res,tstr,TSTRL));
 fc += eS(tstr,"notthat");
 fc += eT(ok = djs_findNamed(whole,"boo",res));
 fc += eI(aryLen = djs_getLength(res),6);

 bool intables[] = { true, true, false, true, false, true };
 int  tvals[]    = { 11, 222, -1, 3333, -1, 55 };
 for (int i=0;i<aryLen;i++) {
  djs_tok_t r2;
  fc += eT(ok = djs_findIndex(res,i,r2));
  int ival;
  fc += eI(djs_getInt(r2,ival),intables[i]);
  if (intables[i]) {
   fc += eI(ival, tvals[i]);
  }
 }

 fc += eT(ok = djs_findNamed(whole,"poop",res));
 fc += eT(ok = djs_findIndex(res,2,r3));
 fc += eT(ok = djs_getStr(r3,tstr,TSTRL));
 fc += eS(tstr, 
   "{ \"in\": 2, \"st\": 333333, \"rs\": [ 3,3,3,3,3,3 ] }");

 if (fc) {
  printf("---- FAIL with %d errors ----\n",fc);
 }

 return fc;
};

int main(int argc, char *argv[]) {
 tests_1();
 tests_2();
 tests_3();
 tests_4();
 tests_6();
 tests_7();
 return 0;

};

