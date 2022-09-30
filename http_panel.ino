/**
 * (24.9.22)
 * Created by t3ch aka B.K. aka grandekos aka w4d4f4k aka etc..
 * For: ESP32, Arduino, similar.?
 * Useful to control GPIO pins trough WiFi + HTTP server and it configurations.
 * -----
 * Recommented editor usage: Geany + Fold all functions so you see better.
 * Compiling with Arduino IDE..
 * ------
 * 
 * 28.9.22 - fixed memory leakings. now program runs smothly!! :) ole.
 * 29.9.22 - more memory fixes
 *           style of code updates
 *           added support for DHT sensors
 * 
 * ------
 * G00d luck to all***
 * t3ch
 * */
#include <Arduino.h>
#include "WiFi.h"
#include <ArduinoJson.h> // https://arduinojson.org/?utm_source=meta&utm_medium=library.properties
#include <GParser.h>     // https://github.com/GyverLibs/GParser
#include <AceCRC.h>      // https://github.com/bxparks/AceCRC
#include <Regexp.h>      // https://github.com/nickgammon/Regexp
#include <DHT.h>         // https://github.com/m5it/DHT_sensor_library_modified, https://www.mouser.com/datasheet/2/758/DHT11-Technical-Data-Sheet-Translated-Version-1143054.pdf
#include <stdio.h>
#include <time.h>

//--
// startup http panel credentials
//const char* admin_user = "esp32Admin";
//const char* admin_pass = "esp32Pass";
// wifi configs.
const char* ssid = "esp32byk0s";
const char* pwd  = "esp32bypwd";
//
struct Options {
    int LoopDefault      = 25;
    int LoopIdle         = 1000;
    int disp_stats_every = 3; // display stats to Serial every 3s
};
//
Options options;
int loopDelay = options.LoopDefault;
//
IPAddress IP;
// start http server on port 80
WiFiServer server(80);

//
struct Statistics {
    int num_loops       = 0;
    int num_clients     = 0;
    int num_restarts    = 0;
    double last_loop_ts = 0; // last loop timestamp in ms
    double last_disp_ts = 0; // (timestamp in ms) to know when statistics was displayed last time (Used for debug and Serial output)
};
Statistics stats;

//
DynamicJsonDocument json_user_panel(4048);
DynamicJsonDocument tasks(4048);
DynamicJsonDocument actions(1024);
String sson_user_panel = "";
String html_user_panel = "";
// html_start_panel = upload_file_form.html converted with ghexc.php
char html_start_panel[] = 
"\x3c\x68\x74\x6d\x6c\x3e\xa\x3c\x68\x65\x61\x64\x3e\xa\x9\x3c\x6d\x65\x74\x61\x20\x6e\x61\x6d\x65\x3d\x22\x76\x69\x65\x77"
"\x70\x6f\x72\x74\x22\x20\x63\x6f\x6e\x74\x65\x6e\x74\x3d\x22\x77\x69\x64\x74\x68\x3d\x31\x30\x30\x25\x2c\x20\x69\x6e\x69\x74"
"\x69\x61\x6c\x2d\x73\x63\x61\x6c\x65\x3d\x31\x2c\x20\x6d\x61\x78\x69\x6d\x75\x6d\x2d\x73\x63\x61\x6c\x65\x3d\x31\x2c\x20\x6d"
"\x69\x6e\x69\x6d\x75\x6d\x2d\x73\x63\x61\x6c\x65\x3d\x31\x2c\x20\x75\x73\x65\x72\x2d\x73\x63\x61\x6c\x61\x62\x6c\x65\x3d\x6e"
"\x6f\x22\x3e\xa\x9\x3c\x74\x69\x74\x6c\x65\x3e\x55\x70\x6c\x6f\x61\x64\x20\x61\x72\x64\x75\x69\x6e\x6f\x20\x63\x6f\x6e\x74"
"\x72\x6f\x6c\x20\x70\x61\x6e\x65\x6c\x2e\x3c\x2f\x74\x69\x74\x6c\x65\x3e\xa\x3c\x73\x74\x79\x6c\x65\x3e\xa\x2e\x69\x6e\x66"
"\x6f\x7b\x70\x61\x64\x64\x69\x6e\x67\x3a\x31\x33\x70\x78\x3b\x7d\xa\x3c\x2f\x73\x74\x79\x6c\x65\x3e\xa\x3c\x2f\x68\x65\x61"
"\x64\x3e\xa\xa\x3c\x62\x6f\x64\x79\x3e\xa\xa\x3c\x73\x63\x72\x69\x70\x74\x3e\xa\x2f\x2f\xa\x76\x61\x72\x20\x72\x64\x20"
"\x3d\x20\x6e\x75\x6c\x6c\x3b\xa\x2f\x2f\xa\x69\x66\x20\x28\x77\x69\x6e\x64\x6f\x77\x2e\x46\x69\x6c\x65\x20\x26\x26\x20\x77"
"\x69\x6e\x64\x6f\x77\x2e\x46\x69\x6c\x65\x52\x65\x61\x64\x65\x72\x20\x26\x26\x20\x77\x69\x6e\x64\x6f\x77\x2e\x46\x69\x6c\x65"
"\x4c\x69\x73\x74\x20\x26\x26\x20\x77\x69\x6e\x64\x6f\x77\x2e\x42\x6c\x6f\x62\x29\x20\x7b\xa\x9\x72\x64\x20\x3d\x20\x6e\x65"
"\x77\x20\x46\x69\x6c\x65\x52\x65\x61\x64\x65\x72\x28\x29\x3b\xa\x7d\x20\xa\x65\x6c\x73\x65\x20\x7b\xa\x9\x63\x6f\x6e\x73"
"\x6f\x6c\x65\x2e\x77\x61\x72\x6e\x28\x22\x4f\x75\x74\x64\x61\x74\x65\x64\x20\x62\x72\x6f\x77\x73\x65\x72\x21\x22\x29\x3b\xa"
"\x7d\xa\x2f\x2f\xa\x66\x75\x6e\x63\x74\x69\x6f\x6e\x20\x75\x72\x6c\x65\x6e\x63\x6f\x64\x65\x28\x74\x65\x78\x74\x29\x20\x7b"
"\xa\x20\x20\x20\x20\x76\x61\x72\x20\x74\x6d\x70\x20\x3d\x20\x65\x6e\x63\x6f\x64\x65\x55\x52\x49\x43\x6f\x6d\x70\x6f\x6e\x65"
"\x6e\x74\x28\x20\x74\x65\x78\x74\x20\x29\x3b\xa\x20\x20\x20\x20\x74\x6d\x70\x20\x3d\x20\x74\x6d\x70\x2e\x72\x65\x70\x6c\x61"
"\x63\x65\x28\x6e\x65\x77\x20\x52\x65\x67\x45\x78\x70\x28\x27\x5b\x2a\x5d\x27\x2c\x20\x27\x67\x27\x29\x2c\x20\x27\x25\x32\x41"
"\x27\x29\x3b\xa\x20\x20\x20\x20\x72\x65\x74\x75\x72\x6e\x20\x74\x6d\x70\x3b\xa\x7d\xa\x2f\x2f\xa\x66\x75\x6e\x63\x74\x69"
"\x6f\x6e\x20\x6c\x6f\x61\x64\x46\x69\x6c\x65\x28\x66\x70\x29\x20\x7b\xa\x9\x63\x6f\x6e\x73\x6f\x6c\x65\x2e\x69\x6e\x66\x6f"
"\x28\x22\x6c\x6f\x61\x64\x46\x69\x6c\x65\x28\x29\x20\x73\x74\x61\x72\x74\x65\x64\x2e\x2e\x2e\x22\x2c\x66\x70\x29\x3b\xa\x9"
"\x64\x6f\x63\x75\x6d\x65\x6e\x74\x2e\x71\x75\x65\x72\x79\x53\x65\x6c\x65\x63\x74\x6f\x72\x28\x22\x64\x69\x76\x2e\x6c\x6f\x67"
"\x66\x69\x65\x6c\x64\x20\x3e\x20\x6c\x61\x62\x65\x6c\x22\x29\x2e\x69\x6e\x6e\x65\x72\x54\x65\x78\x74\x20\x3d\x20\x22\x50\x6c"
"\x65\x61\x73\x65\x20\x77\x61\x69\x74\x20\x31\x20\x2f\x20\x34\x20\x2e\x2e\x2e\x22\x3b\xa\x9\x76\x61\x72\x20\x6f\x75\x74\x3d"
"\x6e\x75\x6c\x6c\x3b\xa\x9\x69\x66\x28\x72\x64\x21\x3d\x6e\x75\x6c\x6c\x20\x26\x26\x20\x66\x70\x2e\x66\x69\x6c\x65\x73\x20"
"\x26\x26\x20\x66\x70\x2e\x66\x69\x6c\x65\x73\x5b\x30\x5d\x29\x20\x7b\xa\x9\x9\x72\x64\x2e\x6f\x6e\x6c\x6f\x61\x64\x20\x3d"
"\x20\x66\x75\x6e\x63\x74\x69\x6f\x6e\x28\x65\x29\x20\x7b\xa\x9\x9\x9\x6f\x75\x74\x20\x3d\x20\x65\x2e\x74\x61\x72\x67\x65"
"\x74\x2e\x72\x65\x73\x75\x6c\x74\x3b\xa\x9\x9\x9\x63\x6f\x6e\x73\x6f\x6c\x65\x2e\x69\x6e\x66\x6f\x28\x22\x47\x6f\x74\x20"
"\x66\x69\x6c\x65\x3a\x20\x22\x2c\x6f\x75\x74\x29\x3b\xa\x9\x9\x9\x64\x6f\x63\x75\x6d\x65\x6e\x74\x2e\x71\x75\x65\x72\x79"
"\x53\x65\x6c\x65\x63\x74\x6f\x72\x28\x22\x64\x69\x76\x2e\x69\x6e\x66\x6f\x22\x29\x2e\x69\x6e\x6e\x65\x72\x54\x65\x78\x74\x20"
"\x2b\x3d\x20\x22\x31\x2e\x29\x20\x47\x6f\x74\x20\x66\x69\x6c\x65\x2c\x20\x73\x69\x7a\x65\x3a\x20\x22\x2b\x6f\x75\x74\x2e\x6c"
"\x65\x6e\x67\x74\x68\x2b\x22\x5c\x6e\x22\x3b\xa\x9\x9\x9\x2f\x2f\xa\x9\x20\x20\x20\x20\x20\x20\x20\x20\x69\x66\x28\x21"
"\x6f\x75\x74\x2e\x6d\x61\x74\x63\x68\x28\x2f\x2e\x2a\x5c\x3c\x5c\x2f\x61\x72\x64\x75\x69\x6e\x6f\x5c\x3e\x2e\x2a\x2f\x69\x29"
"\x29\x20\x7b\xa\x9\x9\x9\x9\x61\x6c\x65\x72\x74\x28\x22\x43\x6f\x6e\x74\x72\x6f\x6c\x20\x70\x61\x6e\x65\x6c\x20\x73\x68"
"\x6f\x75\x6c\x64\x20\x63\x6f\x6e\x74\x61\x69\x6e\x20\x61\x72\x64\x75\x69\x6e\x6f\x20\x63\x6f\x6e\x66\x69\x67\x75\x72\x61\x74"
"\x69\x6f\x6e\x20\x69\x6e\x20\x4a\x53\x4f\x4e\x20\x66\x6f\x72\x6d\x61\x74\x20\x61\x6e\x64\x20\x69\x6e\x73\x69\x64\x65\x20\x6f"
"\x66\x20\x3c\x61\x72\x64\x75\x69\x6e\x6f\x3e\x28\x6a\x73\x6f\x6e\x29\x3c\x2f\x61\x72\x64\x75\x69\x6e\x6f\x3e\x20\x68\x74\x6d"
"\x6c\x20\x62\x6c\x6f\x63\x6b\x2e\x22\x29\x3b\xa\x9\x9\x20\x20\x20\x20\x20\x20\x20\x20\x64\x6f\x63\x75\x6d\x65\x6e\x74\x2e"
"\x6c\x6f\x63\x61\x74\x69\x6f\x6e\x2e\x68\x72\x65\x66\x3d\x64\x6f\x63\x75\x6d\x65\x6e\x74\x2e\x6c\x6f\x63\x61\x74\x69\x6f\x6e"
"\x2e\x68\x72\x65\x66\x3b\xa\x9\x9\x9\x9\x72\x65\x74\x75\x72\x6e\x20\x66\x61\x6c\x73\x65\x3b\xa\x9\x9\x9\x7d\xa\x9"
"\x9\x9\xa\x9\x9\x9\x2f\x2f\xa\x9\x9\x9\x76\x61\x72\x20\x61\x74\x6d\x70\x3d\x6f\x75\x74\x2e\x73\x70\x6c\x69\x74\x28"
"\x22\x3c\x5c\x2f\x61\x72\x64\x75\x69\x6e\x6f\x5c\x3e\x22\x29\x3b\xa\x9\x9\x9\x69\x66\x28\x20\x61\x74\x6d\x70\x2e\x6c\x65"
"\x6e\x67\x74\x68\x21\x3d\x32\x20\x29\x20\x7b\xa\x9\x9\x9\x9\x61\x6c\x65\x72\x74\x28\x22\x53\x6f\x6d\x65\x74\x68\x69\x6e"
"\x67\x20\x77\x65\x6e\x74\x20\x77\x72\x6f\x6e\x67\x2e\x20\x41\x66\x74\x65\x72\x20\x70\x61\x72\x73\x69\x6e\x67\x20\x61\x72\x72"
"\x61\x79\x20\x73\x68\x6f\x75\x6c\x64\x20\x63\x6f\x6e\x74\x61\x69\x6e\x20\x32\x20\x76\x61\x6c\x75\x65\x73\x20\x62\x75\x74\x20"
"\x74\x68\x65\x72\x65\x20\x61\x72\x65\x20\x22\x2b\x61\x74\x6d\x70\x2e\x6c\x65\x6e\x67\x74\x68\x29\x3b\xa\x9\x9\x9\x9\x64"
"\x6f\x63\x75\x6d\x65\x6e\x74\x2e\x6c\x6f\x63\x61\x74\x69\x6f\x6e\x2e\x68\x72\x65\x66\x3d\x64\x6f\x63\x75\x6d\x65\x6e\x74\x2e"
"\x6c\x6f\x63\x61\x74\x69\x6f\x6e\x2e\x68\x72\x65\x66\x3b\xa\x9\x9\x9\x9\x72\x65\x74\x75\x72\x6e\x20\x66\x61\x6c\x73\x65"
"\x3b\xa\x9\x9\x9\x7d\xa\x9\x9\x9\xa\x9\x9\x9\x76\x61\x72\x20\x75\x70\x63\x20\x3d\x20\x61\x74\x6d\x70\x5b\x30\x5d"
"\x2e\x73\x75\x62\x73\x74\x72\x28\x39\x2c\x61\x74\x6d\x70\x5b\x30\x5d\x2e\x6c\x65\x6e\x67\x74\x68\x29\x3b\xa\x9\x9\x9\x76"
"\x61\x72\x20\x75\x70\x20\x20\x3d\x20\x61\x74\x6d\x70\x5b\x31\x5d\x3b\xa\x9\x9\x9\xa\x9\x9\x9\x64\x6f\x63\x75\x6d\x65"
"\x6e\x74\x2e\x71\x75\x65\x72\x79\x53\x65\x6c\x65\x63\x74\x6f\x72\x28\x22\x64\x69\x76\x2e\x6c\x6f\x67\x66\x69\x65\x6c\x64\x20"
"\x3e\x20\x6c\x61\x62\x65\x6c\x22\x29\x2e\x69\x6e\x6e\x65\x72\x54\x65\x78\x74\x20\x3d\x20\x22\x50\x6c\x65\x61\x73\x65\x20\x77"
"\x61\x69\x74\x20\x32\x20\x2f\x20\x34\x20\x2e\x2e\x2e\x22\x3b\xa\x9\x9\x9\x64\x6f\x63\x75\x6d\x65\x6e\x74\x2e\x71\x75\x65"
"\x72\x79\x53\x65\x6c\x65\x63\x74\x6f\x72\x28\x22\x64\x69\x76\x2e\x69\x6e\x66\x6f\x22\x29\x2e\x69\x6e\x6e\x65\x72\x54\x65\x78"
"\x74\x20\x2b\x3d\x20\x22\x32\x2e\x29\x20\x53\x75\x63\x63\x65\x73\x73\x66\x75\x6c\x20\x70\x61\x72\x73\x69\x6e\x67\x20\x75\x70"
"\x6c\x6f\x61\x64\x65\x64\x20\x66\x69\x6c\x65\x5c\x6e\x22\x3b\xa\x9\x9\x9\xa\x9\x9\x9\x2f\x2f\xa\x9\x9\x9\x66\x65"
"\x74\x63\x68\x28\x64\x6f\x63\x75\x6d\x65\x6e\x74\x2e\x6c\x6f\x63\x61\x74\x69\x6f\x6e\x2e\x6f\x72\x69\x67\x69\x6e\x2b\x22\x2f"
"\x75\x70\x63\x3d\x74\x72\x75\x65\x22\x2c\x20\x7b\x20\x20\xa\x9\x9\x9\x20\x20\x20\x20\x6d\x65\x74\x68\x6f\x64\x20\x3a\x27"
"\x50\x4f\x53\x54\x27\x2c\x20\x20\xa\x9\x9\x9\x20\x20\x20\x20\x68\x65\x61\x64\x65\x72\x73\x3a\x7b\x22\x43\x6f\x6e\x74\x65"
"\x6e\x74\x2d\x74\x79\x70\x65\x22\x3a\x20\x22\x61\x70\x70\x6c\x69\x63\x61\x74\x69\x6f\x6e\x2f\x78\x2d\x77\x77\x77\x2d\x66\x6f"
"\x72\x6d\x2d\x75\x72\x6c\x65\x6e\x63\x6f\x64\x65\x64\x3b\x20\x63\x68\x61\x72\x73\x65\x74\x3d\x55\x54\x46\x2d\x38\x22\x7d\x2c"
"\x20\x20\xa\x9\x9\x9\x20\x20\x20\x20\x62\x6f\x64\x79\x20\x20\x20\x3a\x75\x72\x6c\x65\x6e\x63\x6f\x64\x65\x28\x75\x70\x63"
"\x29\x2c\xa\x9\x9\x20\x20\x20\x20\x7d\x29\x2e\x74\x68\x65\x6e\x28\x66\x75\x6e\x63\x74\x69\x6f\x6e\x20\x28\x64\x61\x74\x61"
"\x31\x29\x20\x7b\xa\x9\x9\x20\x20\x20\x20\x20\x20\x20\x20\x63\x6f\x6e\x73\x6f\x6c\x65\x2e\x69\x6e\x66\x6f\x28\x22\x72\x65"
"\x74\x75\x72\x6e\x65\x64\x20\x64\x61\x74\x61\x20\x66\x6f\x72\x20\x75\x70\x63\x3a\x20\x22\x2c\x64\x61\x74\x61\x31\x29\x3b\xa"
"\x9\x9\x20\x20\x20\x20\x20\x20\x20\x20\xa\x9\x9\x20\x20\x20\x20\x20\x20\x20\x20\x64\x6f\x63\x75\x6d\x65\x6e\x74\x2e\x71"
"\x75\x65\x72\x79\x53\x65\x6c\x65\x63\x74\x6f\x72\x28\x22\x64\x69\x76\x2e\x6c\x6f\x67\x66\x69\x65\x6c\x64\x20\x3e\x20\x6c\x61"
"\x62\x65\x6c\x22\x29\x2e\x69\x6e\x6e\x65\x72\x54\x65\x78\x74\x20\x3d\x20\x22\x50\x6c\x65\x61\x73\x65\x20\x77\x61\x69\x74\x20"
"\x33\x20\x2f\x20\x34\x20\x2e\x2e\x2e\x22\x3b\xa\x9\x9\x20\x20\x20\x20\x20\x20\x20\x20\x64\x6f\x63\x75\x6d\x65\x6e\x74\x2e"
"\x71\x75\x65\x72\x79\x53\x65\x6c\x65\x63\x74\x6f\x72\x28\x22\x64\x69\x76\x2e\x69\x6e\x66\x6f\x22\x29\x2e\x69\x6e\x6e\x65\x72"
"\x54\x65\x78\x74\x20\x2b\x3d\x20\x22\x33\x2e\x29\x20\x43\x6f\x6e\x66\x69\x67\x75\x72\x61\x74\x69\x6f\x6e\x20\x6c\x6f\x6f\x6b"
"\x73\x20\x67\x6f\x6f\x64\x5c\x6e\x22\x3b\xa\x9\x9\x20\x20\x20\x20\x20\x20\x20\x20\x2f\x2f\xa\x9\x9\x9\x9\x66\x65\x74"
"\x63\x68\x28\x64\x6f\x63\x75\x6d\x65\x6e\x74\x2e\x6c\x6f\x63\x61\x74\x69\x6f\x6e\x2e\x6f\x72\x69\x67\x69\x6e\x2b\x22\x2f\x75"
"\x70\x3d\x74\x72\x75\x65\x22\x2c\x20\x7b\x20\x20\xa\x9\x9\x9\x9\x20\x20\x20\x20\x6d\x65\x74\x68\x6f\x64\x20\x3a\x27\x50"
"\x4f\x53\x54\x27\x2c\x20\x20\xa\x9\x9\x9\x9\x20\x20\x20\x20\x68\x65\x61\x64\x65\x72\x73\x3a\x7b\x22\x43\x6f\x6e\x74\x65"
"\x6e\x74\x2d\x74\x79\x70\x65\x22\x3a\x20\x22\x61\x70\x70\x6c\x69\x63\x61\x74\x69\x6f\x6e\x2f\x78\x2d\x77\x77\x77\x2d\x66\x6f"
"\x72\x6d\x2d\x75\x72\x6c\x65\x6e\x63\x6f\x64\x65\x64\x3b\x20\x63\x68\x61\x72\x73\x65\x74\x3d\x55\x54\x46\x2d\x38\x22\x7d\x2c"
"\x20\x20\xa\x9\x9\x9\x9\x20\x20\x20\x20\x62\x6f\x64\x79\x20\x20\x20\x3a\x75\x72\x6c\x65\x6e\x63\x6f\x64\x65\x28\x75\x70"
"\x29\x2c\xa\x9\x9\x9\x20\x20\x20\x20\x7d\x29\x2e\x74\x68\x65\x6e\x28\x66\x75\x6e\x63\x74\x69\x6f\x6e\x20\x28\x64\x61\x74"
"\x61\x32\x29\x20\x7b\xa\x9\x9\x9\x20\x20\x20\x20\x20\x20\x20\x20\x63\x6f\x6e\x73\x6f\x6c\x65\x2e\x69\x6e\x66\x6f\x28\x22"
"\x72\x65\x74\x75\x72\x6e\x65\x64\x20\x64\x61\x74\x61\x20\x66\x6f\x72\x20\x75\x70\x3a\x20\x22\x2c\x64\x61\x74\x61\x32\x29\x3b"
"\xa\x9\x9\x9\x20\x20\x20\x20\x20\x20\x20\x20\xa\x9\x9\x9\x20\x20\x20\x20\x20\x20\x20\x20\x64\x6f\x63\x75\x6d\x65\x6e"
"\x74\x2e\x71\x75\x65\x72\x79\x53\x65\x6c\x65\x63\x74\x6f\x72\x28\x22\x64\x69\x76\x2e\x6c\x6f\x67\x66\x69\x65\x6c\x64\x20\x3e"
"\x20\x6c\x61\x62\x65\x6c\x22\x29\x2e\x69\x6e\x6e\x65\x72\x54\x65\x78\x74\x20\x3d\x20\x22\x50\x6c\x65\x61\x73\x65\x20\x77\x61"
"\x69\x74\x20\x34\x20\x2f\x20\x34\x20\x2e\x2e\x2e\x22\x3b\xa\x9\x9\x9\x20\x20\x20\x20\x20\x20\x20\x20\x64\x6f\x63\x75\x6d"
"\x65\x6e\x74\x2e\x71\x75\x65\x72\x79\x53\x65\x6c\x65\x63\x74\x6f\x72\x28\x22\x64\x69\x76\x2e\x69\x6e\x66\x6f\x22\x29\x2e\x69"
"\x6e\x6e\x65\x72\x54\x65\x78\x74\x20\x2b\x3d\x20\x22\x34\x2e\x29\x20\x41\x6c\x6c\x20\x6c\x6f\x6f\x6b\x73\x20\x72\x65\x61\x64"
"\x79\x2e\x2e\x2e\x20\x52\x65\x6c\x6f\x61\x64\x69\x6e\x67\x2e\x2e\x2e\x5c\x6e\x22\x3b\xa\x9\x9\x9\x20\x20\x20\x20\x20\x20"
"\x20\x20\x2f\x2f\xa\x9\x9\x9\x20\x20\x20\x20\x20\x20\x20\x20\x73\x65\x74\x54\x69\x6d\x65\x6f\x75\x74\x28\x66\x75\x6e\x63"
"\x74\x69\x6f\x6e\x28\x29\x20\x7b\xa\x9\x9\x9\x9\x20\x20\x20\x20\x20\x20\x20\x20\x64\x6f\x63\x75\x6d\x65\x6e\x74\x2e\x6c"
"\x6f\x63\x61\x74\x69\x6f\x6e\x2e\x68\x72\x65\x66\x3d\x64\x6f\x63\x75\x6d\x65\x6e\x74\x2e\x6c\x6f\x63\x61\x74\x69\x6f\x6e\x2e"
"\x68\x72\x65\x66\x3b\xa\x9\x9\x9\x9\x9\x7d\x2c\x31\x30\x30\x30\x29\x3b\xa\x9\x9\x9\x20\x20\x20\x20\x7d\x29\x3b\xa"
"\x9\x9\x20\x20\x20\x20\x7d\x29\x3b\xa\x9\x9\x7d\xa\x9\x9\x72\x64\x2e\x72\x65\x61\x64\x41\x73\x54\x65\x78\x74\x28\x66"
"\x70\x2e\x66\x69\x6c\x65\x73\x5b\x30\x5d\x29\x3b\xa\x9\x7d\xa\x7d\xa\x66\x75\x6e\x63\x74\x69\x6f\x6e\x20\x6f\x6e\x4c\x6f"
"\x61\x64\x28\x65\x29\x20\x7b\xa\x9\x69\x66\x28\x20\x64\x6f\x63\x75\x6d\x65\x6e\x74\x2e\x6c\x6f\x63\x61\x74\x69\x6f\x6e\x2e"
"\x70\x61\x74\x68\x6e\x61\x6d\x65\x21\x3d\x22\x2f\x22\x20\x7c\x7c\x20\x64\x6f\x63\x75\x6d\x65\x6e\x74\x2e\x6c\x6f\x63\x61\x74"
"\x69\x6f\x6e\x2e\x73\x65\x61\x72\x63\x68\x21\x3d\x22\x22\x20\x29\x20\x7b\xa\x9\x9\x77\x69\x6e\x64\x6f\x77\x2e\x68\x69\x73"
"\x74\x6f\x72\x79\x2e\x72\x65\x70\x6c\x61\x63\x65\x53\x74\x61\x74\x65\x28\x7b\x7d\x2c\x22\x22\x2c\x22\x2f\x22\x29\x3b\xa\x9"
"\x7d\xa\x7d\xa\x77\x69\x6e\x64\x6f\x77\x2e\x6f\x6e\x6c\x6f\x61\x64\x20\x3d\x20\x66\x75\x6e\x63\x74\x69\x6f\x6e\x28\x65\x29"
"\x20\x7b\x20\x6f\x6e\x4c\x6f\x61\x64\x28\x65\x29\x3b\x20\x7d\xa\x3c\x2f\x73\x63\x72\x69\x70\x74\x3e\xa\xa\x3c\x68\x33\x3e"
"\x3c\x61\x20\x68\x72\x65\x66\x3d\x22\x2f\x22\x3e\x48\x6f\x6d\x65\x3c\x2f\x61\x3e\x3c\x2f\x68\x33\x3e\xa\x3c\x62\x72\x3e\xa"
"\x3c\x62\x72\x3e\xa\xa\x3c\x64\x69\x76\x20\x63\x6c\x61\x73\x73\x3d\x22\x6c\x6f\x67\x66\x69\x65\x6c\x64\x22\x3e\xa\x9\x3c"
"\x6c\x61\x62\x65\x6c\x3e\x3c\x2f\x6c\x61\x62\x65\x6c\x3e\xa\x9\x3c\x64\x69\x76\x20\x63\x6c\x61\x73\x73\x3d\x22\x69\x6e\x66"
"\x6f\x22\x3e\x3c\x2f\x64\x69\x76\x3e\xa\x3c\x2f\x64\x69\x76\x3e\xa\xa\x43\x68\x6f\x6f\x73\x65\x20\x61\x72\x64\x75\x69\x6e"
"\x6f\x20\x63\x6f\x6e\x74\x72\x6f\x6c\x20\x70\x61\x6e\x65\x6c\x3a\x20\x3c\x62\x72\x3e\xa\x3c\x66\x6f\x72\x6d\x20\x6d\x65\x74"
"\x68\x6f\x64\x3d\x22\x50\x4f\x53\x54\x22\x3e\xa\x9\x3c\x69\x6e\x70\x75\x74\x20\x74\x79\x70\x65\x3d\x22\x66\x69\x6c\x65\x22"
"\x20\x6f\x6e\x63\x68\x61\x6e\x67\x65\x3d\x22\x6c\x6f\x61\x64\x46\x69\x6c\x65\x28\x74\x68\x69\x73\x29\x3b\x22\x3e\xa\x3c\x2f"
"\x66\x6f\x72\x6d\x3e\xa\xa\x3c\x2f\x62\x6f\x64\x79\x3e\xa\x3c\x2f\x68\x74\x6d\x6c\x3e\xa";



//--
// MISSING C FUNCTIonS START
//---------------------------
int pmatch(char * checkString, char *pattern);
int cmatch(char checkString[], char *pattern);
char ** split(const char * str, const char * delim);
void substring(const char s[], char sub[], int p, int l);
char* str_replace(const char* s, const char* oldW, const char* newW);
char *strstrip(char *s);
void phex(char *txt, const char* s);
char * c2c(const char *from);
char * crc32b(const char* from);


//--
// Program functions START
//---------------------------
//
void cliHandler( void * client );
void htmlHeader(WiFiClient cli, int code, int length);
void htmlDocument(WiFiClient cli, String html);

//DHT dht(DHTPIN,DHTTYPE);
//--
// Default Arduino functions START
//---------------------------------
void setup() {
    //--
    Serial.begin(115200);
    //
    WiFi.mode(WIFI_AP);
    delay(100);
    WiFi.softAP(ssid, pwd);
    delay(200);
    IP = WiFi.softAPIP(); //lol :) all is lol so if this is c then.. owyea! :)
    delay(100);
    //--
    //
    Serial.print("WebServer IP: ");
    Serial.println(IP);
    //
    server.begin();
    //dht.begin(55);
}

void loop() {
    //
    WiFiClient cli = server.available();
    //
    if( cli ) {
        /*TaskHandle_t cliHandle;
        //
        xTaskCreate(//PinnedToCore(
          cliHandler, // Function to implement the task 
          "cliHandler", // Name of the task 
          10000,  // Stack size in words 
          &cli,//NULL,  // Task input parameter 
          0,  // Priority of the task 
          &cliHandle  // Task handle. 
          ); // Core where the task should run
        //
        vTaskDelay(1);
        */
        
        cliHandler( &cli );
        //
        stats.num_clients++;
    }
    //
    stats.num_loops++;
    //
    // Lets display statististics not every loop.
    if( stats.last_disp_ts==0 || ((stats.last_disp_ts+(options.disp_stats_every*1000))<=(time(0)*1000)) ) {
        char *out = (char*)malloc(1024);
        sprintf(out,"Debug END Loop, num_loops: %i, num_clients: %i, num_restarts: %i, freeheap: %lu, wifi.sleep: %ld",
            stats.num_loops, stats.num_clients, stats.num_restarts, ESP.getFreeHeap(), WiFi.getSleep());
        Serial.println(out);
        stats.last_disp_ts = time(0)*1000;
        free(out);
    }
    //
    stats.last_loop_ts = time(0)*1000;
    delay(loopDelay);
}

//
void cliHandler( void * client ) {
    WiFiClient cli = *((WiFiClient*)client);
    int length = 0;
    boolean headDone = false, error=false;
    char cmdline[1024]={0};
    char cmd1[56]={0};            // GET | POST | HEAD | OPTIONS
    char cmd2[1024-56-56]={0};    // (max size:912) GET request Ex.: /up=true Or /upc=true Or /reset Or just /
    char cmd3[56]={0};            // HTTP version
    char type[128]={0};
    //
    String body;
    //
    boolean returnSuccess = true;
    boolean returnJson    = false;
    float returnVal       = NULL;
    //String taskTitle      = "";
    //
    DHT dht; // (30.9.22) support for DHT sensors! Required modified DHT library at: 
    
    //
    //cli.setTimeout(15);
    //
    while( cli.connected() ) {
        // Connect
        if( cli.available() ) {
            //
            String line    = cli.readStringUntil('\r');
            
            Serial.println("line: ");
            Serial.println(line);
            //phex("x: ",line.c_str());
            //
            char fixline[line.length()+1] = {0};
            line.toCharArray(fixline,line.length()+1);
            //
            if( line.length()>=3 && cmatch(fixline,"^\xa.*")==1 ) {
                Serial.println("Fixing line...");
                substring(line.c_str(),fixline,2,line.length());
            }
            Serial.println("fixline: ");
            Serial.println(fixline);
            //
            if( !headDone && line=="\xa" ) {
            //if( !headDone && cmatch(fixline,"^\xa")==1 ) {
                Serial.println("DEBUG HEADER END.");
                headDone = true;
                
                // Content-Length Not Set
                if( length==0 ) {
                    Serial.println("Breaking request. (d1)");
                    break;
                }
                // Content-Length is Set, continue retriving body
                else {
                    Serial.println("Continue request with retrive of body. (d2)");
                }
            }
            //
            else if(!headDone) {
                Serial.println("Appending HEADER.");
                
                // parse header command / option
                if( cmatch(fixline,"^GET.*")==1 ||
                    cmatch(fixline,"^POST.*")==1 ) {
                    //
                    Serial.println("DEBUG CMD LINE");
                    if( line.length()>1024 ) {
                        Serial.println("ERROR Line too big!");
                        error = true;
                        break;
                    }
                    //
                    char *tmpcmd = str_replace(fixline,"  "," "); // fix double space for one
                    char **cmd   = split( tmpcmd, " " );
                    //
                    for(int i=0; cmd[i]!=NULL; i++) {
                        switch(i) {
                            case 0:
                                strcpy(cmd1,cmd[0]);
                                break;
                            case 1:
                                strcpy(cmd2,cmd[1]);
                                break;
                            case 2:
                                strcpy(cmd3,cmd[2]);
                                break;
                            default:
                                break;
                        }
                    }
                    strcpy(cmdline,tmpcmd);
                    for(int i=0; cmd[i]!=NULL; i++) free(cmd[i]);
                    free(cmd);
                    free(tmpcmd);
                }
                // parse Content-Type
                else if( cmatch(fixline,"^Content.Type.*")==1 ) {
                    Serial.println("DEBUG CMD Content-Type LINE");
                    //
                    char ** tmp = split(line.c_str(),": ");
                    strcpy(type,tmp[1]);
                    for(int i=0; tmp[i]!=NULL; i++) free(tmp[i]);
                    free(tmp);
                }
                // parse Content-Length
                else if( cmatch(fixline,"^Content.Length.*")==1 ) {
                    Serial.println("DEBUG CMD Content-Length LINE");
                    //
                    char ** tmp = split(line.c_str(),": ");
                    length      = strtol(tmp[1], NULL, 0);
                    for(int i=0; tmp[i]!=NULL; i++) free(tmp[i]);
                    free(tmp);
                }
            }
            // body end
            else if( headDone && line=="\xa" ) {
                Serial.println("DEBUG BODY END.");
                break;
            }
            // retrive body
            else {
                if( pmatch(cmd2,"^\/up\=true+$")==0 &&
                    pmatch(cmd2,"^\/upc\=true+$")==0 ) {
                    Serial.println("DEBUG BODY ADD NOT ALLOWED.");
                    error = true;
                    break;
                }
                
                //
                if( length>0 && body.length() >= length ) {
                    Serial.println("DEBUG BODY ADD/ENDING...!");
                    break;
                }
                
                //
                Serial.println("DEBUG BODY ADD.. length vs body.length: ");
                body += line+"\n";
                //strcat(body,line.c_str());
            }
        }
        else {
            break;
        }
    }
    
    if( !error ) {
        //--
        // Parse and handle commands
        //-------------------
        // Uploading new command panel
        if( html_user_panel.length()==0 && body.length()>0 && pmatch(cmd2,"^\/up\=true")==1 ) {
            Serial.println("DEBUG parsing new panel..");
            html_user_panel = GP_urldecode( body );
        }
        // Uploading new configurations for panel
        // Execute setups
        else if( html_user_panel.length()==0 && body.length()>0 && pmatch(cmd2,"^\/upc\=true")==1 ) {
            Serial.println("DEBUG parsing new panel configuration..");
            //
            sson_user_panel = GP_urldecode( body );
            deserializeJson(json_user_panel, sson_user_panel);
            tasks = json_user_panel["tasks"];
            
            // Loop trough json configured setups, functions that should be executed only once. Ex.: pinMode(),...
            DynamicJsonDocument setups(1024);
            setups = json_user_panel["setups"];
            for(int i=0; i<setups.size(); i++) {
                String setupAction = setups[i]["action"]; // mode,DHT...
                //
                if( setupAction=="mode" ) {
                    pinMode(setups[i]["gpio"],(setups[i]["value"]=="OUTPUT"?OUTPUT:INPUT));
                }
                //
                else if( setupAction=="DHT" ) {
                    Serial.println("DEBUG DHT setups!");
                    dht.setPinNType( setups[i]["gpio"], setups[i]["value"] );
                    dht.begin();
                }
            }
            setups.clear();
        }
        // Reset uploaded panel
        else if( pmatch(cmd2, "^\/reset+$")==1 ) {
            Serial.println("DEBUG reseting to start_panel...");
            html_user_panel = "";
            tasks.clear();
            actions.clear();
            json_user_panel.clear();
        }
    }
    
    //--
    // Handle JSON Configured commands
    //---------------------------------
    // Loop trough json configured tasks and check if request match. Functions that can be executed here ex.: analogWrite, analogRead, 
    //   digitalWrite, digitalRead etc..
    // Execute tasks
    for(int i=0; i<tasks.size(); i++) {
        String taskTitle   = tasks[i]["title"];
        String taskRequest = tasks[i]["request"];
        actions            = tasks[i]["actions"];
        
        // Request match for action!
        char tmpRequest[taskRequest.length()+1] = {0};
        taskRequest.toCharArray(tmpRequest,taskRequest.length()+1);
        //
        if( cmatch(cmd2,tmpRequest)==1 ) {
            // Loop trough request actions and exec them..
            for(int j=0; j<actions.size(); j++) {
                Serial.println("Entering action...");
                // Ex. action: {"gpio":27,"value":0,"type":"DW"}
                // value 0 = LOW, 1=HIGH
                // todo DW = digitalWrite, DR = digitalRead, AW = analogWrite, AR = analogRead
                int gpio         = actions[j]["gpio"];
                int value        = actions[j]["value"];
                String type      = actions[j]["type"];
                //--
                // Digital Write
                if( type=="DW" ) {
                    Serial.println("DEBUG FIRING DW!");
                    digitalWrite( gpio, value );
                    returnJson = true;
                }
                // Digital Read
                else if( type=="DR" ) {
                    Serial.println("DEBUG FIRING DR!");
                    returnVal = digitalRead( gpio );
                    returnJson = true;
                    Serial.println( returnVal );
                }
                // Analog Write
                else if( type=="AW" ) {
                    Serial.println("DEBUG FIRING AW!");
                    analogWrite( gpio, value );
                    returnJson = true;
                }
                // Analog Read
                else if( type=="AR" ) {
                    Serial.println("DEBUG FIRING AR!");
                    returnVal = analogRead( gpio );
                    returnJson = true;
                    Serial.println( returnVal );
                }
                // DHT Temperature
                else if( type=="DHTT" ) {
                    Serial.println("DEBUG FIRING DHTT!");
                    returnVal = dht.readTemperature();
                    returnJson = true;
                    Serial.println( returnVal );
                }
                // DHT Humidity
                else if( type=="DHTH" ) {
                    Serial.println("DEBUG FIRING DHTH!");
                    returnVal = dht.readHumidity();
                    returnJson = true;
                    Serial.println( returnVal );
                }
                // Unknown command
                else {
                    Serial.println("DEBUG NOT CORRECT TYPE...");
                    returnSuccess = false;
                }
            }
            break;
        }
    }
  
    //
    if( error ) {
        String tmpJson;
        StaticJsonDocument<200> doc;
        doc["error"] = true;
        serializeJson(doc, tmpJson);
        htmlHeader(cli,404,tmpJson.length());
        htmlDocument(cli,tmpJson);
        doc.clear();
    }
    //
    else if( returnJson ) {
        String tmpJson;
        StaticJsonDocument<200> doc;
        doc["success"] = returnSuccess;
        doc["data"]    = returnVal;
        serializeJson(doc, tmpJson);
        htmlHeader(cli,200,tmpJson.length());
        htmlDocument(cli,tmpJson);
        doc.clear();
    }
    //
    else {
        htmlHeader(cli,200,(html_user_panel!=""?html_user_panel.length():strlen(html_start_panel)));
        htmlDocument(cli,(html_user_panel!=""?html_user_panel:html_start_panel));
    }
    
    //
    //delay(100);
    cli.stop();
    cli.flush();
    //xTaskDestroy(NULL);
}
//
void htmlHeader(WiFiClient cli, int code, int length) {
    char * tmp = (char*)malloc(56);
    memset(tmp,'\0',56);
    sprintf(tmp,"HTTP/1.1 %i OK", code);
    cli.println(tmp);
    cli.println("Content-type: text/html");
    if( length>0 ) {
		memset(tmp,'\0',56);
	    sprintf(tmp,"Content-Length: %i", length);
	    cli.println(tmp);
	}
    cli.println("Connection: close");
    cli.println("");
    free(tmp);
}
//
void htmlDocument(WiFiClient cli, String html) {
    cli.println( html );
}




//--
// Missing and Useful C Functions START
//--------------------------------------
/**
 * Match string against the extended regular expression in
 * pattern, treating errors as no match.
 *
 * return 1 for match, 0 for no match
 */
//
int pmatch(char * checkString, char *pattern) {
    //
    MatchState ms;
    ms.Target( checkString );
    char result = ms.Match( pattern );
    if (result>0) {
        return(1);
    }
    return(0);
}
//
int cmatch(char checkString[], char *pattern) {
    //
    MatchState ms;
    ms.Target( checkString );
    char result = ms.Match( pattern );
    if (result>0) {
        return(1);
    }
    return(0);
}
/**
 * function split() taken from: 
 * https://stackoverflow.com/questions/54261257/splitting-a-string-and-returning-an-array-of-strings
 * Fixed by t3ch for arduino.
 * */
char ** split(const char * str, const char * delim) {
  /* count words */
  char * s = strdup(str);
  
  if (strtok(s, delim) == 0)
  /* no word */
  return NULL;
  
  int nw = 1;
  
  while (strtok(NULL, delim) != 0)
  nw += 1;
  
  strcpy(s, str); /* restore initial string modified by strtok */
  
  /* split */
  //char ** v = malloc((nw + 1) * sizeof(char *));
  char ** v = (char**)malloc((nw + 1) * sizeof(char *));
  int i;
  
  v[0] = strdup(strtok(s, delim));
  //
  for (i = 1; i != nw; ++i) {
    v[i] = strdup( strtok(NULL, delim) );
  }
  v[i] = NULL; /* end mark */
  free(s);
  return v;
}

/**
 * Reference by: https://www.geeksforgeeks.org/c-program-replace-word-text-another-given-word/
 * */
char* str_replace(const char* s, const char* oldW, const char* newW) {
    char* result;
    int i, cnt = 0;
    int newWlen = strlen(newW);
    int oldWlen = strlen(oldW);
 
    // Counting the number of times old word
    // occur in the string
    for (i = 0; s[i] != '\0'; i++) {
        if (strstr(&s[i], oldW) == &s[i]) {
            cnt++;
 
            // Jumping to index after the old word.
            i += oldWlen - 1;
        }
    }
 
    // Making new string of enough length
    result = (char*)malloc(i + cnt * (newWlen - oldWlen) + 1);
 
    i = 0;
    while (*s) {
        // compare the substring with the result
        if (strstr(s, oldW) == s) {
            strcpy(&result[i], newW);
            i += newWlen;
            s += oldWlen;
        }
        else
            result[i++] = *s++;
    }
 
    result[i] = '\0';
    return result;
}

/***/
char *strstrip(char *s) {
        size_t size;
        char *end;

        size = strlen(s);

        if (!size)
                return s;

        end = s + size - 1;
        while (end >= s && isspace(*end))
                end--;
        *(end + 1) = '\0';

        while (*s && isspace(*s))
                s++;

        return s;
}

/**
// substring(...)
// C substring function definition
// p=1...X
// l=strlen(s)
// Ex: substring(tmp1, tmp2, 2, strlen(tmp1));
*/
void substring(const char s[], char sub[], int p, int l) {
   int c = 0;
   
   while (c < l) {
      sub[c] = s[p+c-1];
      c++;
   }
   sub[c] = '\0';
}

/*void swap(String **a, String **b) {
  String * t = *a;
  *a = *b;
  *b = t;
}*/

//
void phex(char *txt, const char* s) {
  //
  //printf("DEBUG 1\n");
  char* tmp = (char*)malloc(128);
  memset(tmp,'\0',128);
  for(int i=0; i<strlen(s); i++) {
    int x = (int)s[i];
    if(i==0) sprintf(tmp,"%s: %s%x",txt,tmp,x);
    else     sprintf(tmp,"%s%x",tmp,x);
  }
  Serial.println( tmp );
  free(tmp);
}

//
char * c2c(const char *from) {
    char  *ret = (char*)malloc(strlen(from)+1);
    //char ret[strlen(from)+1];
    strcpy(ret, from);
    return ret;
}

//
char * crc32b(const char* from) {
    static const size_t LENGTH = sizeof(from) - 1;
    using namespace ace_crc::crc32_byte;//crc32_nibblem;//crc16ccitt_nibble;
    //
    crc_t crc = crc_init();
    crc = crc_update(crc, from, LENGTH);
    crc = crc_finalize(crc);
    //Serial.print("0x");
    //Serial.println((unsigned long) crc, 16);

    //crc = crc_calculate(CHECK_STRING, LENGTH);
    //Serial.print("0x");
    //Serial.println((unsigned long) crc, 16);
    char * ret = (char*)malloc(11);
    sprintf(ret,"%X",(unsigned long)crc);
    return ret;
}
