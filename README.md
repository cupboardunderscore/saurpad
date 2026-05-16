# 🦕 saur'pad
ESP32 based notification center for Home Assistant

## main features
- displays sensor info from Home Assistant
- warning for preset conditions, see [get_warning(unsigned long)
](ESP32/lib/devices/devices.cpp)
- warning for [custom conditions](#custom-warning-conditions)
- support for [custom warning sound and led blinking pattern](#custom-audioled-pattern) (requires an SD card)
- credentials setup using hotspot web server
- ota updates using a web server (requires an SD card)

## screenshots
<details><summary>main screen</summary>
<img width="1600" height="960" alt="image" src="https://github.com/user-attachments/assets/4b03cb79-da8c-478c-80eb-984c09380779" />
</details>
<details><summary>warning pop-up</summary>
<img width="1600" height="960" alt="image" src="https://github.com/user-attachments/assets/3d802e1a-170d-4191-bd7a-03d4e782606e" />
</details>
<details><summary>boot screen</summary>
<img width="1600" height="960" alt="image" src="https://github.com/user-attachments/assets/3e7934b7-0cb9-4568-be0f-3c199b609ce0" />
</details>
<details><summary>setup screen</summary>
<img width="1600" height="960" alt="image" src="https://github.com/user-attachments/assets/2df6288c-fe17-4341-a587-81e69b92e2d6" />
</details>
<details><summary>setup web interface</summary>
<img width="1084" height="1005" alt="image" src="https://github.com/user-attachments/assets/f9eed7ca-0cf1-46ac-8f4e-50d7a58d7cc7" />
</details>
<details><summary>update screen</summary>
<img width="1600" height="960" alt="image" src="https://github.com/user-attachments/assets/f01f7b76-eea2-44d4-a9d0-da07adbb3081" />
</details>
<details><summary>configuration and OTA update interface</summary>
<img width="1084" height="1076" alt="image" src="https://github.com/user-attachments/assets/9101d31e-5bd8-4a88-b9c2-d132773b79f3" />
</details>

## setup
### Home Assistant
- add [MQTT integration](https://www.home-assistant.io/integrations/mqtt) and install [Mosquitto broker](https://github.com/home-assistant/addons/blob/master/mosquitto/DOCS.md)
- setup credentials for Mosquitto broker
- enable [MQTT Discovery](https://www.home-assistant.io/integrations/mqtt#mqtt-discovery)
- import these automation blueprints

blueprints are configured to work with sensors, binary sensors, air quality and covers\
additional devices can be added during import process\
**it is important to add the same devices in every template where asked to**

<details><summary>blueprints</summary>

[![create-device](https://my.home-assistant.io/badges/blueprint_import.svg)](https://my.home-assistant.io/redirect/_change/?redirect=blueprint_import%2F%3Fblueprint_url%3Dhttps%253A%252F%252Fraw.githubusercontent.com%252Fcupboardunderscore%252Fsaurpad%252Frefs%252Fheads%252Fmain%252FHA%252Fsaurpad_create-device.yaml)

[![device-becomes-known](https://my.home-assistant.io/badges/blueprint_import.svg)](https://my.home-assistant.io/redirect/_change/?redirect=blueprint_import%2F%3Fblueprint_url%3Dhttps%253A%252F%252Fraw.githubusercontent.com%252Fcupboardunderscore%252Fsaurpad%252Frefs%252Fheads%252Fmain%252FHA%252Fsaurpad_device-becomes-known.yaml)

[![startup](https://my.home-assistant.io/badges/blueprint_import.svg)](https://my.home-assistant.io/redirect/_change/?redirect=blueprint_import%2F%3Fblueprint_url%3Dhttps%253A%252F%252Fraw.githubusercontent.com%252Fcupboardunderscore%252Fsaurpad%252Frefs%252Fheads%252Fmain%252FHA%252Fsaurpad_startup.yaml)

[![state-update](https://my.home-assistant.io/badges/blueprint_import.svg)](https://my.home-assistant.io/redirect/_change/?redirect=blueprint_import%2F%3Fblueprint_url%3Dhttps%253A%252F%252Fraw.githubusercontent.com%252Fcupboardunderscore%252Fsaurpad%252Frefs%252Fheads%252Fmain%252FHA%252Fsaurpad_state-update.yaml)
</details>

### ESP32
- download `firmware.bin` from the releases page
- flash ESP32 with `firmware.bin`
- connect to Wi-Fi hotspot created by device and fill out credentials in the web interface
- additional configuration and OTA updates can be made on a web server hosted on device

#### custom warning conditions
- send a MQTT message on topic `homeassistant/<device_id>/status` formatted `{"warning": "ON/OFF"}`

#### custom audio/led pattern
- create a folder named `saurpad` on an SD card
- add audio files names `<device_class>.mp3`
- add text files names `<device_class>.txt` formatted:
```
<time in ms for action>,<r/g/b>
```
<details><summary>example</summary>
flashes each color for 1s with 0,5s breaks, then turn every color on for 0,5s

```
1000,r
500
1000,g
500
1000,b
500
500,rgb
```
</details>

- room audio files can be added in `saurpad/rooms` named `<room_name>.mp3`

## Acknowledgments
- Status bar icons form Uicons by [Flaticon](https://www.flaticon.com/uicons)
- Sauropod emoji from [SerenityOS Emoji Font](https://github.com/linusg/serenityos-emoji-font), Copyright (c) 2018-2023, the SerenityOS developers,
Copyright (c) 2022-2023, Gegga Thor <xexxa@serenityos.org> - [LICENSE](https://github.com/linusg/serenityos-emoji-font/blob/main/LICENSE)
- Source Code Pro font from [Google Fonts](https://fonts.google.com/specimen/Source+Code+Pro) by Paul D. Hunt - [SIL OPEN FONT LICENSE Version 1.1](https://fonts.google.com/specimen/Source+Code+Pro/license)
- `esp_panel_drivers_conf.h`, `esp_utils_conf.h`, `lvgl_v8_port.h`, `lvgl_v8_port.cpp` and part of `void setupdisplay()` from [ESP Display Panel](https://github.com/esp-arduino-libs/ESP32_Display_Panel) by ESP Arduino Libs - [Apache-2.0 license](https://github.com/esp-arduino-libs/ESP32_Display_Panel?tab=Apache-2.0-1-ov-file)
- `index.css` partially reused from [ios-rcs](https://github.com/cupboardunderscore/ios-rcs) - [MIT license](https://github.com/cupboardunderscore/ios-rcs/blob/local/LICENSE)
- `config.hpp` and begining of `void sendtoclass(String, devices)` generated using [ArduinoJson Assistant](https://arduinojson.org/v7/assistant/)
- UI created using [EEZ Studio](https://www.envox.eu/studio/studio-introduction/)
