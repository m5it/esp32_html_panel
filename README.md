Arduino http control panel. ( In developement but have it logic..:) )

# How it work:

1.) copy/paste "http_panel.ino" into your ArduinoIDE compiler.<br>
2.) upload<br>
3.) navigate with browser to 192.168.4.1

Control panel should be html file with some additions. Addition is new HTML Block "\<arduino>\</arduino>". Inside of block we should define controller controls for ex.:
\<arduino>
{"setups":[{"gpio":27,"action":"mode", "value":"OUTPUT"},{"gpio":14,"action":"mode", "value":"OUTPUT"},{"gpio":13,"action":"mode", "value":"OUTPUT"}], "tasks":[{"title":"Start motor A","request":"/MOTOR_A_ON", "actions":[{"gpio":27,"value":0,"type":"DW"},{"gpio":14,"value":1,"type":"DW"},{"gpio":13,"value":100,"type":"AW"}]},{"title":"Stop motor A","request":"/MOTOR_A_OFF", "actions":[{"gpio":27,"value":0,"type":"DW"},{"gpio":14,"value":0,"type":"DW"}]}]}
\</arduino>

4.) choose your control panel


# Options
- http://192.168.4.1/reset      # To reset to startup panel.
- (Other options are used trough html panel "upload_file_form.html" or if user create his panel with his configurations trough JSON.)


# File definitions:
 - http_panel.ino    ( Arduino/ESP32 code that runs server and create wifi station )
 - ghexc.php         ( Script that is used to convert html file into C Programming string arrays so it can be included into c code Ex.: char yourwar[]="\x1\x2\x3"; )
 - upload_file_form.php is converted with ghexc.php and used with variable "char html_start_panel[] = ..."
- remote_car_panel.html ( Example of panel that can be configuret with json+html and used with controller )


Thanks for watching.*
