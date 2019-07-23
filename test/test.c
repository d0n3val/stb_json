#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define KB 1024
#define MB 1048576

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
                count = stbj_count_elements(&eye_context);
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

    char buf[3*MB];
    {

        // Huge JSON test ---------------------
        char str[100];

        printf("READING CANADA.JSON ---------------------\n");
        FILE* fp = fopen("canada.json", "rb");
        if(fp)
        {
            int len = fread(buf, 1, 3*MB, fp);
            printf("canada.json opened correctly, size %i\n", len);
            fclose(fp);

            stbj_context context = stbj_create_context(buf, len);
            int count = stbj_count_elements(&context);
            printf("num element from root: %i\n", count);

            if(stbj_is_error(&context)) 
                printf("ERROR: %s\n", stbj_get_last_error(&context));

            stbj_read_string(&context, "type", str, 100, "error!");

            if(stbj_is_error(&context)) 
                printf("ERROR: %s\n", stbj_get_last_error(&context));

            printf("Type : %s\n", str);

            if(stbj_is_error(&context)) 
                printf("ERROR: %s\n", stbj_get_last_error(&context));

            context = stbj_read_context(&context, "features");
            context = stbj_readp_context(&context, 0);

            if(stbj_is_error(&context)) 
                printf("ERROR: %s\n", stbj_get_last_error(&context));

            stbj_read_string(&context, "type", str, 100, "error!");

            if(stbj_is_error(&context)) 
                printf("ERROR: %s\n", stbj_get_last_error(&context));

            printf("-type : %s\n", str);

            context = stbj_read_context(&context, "geometry");
            context = stbj_read_context(&context, "coordinates");

            if(stbj_is_error(&context)) 
                printf("ERROR: %s\n", stbj_get_last_error(&context));

            count = stbj_count_elements(&context);
            
            //for(int a=0; a < count; ++a)
            //{
                //stbj_context context2 = stbj_readp_context(&context, a);
                //int count2 = stbj_count_elements(&context2);
//
                //printf("Shape %i contains %i coords\n", a, count2);
//
                //for(int b=0; b < count2; ++b)
                //{
                    //stbj_context context3 = stbj_readp_context(&context2, b);
                    //printf(">>> Coord %i: %f %f\n", b, 
                            //stbj_readp_double(&context3, 0, 0.0),
                            //stbj_readp_double(&context3, 1, 0.0));
                //}
            //}
        }
        else
            printf("Could not open canada.json\n");
    }

    {
        // Huge JSON test 2 ---------------------
        char str[100];

        printf("READING CITM_CATALOG.JSON ---------------------\n");
        FILE* fp = fopen("citm_catalog.json", "rb");
        if(fp)
        {
            int len = fread(buf, 1, 2*MB, fp);
            printf("citm_catalog.json opened correctly, size %i\n", len);
            fclose(fp);

            stbj_context context = stbj_create_context(buf, len);
            int count = stbj_count_elements(&context);
            printf("Num element from root: %i\n", count);

            if(stbj_is_error(&context)) 
                printf("ERROR: %s\n", stbj_get_last_error(&context));

            context = stbj_read_context(&context, "events");
            count = stbj_count_elements(&context);
            printf("Num element on events: %i\n", count);

            if(stbj_is_error(&context)) 
                printf("ERROR: %s\n", stbj_get_last_error(&context));

            for(int a=0; a < count; ++a)
            {
                stbj_context context2 = stbj_readp_context(&context, a);

                stbj_read_string(&context2, "name", str, 100, "error!");
                printf("Event %i -----------\nName: %s\n", a, str);

                stbj_read_string(&context2, "description", str, 100, "-empty-");
                printf("Description: %s\n", str);

                stbj_read_string(&context2, "logo", str, 100, "-empty-");
                printf("Logo: %s\n", str);

                printf("Id: %i\n", stbj_read_int(&context2, "id", 0));

                if(stbj_is_error(&context)) 
                    printf("ERROR: %s\n", stbj_get_last_error(&context));

            }
        }
        else
            printf("Could not open citm_catalog.json\n");
    }
}

