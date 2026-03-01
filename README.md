# JC4827W543 Wifi Analyzer

<a href="https://www.buymeacoffee.com/thelastoutpostworkshop" target="_blank">
<img src="https://www.buymeacoffee.com/assets/img/custom_images/orange_img.png" alt="Buy Me A Coffee">
</a>

## Youtube Tutorial
[<img src="https://github.com/thelastoutpostworkshop/images/blob/main/wifi_analyser.png" width="500">](https://youtu.be/mnOzfRFQJIM)

## Project Description
This project turns the JC4827W543 (ESP32-S3) development board into a standalone 2.4 GHz WiFi analyzer. It performs continuous WiFi scans and renders a live channel graph on the onboard display so you can quickly see network density, signal strength, and likely lower-noise channels for better WiFi planning.

## Features
- Live 2.4 GHz WiFi scanning and visualization on channels 1-14
- Channel-by-channel RSSI graph with color-coded channel traces
- Strongest network label per channel (SSID/BSSID + RSSI)
- Open network indicator (`*`) when encryption is not enabled
- Automatic low-noise channel suggestion (channels 1-11)
- Per-channel access point count shown under each channel
- Periodic refresh loop (default scan interval: 3 seconds)