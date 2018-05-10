#include <SoftwareSerial.h>
#include <ArduinoJson.h>

#define DEBUG 1
#define delay_senddata_A9G 10


unsigned long int previousMillis = 0;
unsigned long int preMiliTCP = 0;

SoftwareSerial debug_UART(10, 11); // RX TX
SoftwareSerial GPS_UART(12, 13); // RX TX

String latitude, longitude; // toa do kinh , vi do
String latitude_tam, longitude_tam; // bien de tach 2 toa do
float latitude_DK, longitude_DK; // bien dieu kien loc du lieu
String RxData;  // du lieu GPS tra ve
String dulieu; // du lieu tra ve tu module A7

void init_A9G(); // thiet lap module
void sendData_A9G(String command); // gửi lệnh len moule
void getData_A9G(int timeGet); // lay Rxdata 
void check_GPS_Frame(); // kiem tra GPS
void check_tran_GPS(); // check and chuyen doi GPS
void tran_GPS(); // chuyen doi GPS

void setup() {
  Serial.begin (115200); // khoi tao cong serial giao tiep voi module
  debug_UART.begin(9600);  // khoi tao cong serial giao tiep voi may tinh
  GPS_UART.begin (4800);  // khoi tao cong serial giao tiep voi may tinh
  init_A9G();
}
void loop() {
  getData_A9G(2000);
  //Send_TCP_data(5000);
}







/****************************************************
  Author    : Do Hieu
  Description : Khoi tao module A9G
*****************************************************/
void init_A9G()
{
  sendData_A9G("AT");           delay(1000);
  sendData_A9G("AT+GPS=0");     delay(2000);
  sendData_A9G("AT+CREG=1");    delay(3000);
  sendData_A9G("AT+CGATT=1");   delay(3000);
  sendData_A9G("AT+CGDCONT=1,\"IP\",\"V-INTERNET\""); delay(3000);
  sendData_A9G("AT+CGACT=1,1");   delay(3000);
  sendData_A9G("AT+GPS?");      delay(2000);
  sendData_A9G("AT+GPS=1");     delay(1000);
  sendData_A9G("AT+GPSRD=10");  delay(1000);  // Lay sau 10s
}

/****************************************************
  Author      : Do Hieu
  Description : Goi command qua A9G tu Arduino
*****************************************************/
void sendData_A9G(String command) {
  String dulieu = "";
  Serial.println(command); // in ra lenh ra mang hinh
  delay(delay_senddata_A9G); // thoi gian delay khi in du lieu
#if DEBUG
  while ( Serial.available()) 
  {
    char ch = Serial.read(); // doc tung kí tự của Serial rui gắn vào bien ch
    dulieu += ch;
  }
  debug_UART.println(dulieu); // in du lieu phan hoi cua module len man hinh
#endif
}
/****************************************************
  Author      : Do Hieu
  Description : Nhan data tu A9G qua UART
*****************************************************/
void getData_A9G(int timeGet) { // Luon chay
  char ch;
  while ( Serial.available())
  {
    ch = Serial.read();
    RxData += ch;
  }
  if (millis() - previousMillis > timeGet) {
    previousMillis = millis();
    check_GPS_Frame();
    check_tran_GPS();

#if DEBUG
    debug_UART.print(RxData);
#endif
    RxData = ""; // Xoa du lieu
  };
}
/****************************************************
  Author      : Do Hieu
  Description :
*****************************************************/
void check_GPS_Frame()
{
  int i=0;
  while (i < RxData.length())
  {
    if ((RxData.substring(i, i + 5) == "+GPSR")) 
    { 
        
      latitude_tam = RxData.substring(25+i, 34+i); 
      longitude_tam = RxData.substring(37+i, 47+i);
      break;
    }
    i++;
  }
}

/****************************************************
  Author      : Minh Phuoc
  Description : Kiem tra và chuyển đổi
*****************************************************/

void check_tran_GPS() {
  int j;
        latitude_DK=latitude_tam.toInt();
        longitude_DK=longitude_tam.toInt();
       if ((latitude_DK!=0) && (longitude_DK!=0)){       // kiem tra du lieu co dung hay khong
          tran_GPS();  
        }
        else 
        {
        while (j>20){        // reset lai GPS  20*10s=2000s thoi gian lay GPS
          sendData_A9G("AT+GPS=0");
          delay(50);
          sendData_A9G("AT+GPS=1");
          delay(50);
          sendData_A9G("AT+GPSRD=10");
          j=0;
          break;
          }
          j++;
        } 
    }

/****************************************************
  Author      : Minh Phuoc
  Description : Chuyển đổi GPS
*****************************************************/

void tran_GPS() {    //format latitude DDMM.MMMM     longitude DDDMM.MMMM 
  //// doi kinh do latitude
  String LaDD, LaMM, LaMMMM, LaM; // Kinh tuyen
  float XDD, XMM, LaDDMM ; // Kinh tuyen
  long int XM; // kinh tuyen

  LaDD = latitude_tam.substring(0, 2); // tach DD lay độ
  XDD = LaDD.toInt(); // doi DD sang gia tri số
 
  LaMM = latitude_tam.substring(2, 4); // tach MM truoc dau cham
  LaMMMM = latitude_tam.substring(5); // tach MMMM sau dấu chấm
  
  LaM = LaMM + LaMMMM  ; // noi 2 chuoi lại
  XM = LaM.toInt(); // doi MM.MMMM sang gia tri so
  XMM = XM / (600000.0); // doi sang phut 6000000.0 module a7
 
  LaDDMM = XDD + XMM; // cong thuc tinh GPS
  //latitude_DK = M;
  latitude = String(LaDDMM, 6); // lay 6 so sau dau phay


  //// doi vi do longitude
  String LoDDD, LoMM, LoMMMM, LoM; // Vi tuyen
  float ADDD, AMM, LoDDMM; // Vi tuyen
  long int  AM; // Vi tuyen
  
  LoDDD = longitude_tam.substring(0, 3); // tach DD tu chuoi ra
  ADDD = LoDDD.toInt(); // doi DD sang gia tri số vê độ

  LoMM = longitude_tam.substring(3, 5); // tach MM truoc dau cham
  LoMMMM = longitude_tam.substring(6); // tach MMMM con lại sau dấu chấm
  
  LoM = LoMM + LoMMMM  ; // noi 2 chuoi lại
  AM = LoM.toInt(); // doi sang gia tri so
  AMM = AM / (600000.0); // doi sang phut  // 6000000.0 module a7
  
  LoDDMM = ADDD + AMM; // cong thuc tinh GPS
  //longitude_DK = Z;
  longitude = String( LoDDMM, 6); // hien 6 so sau dau phay 

 // hien thi GPS len tren man hinh
 #if DEBUG
   debug_UART.println("Toa do GPS: ");
   debug_UART.println("Kinh do :");
   debug_UART.println(latitude); // hien 6 so sau dau phay
   debug_UART.println("Vi do :");
   debug_UART.println(longitude); // hien 6 so sau dau phay 
 #endif 
}

void Send_TCP_data(int timeGet)
{

  if (millis() - preMiliTCP > timeGet) {
      preMiliTCP = millis();
      sendData_A9G("AT+CIPSTART=\"TCP\",\"159.65.4.55\",1334"); 
      delay(3000);
      sendData_A9G("AT+CIPSEND");
      delay(50);
      sendData_A9G(latitude);
      sendData_A9G(latitude);
      Serial.write(0x1A);          
   }
}








