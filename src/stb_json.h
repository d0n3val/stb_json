/* stb_json - v0.1 - public domain json parser - http://d0n3val.github.org
                                  no warranty implied; use at your own risk

   Do this:
      #define STB_JSON_IMPLEMENTATION
   before you include this file in *one* C or C++ file to create the implementation.

   // i.e. it should look like this:
   #include ...
   #include ...
   #include ...
   #define STB_JSON_IMPLEMENTATION
   #include "stb_json.h"

   You can #define STBI_ASSERT(x) before the #include to avoid using assert.h.
   And #define STBI_MALLOC, STBI_REALLOC, and STBI_FREE to avoid using malloc,realloc,free


   QUICK NOTES:
      Primarily of interest to deploy a non-intrusive json parser

   Full documentation under "DOCUMENTATION" below.


LICENSE

  See end of file for license information.

RECENT REVISION HISTORY:

      0.1  (2019-06-15) Basic parsing

   See end of file for full revision history.


 ============================    Contributors    =========================

 Parsing
    Ricard Pillosu
                                           
 Optimizations & bugfixes

 Bug & warning fixes

*/

#ifndef STBJ_INCLUDE_STB_JSON_H
#define STBJ_INCLUDE_STB_JSON_H

// DOCUMENTATION
//
// Limitations:
//    - no reading from files, only buffers
//    - no writing of json files
//
// Basic usage: 
//    stbj_cursor* cursor = stbj_load(buffer, buffer_size);
//    ...
//    stbj_free(cursor);
//
// ===========================================================================
//
// Philosophy
//
// stb libraries are designed with the following priorities:
//
//    1. easy to use
//    2. easy to maintain
//    3. good performance
//
// Sometimes I let "good performance" creep up in priority over "easy to maintain",
// and for best performance I may provide less-easy-to-use APIs that give higher
// performance, in addition to the easy-to-use ones. Nevertheless, it's important
// to keep in mind that from the standpoint of you, a client of this library,
// all you care about is #1 and #3, and stb libraries DO NOT emphasize #3 above all.
//
// Some secondary priorities arise directly from the first two, some of which
// provide more explicit reasons why performance can't be emphasized.
//
//    - Portable ("ease of use")
//    - Small source code footprint ("easy to maintain")
//    - No dependencies ("ease of use")
//
// ===========================================================================
//

#define STBJ_VERSION 1

#ifdef __cplusplus
extern "C" {
#endif

#ifndef STBJDEF
#ifdef STB_JSON_STATIC
#define STBJDEF static
#else
#define STBJDEF extern
#endif
#endif

//////////////////////////////////////////////////////////////////////////////
//
// PRIMARY API - open and release a buffer
//

////////////////////////////////////
//
// load json buffer
//

/*
STBJDEF void*       stbj_load_from_memory(const char *buffer, int len);
STBJDEF void        stbj_free(void *retval_from_stbi_load);
STBJDEF const char* stbj_get_last_error();
*/

#ifdef __cplusplus
}
#endif

//
//
////   end header file   /////////////////////////////////////////////////////
#endif // STBJ_INCLUDE_STB_JSON_H

////////////////////////////////////
//
// Start of implementation
//

#ifdef STB_JSON_IMPLEMENTATION

//#include <stdlib.h>
//#include <string.h>

#ifndef STBJ_NO_STDIO
//#include <stdio.h>
#endif

#ifndef STBJ_ASSERT
#include <assert.h>
#define STBJ_ASSERT(x) assert(x)
#endif

#ifdef __cplusplus
#define STBJ_EXTERN extern "C"
#else
#define STBJ_EXTERN extern
#endif


#ifndef _MSC_VER
   #ifdef __cplusplus
   #define stbj_inline inline
   #else
   #define stbj_inline
   #endif
#else
   #define stbj_inline __forceinline
#endif


#ifdef _MSC_VER
typedef unsigned short  stbj_uint16;
typedef signed short    stbj_int16;
typedef unsigned int    stbj_uint32;
typedef signed int      stbj_int32;
#else
#include <stdint.h>
typedef uint16_t stbj_uint16;
typedef int16_t  stbj_int16;
typedef uint32_t stbj_uint32;
typedef int32_t  stbj_int32;
#endif

// should produce compiler error if size is wrong
typedef unsigned char validate_uint32[sizeof(stbj_uint32)==4 ? 1 : -1];

#ifdef _MSC_VER
#define STBJ_NOTUSED(v)  (void)(v)
#else
#define STBJ_NOTUSED(v)  (void)sizeof(v)
#endif

#ifdef _MSC_VER
#define STBJ_HAS_LROTL
#endif

#ifdef STBj_HAS_LROTL
   #define stbj_lrot(x,y)  _lrotl(x,y)
#else
   #define stbj_lrot(x,y)  (((x) << (y)) | ((x) >> (32 - (y))))
#endif

#if defined(STBJ_MALLOC) && defined(STBJ_FREE) && (defined(STBJ_REALLOC) || defined(STBJ_REALLOC_SIZED))
// ok
#elif !defined(STBJ_MALLOC) && !defined(STBJ_FREE) && !defined(STBJ_REALLOC) && !defined(STBJ_REALLOC_SIZED)
// ok
#else
#error "Must define all or none of STBJ_MALLOC, STBJ_FREE, and STBJ_REALLOC (or STBJ_REALLOC_SIZED)."
#endif

#ifndef STBJ_MALLOC
#define STBJ_MALLOC(sz)           malloc(sz)
#define STBJ_REALLOC(p,newsz)     realloc(p,newsz)
#define STBJ_FREE(p)              free(p)
#endif

#ifndef STBJ_REALLOC_SIZED
#define STBJ_REALLOC_SIZED(p,oldsz,newsz) STBI_REALLOC(p,newsz)
#endif

// x86/x64 detection
#if defined(__x86_64__) || defined(_M_X64)
#define STBJ__X64_TARGET
#elif defined(__i386) || defined(_M_IX86)
#define STBJ__X86_TARGET
#endif

///////////////////////////////////////////////
//
//  stbj_context struct and related functions

// stbj_context structure is our basic context used by all json buffers
//
typedef struct
{
    enum { STBJ_OBJECT, STBJ_ARRAY, STBJ_ERROR } current_env;
	int len;
	const char* buffer;
    const char* cursor;
} stbj_context;

///////////////////////////////////////////////
//
//  internal helper functions

/*
stbj_inline static int stbj_atoi(const char* string, int default_value)
{
    STBJ_ASSERT(string);

    int ret = 0;
    int is_negative = 0;

    if((is_negative = *string == '-') || *string == '+')
        ++string;

    while(*string)
    {
        int digit = *string++ - '0';
        if(digit > 9)
            break;
        ret = (10*ret) + digit;
    }

    return (is_negative) ? -ret : ret;
}
*/

stbj_inline static const char* find_next(const stbj_context* context, char c, const char* cursor)
{
    if(!cursor)
        cursor = (char*) context->cursor;
    int max_len = context->len - (int)(cursor - context->buffer);
    while(*cursor++ != c) 
        if(--max_len <= 0) return NULL;
    return cursor;
}

stbj_inline static const char* find_next_s(const stbj_context* context, const char* chars, const char* cursor)
{
    if(!cursor)
        cursor = (char*) context->cursor;
    int max_len = context->len - (int)(cursor - context->buffer);
    const char* c = chars;

    while(max_len-- > 0) 
    {
        while(*c && *c != *cursor) ++c;
        if(*c) return cursor;
        c = chars;
        ++cursor;
    }

    return NULL;
}

stbj_inline static int string_compare(const char* cursor_left, const char* cursor_right, const char* tag)
{
    while(*cursor_left++ == *tag++) 
        if(cursor_left == cursor_right) return 1;
    return 0;
}

// stbj_context it is up to the application to keep buffer as valid and readable memory
//
STBJDEF stbj_context stbj_load_from_memory(const char *buffer, int len)
{
    STBJ_ASSERT(buffer);
    STBJ_ASSERT(len > 0);

    stbj_context context;
    context.len = len;
    context.buffer = buffer;
    context.cursor = buffer;
    context.current_env = STBJ_ERROR;

    // place the cursor in the first object '{' or '['
    context.cursor = find_next_s(&context, "{[", NULL);
    if(context.cursor != NULL)
    {
        if(*context.cursor == '{')
            context.current_env = STBJ_OBJECT;
        else
            context.current_env = STBJ_ARRAY;
    }

    return context;
}

STBJDEF const char* stbj_get_last_error()
{
    return "No error";
}

STBJDEF int stbj_count_elements(stbj_context* context)
{
    STBJ_ASSERT(context);
    STBJ_ASSERT(context->cursor);
    STBJ_ASSERT(context->current_env != STBJ_ERROR);

    int result = 0;
    int between_comas = 0;
    char stack[256];
    int stack_index = -1;
    char delimiter = (context->current_env == STBJ_ARRAY) ? ']' : '}';

    const char* cursor = context->cursor;
    int max_len = context->len - (int)(cursor - context->buffer);

    while(max_len-- > 0 && *++cursor && stack_index < 256) 
    {
        // Two different logics if we are in a nested array/object or not
        if(stack_index < 0)
        {
            // we are on the same array/object level
            switch(*cursor)
            {
                case '[': case '{': stack[++stack_index] = *cursor; break;
                case ']': case '}': if(!between_comas && *cursor == delimiter) return ++result; break;
                case ',': result += !between_comas; break;
                case '"': between_comas = !between_comas; break;
            }
        }
        else
        {
            // we are in a nested array/object, keep consuming chars and record progress in out stack
            switch(*cursor)
            {
                case '[': case '{': stack[++stack_index] = *cursor; break;
                case ']': if(stack[stack_index] == '[') --stack_index; else return -1; break;
                case '}': if(stack[stack_index] == '{') --stack_index; else return -1; break;
            }
        }
    }

    // abnormal error quit, "[] or {} mismatch"
    return -1;
}

// Return a pointer to the place in the buffer that a certain element exist
STBJDEF const char* stbj_get_element(stbj_context* context, int index)
{
    STBJ_ASSERT(context);
    STBJ_ASSERT(context->cursor);
    STBJ_ASSERT(context->current_env != STBJ_ERROR);
    STBJ_ASSERT(index >= 0);

    int result = 0;
    int between_comas = 0;
    char stack[256];
    int stack_index = -1;
    char delimiter = (context->current_env == STBJ_ARRAY) ? ']' : '}';

    const char* cursor = context->cursor;
    int max_len = context->len - (int)(cursor - context->buffer);

    while(max_len-- > 0 && *++cursor && stack_index < 256) 
    {
        if(result == index)
            return cursor;

        // Two different logics if we are in a nested array/object or not
        if(stack_index < 0)
        {
            // we are on the same array/object level
            switch(*cursor)
            {
                case '[': case '{': stack[++stack_index] = *cursor; break;
                case ']': case '}': if(!between_comas && *cursor == delimiter) return NULL; break;
                case ',': result += !between_comas; break;
                case '"': between_comas = !between_comas; break;
            }
        }
        else
        {
            // we are in a nested array/object, keep consuming chars and record progress in out stack
            switch(*cursor)
            {
                case '[': case '{': stack[++stack_index] = *cursor; break;
                case ']': if(stack[stack_index] == '[') --stack_index; else return NULL; break;
                case '}': if(stack[stack_index] == '{') --stack_index; else return NULL; break;
            }
        }
    }

    return NULL;
}

STBJDEF stbj_int32 stbj_readp_int(stbj_context* context, int index, stbj_int32 default_value)
{
    STBJ_ASSERT(context);
    STBJ_ASSERT(context->cursor);
    STBJ_ASSERT(context->current_env != STBJ_ERROR);
    STBJ_ASSERT(index >= 0);

    const char* cursor = stbj_get_element(context, index);
    int sign = 1;
    int result = 0;
    int have_result = 0;

    if(cursor != NULL)
    {
        int max_len = context->len - (int)(cursor - context->buffer);
        int between_comas = 0;
        int finish = 0;

        // if object just consume chars until ':'
        if(context->current_env == STBJ_OBJECT)
            while(max_len-- > 0 && *cursor && *cursor++ != ':');

        // iterate chars, embedding the logic for atoi()
        while(max_len-- > 0 && *cursor && !finish) 
        { 
            switch(*cursor)
            {
                case ' ': break;
                case '"': 
                    if(between_comas++) finish = 1;
                    break;
                case '+': 
                    if(have_result) finish = 1;
                    break;
                case '-': 
                    if(have_result || sign == -1) finish = 1;
                    else sign = -1; 
                    break;
                case '0': case '1': case '2': case '3': case '4':
                case '5': case '6': case '7': case '8': case '9': 
                    result = (10 * result) + (*cursor - '0'); have_result = 1;
                    break;
                default: 
                    finish = 1;
                    break;
            }
            ++cursor;
        }
    }

    return (have_result) ? result * sign : default_value; 
}

STBJDEF double stbj_readp_float(stbj_context* context, int index, double default_value)
{
    STBJ_ASSERT(context);
    STBJ_ASSERT(context->cursor);
    STBJ_ASSERT(context->current_env != STBJ_ERROR);
    STBJ_ASSERT(index >= 0);

    const char* cursor = stbj_get_element(context, index);
    double sign = 1.0;
    double result = 0.0;
    int have_result = 0;
    double decimal = 1.0;
    int past_dot = 0;

    if(cursor != NULL)
    {
        int max_len = context->len - (int)(cursor - context->buffer);
        int between_comas = 0;
        int finish = 0;

        // if object just consume chars until ':'
        if(context->current_env == STBJ_OBJECT)
            while(max_len-- > 0 && *cursor && *cursor++ != ':');

        // iterate chars, embedding the logic for atof()
        while(max_len-- > 0 && *cursor && !finish) 
        { 
            switch(*cursor)
            {
                case ' ': break;
                case '"': 
                    if(between_comas++) finish = 1;
                    break;
                case '+': 
                    if(have_result) finish = 1;
                    break;
                case '-': 
                    if(have_result || sign == -1.0) finish = 1;
                    else sign = -1.0; 
                    break;
                case '.':
                    if(!past_dot) past_dot = 1;
                    else finish = 1;
                    break;
                case '0': case '1': case '2': case '3': case '4':
                case '5': case '6': case '7': case '8': case '9': 
                    have_result = 1;
                    result = (10 * result) + (*cursor - '0');
                    if(past_dot) decimal *= 10.0;
                    break;
                default: 
                    finish = 1;
                    break;
            }
            ++cursor;
        }
    }

    return (have_result) ? ((result/decimal) * sign) : default_value; 
}


STBJDEF int stbj_readp_string(stbj_context* context, int index, char* buffer, int buffer_size, const char* default_value)
{
    STBJ_ASSERT(context);
    STBJ_ASSERT(context->cursor);
    STBJ_ASSERT(context->current_env != STBJ_ERROR);
    STBJ_ASSERT(index >= 0);
    STBJ_ASSERT(buffer);
    STBJ_ASSERT(buffer_size > 0);

    const char* cursor = stbj_get_element(context, index);
    int buffer_index = 0;
    int consuming_spaces = 1;
    int between_comas = 0;

    if(cursor != NULL)
    {
        int max_len = context->len - (int)(cursor - context->buffer);
        int finish = 0;

        // if object just consume chars until ':'
        if(context->current_env == STBJ_OBJECT)
            while(max_len-- > 0 && *cursor && *cursor++ != ':');

        // iterate chars, embedding the logic for atoi()
        while(max_len-- > 0 && *cursor && buffer_index < (buffer_size-1) && !finish) 
        { 
            switch(*cursor)
            {
                case ' ': 
                    if(between_comas || !consuming_spaces) buffer[buffer_index++] = *cursor; 
                    break;
                case '"': 
                    if(between_comas++ || !consuming_spaces) finish = 1;
                    break;
                case ']': case '}': case ',':
                    if(!between_comas) finish = 1;
                    break;
                default: 
                    consuming_spaces = 0;
                    buffer[buffer_index++] = *cursor;
                    break;
            }
            ++cursor;
        }
    }

    // if we did not wrote anything, strcpy default value int buf
    if(buffer_index == 0 && default_value != NULL)
        while((buffer[buffer_index++] = *default_value++) && buffer_index < (buffer_size-1));
    else if(!between_comas)
        while(buffer[buffer_index-1] == ' ' && buffer_index-- > 0); // trim last spaces if no commans used

    buffer[buffer_index] = 0;

    return buffer_index; 
}

STBJDEF int stbj_read(stbj_context* context, const char* tag_name, char* buffer, int buffer_size)
{
    STBJ_ASSERT(context);
    STBJ_ASSERT(context->cursor);
    STBJ_ASSERT(context->current_env != STBJ_ERROR);
    STBJ_ASSERT(tag_name);
    STBJ_ASSERT(buffer);
    STBJ_ASSERT(buffer_size > 0);

    const char* cursor1 = NULL;
    const char* cursor2 = NULL;

    {
        cursor1 = find_next(context, '"', cursor2);
        cursor2 = find_next(context, '"', cursor1);
        if(string_compare(cursor1, cursor2, tag_name))
        {
            buffer = (char*) find_next(context, ':', cursor2);
            if(buffer)
            {
                const char* end = find_next(context, ',', buffer);
                if(end)
                    return buffer - end;
                else
                    return buffer - find_next(context, '}', buffer);
            }
        }
    }

    return 0;
}

#endif // STB_JSON_IMPLEMENTATION

/*
   revision history:
      0.10    (2019-07-20)
              first released version
*/


/*
------------------------------------------------------------------------------
This software is available under 2 licenses -- choose whichever you prefer.
------------------------------------------------------------------------------
ALTERNATIVE A - MIT License
Copyright (c) 2019 Ricard Pillosu
Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
of the Software, and to permit persons to whom the Software is furnished to do
so, subject to the following conditions:
The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
------------------------------------------------------------------------------
ALTERNATIVE B - Public Domain (www.unlicense.org)
This is free and unencumbered software released into the public domain.
Anyone is free to copy, modify, publish, use, compile, sell, or distribute this
software, either in source code form or as a compiled binary, for any purpose,
commercial or non-commercial, and by any means.
In jurisdictions that recognize copyright laws, the author or authors of this
software dedicate any and all copyright interest in the software to the public
domain. We make this dedication for the benefit of the public at large and to
the detriment of our heirs and successors. We intend this dedication to be an
overt act of relinquishment in perpetuity of all present and future rights to
this software under copyright law.
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
------------------------------------------------------------------------------
*/
