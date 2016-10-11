/*
 * sketch.ino
 *
 * Author:   Hiromasa Ihara (taisyo)
 * Created:  2016-02-18
 */

// Arduino library
#ifndef ESP8266
#include <Ethernet.h>
#else
#include <ESP8266WiFi.h>
#endif

#include <Wire.h>

// third party library
#include <PFFIAPUploadAgent.h>
#include <TimeLib.h>
#include <SerialCLI.h>
#include <ADT74x0.h>
#include <NTP.h>

// C library
#include <math.h>

//cli
SerialCLI commandline(Serial);
//  ethernet or WiFi
#ifndef ESP8266
MacEntry mac("MAC", "12:34:56:78:9A:BC", "mac address");
#else
StringEntry ssid("SSID", "TAISYO-FREE-WIFI", "wifi ssid");
StringEntry pass("PASS", "PASSWORDPASSWORD", "wifi password");
#endif
//  ip
BoolEntry dhcp("DHCP", "true", "DHCP enable/disable");
IPAddressEntry ip("IP", "192.168.0.2", "IP address");
IPAddressEntry gw("GW", "192.168.0.1", "default gateway IP address");
IPAddressEntry sm("SM", "255.255.255.0", "subnet mask");
//  dns
IPAddressEntry dns_server("DNS", "8.8.8.8", "dns server");
//  ntp
StringEntry ntp("NTP", "ntp.nict.jp", "ntp server");
//  fiap
StringEntry host("HOST", "fiap-dev.gutp.ic.i.u-tokyo.ac.jp", "host of ieee1888 server end point");
IntegerEntry port("PORT", "80", "port of ieee1888 server end point");
StringEntry path("PATH", "/axis2/services/FIAPStorage", "path of ieee1888 server end point");
StringEntry prefix("PREFIX", "http://taisyo.hongo.wide.ad.jp/MyHome/Node1/", "prefix of point id");
StringEntry timezone("TIMEZONE", "+09:00", "timezone");
//  debug
int debug = 0;

//ntp
NTPClient ntpclient;

//fiap
FIAPUploadAgent fiap_upload_agent;
char timezone_str[7];
char temperature_str[16];
struct fiap_element fiap_elements [] = {
  { "Temperature", temperature_str, 0, 0, 0, 0, 0, 0, timezone_str, },
};

//sensor
ADT74x0 tempsensor;

void enable_debug()
{
  debug = 1;
}

void disable_debug()
{
  debug = 0;
}

void setup()
{
#ifdef ESP8266
  ESP.wdtDisable();
#endif

  Serial.begin(9600);
  int ret;

#ifndef ESP8266
  //Ethernet
  commandline.add_entry(&mac);
#else
  //Wifi
  commandline.add_entry(&ssid);
  commandline.add_entry(&pass);
#endif

  commandline.add_entry(&dhcp);
  commandline.add_entry(&ip);
  commandline.add_entry(&gw);
  commandline.add_entry(&sm);
  commandline.add_entry(&dns_server);

  commandline.add_entry(&ntp);

  commandline.add_entry(&host);
  commandline.add_entry(&port);
  commandline.add_entry(&path);
  commandline.add_entry(&prefix);
  commandline.add_entry(&timezone);

  commandline.add_command("debug", enable_debug);
  commandline.add_command("nodebug", disable_debug);

  commandline.begin(9600, "temp_upload ver.0.0.2");

#ifndef ESP8266
  // ethernet & ip connection
  if(dhcp.get_val() == 1){
    ret = Ethernet.begin(mac.get_val());
    if(ret == 0) {
      restart("Failed to configure Ethernet using DHCP", 10);
    }
  }else{
    Ethernet.begin(mac.get_val(), ip.get_val(), dns_server.get_val(), gw.get_val(), sm.get_val());
  }
#else
  // wifi & ip connection
  WiFi.begin(ssid.get_val().c_str(), pass.get_val().c_str());
  unsigned long start = millis();
  while(1){
    ESP.wdtFeed();
    if(WiFi.status() == WL_CONNECTED) {
      break;
    }
    if(10*1000 < millis()-start){
      restart("Failed to configure WiFi", 10);
    }
    delay(100);
  }
  if(!dhcp.get_val()){
    WiFi.config(ip.get_val(), dns_server.get_val(), gw.get_val(), sm.get_val());
  }
#endif

  // fetch time
  uint32_t unix_time;
  ntpclient.begin();
  ret = ntpclient.getTime(ntp.get_val(), &unix_time);
  if(ret < 0){
    restart("Failed to configure time using NTP", 10);
  }
  setTime(unix_time + (9 * 60 * 60));

  // fiap
  fiap_upload_agent.begin(host.get_val(), path.get_val(), port.get_val(), prefix.get_val());

  // sensor
  Wire.begin();
  tempsensor.begin(0x48);

#ifdef ESP8266
  ESP.wdtEnable(2000);
#endif
}

void loop()
{
  static unsigned long old_epoch = 0, epoch;
  float temp;
  char buf[32];

  commandline.process();
  epoch = now();
#ifndef ESP8266
  if(dhcp.get_val() == 1){
    Ethernet.maintain();
  }
#endif

  if(epoch != old_epoch){
    // 計測
    temp = tempsensor.readTemperature();

    sprintf(buf, "%d.%d", (int)temp, (int)(temp*100)%100);
    debug_msg(buf);

    if(epoch % 60 == 0 && !isnan(temp)){
      debug_msg("uploading...");
      sprintf(temperature_str, "%d.%d", (int)temp, (int)(temp*100)%100);
      sprintf(timezone_str, "%s", timezone.get_val().c_str());

      for(int i = 0; i < sizeof(fiap_elements)/sizeof(fiap_elements[0]); i++){
        fiap_elements[i].year = year();
        fiap_elements[i].month  = month();
        fiap_elements[i].day = day();
        fiap_elements[i].hour = hour();
        fiap_elements[i].minute = minute();
        fiap_elements[i].second = second();
      }
      int ret = fiap_upload_agent.post(fiap_elements, sizeof(fiap_elements)/sizeof(fiap_elements[0]));
      if(ret == 0){
        debug_msg("done");
      }else{
        debug_msg("failed");
        debug_msg("code:"+ret);
      }
    }
  }

  old_epoch = epoch;
}

void debug_msg(String msg)
{
  if(debug == 1){
    Serial.print("[");
    print_time();
    Serial.print("]");
    Serial.println(msg);
  }
}

void print_time()
{
  char print_time_buf[32];
  sprintf(print_time_buf, "%04d/%02d/%02d %02d:%02d:%02d",
      year(), month(), day(), hour(), minute(), second());
  Serial.print(print_time_buf);
}

void restart(String msg, int restart_minutes)
{
  Serial.println(msg);
  Serial.print("This system will restart after ");
  Serial.print(restart_minutes);
  Serial.print("minutes.");

  unsigned int start_ms = millis();
  while(1){
    commandline.process();
#ifdef ESP8266
    ESP.wdtFeed();
#endif
    if(millis() - start_ms > restart_minutes*60UL*1000UL){
      commandline.reboot();
    }
  }
}
