ESP32 http control panel. ( In developement but have it logic..:) )

(From 29.9.22 no more memory problems!!! Uiii)


# How it work:

1.) copy/paste "http_panel.ino" into your ArduinoIDE compiler.

2.) upload

3.) navigate with browser to 192.168.4.1

4.) choose your control panel

Your control panel should be html file with some additions. Addition is new HTML Block "\<arduino>\</arduino>". Inside of block we define controller configuration in JSON format. For ex.:

\<arduino>

{"setups":[{"gpio":26,"action":"DHT", "value":11},{"gpio":27,"action":"mode", "value":"OUTPUT"},{"gpio":14,"action":"mode", "value":"OUTPUT"},{"gpio":13,"action":"mode", "value":"OUTPUT"}], "tasks":[{"title":"Start motor A","request":"/MOTOR_A_ON", "actions":[{"gpio":27,"value":0,"type":"DW"},{"gpio":14,"value":1,"type":"DW"},{"gpio":13,"value":130,"type":"AW"}]},{"title":"Stop motor A","request":"/MOTOR_A_OFF", "actions":[{"gpio":27,"value":0,"type":"DW"},{"gpio":14,"value":0,"type":"DW"}]},{"title":"Check battery level","request":"/BATTERY_LEVEL", "actions":[{"gpio":34,"value":0,"type":"AR"}]},{"title":"Check temperature with DHT11","request":"/DHTT", "actions":[{"gpio":0,"value":0,"type":"DHTT"}]},{"title":"Check humidity with DHT11","request":"/DHTH", "actions":[{"gpio":0,"value":0,"type":"DHTH"}]}]}


\</arduino>



# Controller JSON configuration definitions & tree:
- setups   ( actions defined here are executed only once, when panel is installed. Here should exec functions like: pinMode(xx,OUTPUT) etc..
    - gpio
    - action
    - value
- tasks    ( these actions are checked and executed when happen specific request. to start/stop motor, light etc. )
    - title
    - request
    - actions
        - gpio
        - value
        - type

# Options
- http://192.168.4.1/reset      # To reset to startup panel.
- (Other options are used trough html panel "upload_file_form.html" or if user create his panel with his configurations trough JSON.)

# Supported actions trough JSON and WEB
- analogRead(), analogWrite()
- digitalRead(), digitalWrite()
- executing multiple actions to run motor driver L298 and similar.
- reading data for DHT11, DHT22 and DHT21 sensors. (Required modified DHT library that can be found <a href="https://github.com/m5it/DHT_sensor_library_modified">here</a>)
- (30.9.22) added support to upload user panel bigger than is defined stock size.

# File definitions:
 - http_panel.ino    ( Arduino/ESP32 code that runs json configurable web server and currently support wifi station only. )
 - ghexc.php         ( Script that is used to convert html file into C Programming string arrays so it can be included into c code Ex.: char yourwar[]="\x1\x2\x3"; )
 - upload_file_form.php ( is converted with ghexc.php and used with variable "char html_start_panel[] = ..." )
- remote_car_panel.html ( Example of panel that can be configuret with json+html and used with controller )
- testlogserv.sh        ( in loop trigger multiple requests to the server to test for memory problems )


Thanks for watching.*


# Screenshots

![alt text](https://github.com/m5it/http_panel/blob/main/screen2_v0.1.png)

![alt text](https://github.com/m5it/http_panel/blob/main/screen4_v0.02.png)
