//**********************************************************************
//本程序是网上开源的，我做了整理规范，修改了部分内容，添加了注释，编写了教程。

//V201，在V101基础上修改
//增加了年的显示
//天气部分滚动修改成静态，这样太空人旋转不会卡断
//账号密码写死修改为一键配网
//V202，在V201基础上修改
//去掉城市自动定位，这个有的时候会出错，增加网页设置城市代码
//**********************************************************************
#include "main.h"

#define  VERSION   "V202"

WiFiClient client;

void Led_On(void)                               //模块上二极管亮
{
   digitalWrite(Led, LOW);
   Led_State = true;
}

void Led_Off(void)                              //模块上二极管灭
{
   digitalWrite(Led, HIGH);                     //高为关，低为开
   Led_State = false;
}

bool autoConfig()                                 //用之前的配网参数自动联网
{
  WiFi.mode(WIFI_STA);
  WiFi.begin();
  for (int i = 0; i < 20; i++){                   //超过20S未连上的话返回错误
    int wstatus = WiFi.status();
    if (wstatus == WL_CONNECTED){                 //连接上网络了
      GotIP_Flag = true;
      while(loadNum < 194){                       //让动画走完
        PowerOn_Loading(1, 1);
      }
      Serial.println("AutoConfig Success");
      Serial.printf("SSID: %s\r\n", WiFi.SSID().c_str());
      Serial.printf("PSW:  %s\r\n", WiFi.psk().c_str());
      Serial.print("IP:   ");
      Serial.println(WiFi.localIP());             //得到IP地址
      return true;
    }
    else{
      Serial.print("AutoConfig Waiting......");
      Serial.println(wstatus);
      for (uint8_t n = 0; n < 10; n++){            //每1秒检测一次状态
        PowerOn_Loading(100, 1);
      }
      Led_Flag = !Led_Flag;
      digitalWrite(Led, Led_Flag);
    }
  }
  Serial.println("AutoConfig Faild!" );
  return false;
}

void smartConfig()                                //开始一键配网
{
  WiFi.mode(WIFI_STA);
  Serial.println("\r\nWaiting for Smartconfig");
  delay(1000);
  WiFi.beginSmartConfig();                        //等待配网
  loadNum = 0;                                    //进度条从头开始
 
  while (1){
    Serial.print(".");
    for (uint8_t n = 0; n < 10; n++){             //每0.4秒检测一次状态
      PowerOn_Loading(40, 2);
    }
    if (loadNum >= 194){                          //进度条走完了，从头开始
      loadNum = 0;
    }
    Led_Flag = !Led_Flag;
    digitalWrite(Led, Led_Flag);
    if (WiFi.smartConfigDone()){                  //配网完成
      Serial.println("SmartConfig Success");
      Serial.printf("SSID:%s\r\n", WiFi.SSID().c_str());
      Serial.printf("PSW:%s\r\n", WiFi.psk().c_str());
      WiFi.setAutoConnect(true);                  //设置自动连接
      delay(2000);
      ESP.restart();                              //配网完成自动复位一下
      break;
    }
  }
 
  Serial.println("WiFi connected");  
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());                 //得到IP地址
}

void PowerOn_Loading(uint8_t delayTime, uint8_t type)//开机联网显示的进度条，输入延时时间。type=0显示Connecting to WiFi，type=1显示Start smartconfig...
{
  clk.setColorDepth(8);

  clk.createSprite(200, 70);                      //创建Sprite
  clk.fillSprite(0x0000);                         //填充颜色

  clk.drawRoundRect(0,0,200,16,8,0xFFFF);         //画一个圆角矩形
  clk.fillRoundRect(3,3,loadNum,10,5,0xFFFF);     //画一个填充的圆角矩形
  clk.setTextDatum(CC_DATUM);                     //显示对齐方式
  clk.setTextColor(TFT_GREEN, 0x0000);            //文本的前景色和背景色
  if (type == 1){
    clk.drawString("Connecting to WiFi",100,40,2);//显示文本
  }
  else if (type == 2){
    clk.drawString("Start smartconfig",100,40,2); //显示文本
  }
  if (GotIP_Flag == true){                        //得到IP地址，显示
    clk.drawString(WiFi.localIP().toString(),100,60,2);      //显示文本
  }
  clk.pushSprite(20,110);                         //Sprite中内容一次推向屏幕
  clk.deleteSprite();                             //删除Sprite
  loadNum += 1;                                   //进度条位置变化，直到加载完成
  if(loadNum>=194){
    loadNum = 194;
  }
  delay(delayTime);
}

void digitalClockDisplay()                        //时间显示
{
  clk.setColorDepth(8);

  Time.Year = year();
  Time.Date = day();
  Time.Minute = minute();

  //--------------------中间时间区显示开始--------------------
  //时分
  if (Last_Time.Minute != Time.Minute){           //时间和上次显示不一样才刷新
    clk.createSprite(140, 48);                    //创建Sprite，先在Sprite内存中画点，然后将内存中的点一次推向屏幕，这样刷新比较快
    clk.fillSprite(bgColor);                      //背景色
    //clk.loadFont(FxLED_48);
    clk.setTextDatum(CC_DATUM);                   //显示对齐方式
    clk.setTextColor(TFT_BLACK, bgColor);         //文本的前景色和背景色
    clk.drawString(hourMinute(),70,24,7);         //绘制时和分
    //clk.unloadFont();
    clk.pushSprite(28,40);                        //Sprite中内容一次推向屏幕
    clk.deleteSprite();                           //删除Sprite
    Last_Time.Minute = Time.Minute;
  }
  //秒
  clk.createSprite(40, 32);
  clk.fillSprite(bgColor);
  clk.loadFont(FxLED_32);                         //加载字体
  clk.setTextDatum(CC_DATUM);
  clk.setTextColor(TFT_BLACK, bgColor); 
  clk.drawString(num2str(second()),20,16);
  clk.unloadFont();                               //卸载字体
  clk.pushSprite(170,60);
  clk.deleteSprite();
  //--------------------中间时间区显示结束--------------------

  //--------------------底部时间区显示开始--------------------
  clk.loadFont(ZdyLwFont_20);                     //加载汉字字体

  if (Last_Time.Year != Time.Year){
    //年
    clk.createSprite(80, 32);
    clk.fillSprite(bgColor);
    clk.setTextDatum(CC_DATUM);
    clk.setTextColor(TFT_BLACK, bgColor);  
    clk.drawString(Dis_Year(),39,16);             //2021年
    clk.pushSprite(1,168);
    clk.deleteSprite();
    Last_Time.Year = Time.Year;
  }
  
  if (Last_Time.Date != Time.Date){
    //月日
    clk.createSprite(98, 32);
    clk.fillSprite(bgColor);
    clk.setTextDatum(CC_DATUM);
    clk.setTextColor(TFT_BLACK, bgColor);  
    clk.drawString(monthDay(),49,16);             //几月几日
    clk.pushSprite(85,168);
    clk.deleteSprite();

    //星期
    clk.createSprite(55, 32);
    clk.fillSprite(bgColor);
    clk.setTextDatum(CC_DATUM);
    clk.setTextColor(TFT_BLACK, bgColor);
    clk.drawString(week(),29,16);                 //周几
    clk.pushSprite(185,168);
    clk.deleteSprite();
    Last_Time.Date = Time.Date;
  }
  
  clk.unloadFont();                               //卸载字体
  //--------------------底部时间区显示结束--------------------
}

String week()                                     //星期
{
  String wk[7] = {"日","一","二","三","四","五","六"};
  String s = "周" + wk[weekday()-1];
  return s;
}

String monthDay()                                 //月日
{
  String s = String(month());
  s = s + "月" + day() + "日";
  return s;
}

String Dis_Year()                                 //2021年
{
  String s = String(year());
  s = s + "年";
  return s;
}

String hourMinute()                               //时分
{
  String s = num2str(hour());
  s = s + ":" + num2str(minute());
  return s;
}

String num2str(int digits)                        //数字转换成字符串，保持2位显示
{
  String s = "";
  if (digits < 10)
    s = s + "0";
  s = s + digits;
  return s;
}

void printDigits(int digits)                      //打印时间数据
{
  Serial.print(":");
  if (digits < 10)                                //打印两位数字
    Serial.print('0');
  Serial.print(digits);
}

time_t getNtpTime()                               //获取NTP时间
{
  IPAddress ntpServerIP;                          //NTP服务器的IP地址

  while (Udp.parsePacket() > 0) ;                 //之前的数据没有处理的话一直等待 discard any previously received packets
  WiFi.hostByName(ntpServerName, ntpServerIP);    //从网站名获取IP地址
  
  sendNTPpacket(ntpServerIP);                     //发送数据包
  uint32_t beginWait = millis();
  while (millis() - beginWait < 1500) {
    int size = Udp.parsePacket();                 //接收数据
    if (size >= NTP_PACKET_SIZE) {
      Serial.println("Receive NTP Response");
      Udp.read(packetBuffer, NTP_PACKET_SIZE);    //从缓冲区读取数据
      
      unsigned long secsSince1900;
      secsSince1900 =  (unsigned long)packetBuffer[40] << 24;
      secsSince1900 |= (unsigned long)packetBuffer[41] << 16;
      secsSince1900 |= (unsigned long)packetBuffer[42] << 8;
      secsSince1900 |= (unsigned long)packetBuffer[43];
      return secsSince1900 - 2208988800UL + timeZone * SECS_PER_HOUR;
    }
  }
  Serial.println("No NTP Response :-(");
  return 0;                                       //没获取到数据的话返回0
}

void sendNTPpacket(IPAddress &address)            //发送数据包到NTP服务器
{
  memset(packetBuffer, 0, NTP_PACKET_SIZE);       //缓冲区清零

  packetBuffer[0] = 0b11100011;                   //LI, Version, Mode   填充缓冲区数据
  packetBuffer[1] = 0;                            //Stratum, or type of clock
  packetBuffer[2] = 6;                            //Polling Interval
  packetBuffer[3] = 0xEC;                         //Peer Clock Precision
  packetBuffer[12] = 49;
  packetBuffer[13] = 0x4E;
  packetBuffer[14] = 49;
  packetBuffer[15] = 52;

  Udp.beginPacket(address, 123);                  //NTP服务器端口123
  Udp.write(packetBuffer, NTP_PACKET_SIZE);       //发送udp数据
  Udp.endPacket();                                //发送结束
}

void Room_Display()                               //室内温湿度显示
{
  clk.setColorDepth(8);
  clk.loadFont(ZdyLwFont_20);                     //加载汉字字体

  if (PowerOn_Flag == false){
    //室温显示
    clk.createSprite(60, 32); 
    clk.fillSprite(bgColor);
    clk.setTextDatum(CC_DATUM);
    clk.setTextColor(TFT_BLACK, bgColor); 
    clk.drawString("房间",30,16);
    clk.pushSprite(170,90);
    clk.deleteSprite();
    PowerOn_Flag = true;
  }

  //温度显示
  clk.createSprite(60, 32);                       //创建Sprite
  clk.fillSprite(bgColor);                        //填充颜色
  clk.setTextDatum(CC_DATUM);                     //显示对齐方式
  clk.setTextColor(TFT_BLACK, bgColor);           //文本的前景色和背景色 
  String s = "";
  s += (uint8_t)rtemp;
  //s += ".";
  //s += (uint8_t)(rtemp * 10) % 10;
  s += "℃";
  clk.drawString(s,27,16);                        //显示文本
  clk.pushSprite(180,115);                        //Sprite中内容一次推向屏幕
  clk.deleteSprite();                             //删除Sprite

  //湿度显示
  clk.createSprite(56, 24); 
  clk.fillSprite(bgColor);
  clk.setTextDatum(CC_DATUM);
  clk.setTextColor(TFT_BLACK, bgColor); 
  //String s = "";
  s = rhumi;
  s += "%";
  clk.drawString(s,28,13);
  clk.pushSprite(180,140);
  clk.deleteSprite();
  
  clk.unloadFont();                               //卸载字体
  //--------------------底部时间区显示结束--------------------
}

bool tft_output(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t* bitmap)//显示回调函数
{
  if (y >= tft.height()) return 0;
  tft.pushImage(x, y, w, h, bitmap);
  return 1;
}

void getCityCode()                                //发送HTTP请求并且将服务器响应通过串口输出
{
  String URL = "http://wgeo.weather.com.cn/ip/?_="+String(now());

  httpClient.begin(client,URL);                          //配置请求地址。此处也可以不使用端口号和PATH而单纯的
  httpClient.setUserAgent("esp8266");             //用户代理版本，其实没什么用 最重要是后端服务器支持
  //httpClient.setUserAgent("Mozilla/5.0 (iPhone; CPU iPhone OS 11_0 like Mac OS X) AppleWebKit/604.1.38 (KHTML, like Gecko) Version/11.0 Mobile/15A372 Safari/604.1");//设置请求头中的User-Agent
  httpClient.addHeader("Referer", "http://www.weather.com.cn/");
 
  int httpCode = httpClient.GET();                //启动连接并发送HTTP请求
  Serial.print("Send GET request to URL: ");
  Serial.println(URL);
  
  if (httpCode == HTTP_CODE_OK) {                 //如果服务器响应OK则从服务器获取响应体信息并通过串口输出
    String str = httpClient.getString();
    int aa = str.indexOf("id=");
    if (aa > -1){                                 //应答包里找到ID了
       cityCode = str.substring(aa+4,aa+4+9);     //9位长度
       Serial.println(cityCode); 
       getCityWeater();                           //获取天气信息
       LastTime2 = millis();
    }
    else{                                         //没有找到ID
      Serial.println("获取城市代码失败");  
    }
  } 
  else{
    Serial.println("请求城市代码错误：");
    Serial.println(httpCode);
  }
 
  httpClient.end();                               //关闭与服务器连接
}

void getCityWeater()                              //获取城市天气
{
  //String URL = "http://d1.weather.com.cn/weather_index/" + pCity + ".html?_="+String(now());//cityCode
  String URL = "http://d1.weather.com.cn/weather_index/";
  URL += pCity;
  URL += ".html?_="+String(now());//cityCode

  httpClient.begin(client,URL);                          //配置请求地址。
  httpClient.setUserAgent("esp8266");             //用户代理版本，其实没什么用 最重要是后端服务器支持
  httpClient.addHeader("Referer", "http://www.weather.com.cn/");
 
  int httpCode = httpClient.GET();                //启动连接并发送HTTP请求
  Serial.print("Send GET request to URL: ");
  Serial.println(URL);
  
  if (httpCode == HTTP_CODE_OK) {                 //如果服务器响应OK则从服务器获取响应体信息并通过串口输出
    String str = httpClient.getString();
    int indexStart = str.indexOf("weatherinfo\":");//寻找起始和结束位置
    int indexEnd = str.indexOf("};var alarmDZ");

    String jsonCityDZ = str.substring(indexStart+13,indexEnd);//复制字符串
    Serial.println(jsonCityDZ);

    indexStart = str.indexOf("dataSK =");         //寻找起始和结束位置
    indexEnd = str.indexOf(";var dataZS");
    String jsonDataSK = str.substring(indexStart+8,indexEnd);//复制字符串
    Serial.println(jsonDataSK);

    indexStart = str.indexOf("\"f\":[");          //寻找起始和结束位置
    indexEnd = str.indexOf(",{\"fa");
    String jsonFC = str.substring(indexStart+5,indexEnd);//复制字符串
    Serial.println(jsonFC);
    
    weaterData(&jsonCityDZ,&jsonDataSK,&jsonFC);  //显示天气信息
    Serial.println("获取成功");
    
  } 
  else {
    Serial.println("请求城市天气错误：");
    Serial.print(httpCode);
  }
 
  httpClient.end();                               //关闭与服务器连接
}

void weaterData(String *cityDZ,String *dataSK,String *dataFC)//天气信息写到屏幕上
{
  DynamicJsonDocument doc(512);
  deserializeJson(doc, *dataSK);
  JsonObject sk = doc.as<JsonObject>();

  clk.setColorDepth(8);
  clk.loadFont(ZdyLwFont_20);                     //加载汉字字体
  
  //温度显示
  clk.createSprite(54, 32);                       //创建Sprite
  clk.fillSprite(bgColor);                        //填充颜色
  clk.setTextDatum(CC_DATUM);                     //显示对齐方式
  clk.setTextColor(TFT_BLACK, bgColor);           //文本的前景色和背景色 
  clk.drawString(sk["temp"].as<String>()+"℃",27,16);//显示文本
  clk.pushSprite(25,115);                         //Sprite中内容一次推向屏幕
  clk.deleteSprite();                             //删除Sprite

  //城市名称显示
  clk.createSprite(88, 32); 
  clk.fillSprite(bgColor);
  clk.setTextDatum(CC_DATUM);
  clk.setTextColor(TFT_BLACK, bgColor); 
  clk.drawString(sk["cityname"].as<String>(),44,16);
  clk.pushSprite(151,1);
  clk.deleteSprite();
  
  //PM2.5空气指数显示
  uint16_t pm25BgColor = tft.color565(156,202,127);//优
  String aqiTxt = "优";
  int pm25V = sk["aqi"];
  if (pm25V > 200){
    pm25BgColor = tft.color565(136,11,32);        //重度，显示颜色和空气质量程度
    aqiTxt = "重度";
  }
  else if (pm25V > 150){
    pm25BgColor = tft.color565(186,55,121);       //中度
    aqiTxt = "中度";
  }
  else if (pm25V > 100){
    pm25BgColor = tft.color565(242,159,57);       //轻
    aqiTxt = "轻度";
  }
  else if (pm25V > 50){
    pm25BgColor = tft.color565(247,219,100);      //良
    aqiTxt = "良";
  }
  clk.createSprite(50, 24); 
  clk.fillSprite(bgColor);
  clk.fillRoundRect(0,0,50,24,4,pm25BgColor);
  clk.setTextDatum(CC_DATUM);
  clk.setTextColor(0xFFFF); 
  clk.drawString(aqiTxt,25,13);
  clk.pushSprite(15,90);
  clk.deleteSprite();

  //湿度显示
  clk.createSprite(56, 24); 
  clk.fillSprite(bgColor);
  clk.setTextDatum(CC_DATUM);
  clk.setTextColor(TFT_BLACK, bgColor); 
  clk.drawString(sk["SD"].as<String>(),28,13);
  //clk.drawString("100%",28,13);
  clk.pushSprite(25,140);
  clk.deleteSprite();

  scrollText[0] = "实时天气 " + sk["weather"].as<String>();//滚动显示的数据缓冲区
  scrollText[1] = "空气质量 " + aqiTxt;
  scrollText[2] = "风向 " + sk["WD"].as<String>()+sk["WS"].as<String>();
  
  //左上角滚动字幕
  deserializeJson(doc, *cityDZ);
  JsonObject dz = doc.as<JsonObject>();
  //Serial.println(sk["ws"].as<String>());
  //String aa = "今日天气:" + dz["weather"].as<String>() + "，温度:最低" + dz["tempn"].as<String>() + "，最高" + dz["temp"].as<String>() + " 空气质量:" + aqiTxt + "，风向:" + dz["wd"].as<String>() + dz["ws"].as<String>();
  //Serial.println(aa);
  scrollText[3] = "今日" + dz["weather"].as<String>();
  
  deserializeJson(doc, *dataFC);
  JsonObject fc = doc.as<JsonObject>();
  scrollText[4] = "最低温度"+fc["fd"].as<String>()+"℃";
  scrollText[5] = "最高温度"+fc["fc"].as<String>()+"℃";
  
  clk.unloadFont();                               //卸载字体
}

void scrollBanner()                               //天气滚动条显示
{
  now1 = millis();
  if(now1 - LastTime1 > 2500){                    //2.5秒切换一次显示内容
    if(scrollText[Dis_Count]){                    //如果滚动显示缓冲区有数据
      clkb.setColorDepth(8);
      clkb.loadFont(ZdyLwFont_20);                //加载汉字字体
      
      //for(int pos = 24; pos>0; pos-=1){            //24点，每次移动一个点，从下往上移，这里小人会有点卡
      //  Dis_Scroll(pos);
      //}

      //滚动显示修改为静态显示，滚动的时候太空人会卡顿
      clkb.createSprite(148, 24);                 //创建Sprite，先在Sprite内存中画点，然后将内存中的点一次推向屏幕，这样刷新比较快
      clkb.fillSprite(bgColor);                   //背景色
      clkb.setTextWrap(false);
      clkb.setTextDatum(CC_DATUM);                //显示对齐方式
      clkb.setTextColor(TFT_BLACK, bgColor);      //文本的前景色和背景色
      clkb.drawString(scrollText[Dis_Count],74,12);//打显示内容
      clkb.pushSprite(2,4);                       //Sprite中内容一次推向屏幕
      clkb.deleteSprite();                        //删除Sprite
      
      //clkb.deleteSprite();                      //删除Sprite，这个我移动到Dis_Scroll函数里了
      clkb.unloadFont();                          //卸载字体
  
      if (Dis_Count >= 5){                        //总共显示五条信息
        Dis_Count = 0;                            //回第一个
      }
      else{
        Dis_Count += 1;                           //准备切换到下一个  
      }
      //Serial.println(Dis_Count);
    }
    LastTime1 = now1;
  }
}

void Dis_Scroll(int pos){                         //字体滚动
  clkb.createSprite(148, 24);                     //创建Sprite，先在Sprite内存中画点，然后将内存中的点一次推向屏幕，这样刷新比较快
  clkb.fillSprite(bgColor);                       //背景色
  clkb.setTextWrap(false);
  clkb.setTextDatum(CC_DATUM);                    //显示对齐方式
  clkb.setTextColor(TFT_BLACK, bgColor);          //文本的前景色和背景色
  clkb.drawString(scrollText[Dis_Count],74,pos+12);//打显示内容
  clkb.pushSprite(2,4);                           //Sprite中内容一次推向屏幕
  clkb.deleteSprite();                            //删除Sprite
}

void imgAnim(){
  int x=80,y=94,dt=30;                            //瘦子版dt=10毫秒 胖子30较为合适

  TJpgDec.drawJpg(x,y,i0, sizeof(i0));            //打一张图片延时一段时间，达到动画效果
  delay(dt);
  TJpgDec.drawJpg(x,y,i1, sizeof(i1));
  delay(dt);
  TJpgDec.drawJpg(x,y,i2, sizeof(i2));
  delay(dt);
  TJpgDec.drawJpg(x,y,i3, sizeof(i3));
  delay(dt);  
  TJpgDec.drawJpg(x,y,i4, sizeof(i4));
  delay(dt);  
  TJpgDec.drawJpg(x,y,i5, sizeof(i5));
  delay(dt);  
  TJpgDec.drawJpg(x,y,i6, sizeof(i6));
  delay(dt);  
  TJpgDec.drawJpg(x,y,i7, sizeof(i7));
  delay(dt);  
  TJpgDec.drawJpg(x,y,i8, sizeof(i8));
  delay(dt);  
  TJpgDec.drawJpg(x,y,i9, sizeof(i9));
  delay(dt);  
}

//---------------------------WEB 设置开始-----------------------------------
void handleSet()                            //打开网页设置页面
{
  WebServer.send(200, "text/html", SetForm);
}

void handleSetDeal()                        //网页设置确定后处理
{
  uint8 len = 0;
  bool Write_Flag = false;                  //为1的话表示接收到有效数据要写flash了
  uint8 write_data[100];                    //用户数据存储空间
  uint16_t i;
  uint8_t j;
  
  if (WebServer.method() != HTTP_POST){
    WebServer.send(405, "text/plain", "Method Not Allowed");
  } 
  else{
    Serial.println("Post Web Set Deal");

    EEPROM.begin(100);                        //申请操作100字节数据，读-修改-写，begin的数字要大于64才能有效，我也不知道为什么
    for(i = 0; i < 100; i ++){
      write_data[i] = EEPROM.read(i);         //读数据
    }
  
    String message = "POST form was:\r\n";
    for (i = 0; i < (WebServer.args() - 2); i++){//最后两个，按键和总体的就不打印了
      message += " " + WebServer.argName(i) + ": " + WebServer.arg(i) + "\n";
      Serial.println(WebServer.argName(i) + ": " + WebServer.arg(i));

      j = 0;
      while (WebServer.arg(i)[j] != '\0'){    //获取字符串的长度
        j ++;
        if (j > 20){
          break;
        }
      }
      len = j;
      if (len > 0){                           //如果有输入数据的话就进行修改EEPROM内容
        Write_Flag = true;
        write_data[i] = len;
        for (j = 0; j < len; j ++){
          write_data[i * 20 + 10 + j] = WebServer.arg(i)[j];
        }
      }
    }

    if (Write_Flag == true){                  //要写EEPROM了
      write_data[0] = 0x55;                   //置标志位
      write_data[1] = 0xAA;
      for(i = 0; i < 100; i ++){
         EEPROM.write(i,write_data[i]);       //写数据
      }
      EEPROM.commit();                        //保存数据

      WebServer.send(200, "text/plain", message);
      WebServer.handleClient();               //WEB处理函数
      ESP.restart();                          //保存数据后立刻重启
    }

    WebServer.send(200, "text/plain", message);
  }
}

//读取失败或读到密钥或长度不对，就采用默认值
bool EEPROM_Init(void)                        //FLASH初始化
{
  bool ret = false;
  char write_data[100];                       //用户数据存储空间
  uint16_t i;
  uint8_t City_len;

  EEPROM.begin(100);                          //申请操作80字节数据
  for(i = 0; i < 100; i ++){
    write_data[i] = EEPROM.read(i);           //读数据
  }

  if ((write_data[0] == 0x55) && (write_data[1] == 0xAA)){//有密码的话，读取设定的城市代码
    pCity = (char *)zalloc(CITY_LEN + 1);
    strncpy(pCity, write_data + 10, CITY_LEN); 
  }
  else{
    pCity = CITY;
  }

  Serial.printf("City:%s\r\n", pCity);
}

void handleNotFound()                         //网页不存在打印
{
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += WebServer.uri();
  message += "\nMethod: ";
  message += (WebServer.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += WebServer.args();
  message += "\n";
  
  for (uint8_t i = 0; i < WebServer.args(); i++) {
    message += " " + WebServer.argName(i) + ": " + WebServer.arg(i) + "\n";
  }
  
  WebServer.send(404, "text/plain", message);
}
//---------------------------WEB 设置结束-----------------------------------

void setup()
{
  Serial.begin(115200);                           //初始化串口
  Serial.println();                               //打印回车换行

  pinMode(Led, OUTPUT);                           //初始化IO
  digitalWrite(Led, HIGH);                        //高为关，低为开
  Last_Time.Year = 0xFFFF;
  Last_Time.Date = 0xFF;
  Last_Time.Minute = 0xFF;

  EEPROM_Init();                                  //FLASH初始化
  
  tft.init();                                     //TFT初始化
  tft.setRotation(0);                             //旋转角度0-3
  tft.fillScreen(0x0000);                         //清屏
  tft.setTextColor(TFT_BLACK, bgColor);           //设置字体颜色

  if (!autoConfig()){                             //如果自动联网失败的话，就启动smartconfig
    Serial.println("Start smartconfig");
    smartConfig();
  }
  delay(1000);
  Led_Off();

  //-------------------------------WEB 设置开始-----------------------------
  WebServer.on("/", handleSet);                     //请求打开设置网页
  WebServer.on ("/weatherset", handleSetDeal);      //在网页上进行设置处理
  WebServer.on("/inline", []() 
  {
    WebServer.send(200, "text/plain", "this works as well");
  });
  WebServer.onNotFound(handleNotFound);
  WebServer.begin();
  Serial.println("HTTP server started");
  //-------------------------------WEB 设置结束-----------------------------

  Serial.println("Starting UDP");                   //连接时间服务器
  Udp.begin(localPort);
  Serial.print("Local port: ");
  Serial.println(Udp.localPort());
  Serial.println("waiting for sync");
  setSyncProvider(getNtpTime);
  setSyncInterval(300);

  TJpgDec.setJpgScale(1);                         //设置放大倍数
  TJpgDec.setSwapBytes(true);                     //交换字节
  TJpgDec.setCallback(tft_output);                //回调函数

  TJpgDec.drawJpg(0,0,watchtop, sizeof(watchtop));//显示顶部图标 240*20
  TJpgDec.drawJpg(0,220,watchbottom, sizeof(watchbottom));//显示底部图标 240*20

  //绘制一个窗口
  tft.setViewport(0, 20, 240, 200);               //中间的显示区域大小
  tft.fillScreen(0x0000);                         //清屏
  tft.fillRoundRect(0,0,240,200,5,bgColor);       //实心圆角矩形
  //tft.resetViewport();

  //绘制线框
  tft.drawFastHLine(0,34,240,TFT_BLACK);          //这些坐标都是窗体内部坐标
  tft.drawFastVLine(150,0,34,TFT_BLACK);
  tft.drawFastHLine(0,166,240,TFT_BLACK);
  tft.drawFastVLine(83,166,34,TFT_BLACK);
  tft.drawFastVLine(184,166,34,TFT_BLACK);

  //getCityCode();                                  //通过IP地址获取城市代码
  getCityWeater();                                  //获取天气信息
  
  TJpgDec.drawJpg(5,115,temperature, sizeof(temperature));//温度图标
  TJpgDec.drawJpg(5,140,humidity, sizeof(humidity));//湿度图标
  TJpgDec.drawJpg(155,115,temperature, sizeof(temperature));//温度图标  室温
  TJpgDec.drawJpg(155,140,humidity, sizeof(humidity));//湿度图标
}

void loop()
{
  WebServer.handleClient();                       //WEB处理函数
  
  if (timeStatus() != timeNotSet){                //已经获取到数据的话
    if (now() != prevDisplay){                    //如果本次数据和上次不一样的话，刷新
      prevDisplay = now();
      digitalClockDisplay();
    }

    now3 = millis();
    if ((now3 - LastTime3 >= 10000) || (LastTime3 == 0)){ //定时更新数据
      LastTime3 = now3;
  
      rtemp = dht.readTemperature(); 
      rhumi = (uint8_t)dht.readHumidity(); 
      if (rhumi <= 100){
        Room_Display();                           //室内温湿度显示
      }
    }
  }

  if(millis() - LastTime2 > 600000){              //10分钟更新一次天气
    LastTime2 = millis();
    getCityWeater();
  }
  scrollBanner();                                 //天气数据滚动显示
  imgAnim();                                      //太空人显示
}
