#include "defs.h"
#include "eventhandler.h"
#include "heartbeat.h"
#include "nvs.h"

#include <string>

#include "cJSON.h"

#include "esp_log.h"
#include "esp_http_client.h"

void heartbeat(const struct tm& current,
               time_t last_pic)
{
    char ts[35] = { 0 };
    if (last_pic)
    {
        struct tm timeinfo;
        gmtime_r(&last_pic, &timeinfo);
        strftime(ts, sizeof(ts), "&last_pic=%Y-%m-%d%%20%H:%M:%S", &timeinfo);
    }
    char resource[85];
    sprintf(resource, "/camera/%d?active=%d&continuous=%d&version=%s%s",
            (int) get_instance(),
            (int) config_active,
            (int) config_continuous,
            VERSION,
            ts);
}

// Local Variables:
// compile-command: "(cd ..; idf.py build)"
// End:
