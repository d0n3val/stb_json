#include <stdio.h>
#include <string.h>

#define STB_JSON_IMPLEMENTATION
#include "stb_json.h"

char buffer1[] = "{\"name\":\"John\", \"age\":31}";
char buffer2[] = "[0,1,2,3]";
char buffer3[] = "[       5,6     ,   7   ,8]";

int main()
{
    printf("Test environment for STB_JSON lib:\n");

    {
        // Object reading test ---------------
        stbj_context context = stbj_load_from_memory(buffer1, strlen(buffer1)+1);

        int count = stbj_count_elements(&context);

        printf("buffer1 contains %i elements\n", count);
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
        stbj_context context = stbj_load_from_memory(buffer2, strlen(buffer2)+1);

        int count = stbj_count_elements(&context);
        printf("buffer2 contains %i elements\n", count);

        for(int i = 0; i < count; ++i)
            printf("Element %i: %i\n", i, stbj_readp_int(&context,i, 0)); 
    }

    {
        // Array reading test -----------------
        stbj_context context = stbj_load_from_memory(buffer3, strlen(buffer2)+1);

        int count = stbj_count_elements(&context);
        printf("buffer3 contains %i elements\n", count);

    }
}

