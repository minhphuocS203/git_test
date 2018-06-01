#include "A9G.h"
#include "OBDlib.h"


//OBD OBD2;


//// đọc lỗi khi uart gửi về
//void OBD::getResponseerror(void){
//  char inChar=0;
//  while( Serial.available() > 0){
//    char c = Serial.read();
//    if( c == '\0'){
//    }
//    else{
//    rxDtaerror += c;
//    };
//    delay(30);
//  }
//}
//
//// đọc lỗi (mode3)
//int OBD::Readerror(void){
//  int error = 0;
//  Serial.write("03\r"); 
//  delay(200);
//  getResponseerror();// 
//  String rxDatanew1 = "";
//  String rxDatanew2 = "";
//  for (int i = 0; i < rxDtaerror.length(); i ++){
//    if(( rxDtaerror[i] == '4') && ( rxDtaerror[i +1] == '3')){
//      rxDatanew1 += rxDtaerror.substring(i+5,rxDtaerror.length());
//        }
//  
//     }
//        
//  for (int i = 0; i < rxDatanew1.length();  i ++){
//    if( rxDatanew1[i] == ':') {
//      rxDatanew1[i]=' ';
//      rxDatanew1[i -1]=' ';}
//      }
//      
//  for (int i = 0; i < rxDatanew1.length();  i ++){  
//    if   (rxDatanew1[i] != ' '){                        
//      rxDatanew += rxDatanew1[i];}
//       }
// }

// đọc dữ liệu khi uart gửi về
int OBD::getResponse(){
  char inChar=0;
  while( Serial.available()){
    char c = Serial.read();
    rxDta += c;
  }

#if debug
  Serial1.println(rxDta);
#endif   
}

// đọc nhiệt độ nước làm mát
int OBD::ReadTemp(void){

//  if(modedata[5] == 0){return -1;} // kiểm tra PID 05 có được hỗ trợ không?
  Serial.write("0105\r"); // 05 là PID của đọc nhiệt độ nước
  delay(200);
  getResponse();// gọi hàm đọc giá trị từ uart
  Temp = strtol(&rxDta[10],0,16)-40; // chuyển đổi giá trị hex nhận được sang dec
  rxDta =""; // xóa chuỗi vừa lưu để tiếp tục đọc giá trị khác

#if debug
  Serial1.print("THW: ");
  Serial1.println(rxDta);
#endif   
  }

// đọc điện áp của accu
float OBD::ReadVoltage(void){
  float Voltage = 0;
  Serial.flush();
  Serial.write("atrv\r");
  delay(200);
  getResponse(); 
  Voltage = rxDta.substring(4,'V').toFloat();
  rxDta ="";
  return Voltage;
}
// đọc tốc độ động cơ
int OBD::ReadRPM (void){
  int vehicleRPM = 0;
  if(modedata[12] == 0){return -1;}
  Serial.flush();
  Serial.write("010c\r");
  delay(200);
  getResponse(); 
 vehicleRPM = ((strtol(&rxDta[10],0,16)*256)+strtol(&rxDta[13],0,16))/4;
  rxDta = ""; 
 return vehicleRPM;
}
//đọc tốc độ xe
int OBD::ReadSpeed (void) {
  if(modedata[13] == 0){return -1;}
  int vspeed = 0;
  Serial.flush();
  Serial.write("010d\r");
  delay(200);
  getResponse();
  vspeed = strtol(&rxDta[10],0,16);
  rxDta ="";
   return vspeed;
}
//reset OBD
int OBD::ResetOBDII (void){
  byte ResetBoard = 0;
  rxDta ="";
  Serial.flush();
  Serial.write("atz\r");
  delay(2000);
  getResponse();
  if(rxDta.substring(3,9) == "ELM327"){ResetBoard =1;} else {ResetBoard =0;}
  rxDta ="";
  return ResetBoard;
}
// settup chế độ cho ELM327 tự quét giao thức phù hợp với xe
/*
int OBD::SetupBoard (void){
  int Setup = 0;
   Serial.flush();
  Serial.write("atsp0\r");
  delay(2000);
  getResponse();
  if( rxDta.substring(5,7) == "OK"){Setup =1;} else {Setup =0;}
  rxDta ="";
  return Setup;
}
*/
// đọc nhiệt độ khí nạp
int OBD::ReadIntemperature (void) {
  int Intemp = 0;
  if(modedata[15] == 0){return -1;}
  Serial.flush();
  Serial.write("010f\r");
  delay(200);
  getResponse();
  Intemp =( strtol(&rxDta[10],0,16)) -40;
  rxDta ="";
   return Intemp;
}
// đọc vị trí bàn đạp ga
int OBD::ReadPedalposition (void) {
  int Pposition = 0;
  if(modedata[90] == 0){return -1;}
  Serial.flush();
  Serial.write("015a\r");
  delay(200);
  getResponse();
  Pposition = (strtol(&rxDta[10],0,16))*100/255;
  rxDta ="";
   return Pposition;
}

//đọc góc đánh lửa sớm
int OBD::ReadTimingadvance (void) {
  int a = 0;
   if(modedata[14] == 0){return -1;}
  Serial.flush();
  Serial.write("010e\r");
  delay(200);
  getResponse();
 a = ((strtol(&rxDta[10],0,16))/2) - 64;
  rxDta ="";
   return a;
}
// đọc thời gian phun nhiên liệu
int OBD::ReadFuelinjectiontiming (void){
  
  int b = 0;
   if(modedata[93] == 0){return -1;}
  Serial.flush();
  Serial.write("015d\r");
  delay(200);
  getResponse(); 
 b= (((strtol(&rxDta[10],0,16)*256)+strtol(&rxDta[13],0,16))/128)-210;
  rxDta = ""; 
 return b;
}
// đọc nhiệt độ dầu động cơ
int OBD::ReadEngineoiltemperature (void) {
  int c = 0;
    if(modedata[92] == 0){return -1;}
  Serial.flush();
  Serial.write("015c\r");
  delay(200);
  getResponse();
  c =( strtol(&rxDta[10],0,16)) -40;
  rxDta ="";
   return c;
}
//đọc thời gian chạy của động cơ từ khi bắt đầu khởi động
int OBD::ReadRuntime(void){
  int d = 0;
 if(modedata[31] == 0){return -1;}
  Serial.flush();
  Serial.write("011f\r");
  delay(200);
  getResponse(); 
 d= (strtol(&rxDta[10],0,16)*256)+strtol(&rxDta[13],0,16);
  rxDta = ""; 
 return d;
}
// đọc vị trí bướm ga
int OBD::ReadThrottleposition (void) {
  int e = 0;
  if(modedata[17] == 0){return -1;}
  Serial.flush();
  Serial.write("0111\r");
  delay(200);
  getResponse();
  e = (strtol(&rxDta[10],0,16))*100/255;
  rxDta ="";
   return e;
}
// đọc khối lượng khí nạp
int OBD::ReadMAF (void){
  int f = 0;
  if(modedata[18] == 0){return -1;}
  Serial.flush();
  Serial.write("0110\r");
  delay(200);
  getResponse(); 
 f= ((strtol(&rxDta[10],0,16)*256)+strtol(&rxDta[13],0,16))/100;
  rxDta = ""; 
 return f;
}
//kiểm tra kết nối với board
int OBD::SetupConnect (void){
  int g = 0;
  Serial.flush();
  Serial.write("0100\r"); 
  delay(2000);
  getResponse();
  if( rxDta.substring(0,9) == "010041 00"){g =1;} else {g =0;}
  rxDta ="";
  return g;
}
//kiểm tra những PID nào được hỗ trợ
void OBD::SupportBoard(void){
 // mang 0100
 rxDta = ""; 
  Serial.flush();
  Serial.write("0100\r");
  delay(1000);
  getResponse();
  
 if(( rxDta.substring(4,6) == "NO") || (rxDta.substring(4,6) == "SE")) {
  for ( int i = 0; i < 32; i++) {
  arxDta1[i] = 0;
  }
  }else{

 a = strtol(&rxDta[10],0,16);
 b = strtol(&rxDta[13],0,16); 
 c = strtol(&rxDta[16],0,16); 
 d = strtol(&rxDta[19],0,16);  
   rxDta = "";

 //chuyen a
 if (d > 0) {
 for(int i = 0; d>0;i++){
 rxData [i]= d%2;
 d=d/2; 
 }
 }
 
  //chuyen b
 if (c > 0) {
 for(int i = 8; c>0;i++){
 rxData [i]= c%2;
 c=c/2;  
 }
 }
 
  //chuyen c
 if (b > 0) {
 for(int i = 16; b>0;i++){
 rxData [i]= b%2;
 b=b/2;  
 }
 }
  //chuyen d
 if (a > 0) {
 for(int i = 24; a>0;i++){
 rxData [i]= a%2;
 a=a/2;  
 }
 }
 
 m =31;
  for ( int i = 0; i < 32; i++) {
  arxDta1[m] = rxData[i];
  m = m -1;}
  }
  
// mang 0120
  Serial.flush();
  Serial.write("0120\r");
  delay(1000);
  getResponse();
 if(( rxDta.substring(4,6) == "NO") ||( rxDta.substring(4,6) == "SE")) {
    for ( int i = 0; i < 32; i++) {
    arxDta2[i] = 0;}
  } else{ 
 a = strtol(&rxDta[10],0,16);
 b = strtol(&rxDta[12],0,16); 
 c = strtol(&rxDta[15],0,16); 
 d = strtol(&rxDta[18],0,16);
   rxDta = "";
 //chuyen a
 if (d > 0) {
 for(int i = 0; d>0;i++){
 rxData [i]= d%2;
 d=d/2; 
 }
 }
 
  //chuyen b
 if (c > 0) {
 for(int i = 8; c>0;i++){
 rxData [i]= c%2;
 c=c/2;  
 }
 }
 
  //chuyen c
 if (b > 0) {
 for(int i = 16; b>0;i++){
 rxData [i]= b%2;
 b=b/2;  
 }
 }
 
  //chuyen d
 if (a > 0) {
 for(int i = 24; a>0;i++){
 rxData [i]= a%2;
 a=a/2;  
 }
 }
 
 m =31;
  for ( int i = 0; i < 32; i++) {
  arxDta2[m] = rxData[i];
  m = m -1;}
  } 


  // mang 0140

  Serial.flush();
  Serial.write("0140\r");
  delay(1000);
  getResponse();
 if(( rxDta.substring(4,6) == "NO") || (rxDta.substring(4,6) == "SE")) {
     for ( int i = 0; i < 32; i++) {
     arxDta3[i] = 0;}
  }else{ 
 a = strtol(&rxDta[10],0,16);
 b = strtol(&rxDta[12],0,16); 
 c = strtol(&rxDta[15],0,16); 
 d = strtol(&rxDta[18],0,16);
   rxDta = "";
 
 //chuyen a
 if (d > 0) {
 for(int i = 0; d>0;i++){
 rxData [i]= d%2;
 d=d/2; 
 }
 }
 
  //chuyen b
 if (c > 0) {
 for(int i = 8; c>0;i++){
 rxData [i]= c%2;
 c=c/2;  
 }
 }
 
  //chuyen c
 if (b > 0) {
 for(int i = 16; b>0;i++){
 rxData [i]= b%2;
 b=b/2;  
 }
 }
 
  //chuyen d
 if (a > 0) {
 for(int i = 24; a>0;i++){
 rxData [i]= a%2;
 a=a/2;  
 }
 }
 
 m =31;
  for ( int i = 0; i < 32; i++) {
  arxDta3[m] = rxData[i];
  m = m -1;}
 
} 

  // mang 0160

  Serial.flush();
  Serial.write("0160\r");
  delay(1000);
  getResponse();
 if(( rxDta.substring(4,6) == "NO") || (rxDta.substring(4,6) == "SE")) {
   for ( int i = 0; i < 32; i++) {
   arxDta4[i] = 0;}
   }else{
 a = strtol(&rxDta[10],0,16);
 b = strtol(&rxDta[12],0,16); 
 c = strtol(&rxDta[15],0,16); 
 d = strtol(&rxDta[18],0,16);
   rxDta = "";
 //chuyen a
 if (d > 0) {
 for(int i = 0; d>0;i++){
 rxData [i]= d%2;
 d=d/2; 
 }
 }
 
  //chuyen b
 if (c > 0) {
 for(int i = 8; c>0;i++){
 rxData [i]= c%2;
 c=c/2;  
 }
 }
 
  //chuyen c
 if (b > 0) {
 for(int i = 16; b>0;i++){
 rxData [i]= b%2;
 b=b/2;  
 }
 }
 
  //chuyen d
 if (a > 0) {
 for(int i = 24; a>0;i++){
 rxData [i]= a%2;
 a=a/2;  
 }
 }
 
 m =31;
  for ( int i = 0; i < 32; i++) {
  arxDta4[m] = rxData[i];
  m = m -1;}
 } 
 
  // mang 0180

  Serial.flush();
  Serial.write("0180\r");
  delay(1000);
  getResponse();
 if(( rxDta.substring(4,6) == "NO") || (rxDta.substring(4,6) == "SE")) {
  for ( int i = 0; i < 32; i++) {
  arxDta5[i] = 0;}
  } else {  
 a = strtol(&rxDta[10],0,16);
 b = strtol(&rxDta[12],0,16); 
 c = strtol(&rxDta[15],0,16); 
 d = strtol(&rxDta[18],0,16);
   rxDta = "";
 
 //chuyen a
 if (d > 0) {
 for(int i = 0; d>0;i++){
 rxData [i]= d%2;
 d=d/2; 
 }
 }

  //chuyen b
 if (c > 0) {
 for(int i = 8; c>0;i++){
 rxData [i]= c%2;
 c=c/2;  
 }
 }
 
  //chuyen c
 if (b > 0) {
 for(int i = 16; b>0;i++){
 rxData [i]= b%2;
 b=b/2;  
 }
 }
 
  //chuyen d
 if (a > 0) {
 for(int i = 24; a>0;i++){
 rxData [i]= a%2;
 a=a/2;  
 }
 }
 
 m =31;
  for ( int i = 0; i < 32; i++) {
  arxDta5[m] = rxData[i];
  m = m -1;}
}

  for ( int i = 0; i<= 31; i++){
    modedata[i] = arxDta1[i];
    }
  for ( int i = 32; i<= 63; i++){
    modedata[i] = arxDta2[i-32];
    }
  for ( int i = 64; i<= 95; i++){
    modedata[64] = arxDta3[i-64];
    }
  for ( int i = 96; i<= 127; i++){
    modedata[96] = arxDta4[i-96];
    }
  for ( int i = 128; i<= 159; i++){
    modedata[i] = arxDta5[i-128];
    }
}
void OBD::settupuart(void){
 Serial.begin(115200);
}
#endif
