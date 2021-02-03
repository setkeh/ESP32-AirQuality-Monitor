# ESP32-Template-Project 

This example is based on `esp_https_ota` component's APIs.

## This is my standard ESP IDF template with OTA

This Template has been wired so it has OTA out of the box

You will need to configure the OTA and wifi with `idf.py menuconfig` for it to work.

I Have trimmed down a bunch of the fat in main.c to make it easier to read all the code and see what all the various
funcs do in their respective files.

There is a Template firmware.json file that should live on the webserver so the ota handler can figure out where the new
FW is for download and the versions.

The Build version of the current firmware that the firmware compares to the webserver version of the ota firmware is
stored in `ota.h`

## Setup

First things first if you fork this repo you should delete `sdkconfig.gpg` as my config wont help you in ints encrypted
form and you should generate your own with the idf tools.

For security purposes i have removed sdkconfig from the repo with `.gitignore` but it is needed at compile time for wifi
and ota Configuration to genertae you're own encrypted version run the following command in your work dir.

`gpg --symmetric --cipher-algo AES256 sdkconfig`

and check it into your repo.

Then head over to your repo's setting and then the secrets page and add a new repository secret named
`SDKCONFIG_SECRET_PASSPHRASE` with the value of the password you gave to the gpg command you just ran.

I am using a local runner for my build and release actions to automaticly update the ESP firmware on the webserver i
host the ota firmware this is easy to replicate following githubs instructions on how to setup a runner then just update
the paths to your http directories i mount my http directories to an nginx container on my server but there is about a
billion ways you can acheive the same result.

## Things to Rememeber

Dont forget to bump the firmware version variable in `ota.h` as if you do forget the ESP will end up in a boot loop as
it will always think there is an ota update available (i havent figured out a better way to fix this yet) the solution
to the issue though is to bump the version (e.g if you had version 0.1 on the esp and you released 0.2 and forgot to
update ota.h you update ota.h to 1.3 and re release the repo at 0.3)
