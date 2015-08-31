
# djs

## A really minimalist JSON parser

Sure, there are plenty of JSON parsers out there, including some 
that claim to be fast, some that claim to be complete and super-
heavily compliant and tested, and some that claim to be small and 
minimal.

But there are none that are as minimal as this one. It is designed
to parse JSON messages on embedded platforms where memory is 
very scarce. So scarce, in fact, that this parser works entirely
with the string buffer containing the message. This memory efficiency
does come with a price: performance can be slow, particularly when
working with long JSON strings that do not use much hierarchy (like 
a very long array or object with many members)

### Pros

* very small code size, written in C
* no memory usage except for any tokens your program may hold at any point in time, containing pointers to substrings in the original JSON string
* no use of floating point, so no fp lib stuff gets pulled into your otherwise all-fixed point program
* rather forgiving and laissez fair pasting approach. Any scalar can be read as a string, even if it is an int. Anything that looks like an int -- including a string -- can be read as an int.

### Cons

* not compliant with the full JSON spec, particularly unicode and escaping
* does not handle floats (fp numbers will be treated as the integers that precede their decimal points
* not the fastest, because the JSON string (or more likely, substrings, will be scanned repeatedly looking for requested data

### Example

The following simple program shows how to use the library.
Essentially, you initialize a token with a JSON string. From there,
you can create new tokens by finding them by name for object elements,
or by index, for array elements. The scalar value of a token can be 
accessed with the get routines.

For brevity, this example does no error checking, but most routines
provide return values for checking success.

```C
int main(int argc, char *argv[]) {

    const char *json = "{\"foo\":\"fooval\", \"bar\": 42, \"baz\": [ false, 1,
 \"two\" ] }";

   char tempbuf[80];
   int  tempint;
   bool tempbool;
   djs_tok_t temptok;

   djs_tok_t top = djs_createFrom(json);;

   // get the object named "foo"
   djs_findNamed(top,"foo",&temptok);

   // get the string contained in foo
   djs_getStr(temptok, tempbuf, 80);
   printf("foo value was: %s\n",tempbuf);

   // get the object named "bar"
   djs_findNamed(top,"bar",&temptok);
   djs_getInt(temptok, &tempint);
   printf("bar value was: %d\n",tempint);

   // get the object named "baz"
   djs_findNamed(top,"baz",&temptok);

   // baz is an array. Access its members
   djs_tok_t temp2;
   printf("baz has %d elems\n",djs_getLength(temptok));
   djs_findIndex(temptok,0,&temp2);
   djs_getBool(temp2,&tempbool);
   djs_findIndex(temptok,1,&temp2);
   djs_getInt(temp2,&tempint);
   djs_findIndex(temptok,2,&temp2);
   djs_getStr(temp2, tempbuf, 80);
   printf("baz array content: %d, %d, %s\n",tempbool,tempint,tempbuf);
   return 0;

};

```

Output:

```
foo value was: fooval
bar value was: 42
baz has 3 elems
baz array content: 0, 1, two
```


### Testing

This module has only been informally tested, so it is best in 
applications where you control the JSON to be processed. A short
and sweet test program is included in the src/ folder, along with 
some input JSON files in inputs/



