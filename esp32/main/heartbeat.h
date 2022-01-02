#pragma once

#include <string>
#include <time.h>

void heartbeat(const struct tm& current,
               time_t last_pic);
