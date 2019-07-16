#include <stdio.h>
#include <string.h>

#define STB_JSON_IMPLEMENTATION
#include "../src/stb_json.h"

char buffer1[] = "{\"name\": \"  John the Great\", \"last name\" : a e o  u   , \"age\":1234567890123  , \"height\": \"  -345.1234567\", \"eye colors\": [\"green\",\"blue\"] }";
char buffer2[] = "[0,1,true,false,   null  ,    true , tnull, 3, 3true, false4, null 45]";
char buffer3[] = "[      -5,-665.65.367     ,  +7   ,-8, 1.1, 23.45, 444444.0000, 0.12345, .3141621   ]";
//char buffer4[] = "[[1,2,3],["a","b",[]], {"a":34},{}, [], 4, {}]";
char buffer4[] = "[\"34\", [1,2,[{},{}]], 4, {},true,4,\"false\",\"null\",  null]";

int main()
{
    printf("Test environment for STB_JSON lib:\n");

    {
        // Object reading test ---------------
        printf("Loading buffer: %s\n", buffer1);
        stbj_context context = stbj_create_context(buffer1, strlen(buffer1)+1);

        stbj_context c = stbj_read_context(&context, "haha");
        printf("error: %s\n", stbj_get_last_error(&c));

        int count = stbj_count_elements(&context);

        printf("buffer1 contains %i elements\n", count);

        char buf[10];

        for(int i = 0; i < count; ++i)
        {
            printf("Element %i -------------------\n", i);
            printf("Element: %s\n", stbj_get_element(&context,i)); 
            printf("int(): %i\n", stbj_readp_int(&context,i, 0)); 
            printf("float(): %f\n", stbj_readp_double(&context,i, 0)); 

            int len = stbj_readp_string(&context, i, buf, 10, NULL);
            printf("string() (len %i): \"%s\"\n", len, buf); 
        }

        stbj_read_string(&context, "name", buf, 10, "error");
        printf("tag \"name\" found at %i is \"%s\"\n", 
                stbj_find_element(&context, "name"), 
                buf);

        stbj_read_string(&context, "last name", buf, 10, "error");
        printf("tag \"last name\" found at %i is \"%s\"\n", 
                stbj_find_element(&context, "last name"),
                buf);

        printf("tag \"age\" found at %i is %i\n", 
                stbj_find_element(&context, "age"),
                stbj_read_int(&context, "age", 0));

        printf("tag \"height\" found at %i is %f\n", 
                stbj_find_element(&context, "height"),
                stbj_read_double(&context, "height", 0.0));

        printf("tag \"error\" found at %i\n", 
                stbj_find_element(&context, "error"));

        printf("tag \"eye colors\" found at %i ...\n",
                stbj_find_element(&context, "eye colors"));

        {
            stbj_context eye_context = stbj_read_context(&context, "eye colors");
            if(eye_context.context == STBJ_ARRAY)
            {
                int count = stbj_count_elements(&eye_context);
                printf("tag \"eye colors\" is an array of %i elements\n", count);

                for(int i = 0; i < count; ++i)
                {
                    printf("Element %i -------------------\n", i);
                    printf("Element: %s\n", stbj_get_element(&eye_context,i)); 
                    printf("int(): %i\n", stbj_readp_int(&eye_context,i, 0)); 
                    printf("float(): %f\n", stbj_readp_double(&eye_context,i, 0)); 

                    int len = stbj_readp_string(&eye_context, i, buf, 10, NULL);
                    printf("string() (len %i): \"%s\"\n", len, buf); 
                }
            }
            else
                printf("tag \"eye colors\" is not an array??\n");
        }

    /*
        char result[25];
        int len = stbj_read(&context, "name", &result[0], 25);
        if(len>0)
        {
            printf("contents of 'name': %s", result);
        }
    */
    }

    {
        // Array reading test -----------------
        printf("Loading buffer: %s\n", buffer2);
        stbj_context context = stbj_create_context(buffer2, strlen(buffer2)+1);

        int count = stbj_count_elements(&context);
        printf("buffer2 contains %i elements\n", count);

        for(int i = 0; i < count; ++i)
            printf("Element %i: %s\n", i, stbj_get_element(&context,i)); 

        for(int i = 0; i < count; ++i)
            printf("Element %i: %i Error: %s\n", 
                    i, stbj_readp_int(&context,i, -1), stbj_get_last_error(&context)); 
    }

    {
        // Array reading test -----------------
        printf("Loading buffer: %s\n", buffer3);
        stbj_context context = stbj_create_context(buffer3, strlen(buffer3)+1);

        int count = stbj_count_elements(&context);
        printf("buffer3 contains %i elements\n", count);

        for(int i = 0; i < count; ++i)
            printf("Element %i: %s\n", i, stbj_get_element(&context,i)); 

        for(int i = 0; i < count; ++i)
            printf("Element %i: INT %i (%s)\n", 
                    i, stbj_readp_int(&context,i, -1), 
                    stbj_get_last_error(&context)); 

        for(int i = 0; i < count; ++i)
            printf("Element %i: DOUBLE %f (%s)\n", 
                    i, stbj_readp_double(&context, i, -1.0),
                    stbj_get_last_error(&context)); 
    }

    {
        // Array reading test -----------------
        printf("Loading buffer: %s\n", buffer4);
        stbj_context context = stbj_create_context(buffer4, strlen(buffer4)+1);

        int count = stbj_count_elements(&context);
        printf("buffer4 contains %i elements\n", count);

        char buf[25];
        for(int i = 0; i < count; ++i)
        {
            printf("Element INT %i: %i Error: %s\n", 
                    i, stbj_readp_int(&context,i, -1), stbj_get_last_error(&context)); 

            int len = stbj_readp_string(&context, i, buf, 25, NULL);
            printf("string() (len %i): \"%s\" Error: %s\n", 
                    len, (len) ? buf : "<null>", stbj_get_last_error(&context)); 
        }
    }
}

