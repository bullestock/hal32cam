#include "defs.h"
#include "motion.h"
#include "upload.h"

#include "JPEGDEC.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_log.h"
#include "esp_system.h"

constexpr const int FACTOR = 8;
constexpr const int BUFSIZE_X = FRAMESIZE_X/FACTOR;
constexpr const int BUFSIZE_Y = FRAMESIZE_Y/FACTOR;
constexpr const int X_FACTOR = 4;

constexpr const int BUFFER_BYTESIZE = BUFSIZE_X/X_FACTOR * BUFSIZE_Y;

uint8_t buf1[BUFFER_BYTESIZE];
uint8_t buf2[BUFFER_BYTESIZE];
bool current_buf = false;
bool first_time = true;

// Callback
static uint8_t* draw_cb_buf = nullptr;

int draw_cb(JPEGDRAW* draw)
{
    //printf("Draw: %d, %d\n", (int) draw->x, (int) draw->y);
    uint8_t* p = draw_cb_buf + draw->y * BUFSIZE_X / X_FACTOR;
    int i = 0;
    while (i < draw->iWidth)
    {
        int sum = 0;
        for (int j = 0; j < X_FACTOR; ++j)
        {
            uint16_t pixel = draw->pPixels[i + j];
            uint16_t red = ((pixel & 0xF800)>>11);
            uint16_t green = ((pixel & 0x07E0)>>5);
            uint16_t blue = (pixel & 0x001F);
            uint16_t grayscale = 8*((0.2126 * red) + (0.7152 * green / 2.0) + (0.0722 * blue));
            sum += grayscale;
        }
        *p++ = sum/X_FACTOR;
        i += X_FACTOR;
    }
    return 1;
}

void downsample(const camera_fb_t* fb,
                uint8_t* buf)
{
    draw_cb_buf = buf;
    JPEGDEC decoder;
    ESP_ERROR_CHECK(!decoder.openRAM(fb->buf, fb->len, draw_cb));
    /*ESP_ERROR_CHECK*/(!decoder.decode(0, 0, JPEG_SCALE_EIGHTH)); // can fail
}

bool motion_detect(bool force, const camera_fb_t* fb, const struct tm& cur_tm)
{
    if (!force)
    {
        const auto old_buf = current_buf ? buf1 : buf2;
        auto new_buf = current_buf ? buf2 : buf1;
        current_buf = !current_buf;

        downsample(fb, new_buf);
        if (first_time)
        {
            // First time: Save reference image and return false
            first_time = false;
            ESP_LOGI(TAG, "Saved reference image");
            return false;
        }

        int changes = 0;
        for (int i = 0; i < sizeof(buf1); ++i)
        {
            const auto diff = abs(static_cast<int>(new_buf[i]) - static_cast<int>(old_buf[i]));
            if (diff > PIXEL_THRESHOLD)
                ++changes;
        }
        printf("%d changes\n", changes);
        if ((changes*100)/BUFFER_BYTESIZE < PERCENT_THRESHOLD)
            return false;
    }
    upload(fb, cur_tm);

    return true;
}
