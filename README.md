# ESP32-Template-Project 

This example is based on `esp_https_ota` component's APIs.

## This is my standard ESP IDF template with OTA

This Template has been wired so it has OTA out of the box

You will need to configure the OTA and wifi with `idf.py menueconfig` for it to work.

I Have trimmed down a bunch of the fat in main.c to make it easier to read all the code and see what all the various
funcs do in their respective files.

There is a Template firmware.json file that should live on the webserver so the ota handler can figure out where the new
FW is for download and the versions.

The Build version of the current firmware that the firmware compares to the webserver version of the ota firmware is
stored in `ota.h`
