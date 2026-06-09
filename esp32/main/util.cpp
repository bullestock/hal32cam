#include "util.h"

#include <stdio.h>
#include <memory>

#include "cJSON.h"

namespace util
{

time_t make_timestamp(char* stamp)
{
    time_t current = 0;
    time(&current);
    make_timestamp(current, stamp);
    return current;
}

void make_timestamp(time_t t, char* stamp)
{
    struct tm timeinfo;
    gmtime_r(&t, &timeinfo);
    strftime(stamp, TIMESTAMP_SIZE, "%Y-%m-%dT%H:%M:%S", &timeinfo);
}

} // end namespace

cJSON_wrapper::cJSON_wrapper(cJSON*& json)
    : json(json)
{
}

cJSON_wrapper::~cJSON_wrapper()
{
    if (json)
        cJSON_Delete(json);
    json = nullptr;
}

cJSON_Print_wrapper::cJSON_Print_wrapper(char*& json)
    : json(json)
{
}

cJSON_Print_wrapper::~cJSON_Print_wrapper()
{
    if (json)
        free(json);
    json = nullptr;
}

// Local Variables:
// compile-command: "cd .. && idf.py build"
// End:

