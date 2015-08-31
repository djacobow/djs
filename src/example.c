
#include "djs.h"
#include <stdio.h>

int main(int argc, char *argv[]) {

    const char *json = "{\"foo\":\"fooval\", \"bar\": 42, \"baz\": [ false, 1, \"two\" ] }";

   char tempbuf[80];
   int  tempint;
   bool tempbool;
   djs_tok_t temptok;

   djs_tok_t top = djs_createFrom(json);;

   // get the object named "foo"
   djs_findNamed(&top,"foo",&temptok);

   // get the string contained in foo
   djs_getStr(&temptok, tempbuf, 80);
   printf("foo value was: %s\n",tempbuf);

   // get the object named "bar"
   djs_findNamed(&top,"bar",&temptok);
   djs_getInt(&temptok, &tempint);
   printf("bar value was: %d\n",tempint);
   
   // get the object named "baz"
   djs_findNamed(&top,"baz",&temptok);

   // baz is an array. Access its members
   djs_tok_t temp2;
   printf("baz has %d elems\n",djs_getLength(&temptok));
   djs_findIndex(&temptok,0,&temp2);
   djs_getBool(&temp2,&tempbool);
   djs_findIndex(&temptok,1,&temp2);
   djs_getInt(&temp2,&tempint);
   djs_findIndex(&temptok,2,&temp2);
   djs_getStr(&temp2, tempbuf, 80);
   printf("baz array content: %d, %d, %s\n",tempbool,tempint,tempbuf);
   return 0;

};

