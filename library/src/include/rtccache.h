// Copyright (c) 2021 - present Advanced Micro Devices, Inc. All rights reserved.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.

#include "rocfft/rocfft.h"
#include "sqlite3.h"
#include <memory>
#include <mutex>
#include <string>
#include <vector>

#if __has_include(<filesystem>)
#include <filesystem>
#else
#include <experimental/filesystem>
namespace std
{
    namespace filesystem = experimental::filesystem;
}
#endif

// deleters for sqlite objects
struct sqlite3_deleter
{
    void operator()(sqlite3* db)
    {
        sqlite3_close(db);
    }
};
struct sqlite3_stmt_deleter
{
    void operator()(sqlite3_stmt* stmt)
    {
        sqlite3_finalize(stmt);
    }
};

// smart pointers for sqlite objects
typedef std::unique_ptr<sqlite3, sqlite3_deleter>           sqlite3_ptr;
typedef std::unique_ptr<sqlite3_stmt, sqlite3_stmt_deleter> sqlite3_stmt_ptr;

struct RTCCache
{
    RTCCache();
    ~RTCCache() = default;

    // get bytes for a matching code object from the cache.
    // returns empty vector if a matching kernel was not found.
    std::vector<char> get_code_object(const std::string&       kernel_name,
                                      const std::string&       gpu_arch,
                                      int                      hip_version,
                                      const std::vector<char>& generator_sum);

    // store the code object into the cache.
    void store_code_object(const std::string&       kernel_name,
                           const std::string&       gpu_arch,
                           int                      hip_version,
                           const std::vector<char>& generator_sum,
                           const std::vector<char>& code);

    // allocates buffer, call serialize_free to free it
    rocfft_status serialize(void** buffer, size_t* buffer_len_bytes);
    void          serialize_free(void* buffer);
    rocfft_status deserialize(const void* buffer, size_t buffer_len_bytes);

    // singleton allocated in rocfft_setup and freed in rocfft_cleanup
    static std::unique_ptr<RTCCache> single;

private:
    sqlite3_ptr connect_db(const std::filesystem::path& path);

    // database handle
    sqlite3_ptr db;

    // query handles, with mutexes to prevent concurrent queries that
    // might stomp on one another's bound values
    sqlite3_stmt_ptr get_stmt;
    std::mutex       get_mutex;
    sqlite3_stmt_ptr store_stmt;
    std::mutex       store_mutex;

    // lock around deserialization, since that attaches a fixed-name
    // schema to the db and we don't want a collision
    std::mutex deserialize_mutex;
};
