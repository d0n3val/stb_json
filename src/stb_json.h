/* stb_json - v0.2 - public domain json parser - http://d0n3val.github.org
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
   This lib has zero dependencies, even with standard libraries. It does not
   allocate any memory in the heap.

   QUICK NOTES:
      Primarily of interest to deploy a non-intrusive json parser

   Full documentation under "DOCUMENTATION" below.


LICENSE

  See end of file for license information.

RECENT REVISION HISTORY:

      0.1  (2019-06-15) Basic parsing
      0.2  (2019-06-27) Parse of other arrays / objects

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
// TODO ======================================================================
//
// Understant null / true / false
// Benchmark the lib against others
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

#ifndef STBJ_ASSERT
#include <assert.h>
#define STBJ_ASSERT(x) assert(x)
#endif

#ifdef __cplusplus
#define STBJ_EXTERN extern "C"
#else
#define STBJ_EXTERN extern
#endif

///////////////////////////////////////////////
//
//  stbj_context struct and related functions

// stbj_context structure is our basic context used by all json buffers
//
enum context_type { STBJ_OBJECT, STBJ_ARRAY, STBJ_ERROR };

typedef struct
{
    enum context_type context;
	unsigned int len;
	const char* buffer;
    const char* cursor;
    char error;
} stbj_context;

STBJDEF int stbj_is_error(const stbj_context* context)
{
    STBJ_ASSERT(context);

    return(context->error);
}

STBJDEF const char* stbj_get_last_error(const stbj_context* context)
{
    STBJ_ASSERT(context);

    switch(context->error)
    {
        case 0: return "No error";
        case 1: return "JSON parse error, could not find { or [";
        case 2: return "Element not found in current context";
        case 3: return "JSON parse error, [] or {} mismatch";
        case 4: return "JSON parse error, [] mismatch";
        case 5: return "JSON parse error, {} mismatch";
        case 6: return "Context must be of type Object";
        case 7: return "JSON error parsing string to number";
        case 8: return "JSON error parsing to string";
        default: return "Unknown error";
    }
}

// NOTE: it is up to the application to keep buffer as valid and readable memory
STBJDEF stbj_context stbj_create_context(const char *buffer, unsigned int len)
{
    STBJ_ASSERT(buffer);
    STBJ_ASSERT(len > 0);

    stbj_context context;
    context.len = len;
    context.buffer = buffer;
    context.cursor = buffer;
    context.context = STBJ_ERROR;
    context.error = 1;

    unsigned int max_len = context.len - (unsigned int)(context.cursor - context.buffer);

    while(max_len-- > 0 && *context.cursor) 
    { 
        switch(*context.cursor)
        {
            case ' ': case '\n': case '\r': break;
            case '[': context.context = STBJ_ARRAY; context.error = 0; return context;
            case '{': context.context = STBJ_OBJECT; context.error = 0; return context;
            default: return context;
        }
        ++context.cursor;
    }

    return context;
}

STBJDEF int stbj_count_elements(stbj_context* context)
{
    STBJ_ASSERT(context);
    STBJ_ASSERT(context->cursor);
    STBJ_ASSERT(context->context != STBJ_ERROR);

    context->error = 0;
    int result = 0;
    int between_comas = 0;
    char stack[256];
    int stack_index = -1;
    char delimiter = (context->context == STBJ_ARRAY) ? ']' : '}';

    const char* cursor = context->cursor;
    unsigned int max_len = context->len - (unsigned int)(cursor - context->buffer);

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
                case ']': 
                    if(stack[stack_index] == '[') --stack_index; 
                    else { context->error = 4; return -1; } 
                break;
                case '}': 
                    if(stack[stack_index] == '{') --stack_index; 
                    else { context->error = 5; return -1; } 
                break;
            }
        }
    }

    context->error = 3;
    return -1;
}

// Return a pointer to the place in the buffer that a certain element exist
STBJDEF const char* stbj_get_element(stbj_context* context, int index)
{
    STBJ_ASSERT(context);
    STBJ_ASSERT(context->cursor);
    STBJ_ASSERT(context->context != STBJ_ERROR);
    STBJ_ASSERT(index >= 0);

    context->error = 3;
    int result = 0;
    int between_comas = 0;
    char stack[256];
    int stack_index = -1;
    char delimiter = (context->context == STBJ_ARRAY) ? ']' : '}';

    const char* cursor = context->cursor;
    unsigned int max_len = context->len - (unsigned int)(cursor - context->buffer);

    while(max_len-- > 0 && *++cursor && stack_index < 256) 
    {
        if(result == index)
        {
            context->error = 0;

            // if object just consume chars until ':'
            if(context->context == STBJ_OBJECT)
                while(max_len-- > 0 && *cursor && *cursor++ != ':');

            return cursor;
        }

        // Two different logics if we are in a nested array/object or not
        if(stack_index < 0)
        {
            // we are on the same array/object level
            switch(*cursor)
            {
                case '[': case '{': stack[++stack_index] = *cursor; break;
                case ']': case '}': if(!between_comas && *cursor == delimiter) return 0; break;
                case ',': result += !between_comas; break;
                case '"': between_comas = !between_comas; break;
            }
        }
        else
        {
            // we are in a nested array/object, keep consuming chars and record progress in our stack
            switch(*cursor)
            {
                case '[': case '{': stack[++stack_index] = *cursor; break;
                case ']': 
                    if(stack[stack_index] == '[') --stack_index; 
                    else { context->error = 4; return 0; }
                break;
                case '}': 
                    if(stack[stack_index] == '{') --stack_index;
                    else { context->error = 5; return 0; }
                break;
            }
        }
    }

    return 0;
}

STBJDEF int stbj_find_element(stbj_context* context, const char* name)
{
    STBJ_ASSERT(context);
    STBJ_ASSERT(context->cursor);
    STBJ_ASSERT(context->context != STBJ_ERROR);
    STBJ_ASSERT(name);

    context->error = 6;
    if(context->context == STBJ_ARRAY)
        return -1;

    context->error = 0;
    int result = 0;
    int between_comas = 0;
    char stack[256];
    int stack_index = -1;

    const char* cursor = context->cursor;
    const char* compare_cursor = name;
    unsigned int max_len = context->len - (unsigned int)(cursor - context->buffer);

    while(max_len-- > 0 && *++cursor && stack_index < 256) 
    {
        // Two different logics if we are in a nested array/object or not
        if(stack_index < 0)
        {
            // we are on the same array/object level
            switch(*cursor)
            {
                case '[': case '{': stack[++stack_index] = *cursor; break;
                case ']': if(!between_comas) { context->error = 4; return -1; } break;
                case '}': if(!between_comas) { context->error = 5; return -1; } break; 
                case ',': result += !between_comas; break;
                case '"': between_comas = !between_comas; break;
                default:
                    if(between_comas && *compare_cursor == *cursor)
                        { if(*++compare_cursor == 0 && *(cursor+1) == '"') return result; }
                    else
                        compare_cursor = name;
                    break;
            }
        }
        else
        {
            // we are in a nested array/object, keep consuming chars and record progress in our stack
            switch(*cursor)
            {
                case '[': case '{': stack[++stack_index] = *cursor; break;
                case ']': 
                    if(stack[stack_index] == '[') --stack_index; 
                    else { context->error = 4; return -1; }
                break;
                case '}': 
                    if(stack[stack_index] == '{') --stack_index; 
                    else { context->error = 5; return -1; }  
                break;
            }
        }
    }

    context->error = 2;
    return -1;
}

STBJDEF int stbj_readp_int(stbj_context* context, int index, int default_value)
{
    STBJ_ASSERT(context);
    STBJ_ASSERT(context->cursor);
    STBJ_ASSERT(context->context != STBJ_ERROR);
    STBJ_ASSERT(index >= 0);

    const char* cursor = stbj_get_element(context, index);

    if(!cursor)
        return default_value; // error num already set by get_element()

    enum parse_states
    {
        before_value,
        at_num_value,       // 123
        at_special_value,   // true/false/null
        after_value,
        finish,
        error
    } state = before_value;

    context->error = 0;
    unsigned int max_len = context->len - (unsigned int)(cursor - context->buffer);
    int between_comas = 0;
    int sign = 1;
    int result = 0;
    int have_result = 0;
    int past_dot = 0;
    char special[] = "rue\0ull\0alse";
    char* c_special = &special[0];

    while(max_len-- > 0 && *cursor && state != finish) 
    {
        switch(state)
        {
            case before_value:
            {
                // Parsing done before encountering any value of meaning -----------------
                switch(*cursor)
                {
                    case ' ': case '\n': case '\r': break;
                    case '.': --cursor; have_result = 1; state = at_num_value; break;
                    case '"': if(between_comas++) state = finish; break;
                    case '-': sign = -1; // fall to next option
                    case '+': state = at_num_value; have_result = 1; break;
                    case '0': case '1': case '2': case '3': case '4':
                    case '5': case '6': case '7': case '8': case '9': 
                        result = *cursor - '0'; state = at_num_value; have_result = 1; break;
                    case 't': state = at_special_value; have_result = 1; result = 1; break;
                    case 'f': c_special += 8; state = at_special_value; have_result = 1;  break;
                    case 'n': c_special += 4; state = at_special_value; have_result = 1;  break;
                    default: --cursor; state = after_value; break;
                }

            } break;

            case at_num_value:
            {
                // Parsing on a number ---------------------------------------------------
                switch(*cursor)
                {
                    case '.': if(past_dot++) state = error; break;
                    case '0': case '1': case '2': case '3': case '4':
                    case '5': case '6': case '7': case '8': case '9':
                        if(!past_dot) result = (10 * result) + (*cursor - '0'); break;
                    default: --cursor; state = after_value; break;
                }
            } break;

            case at_special_value:
            {
                // Parsing on a special token (true/false/null) --------------------------
                switch(*cursor)
                {
                    case 'r': case 'u':
                    case 'a': case 's':
                        if(*cursor != *c_special++) state = error; break;
                    case 'e':
                        state = (*cursor != *c_special) ? error : after_value; break;
                    case 'l':
                    {
                        if(*cursor != *c_special++)
                            state = error;
                        else 
                            if(*c_special == 0) state = after_value;
                    } break;
                    default: state = error; break;
                }
            } break;

            // We are finish parsing, wait until we end the element ----------------------
            case after_value:
            {
                switch(*cursor)
                {
                    case ' ': case '\n': case '\r': break;
                    case '"': state = (between_comas) ? after_value : error; break;
                    case ',': state = finish; break;
                    case ']': state = (context->context == STBJ_ARRAY) ? finish : error; break;
                    case '}': state = (context->context == STBJ_OBJECT) ? finish : error; break;
                    default: state = error; break;
                }
            } break;

            case finish: case error: default: STBJ_ASSERT(0 && "this should never happen!"); break;
        }

        // corner case where the error case is not evaluated at the end of max_len ...
        if(state == error)
        {
            context->error = 7; 
            return default_value;
        }
        
        ++cursor;
    }

    return (have_result) ? result * sign : default_value;
}

STBJDEF int stbj_read_int(stbj_context* context, const char* name, int default_value)
{
    STBJ_ASSERT(context);
    STBJ_ASSERT(context->cursor);
    STBJ_ASSERT(context->context != STBJ_ERROR);
    STBJ_ASSERT(name);

    int pos = stbj_find_element(context, name);
    return (pos >= 0) ? stbj_readp_int(context, pos, default_value) : default_value;
}

STBJDEF double stbj_readp_double(stbj_context* context, int index, double default_value)
{
    STBJ_ASSERT(context);
    STBJ_ASSERT(context->cursor);
    STBJ_ASSERT(context->context != STBJ_ERROR);
    STBJ_ASSERT(index >= 0);

    const char* cursor = stbj_get_element(context, index);

    if(!cursor)
        return default_value; // error num already set by get_element()

    enum parse_states
    {
        before_value,
        at_num_value,       // 123
        at_special_value,   // true/false/null
        after_value,
        finish,
        error
    } state = before_value;

    context->error = 0;
    unsigned int max_len = context->len - (unsigned int)(cursor - context->buffer);
    int between_comas = 0;
    double sign = 1.0;
    double result = 0.0;
    int have_result = 0;
    double decimal = 1.0;
    int past_dot = 0;
    char special[] = "rue\0ull\0alse";
    char* c_special = &special[0];

    while(max_len-- > 0 && *cursor && state != finish) 
    {
        switch(state)
        {
            case before_value:
            {
                // Parsing done before encountering any value of meaning -----------------
                switch(*cursor)
                {
                    case ' ': case '\n': case '\r': break;
                    case '"': if(between_comas++) state = finish; break;
                    case '-': sign = -1.0; // fall to next option
                    case '+': state = at_num_value; have_result = 1; break;
                    case '.': past_dot = 1; state = at_num_value; have_result = 1; break;
                    case '0': case '1': case '2': case '3': case '4':
                    case '5': case '6': case '7': case '8': case '9': 
                        result = *cursor - '0'; state = at_num_value; have_result = 1; break;
                    case 't': state = at_special_value; have_result = 1; result = 1; break;
                    case 'f': c_special += 8; state = at_special_value; have_result = 1;  break;
                    case 'n': c_special += 4; state = at_special_value; have_result = 1;  break;
                    default: --cursor; state = after_value; break;
                }

            } break;

            case at_num_value:
            {
                // Parsing on a number ---------------------------------------------------
                switch(*cursor)
                {
                    case '.': if(past_dot++) state = error; break;
                    case '0': case '1': case '2': case '3': case '4':
                    case '5': case '6': case '7': case '8': case '9':
                        result = (10 * result) + (*cursor - '0');
                        if(past_dot) decimal *= 10.0;
                    break;
                    default: --cursor; state = after_value; break;
                }
            } break;

            case at_special_value:
            {
                // Parsing on a special token (true/false/null) --------------------------
                switch(*cursor)
                {
                    case 'r': case 'u':
                    case 'a': case 's':
                        if(*cursor != *c_special++) state = error; break;
                    case 'e':
                        state = (*cursor != *c_special) ? error : after_value; break;
                    case 'l':
                    {
                        if(*cursor != *c_special++)
                            state = error;
                        else 
                            if(*c_special == 0) state = after_value;
                    } break;
                    default: state = error; break;
                }
            } break;

            // We are finish parsing, wait until we end the element ----------------------
            case after_value:
            {
                switch(*cursor)
                {
                    case ' ': case '\n': case '\r': break;
                    case '"': state = (between_comas) ? after_value : error; break;
                    case ',': state = finish; break;
                    case ']': state = (context->context == STBJ_ARRAY) ? finish : error; break;
                    case '}': state = (context->context == STBJ_OBJECT) ? finish : error; break;
                    default: state = error; break;
                }
            } break;

            case finish: case error: default: STBJ_ASSERT(0 && "this should never happen!"); break;
        }

        // corner case where the error case is not evaluated at the end of max_len ...
        if(state == error)
        {
            context->error = 7; 
            return default_value;
        }
        
        ++cursor;
    }

    return (have_result) ? ((result/decimal) * sign) : default_value; 
}

STBJDEF double stbj_read_double(stbj_context* context, const char* name, double default_value)
{
    STBJ_ASSERT(context);
    STBJ_ASSERT(context->cursor);
    STBJ_ASSERT(context->context != STBJ_ERROR);
    STBJ_ASSERT(name);

    int pos = stbj_find_element(context, name);
    return (pos >= 0) ? stbj_readp_double(context, pos, default_value) : default_value;
}


STBJDEF int stbj_readp_string(stbj_context* context, int index, char* buffer, int buffer_size, const char* default_value)
{
    STBJ_ASSERT(context);
    STBJ_ASSERT(context->cursor);
    STBJ_ASSERT(context->context != STBJ_ERROR);
    STBJ_ASSERT(index >= 0);
    STBJ_ASSERT(buffer);
    STBJ_ASSERT(buffer_size > 0);

    const char* cursor = stbj_get_element(context, index);
    int buffer_index = 0;

    if(!cursor)
    {
        // TODO check if we can avoid replicating this code
        if(default_value != 0)
            while((buffer[buffer_index++] = *default_value++) && buffer_index < (buffer_size-1));
        buffer[buffer_index] = 0;
        return buffer_index;
    }

    enum parse_states
    {
        before_value,
        at_string_value,    // hellot world
        at_special_value,   // true/false/null
        after_value,
        finish,
        error
    } state = before_value;

    context->error = 0;
    unsigned int max_len = context->len - (unsigned int)(cursor - context->buffer);
    int between_comas = 0;
    char special[] = "ull";
    char* c_special = &special[0];

    while(max_len-- > 0 && *cursor && state != finish) 
    {
        switch(state)
        {
            case before_value:
            {
                // Parsing done before encountering any value of meaning -----------------
                switch(*cursor)
                {
                    case ' ': case '\n': case '\r': break;
                    case '"': between_comas = 1; state = at_string_value; break;
                    case 'n': state = at_special_value; break;
                    case '[': case '{': case ']': case '}': case ',': 
                              --cursor; state = after_value; break;
                    default: --cursor; state = at_string_value; break;
                }

            } break;

            case at_string_value:
            {
                // Parsing on a string ---------------------------------------------------
                switch(*cursor)
                {
                    case '"': --cursor; state = after_value; break;
                    case '[': case '{': case ']': case '}': case ',': 
                        if(!between_comas) { --cursor; state = after_value; } break;
                    default: 
                        if(buffer_index < buffer_size-1) 
                            buffer[buffer_index++] = *cursor; 
                        else
                            state = finish;
                        break;
                }
            } break;

            case at_special_value:
            {
                // Parsing on a special token (null) --------------------------
                switch(*cursor)
                {
                    case 'u':
                        if(*cursor != *c_special++) state = error; break;
                    case 'l':
                    {
                        if(*cursor != *c_special++)
                            state = error;
                        else 
                            if(*c_special == 0) state = after_value;
                    } break;
                    default: state = error; break;
                }
            } break;

            // We are finish parsing, wait until we end the element ----------------------
            case after_value:
            {
                switch(*cursor)
                {
                    case ' ': case '\n': case '\r': break;
                    case '"': state = (between_comas) ? after_value : error; break;
                    case ',': state = finish; break;
                    case ']': state = (context->context == STBJ_ARRAY) ? finish : error; break;
                    case '}': state = (context->context == STBJ_OBJECT) ? finish : error; break;
                    default: state = error; break;
                }
            } break;

            case finish: case error: default: STBJ_ASSERT(0 && "this should never happen!"); break;
        }

        // corner case where the error case is not evaluated at the end of max_len ...
        if(state == error)
        {
            context->error = 8; 
            return 0;
        }
        
        ++cursor;
    }

    // if we did not wrote anything, strcpy default value into buffer
    if(buffer_index == 0 && default_value != 0)
        while((buffer[buffer_index++] = *default_value++) && buffer_index < (buffer_size-1));

    buffer[buffer_index] = 0;
    return buffer_index; 
}

STBJDEF int stbj_read_string(stbj_context* context, const char* name, char* buffer, int buffer_size, const char* default_value)
{
    STBJ_ASSERT(context);
    STBJ_ASSERT(context->cursor);
    STBJ_ASSERT(context->context != STBJ_ERROR);
    STBJ_ASSERT(name);
    STBJ_ASSERT(buffer);
    STBJ_ASSERT(buffer_size > 0);

    int pos = stbj_find_element(context, name);
    if (pos >= 0) 
        return stbj_readp_string(context, pos, buffer,buffer_size, default_value);
    else
    {
        int buffer_index = 0;
        while((buffer[buffer_index++] = *default_value++) && buffer_index < (buffer_size-1));
    }
    return 0;
}

STBJDEF stbj_context stbj_readp_context(stbj_context* context, int index)
{
    STBJ_ASSERT(context);
    STBJ_ASSERT(context->cursor);
    STBJ_ASSERT(context->context != STBJ_ERROR);
    STBJ_ASSERT(index >= 0);

    stbj_context ret;
    ret.context = STBJ_ERROR;
    ret.len = context->len;
    ret.buffer = context->buffer;
    ret.cursor = stbj_get_element(context, index);
    ret.error = 1;

    if(ret.cursor != 0)
    {
        unsigned int max_len = context->len - (unsigned int)(ret.cursor - context->buffer);

        // iterate chars
        while(max_len-- > 0 && *ret.cursor) 
        { 
            switch(*ret.cursor)
            {
                case ' ': case '\n': case '\r': break;
                case '[': ret.context = STBJ_ARRAY; ret.error = 0; return ret;
                case '{': ret.context = STBJ_OBJECT; ret.error = 0; return ret;
                default: return ret;
            }
            ++ret.cursor;
        }
    }

    return ret; 
}

STBJDEF stbj_context stbj_read_context(stbj_context* context, const char* name)
{
    STBJ_ASSERT(context);
    STBJ_ASSERT(context->cursor);
    STBJ_ASSERT(context->context != STBJ_ERROR);
    STBJ_ASSERT(name);

    int pos = stbj_find_element(context, name);
    if (pos >= 0) 
        return stbj_readp_context(context, pos);

    stbj_context ret;
    ret.context = STBJ_ERROR;
    ret.len = 0;
    ret.buffer = 0;
    ret.cursor = 0;
    ret.error = 2;

    return ret;
}

#endif // STB_JSON_IMPLEMENTATION

/*
   revision history:
      0.1     (2019-06-20)
              Parsing basic numbers, reals and string
      0.2     (2019-06-20)
              Parsing other context (array/objects)
      0.3     (2019-06-28)
              Parsing errors properly communicated to client
      0.4     (2019-07-16)
              Management of special values true/false/null
      0.5     (2019-07-17)
              Handling of newline and carriage return chars
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
