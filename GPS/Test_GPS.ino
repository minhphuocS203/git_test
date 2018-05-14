#include <SoftwareSerial.h>
#include <ArduinoJson.h>

#define DEBUG 1
#define delay_senddata_A9G 10

typedef enum {    // khai bao cac trai thai của SendTCP_State
  DoNothing,
  Idle,
  SendedTCPStart,
  SendedTCPsend,
  SendedData
} SendTCP_State;

SendTCP_State state  = DoNothing;
unsigned long int previousMillis = 0;
unsigned long int previousTCP = 0;

//unsigned long int previous = 0;

SoftwareSerial debug_UART(10, 11); // RX TX


String RxData;  // du lieu GPS tra ve
String latitude_tam, longitude_tam; // bien de tach 2 toa do
String lo,la; // bien hien thi KD,VD
float latitude, longitude; // toa do kinh , vi do
char Jsonstring[200];  // Tạo một chuỗi tên là Jsonstring

void init_A9G(); // thiet lap module
void sendData_A9G(String command); // gửi lệnh len moule
void getData_A9G(int timeGet); // lay Rxdata 
bool check_GPS_Frame(); // kiem tra GPS
void tran_GPS(); // chuyen doi GPS
void Send_TCP_data(); // gui TCP
void JsonWrap();  // Dong goi chuoi JSON

void setup() {
  
  Serial.begin (115200); // khoi tao cong serial giao tiep voi module
  debug_UART.begin(9600);  // khoi tao cong serial giao tiep voi may tinh
  init_A9G();

}
void loop() {
  getData_A9G(2000);
  Send_TCP_data();
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
void getData_A9G(int timeGet) {      // Luon chay
  char ch;
  while ( Serial.available())
  {
    ch = Serial.read();
    RxData += ch;
  }
  if (millis() - previousMillis > timeGet) {
    previousMillis = millis();
    
    #if DEBUG
    debug_UART.print(RxData);
    #endif
    
    check_GPS_Frame();
    if (state == DoNothing){
      state = Idle;
      }
    
//#if DEBUG 
//    if(check_GPS_Frame()==1)
//     {
//      debug_UART.println("Chuyen doi thanh cong");
//      }
//    else {
//      debug_UART.println("Chuyen doi khong thanh cong");
//      }
//#endif

    RxData = ""; // Xoa du lieu
  };
}
/****************************************************
  Author      : Do Hieu
  Description :
*****************************************************/
bool check_GPS_Frame()
{
  int i=0;
  while (i < RxData.length())
  {
    if ((RxData.substring(i, i + 5) == "+GPSR")) 
    {    
      latitude_tam = RxData.substring(25+i, 34+i); 
      longitude_tam = RxData.substring(37+i, 47+i);
      
      if ((latitude_tam.toInt()!=0) && (longitude_tam.toInt()!=0)){       // kiem tra du lieu co dung hay khong
        tran_GPS();  
        return 1; // Chuyen doi thanh cong
      }
      else{
        return 0; // Chuyen doi khong thanh cong
      } 
    }
    i++;
  }
  return 0; // Chuyen doi khong thanh cong
}


/****************************************************
  Author      : Minh Phuoc
  Description : Chuyển đổi GPS
*****************************************************/
void tran_GPS() {    
//* Doi kinh do latitude, format latitude DDMM.MMMM *//
  String LaDD, LaMM, LaMMMM, LaM; // Kinh tuyen
  float XMM, LaDDMM ; // Kinh tuyen

  LaDD = latitude_tam.substring(0, 2); // tach DD lay độ
  // LaDD.toInt();  doi DD sang gia tri số
 
  LaMM = latitude_tam.substring(2, 4); // tach MM truoc dau cham
  LaMMMM = latitude_tam.substring(5); // tach MMMM sau dấu chấm
  LaM = LaMM + LaMMMM  ; // noi 2 chuoi lại
  XMM = LaM.toInt() / (600000.0); // doi sang phut (6000000.0 module a7)
  // LaM.toInt();  doi MM.MMMM sang gia tri so
 
  LaDDMM = LaDD.toInt() + XMM; // cong thuc tinh GPS
  latitude = LaDDMM; // ket qua kinh tuyen
  la = String(LaDDMM, 6); // lay 6 so sau dau phay
  
//** Doi vi do longitude, format longitude DDDMM.MMMM **//
  String LoDDD, LoMM, LoMMMM, LoM; // Vi tuyen
  float YMM, LoDDMM; // Vi tuyen
  
  LoDDD = longitude_tam.substring(0, 3); // tach DD tu chuoi ra
  // ADDD = LoDDD.toInt();  doi DD sang gia tri số vê độ

  LoMM = longitude_tam.substring(3, 5); // tach MM truoc dau cham
  LoMMMM = longitude_tam.substring(6); // tach MMMM con lại sau dấu chấm
  
  LoM = LoMM + LoMMMM  ; // noi 2 chuoi lại
  YMM = LoM.toInt() / (600000.0); // doi sang phut (6000000.0 module a7)
  // LoM.toInt(); doi sang gia tri so
  
  LoDDMM = LoDDD.toInt() + YMM; // cong thuc tinh GPS
  longitude = LoDDMM;  // ket qua vi do
  lo = String(LoDDMM, 6); // hien 6 so sau dau phay 
 
#if DEBUG
   debug_UART.println("Toa do GPS: ");
   debug_UART.println("Kinh do :");
   debug_UART.println(la); // hien 6 so sau dau phay
   debug_UART.println("Vi do :");
   debug_UART.println(lo); // hien 6 so sau dau phay 
#endif 

}

void Send_TCP_data()
{
 if (millis() - previousTCP > 2000) {
    previousTCP = millis();
    switch(state)
    {
      case Idle :
      sendData_A9G("AT+CIPSTART=\"TCP\",\"159.65.4.55\",1334");
      state = SendedTCPStart;
      break;

      case SendedTCPStart :
      sendData_A9G("AT+CIPSEND");
      state = SendedTCPsend;
      break;

      case SendedTCPsend :
      JsonWrap();
      sendData_A9G(Jsonstring);
      state = SendedData;
      break;

      case SendedData :
      Serial.write(0x1A);
      state =Idle;
      break;
    }
  } 
}

void JsonWrap() {                      // đóng gói dữ liệu lại theo chuẩn
    memset(Jsonstring,'\0',200);
    StaticJsonBuffer<400> jsonBuffer;
    JsonObject& root = jsonBuffer.createObject();
    JsonArray& data = root.createNestedArray("GPS");
    
    data.add(latitude);  // thêm 2 tọa độ vào Data
    data.add(longitude);
    
    int adc=1; 
    root["a1"] = adc;                     // các du lieu dc truyen vao
    root["a2"] = adc + 5;
    root["a3"] = adc + 10;
    root["io12"] = digitalRead(12);
    root["io13"] = digitalRead(13);
    root["io14"] = digitalRead(14);
    root["io15"] = digitalRead(15);
    root["io16"] = digitalRead(16);
    root["d1"] = random(100);
    root["d2"] = random(100);
    root["d3"] = random(100);
    root["d4"] = random(100);
    root["d5"] = random(100);

    root.printTo(Jsonstring); // lưu chuối Json vừa tạo vào chuỗi Jsonstring
      
}








