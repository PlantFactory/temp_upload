# temp_upload

This is a sample program of IEEE1888.
This sample program will upload the temperature every minute.

##Equipment Requirements

1. Arduino (AVR-based or ESP8266-based)
2. Ethernet Shield (if you use AVR-based Arduino)
3. Temperature sensor ADT7410(or ADT7420)

##Electrical Connection

    Arduino : ADT7410(or ADT7420)
    VCC <-> VDD
    GND <-> GND
    SCL <-> SCL
    SDA <-> SDA

## Dependent Libraries

- PFFIAPUploadAgent https://github.com/PlantFactory/PFFIAPUploadAgent
- SerialCLI https://github.com/PlantFactory/SerialCLI
- ADT74x0 https://github.com/PlantFactory/ADT74x0
- NTP https://github.com/PlantFactory/NTP
- Time https://github.com/PaulStoffregen/Time

## How To Use

This program has a command line interface via serial.

### Configuration Fields

    Network: DHCP, IP, GW, SM, DNS, NTP
    AVR-based Arduino: MAC
    ESP8266-based Arduino: SSID, PASS
    IEEE1888: HOST, PORT, PATH, PREFIX, TIMEZONE

### Configuration Sample

    temp_upload Ver.0.02
    SerialCLI Ver.0.02
    current setting
        SSID=samplessid
        PASS=samplepass
        DHCP=true
        ...
    setting description (and default value)
        SSID... wifi ssid(default TAISYO-FREE-WIFI)
        PASS... wifi password(default PASSWORDPASSWORD)
        DHCP... DHCP enable/disable(default true)
        ..
    commands
        help, show, conf, exit, load [default], save,
        debug, nodebug,
    >
    Failed to configure WiFi
    >conf
    enter conf mode
    #load default
    loading(default)...done.
    #show
        SSID=TAISYO-FREE-WIFI
        PASS=PASSWORDPASSWORD
        DHCP=true
        ...
    #SSID=samplessid
    #PASS=samplepass
    #save defaul
    saving...done.
    #reboot

    temp_upload Ver.0.02
    SerialCLI Ver.0.02
    ...
    >debug
    [2016/08/07 06:01:58]26.64
    [2016/08/07 06:01:59]26.64
    [2016/08/07 06:02:00]26.64
    [2016/08/07 06:02:00]uploading...
    [2016/08/07 06:02:00]done
    [2016/08/07 06:02:01]26.64
    [2016/08/07 06:02:01]26.64
    [2016/08/07 06:02:01]26.64
    >nodebug
    >
