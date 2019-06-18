#include <stdio.h>
#include <string.h>

#define STB_JSON_IMPLEMENTATION
#include "stb_json.h"

char buffer1[] = "{\"name\": \"  John the Great\", \"last name\" : a e o  u   , \"age\":31  , \"height\": \"  -345.1234567\"}";
char buffer2[] = "[0,1,2,3]";
char buffer3[] = "[      -5,-66565367     ,  +7   ,-8]";
//char buffer4[] = "[[1,2,3],["a","b",[]], {"a":34},{}, [], 4, {}]";
char buffer4[] = "[\"34\", [[{},{}]], 4, {},3,4,5,6,7]";

int main()
{
    printf("Test environment for STB_JSON lib:\n");

    {
        // Object reading test ---------------
        printf("Loading buffer: %s\n", buffer1);
        stbj_context context = stbj_load_from_memory(buffer1, strlen(buffer1)+1);

        int count = stbj_count_elements(&context);

        printf("buffer1 contains %i elements\n", count);

        for(int i = 0; i < count; ++i)
        {
            printf("Element %i -------------------\n", i);
            printf("Element: %s\n", stbj_get_element(&context,i)); 
            printf("int(): %i\n", stbj_readp_int(&context,i, 0)); 
            printf("float(): %f\n", stbj_readp_float(&context,i, 0)); 

            char buf[10];
            int len = stbj_readp_string(&context, i, buf, 10, NULL);
            printf("string() (len %i): \"%s\"\n", len, buf); 
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
        stbj_context context = stbj_load_from_memory(buffer2, strlen(buffer2)+1);

        int count = stbj_count_elements(&context);
        printf("buffer2 contains %i elements\n", count);

        for(int i = 0; i < count; ++i)
            printf("Element %i: %s\n", i, stbj_get_element(&context,i)); 

        for(int i = 0; i < count; ++i)
            printf("Element %i: %i\n", i, stbj_readp_int(&context,i, 0)); 
    }

    {
        // Array reading test -----------------
        printf("Loading buffer: %s\n", buffer3);
        stbj_context context = stbj_load_from_memory(buffer3, strlen(buffer3)+1);

        int count = stbj_count_elements(&context);
        printf("buffer3 contains %i elements\n", count);

        for(int i = 0; i < count; ++i)
            printf("Element %i: %s\n", i, stbj_get_element(&context,i)); 

        for(int i = 0; i < count; ++i)
            printf("Element %i: %i\n", i, stbj_readp_int(&context,i, 0)); 
    }

    {
        // Array reading test -----------------
        printf("Loading buffer: %s\n", buffer4);
        stbj_context context = stbj_load_from_memory(buffer4, strlen(buffer4)+1);

        int count = stbj_count_elements(&context);
        printf("buffer4 contains %i elements\n", count);

        for(int i = 0; i < count; ++i)
            printf("Element %i: %i\n", i, stbj_readp_int(&context,i, 0)); 
    }
}

