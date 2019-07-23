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

char buffer0[] = "{\"name\": \"John\", \"married\": true, \"height\": 181.55, \"eye colors\": [3,3] }";

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
		stbj_cursor cursor = stbj_load_buffer(buffer0, strlen(buffer0) + 1);

		if (stbj_any_error(&cursor))
		{
			printf("Error loading JSON buffer %s", stbj_get_last_error(&cursor));
		}
		else
		{
			printf("Found %i values at current cursor\n", stbj_count_values(&cursor));

			char buf[10];
			stbj_read_string_name(&cursor, "name", buf, 10, "not found");
			printf("Name: %s\n", buf);

			if (stbj_read_int_name(&cursor, "married", 0))
				printf("... is married\n");
			else
				printf("... is not married\n");

			printf("Height is: %0.3f\n", stbj_read_double_name(&cursor, "height", 0.0));
			printf("Weight is: %0.3f\n", stbj_read_double_name(&cursor, "weight", 0.0)); // does not exist

			stbj_cursor cursor_eyes = stbj_move_cursor_name(&cursor, "eye colors");
			int left_eye = stbj_read_int_index(&cursor_eyes, 0, -1);
			int right_eye = stbj_read_int_index(&cursor_eyes, 1, -1);

			printf("Eyes: %i - %i\n", left_eye, right_eye);
		}

	}

    {
        // Object reading test ---------------
        printf("Loading buffer: %s\n", buffer1);
        stbj_cursor context = stbj_load_buffer(buffer1, strlen(buffer1)+1);

        stbj_cursor c = stbj_move_cursor_name(&context, "haha");
        printf("error: %s\n", stbj_get_last_error(&c));

        int count = stbj_count_values(&context);

        printf("buffer1 contains %i elements\n", count);

        char buf[10];

        for(int i = 0; i < count; ++i)
        {
            printf("Element %i -------------------\n", i);
            printf("Element: %s\n", stbj_find_index(&context,i)); 
            printf("int(): %i\n", stbj_read_int_index(&context,i, 0)); 
            printf("float(): %f\n", stbj_read_double_index(&context,i, 0)); 

            int len = stbj_read_string_index(&context, i, buf, 10, NULL);
            printf("string() (len %i): \"%s\"\n", len, buf); 
        }

        stbj_read_string_name(&context, "name", buf, 10, "error");
        printf("tag \"name\" found at %i is \"%s\"\n", 
                stbj_find_name(&context, "name"), 
                buf);

        stbj_read_string_name(&context, "last name", buf, 10, "error");
        printf("tag \"last name\" found at %i is \"%s\"\n", 
                stbj_find_name(&context, "last name"),
                buf);

        printf("tag \"age\" found at %i is %i\n", 
                stbj_find_name(&context, "age"),
                stbj_read_int_name(&context, "age", 0));

        printf("tag \"height\" found at %i is %f\n", 
                stbj_find_name(&context, "height"),
                stbj_read_double_name(&context, "height", 0.0));

        printf("tag \"error\" found at %i\n", 
                stbj_find_name(&context, "error"));

        printf("tag \"eye colors\" found at %i ...\n",
                stbj_find_name(&context, "eye colors"));

        {
            stbj_cursor eye_context = stbj_move_cursor_name(&context, "eye colors");
            if(eye_context.type == STBJ_ARRAY)
            {
                count = stbj_count_values(&eye_context);
                printf("tag \"eye colors\" is an array of %i elements\n", count);

                for(int i = 0; i < count; ++i)
                {
                    printf("Element %i -------------------\n", i);
                    printf("Element: %s\n", stbj_find_index(&eye_context,i)); 
                    printf("int(): %i\n", stbj_read_int_index(&eye_context,i, 0)); 
                    printf("float(): %f\n", stbj_read_double_index(&eye_context,i, 0)); 

                    int len = stbj_read_string_index(&eye_context, i, buf, 10, NULL);
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
        stbj_cursor context = stbj_load_buffer(buffer2, strlen(buffer2)+1);

        int count = stbj_count_values(&context);
        printf("buffer2 contains %i elements\n", count);

        for(int i = 0; i < count; ++i)
            printf("Element %i: %s\n", i, stbj_find_index(&context,i)); 

        for(int i = 0; i < count; ++i)
            printf("Element %i: %i Error: %s\n", 
                    i, stbj_read_int_index(&context,i, -1), stbj_get_last_error(&context)); 
    }

    {
        // Array reading test -----------------
        printf("Loading buffer: %s\n", buffer3);
        stbj_cursor context = stbj_load_buffer(buffer3, strlen(buffer3)+1);

        int count = stbj_count_values(&context);
        printf("buffer3 contains %i elements\n", count);

        for(int i = 0; i < count; ++i)
            printf("Element %i: %s\n", i, stbj_find_index(&context,i)); 

        for(int i = 0; i < count; ++i)
            printf("Element %i: INT %i (%s)\n", 
                    i, stbj_read_int_index(&context,i, -1), 
                    stbj_get_last_error(&context)); 

        for(int i = 0; i < count; ++i)
            printf("Element %i: DOUBLE %f (%s)\n", 
                    i, stbj_read_double_index(&context, i, -1.0),
                    stbj_get_last_error(&context)); 
    }

    {
        // Array reading test -----------------
        printf("Loading buffer: %s\n", buffer4);
        stbj_cursor context = stbj_load_buffer(buffer4, strlen(buffer4)+1);

        int count = stbj_count_values(&context);
        printf("buffer4 contains %i elements\n", count);

        char buf[25];
        for(int i = 0; i < count; ++i)
        {
            printf("Element INT %i: %i Error: %s\n", 
                    i, stbj_read_int_index(&context,i, -1), stbj_get_last_error(&context)); 

            int len = stbj_read_string_index(&context, i, buf, 25, NULL);
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

            stbj_cursor context = stbj_load_buffer(buf, len);
            int count = stbj_count_values(&context);
            printf("num element from root: %i\n", count);

            if(stbj_any_error(&context)) 
                printf("ERROR: %s\n", stbj_get_last_error(&context));

            stbj_read_string_name(&context, "type", str, 100, "error!");

            if(stbj_any_error(&context)) 
                printf("ERROR: %s\n", stbj_get_last_error(&context));

            printf("Type : %s\n", str);

            if(stbj_any_error(&context)) 
                printf("ERROR: %s\n", stbj_get_last_error(&context));

            context = stbj_move_cursor_name(&context, "features");
            context = stbj_move_cursor_index(&context, 0);

            if(stbj_any_error(&context)) 
                printf("ERROR: %s\n", stbj_get_last_error(&context));

            stbj_read_string_name(&context, "type", str, 100, "error!");

            if(stbj_any_error(&context)) 
                printf("ERROR: %s\n", stbj_get_last_error(&context));

            printf("-type : %s\n", str);

            context = stbj_move_cursor_name(&context, "geometry");
            context = stbj_move_cursor_name(&context, "coordinates");

            if(stbj_any_error(&context)) 
                printf("ERROR: %s\n", stbj_get_last_error(&context));

            count = stbj_count_values(&context);
            
            //for(int a=0; a < count; ++a)
            //{
                //stbj_context context2 = stbj_move_cursor_index(&context, a);
                //int count2 = stbj_count_elements(&context2);
//
                //printf("Shape %i contains %i coords\n", a, count2);
//
                //for(int b=0; b < count2; ++b)
                //{
                    //stbj_context context3 = stbj_move_cursor_index(&context2, b);
                    //printf(">>> Coord %i: %f %f\n", b, 
                            //stbj_read_double_index(&context3, 0, 0.0),
                            //stbj_read_double_index(&context3, 1, 0.0));
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

            stbj_cursor context = stbj_load_buffer(buf, len);
            int count = stbj_count_values(&context);
            printf("Num element from root: %i\n", count);

            if(stbj_any_error(&context)) 
                printf("ERROR: %s\n", stbj_get_last_error(&context));

            context = stbj_move_cursor_name(&context, "events");
            count = stbj_count_values(&context);
            printf("Num element on events: %i\n", count);

            if(stbj_any_error(&context)) 
                printf("ERROR: %s\n", stbj_get_last_error(&context));

            for(int a=0; a < count; ++a)
            {
                stbj_cursor context2 = stbj_move_cursor_index(&context, a);

                stbj_read_string_name(&context2, "name", str, 100, "error!");
                printf("Event %i -----------\nName: %s\n", a, str);

                stbj_read_string_name(&context2, "description", str, 100, "-empty-");
                printf("Description: %s\n", str);

                stbj_read_string_name(&context2, "logo", str, 100, "-empty-");
                printf("Logo: %s\n", str);

                printf("Id: %i\n", stbj_read_int_name(&context2, "id", 0));

                if(stbj_any_error(&context)) 
                    printf("ERROR: %s\n", stbj_get_last_error(&context));

            }
        }
        else
            printf("Could not open citm_catalog.json\n");
    }
}

