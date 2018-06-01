#include "A9G.h"
#include "OBDlib.h"

/********************************************************************
 * Fucntion   : SendData_A9
 * Author     : 
 * Date       : 19/5/2018
 * Description: send a string command to A9 via UART.
                SoftwareSerial is not recommended
 ********************************************************************/
 uint8_t A9G_Module::SendData_A9G(char* ATcommand, char* ex_answer,unsigned long int timeout){
  uint8_t x, answer;
  memset(response,'\0', 100);

  Serial.println(ATcommand); // in ra lenh ra mang hinh

  // cho phan hoi 
  answer=0;
  x=0;
  previousSend=millis();
  
  do{
  if (Serial.available())
  {
    response[x]=Serial.read();
    x++;
    if (strstr(response,ex_answer)!=NULL )
    {
      answer=1;
    }
    else
    {
      answer=0;
    }
  }
 }while ((answer==0) && (millis() - previousSend < timeout));
 
#if DEBUG
  Serial1.println(response);
#endif    
}

/********************************************************************
 * Fucntion   : init
 * Author     : Do Hieu
 * Date       : 19/5/2018
 * Description: A9G initialization incldued: 
 ********************************************************************/
void A9G_Module::init(){
  #if DEBUG
    Serial1.println("Initializing A9G module....."); 
  #endif
  SendData_A9G("AT","OK",1000);           
  SendData_A9G("AT+CREG=1","OK",2000); 
  SendData_A9G("AT+CGATT=1","OK",2000);
  SendData_A9G("AT+CGDCONT=1,\"IP\",\"V-INTERNET\"","OK",2000); 
  SendData_A9G("AT+CGACT=1,1","OK",2000);
  SendData_A9G("AT+GPS=0","OK",1000);     
  SendData_A9G("AT+GPS?","OK",1000);     
  SendData_A9G("AT+GPS=1","OK",1000);
  SendData_A9G("AT+GPSRD=10","OK",1000);      // Lay sau 10s
}

/********************************************************************
 * Fucntion   : init
 * Author     : Do Hieu
 * Date       : 19/5/2018
 * Description: get data from A9G via UART.
                @timeGet :  time for waiting transfed data
 ********************************************************************/
void A9G_Module::getData(int timeGet) {  
  char ch;
  while ( Serial.available()){
    ch = Serial.read();
    RxData += ch;
  }
  if (millis() - previousMillis > timeGet) {
    previousMillis = millis(); 

    if(check_GPS_Frame()){
      tran_GPS();    // convert GPS data include latitude and longitude 
      OBD2.ReadTemp();
      #if DEBUG
        Serial1.print(RxData);
        
        Serial1.print("Hour: ");
        Serial1.print(timehhVN);
        Serial1.print("h");
        Serial1.print(timemm);
        Serial1.print("m");
        Serial1.print(timess);
        Serial1.print("s   ");
        
        Serial1.print("Date: ");
        Serial1.print(datedd);
        Serial1.print("/");
        Serial1.print(datemm);
        Serial1.print("/");
        Serial1.println(dateyy);
        
        Serial1.print("Mean Sea level altitude:");
        Serial1.print(altitude);
        Serial1.println(" M");
        
        Serial1.print("latitude :");
        Serial1.println(latitude); 
        Serial1.print("longitude :");
        Serial1.println(longitude); 
        Serial1.println("Chuyen doi thanh cong");
      #endif
      } else {
        #if DEBUG
          Serial1.print(RxData);
          Serial1.println("Chuyen doi khong thanh cong");
        #endif
      }
 
    if (state == DoNothing){ // ???
      state = Idle;
    }
    RxData = ""; // Xoa du lieu
  };
}


/********************************************************************
 * Fucntion   : check_GPS_Frame
 * Author     : Do Hieu
 * Date       : 19/5/2018
 * Description: 
 ********************************************************************/
bool A9G_Module::check_GPS_Frame()
{
  int i=0;
  while (i < RxData.length())
  {
    if ((RxData.substring(i, i + 5) == "+GPSR")) 
    {
        timehhUTC = RxData.substring(14+i, 16+i);
        timehhVN= timehhUTC.toInt()+7;
        timemm = RxData.substring(16+i, 18+i);
        timess = RxData.substring(18+i, 20+i);
        
        temp_lat    = RxData.substring(25+i, 34+i); 
        temp_long   = RxData.substring(37+i, 47+i);

        altitude = RxData.substring(55+i, 60+i);
        
//        datedd = RxData.substring(222+i, 15+i);
//        datemm = RxData.substring(57+i, 59+i);
//        dateyy = RxData.substring(515+i, 517+i);

        if ((temp_lat.toInt()!=0) && (temp_long.toInt()!=0)){// kiem tra du lieu co dung hay khong
          return 1; 
        }
        else{
          return 0; 
        } 
    }
    i++;
  }
  return 0; 
}


/********************************************************************
 * Fucntion   : Tran_GPS
 * Author     : Minh Phuoc
 * Date       : 19/5/2018
 * Description: Convert GPS data include latitude and longitude
 ********************************************************************/
void A9G_Module::tran_GPS() {
    String LaDD, LaMM, LaMMMM, LaM; // Kinh tuyen
    String LoDDD, LoMM, LoMMMM, LoM; // Vi tuyen
    float XMM; // Kinh tuyen
    float YMM; // Vi tuyen
//**Doi kinh do latitude, format latitude DDMM.MMMM**//
    LaDD = temp_lat.substring(0, 2); // tach DD lay độ
    LaMM = temp_lat.substring(2, 4); // tach MM truoc dau cham
    LaMMMM = temp_lat.substring(5); // tach MMMM sau dấu chấm
    LaM = LaMM + LaMMMM  ; // noi 2 chuoi lại
    XMM = LaM.toInt() / (600000.0); // doi sang phut (6000000.0 module a7)
    LaDDMM = LaDD.toInt() + XMM; // cong thuc tinh GPS
    latitude = String(LaDDMM, 6); // lay 6 so sau dau phay
//** Doi vi do longitude, format longitude DDDMM.MMMM **//
    LoDDD = temp_long.substring(0, 3); // tach DD tu chuoi ra
    LoMM = temp_long.substring(3, 5); // tach MM truoc dau cham
    LoMMMM = temp_long.substring(6); // tach MMMM con lại sau dấu chấm
    LoM = LoMM + LoMMMM  ; // noi 2 chuoi lại
    YMM = LoM.toInt() / (600000.0); // doi sang phut (6000000.0 module a7) 
    LoDDMM = LoDDD.toInt() + YMM; // cong thuc tinh GPS
    longitude = String(LoDDMM, 6); // hien 6 so sau dau phay 
}


/********************************************************************
 * Fucntion   : Send_TCP_data
 * Author     : Minh Phuoc
 * Date       : 19/5/2018
 * Description: Send package data upto the website
 ********************************************************************/
void A9G_Module::Send_TCP_data()
{
 if (millis() - previousTCP > 2000) {
    previousTCP = millis();
    switch(state)
    {
      case Idle :
        SendData_A9G("AT+CIPSTART=\"TCP\",\"159.65.4.55\",1334","OK",1000);
        state = SendedTCPStart;
        break;

      case SendedTCPStart :
        SendData_A9G("AT+CIPSEND",">",1000);
        state = SendedTCPsend;
        break;

      case SendedTCPsend :
        if ((LaDDMM!=0) && (LoDDMM!=0)){
          JsonWrap();
        }
        SendData_A9G(Jsonstring,"",1000);
        state = SendedData;
      break;

      case SendedData :
        Serial.write(0x1A);
        state =DoNothing;
      break;
    }
  } 
}


/********************************************************************
* Fucntion   : JsonWrap
* Author     : Do Hieu
* Date       : 19/5/2018
* Description: Packet data format
********************************************************************/
void A9G_Module::JsonWrap() {                     // đóng gói dữ liệu lại theo chuẩn
    memset(Jsonstring,'\0',200);
    StaticJsonBuffer<400> jsonBuffer;
    JsonObject& root = jsonBuffer.createObject();
    JsonArray& data = root.createNestedArray("GPS");

    data.add(latitude);  // thêm 2 tọa độ vào Data
    data.add(longitude);
    
    int adc; 
    root["a1"] = analogRead(A0);                     // các du lieu dc truyen vao
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






