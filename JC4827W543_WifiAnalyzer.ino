// Tutorial : https://youtu.be/mnOzfRFQJIM
/*
 * ESP WiFi Analyzer
 * Adapted for the JC4827W543 development board from the example provided in https://github.com/moononournation/Arduino_GFX
 */

// Use board "ESP32S3 Dev Module" (last tested on v3.2.0)
// Install "Dev Device Pins" with the Library Manager (last tested on v0.0.2)
#include <PINS_JC4827W543.h> // Install "GFX Library for Arduino" with the Library Manager (last tested on v1.5.6)
#include "WiFi.h"            // Included with the Espressif Arduino Core (last tested on v3.2.0)

#define SCAN_INTERVAL 3000

int16_t w, h, text_size, banner_height, graph_baseline, graph_height, channel_width, signal_width;

// RSSI RANGE
#define RSSI_CEILING -40
#define RSSI_FLOOR -100

// Channel color mapping from channel 1 to 14
uint16_t channel_color[] = {
    RGB565_RED, RGB565_ORANGE, RGB565_YELLOW, RGB565_GREEN, RGB565_CYAN, RGB565_BLUE, RGB565_MAGENTA,
    RGB565_RED, RGB565_ORANGE, RGB565_YELLOW, RGB565_GREEN, RGB565_CYAN, RGB565_BLUE, RGB565_MAGENTA};

uint8_t scan_count = 0;

void setup()
{
    Serial.begin(115200);
    Serial.println("Arduino_GFX ESP WiFi Analyzer example");

    // Set WiFi to station mode and disconnect from an AP if it was previously connected
    WiFi.mode(WIFI_STA);
    WiFi.disconnect();
    delay(100);

#ifdef GFX_BL
    // Set the backlight of the screen to High intensity
    pinMode(GFX_BL, OUTPUT);
    digitalWrite(GFX_BL, HIGH);
#endif

    // Init Display
    if (!gfx->begin())
    {
        Serial.println("gfx->begin() failed!");
    }
    w = gfx->width();
    h = gfx->height();
    text_size = (h < 200) ? 1 : 2;
    banner_height = text_size * 3 * 4;
    graph_baseline = h - 20;                            // minus 2 text lines
    graph_height = graph_baseline - banner_height - 30; // minus 3 text lines
    channel_width = w / 17;
    signal_width = channel_width * 2;

    // init banner
    gfx->setTextSize(text_size);
    gfx->fillScreen(RGB565_BLACK);
    gfx->setTextColor(RGB565_RED);
    gfx->setCursor(0, 0);
    gfx->print("ESP32-S3");
    gfx->setTextColor(RGB565_WHITE);
    gfx->print(" WiFi Analyzer");
}

bool matchBssidPrefix(uint8_t *a, uint8_t *b)
{
    for (uint8_t i = 0; i < 5; i++)
    { // only compare first 5 bytes
        if (a[i] != b[i])
        {
            return false;
        }
    }
    return true;
}

void loop()
{
    uint8_t ap_count_list[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    int32_t noise_list[] = {RSSI_FLOOR, RSSI_FLOOR, RSSI_FLOOR, RSSI_FLOOR, RSSI_FLOOR, RSSI_FLOOR, RSSI_FLOOR, RSSI_FLOOR, RSSI_FLOOR, RSSI_FLOOR, RSSI_FLOOR, RSSI_FLOOR, RSSI_FLOOR, RSSI_FLOOR};
    int32_t peak_list[] = {RSSI_FLOOR, RSSI_FLOOR, RSSI_FLOOR, RSSI_FLOOR, RSSI_FLOOR, RSSI_FLOOR, RSSI_FLOOR, RSSI_FLOOR, RSSI_FLOOR, RSSI_FLOOR, RSSI_FLOOR, RSSI_FLOOR, RSSI_FLOOR, RSSI_FLOOR};
    int16_t peak_id_list[] = {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1};
    int32_t channel;
    int16_t idx;
    int32_t rssi;
    uint8_t *bssid;
    String ssid;
    uint16_t color;
    int16_t height, offset, text_width;

    // WiFi.scanNetworks will return the number of networks found
    int n = WiFi.scanNetworks(false /* async */, true /* show_hidden */, true /* passive */, 500 /* max_ms_per_chan */);

    // clear old graph
    gfx->fillRect(0, banner_height, w, h - banner_height, RGB565_BLACK);
    gfx->setTextSize(1);

    if (n == 0)
    {
        gfx->setTextColor(RGB565_WHITE);
        gfx->setCursor(0, banner_height);
        gfx->println("no networks found");
    }
    else
    {
        for (int i = 0; i < n; i++)
        {
            channel = WiFi.channel(i);
            idx = channel - 1;
            rssi = WiFi.RSSI(i);
            bssid = WiFi.BSSID(i);

            // channel peak stat
            if (peak_list[idx] < rssi)
            {
                peak_list[idx] = rssi;
                peak_id_list[idx] = i;
            }

            // check signal come from same AP
            bool duplicate_SSID = false;
            for (int j = 0; j < i; j++)
            {
                if ((WiFi.channel(j) == channel) && matchBssidPrefix(WiFi.BSSID(j), bssid))
                {
                    duplicate_SSID = true;
                    break;
                }
            }

            if (!duplicate_SSID)
            {
                ap_count_list[idx]++;

                // noise stat
                int32_t noise = rssi - RSSI_FLOOR;
                noise *= noise;
                if (channel > 4)
                {
                    noise_list[idx - 4] += noise;
                }
                if (channel > 3)
                {
                    noise_list[idx - 3] += noise;
                }
                if (channel > 2)
                {
                    noise_list[idx - 2] += noise;
                }
                if (channel > 1)
                {
                    noise_list[idx - 1] += noise;
                }
                noise_list[idx] += noise;
                if (channel < 14)
                {
                    noise_list[idx + 1] += noise;
                }
                if (channel < 13)
                {
                    noise_list[idx + 2] += noise;
                }
                if (channel < 12)
                {
                    noise_list[idx + 3] += noise;
                }
                if (channel < 11)
                {
                    noise_list[idx + 4] += noise;
                }
            }
        }

        // plot found WiFi info
        for (int i = 0; i < n; i++)
        {
            channel = WiFi.channel(i);
            idx = channel - 1;
            rssi = WiFi.RSSI(i);
            color = channel_color[idx];
            height = constrain(map(rssi, RSSI_FLOOR, RSSI_CEILING, 1, graph_height), 1, graph_height);
            offset = (channel + 1) * channel_width;

            // trim rssi with RSSI_FLOOR
            if (rssi < RSSI_FLOOR)
            {
                rssi = RSSI_FLOOR;
            }

            // plot chart
            // gfx->drawLine(offset, graph_baseline - height, offset - signal_width, graph_baseline + 1, color);
            // gfx->drawLine(offset, graph_baseline - height, offset + signal_width, graph_baseline + 1, color);
            gfx->startWrite();
            gfx->writeEllipseHelper(offset, graph_baseline + 1, signal_width, height, 0b0011, color);
            gfx->endWrite();

            if (i == peak_id_list[idx])
            {
                // Print SSID, signal strengh and if not encrypted
                String ssid = WiFi.SSID(i);
                if (ssid.length() == 0)
                {
                    ssid = WiFi.BSSIDstr(i);
                }
                text_width = (ssid.length() + 6) * 6;
                if (text_width > w)
                {
                    offset = 0;
                }
                else
                {
                    offset -= signal_width;
                    if ((offset + text_width) > w)
                    {
                        offset = w - text_width;
                    }
                }
                gfx->setTextColor(color);
                gfx->setCursor(offset, graph_baseline - 10 - height);
                gfx->print(ssid);
                gfx->print('(');
                gfx->print(rssi);
                gfx->print(')');
                if (WiFi.encryptionType(i) == WIFI_AUTH_OPEN)
                {
                    gfx->print('*');
                }
            }
        }
    }

    // print WiFi stat
    gfx->setTextColor(RGB565_WHITE);
    gfx->setCursor(0, banner_height);
    gfx->print(n);
    gfx->print(" networks found, lesser noise channels: ");
    bool listed_first_channel = false;
    int32_t min_noise = noise_list[0];          // init with channel 1 value
    for (channel = 2; channel <= 11; channel++) // channels 12-14 may not available
    {
        idx = channel - 1;
        log_i("min_noise: %d, noise_list[%d]: %d", min_noise, idx, noise_list[idx]);
        if (noise_list[idx] < min_noise)
        {
            min_noise = noise_list[idx];
        }
    }

    for (channel = 1; channel <= 11; channel++) // channels 12-14 may not available
    {
        idx = channel - 1;
        // check channel with min noise
        if (noise_list[idx] == min_noise)
        {
            if (!listed_first_channel)
            {
                listed_first_channel = true;
            }
            else
            {
                gfx->print(", ");
            }
            gfx->print(channel);
        }
    }

    // draw graph base axle
    gfx->drawFastHLine(0, graph_baseline, gfx->width(), RGB565_WHITE);
    for (channel = 1; channel <= 14; channel++)
    {
        idx = channel - 1;
        offset = (channel + 1) * channel_width;
        gfx->setTextColor(channel_color[idx]);
        gfx->setCursor(offset - ((channel < 10) ? 3 : 6), graph_baseline + 2);
        gfx->print(channel);
        if (ap_count_list[idx] > 0)
        {
            gfx->setCursor(offset - ((ap_count_list[idx] < 10) ? 9 : 12), graph_baseline + 8 + 2);
            gfx->print('{');
            gfx->print(ap_count_list[idx]);
            gfx->print('}');
        }
    }

    // Wait a bit before scanning again
    delay(SCAN_INTERVAL);
}
