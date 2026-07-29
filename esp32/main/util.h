#pragma once

#include <time.h>

void fatal_error(const char* msg);

namespace util
{
static constexpr int TIMESTAMP_SIZE = 26;
    
/// Make a timestamp string. Buffer must be TIMESTAMP_SIZE bytes.
time_t make_timestamp(char* stamp, bool with_tz=false);

void make_timestamp(time_t t, char* stamp, bool with_tz=false);

} // end namespace

struct cJSON;

class cJSON_wrapper
{
public:
    cJSON_wrapper(cJSON*& json);

    ~cJSON_wrapper();

private:
    cJSON*& json;
};

class cJSON_Print_wrapper
{
public:
    cJSON_Print_wrapper(char*& json);

    ~cJSON_Print_wrapper();

private:
    char*& json;
};
