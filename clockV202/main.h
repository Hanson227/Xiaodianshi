#ifndef __MAIN_H__
#define __MAIN_H__


#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266WebServer.h>
#include <WiFiUdp.h>
#include <ArduinoJson.h>
#include <TimeLib.h>
#include <TFT_eSPI.h> 
#include <SPI.h>
#include <TJpg_Decoder.h>
#include "DHT.h"
#include <EEPROM.h>

#include "font/ZdyLwFont_20.h"          //字体头文件
#include "font/FxLED_32.h"

#include "img/pangzi/i0.h"              //太空人图片
#include "img/pangzi/i1.h"
#include "img/pangzi/i2.h"
#include "img/pangzi/i3.h"
#include "img/pangzi/i4.h"
#include "img/pangzi/i5.h"
#include "img/pangzi/i6.h"
#include "img/pangzi/i7.h"
#include "img/pangzi/i8.h"
#include "img/pangzi/i9.h"

#include "img/temperature.h"            //温度图标
#include "img/humidity.h"               //湿度图标
#include "img/watch_top.h"              //顶部图标
#include "img/watch_bottom.h"           //底部图标

TFT_eSPI tft = TFT_eSPI();              //引脚请自行配置tft_espi库中的 User_Setup.h文件
TFT_eSprite clk = TFT_eSprite(&tft);
TFT_eSprite clkb = TFT_eSprite(&tft);   //字体滚动用

int Led = 2;                            //GPIO2为LED输出，低电平亮
int DHTPin = 4;                         //GPIO4为DHT11引脚D2
#define DHTTYPE DHT11                   //DHT 11
float rtemp;
uint8_t rhumi;
DHT dht(DHTPin, DHTTYPE);               //DHT定义

HTTPClient httpClient;                  //创建 HTTPClient 对象
uint8_t loadNum = 6;                    //开机启动进度条的起始位置
uint16_t bgColor = 0xFFFF;              //背景颜色
String cityCode = "101040100";          //天气城市代码
uint8_t Dis_Count = 0;                  //滚动显示内容计数
String scrollText[6];                   //滚动显示的数据缓冲区

#define CITY    "101010100"             //默认的城市
#define CITY_LEN      9                 //默认的城市长度

char *pCity;


//---------------------NTP相关参数---------------------
static const char ntpServerName[] = "ntp1.aliyun.com"; //NTP服务器   
const int timeZone = 8;                 //时区，东八区为北京时间
WiFiUDP Udp;
unsigned int localPort = 8888;          //连接时间服务器的本地端口号
time_t prevDisplay = 0;                 //上一次获取到的时间
const int NTP_PACKET_SIZE = 48;         //NTP发送数据包长度
byte packetBuffer[NTP_PACKET_SIZE];     //NTP数据包缓冲区

//---------------------Time 相关参数---------------------
int Led_Flag = HIGH;                    //默认当前灭灯
bool Led_State = false;                 //灯状态
unsigned long now1;                     //轮流滚动时间计时
unsigned long LastTime1 = 0;
unsigned long now2;                     //定时获取天气
unsigned long LastTime2 = 0;
unsigned long now3;
unsigned long LastTime3 = 0;            //定时采集温湿度数据

typedef struct
{
  uint8_t Date;                         //RTC Date.1-31
  uint16_t Year;                        //RTC Date Year.2000-2099    
  uint8_t Minute;
}RTC_DateTypeDef;                       //时间结构体

bool GotIP_Flag = false;                //获取到IP地址标志位
bool PowerOn_Flag = false;              //上电标志位
RTC_DateTypeDef Time;                   //本次读到的时间
RTC_DateTypeDef Last_Time;              //上次读到的时间

//---------------------WEB 相关参数---------------------
ESP8266WebServer WebServer(80);       //WEB服务器使用80端口

//网页数据内容
const String SetForm =   "<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.0 Transitional//EN\" \"http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd\">\
  <html xmlns=\"http://www.w3.org/1999/xhtml\">\
  <head>\
  <style type=\"text/css\">\
  INPUT {font-size:20px};\
  TEXTAREA{font-size:20px};\
  Button {font-size :20px};\
  </style>\
  <meta name=\"viewport\" content=\"width=320px, user-scalable=no, initial-scale=1, maximum-scale=1\">\
  <title>8266 Set</title>\
  </head>\
  <body>\
  <form action=\"weatherset\" method=\"post\" enctype=\"application/x-www-form-urlencoded\">\
    <table width=\"0\" border=\"0\">\
    <tr>\
      <td>City Number:</td>\
      <td><input name=\"City\" type=\"text\" /></td>\
    </tr>\
  </table>\
  <input name=\"submitOK\" type=\"submit\" value=\"Set\" />\
  </form>\
  </body>\
  </html>";
  
time_t getNtpTime();
void digitalClockDisplay();
void printDigits(int digits);
String num2str(int digits);
void sendNTPpacket(IPAddress &address);

#endif
