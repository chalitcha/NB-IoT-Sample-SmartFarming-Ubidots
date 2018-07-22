#include "True_NB_bc95.h"
#include <AltSoftSerial.h>
#include <Dns.h>
#include <SPI.h>
AltSoftSerial Serial2;

String check_udp_incoming();

True_NB_bc95 modem;

String ubidots_serv_ip = "50.23.124.66"; /* translate.ubidots.com */
String ubidots_serv_port = "9012";

//String useragent = "nb002/1.1";  // Up to you...
String useragent = "farming/1.1";  // Up to you...
String token = "BBFF-xxxxxxxxXbnpBaxxxhCmpxxxxxOa1y"; // Assign your Ubidots TOKEN
String device_name = "farming001"; // Assign the unique device label (device name)
String val_name1 = "temperature:";
String val_name2 = "humidity:";
String val_name3 = "fan:";
String data_u = "";

long start = 0;
long start2 = 0;
char sock[] = "0\0";
int  fan_stts = 0; // 0:off, 1:on
bool rec_flag = false;

void setup() {

  Serial.begin(9600);
  Serial2.begin(9600);
  pinMode(13, OUTPUT);
  delay(3000);

  Serial.println("Starting...");

  modem.init(Serial2);
  modem.initModem();

  Serial.println( "IMEI = " + modem.getIMEI() );
  Serial.println( "IMSI = " + modem.getIMSI() );

  while (!modem.register_network());
  delay(1000);

  Serial.println( modem.check_ipaddr() );
  modem.create_UDP_socket( 6000, sock);

  modem.sendUDPstr(ubidots_serv_ip, ubidots_serv_port, "Hello"); // first connect
}

void loop()
{
  String str_fan = "";
  String _packet = "";
  int data0, data1, data2, data3;


  /*** Retirve lamp status ***/
  if (millis() - start2 > 5000)  // 5 sec
  {
    start2 = millis();

    _packet = useragent + "|LV|" + token + "|" + device_name + ":fan" + "|end";
    Serial.println("retrive:" + _packet);
    modem.sendUDPstr(ubidots_serv_ip, ubidots_serv_port, _packet);
  }


  /*** Post data to platform ***/
  if (millis() - start > 30000)  // 30
  {
    start = millis();

    /* Example data generating */
    data0 = random(20, 80);
    data1 = random(30, 70);

    /* Send data to ubidot dashboard */
    /* Refer: http://help.ubidots.com/developer-guides/sendretrieve-data-from-ubidots-over-tcp-or-udp */
    data_u = val_name1 + String(data0) + "," + val_name2 + String(data1) + "," + val_name3 + String(fan_stts);
    _packet = useragent + "|POST|" + token + "|" + device_name + "=>" + data_u + "|end";
    //Serial.println("post:" + _packet);
    Serial.println("post");
    modem.sendUDPstr(ubidots_serv_ip, ubidots_serv_port, _packet);
  }



  delay(300);
  // get lamp status
  str_fan = check_udp_incoming();
  if ( str_fan == "31") {
    fan_stts = 1; // On
    digitalWrite(13, HIGH);
  }
  else if ( str_fan == "30") {
    fan_stts = 0; // Off
    digitalWrite(13, LOW);
  }


}

String check_udp_incoming()
{
  String tmp_buf;
  String data_in;
  String str_fanstts = "";
  int indx = 0;


  Serial2.println("AT+NSORF=0,8"); // send read data from modem
  delay(300);

  tmp_buf = Serial2.readString();
  //Serial.println("incomeing" + tmp_buf); // debug
  indx = tmp_buf.indexOf("4F4B7C"); // find '|'
  if (indx >= 0) // search --> OK|
  {    
    str_fanstts = tmp_buf.substring(indx + 6, indx + 8);
    Serial.println("data1:" + str_fanstts);
  }

  return str_fanstts;
}


