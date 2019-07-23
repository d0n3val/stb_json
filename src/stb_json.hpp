// Example of a wrapper C++ class for stb_json
//
// TODO:
// Use of std::string instead of char buffers
// Throw exceptions for error handling (?)
//

#ifndef STB_JSON_HPP
#define STB_JSON_HPP

#define STB_JSON_IMPLEMENTATION
#include "stb_json.h"

class stb_json
{
    public:
        stb_json(const char* buffer, unsigned int buffer_length) { context = stbj_create_context(buffer, buffer_length); }
        stb_json(const stbj_context& context) : context(context) {}

        // Parsing methods ------------
        void GetString(const char* field_name, char* dest_buffer, int dest_size, const char* default_value = "?") const
        { stbj_read_string(&context, field_name, dest_buffer, dest_size, default_value); }

        void GetString(int index, char* dest_buffer, int dest_size, const char* default_value = "?") const
        { stbj_readp_string(&context, index, dest_buffer, dest_size, default_value); }

        int GetInt(const char* field_name, int default_value = 0) const
        { return stbj_read_int(&context, field_name, default_value); }

        int GetInt(int index, int default_value = 0) const
        { return stbj_readp_int(&context, index, default_value); }

        double GetDouble(const char* field_name, double default_value = 0.0) const
        { return stbj_read_double(&context, field_name, default_value); }

        double GetDouble(int index, double default_value = 0.0) const
        { return stbj_readp_double(&context, index, default_value); }

        stb_json MoveCursor(const char* field_name) const
        { return stbj_context(stbj_read_context(&context, field_name)); }

        stb_json MoveCursor(int index) const
        { return stbj_context(stbj_readp_context(&context, index)); }

        // Helper methods ------------
        int Count() const { return stbj_count_elements(&context); }
        const char* GetError() const { return stbj_get_last_error(&context); }
        bool HasError() const { return stbj_is_error(&context); }

    private:
        mutable stbj_context context;
};

#endif // STB_JSON_HPP
