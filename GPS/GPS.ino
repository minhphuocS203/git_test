#include <SoftwareSerial.h>
#include <ArduinoJson.h>
SoftwareSerial GPSdata (10,11); // RX TX
String ch;
String dulieu; // du lieu tra ve tu module A7
String latitude,longitude; // toa do kinh , vi do
String latitude1,longitude1; // bien de tach 2 toa do
float latitude2,longitude2; // bien dieu kien loc du lieu


void setup() {
   Serial.begin (115200); // khoi tao cong serial giao tiep voi module
   GPSdata.begin (9600);  // khoi tao cong serial giao tiep voi may tinh
   sendData("AT",100); // gửi lệnh AT từ adruino cho module
   delay(1000);
//   sendData("AT+GPS=0",100);
//   delay(2000);
   sendData("AT+CREG=1",200);   // thiet lap mạng 3g cua Viettel
   delay(3000);
   sendData("AT+CGATT=1",200);
   delay(3000);
   sendData("AT+CGDCONT=1,\"IP\",\"V-INTERNET\"",200);
   delay(3000);
   sendData("AT+CGACT=1,1",200);
   delay(3000);
   
   sendData("AT+GPS?",200);
   delay(2000);
   sendData("AT+GPS=1",100); // lệnh bật chế độ GPS
   delay(1000);
   sendData("AT+GPSRD=10",100); // sau 10s module đưa lại dữ liệu GPS 1 lần
   delay(1000);
     
}
void loop() {
 getData(100); // lấy dữ liệu GPS
 transdata(); // chuyển đổi dữ liệu
 if ((latitude2>1) && (longitude!=0)) // điều kiện lọc dữ liệu để gửi tọa độ lên server
 {
 sendTCP1(); // hàm gửi tạo độ lên server
 }
 delay(10000);
// JsonWrap();
// delay(2000);
  }

void sendData(String command,const int timeout){   // gửi lệnh từ arduino lên module A7
  String dulieu= ""; // reset lai bien du lieu
  Serial.println(command); // in ra lenh ra mang hinh
  delay(timeout); // thoi gian delay khi in du lieu
  while ( Serial.available()) // 
    {
      char ch = Serial.read(); // doc tung kí tự của Serial rui gắn vào bien ch
      dulieu+=ch;
    }
   GPSdata.println(dulieu); // in du lieu phan hoi cua module len man hinh
   }
   
void getData(const int timeout){ // lay du lieu GPS tu module A7
  String dulieu= "";
  delay(timeout);
  while ( Serial.available())
    {
      char ch = Serial.read();  
      dulieu+=ch;
    }
   GPSdata.println(dulieu);
   dulieu.substring(27,37); //27  37 | 34  44 // tach du lieu vi do ra khoi du lieu 
   dulieu.substring(40,51); // 40 51 | 47  58 // tach du lieu kinh do ra khoi du lieu
   latitude1= dulieu.substring(27,37); // gan bien cho du lieu oi tach
   longitude1= dulieu.substring(40,51); // gan bien cho du lieu oi tach
   
 }

void transdata(){ // đổi toa do GPS
  // doi kinh do latitude
  String La,L,A,La1; // Kinh tuyen
  float X,M,Y; // Kinh tuyen
  long int T; // kinh tuyen 
  
  La=latitude1.substring(0,2); // tach 2 so dau tu chuoi ra
  X=La.toInt(); // doi tu chuoi sang gia tri số vê độ

  L=latitude1.substring(2,4); // tach 2 so tiep theo truoc dau cham
  A=latitude1.substring(5); // tach các so con lại sau dấu chấm
  La1 = L + A  ; // noi 2 chuoi lại
  T=La1.toInt(); // doi sang gia tri so
  Y=T/(6000000.0); // doi sang phut
  M=X+Y; // cong thuc tinh GPS
  latitude2=M;
  latitude=String(M,6); // lay 6 so sau dau phay

  // doi vi do longitude
  String Lo,L1,O,Lo1; // Vi tuyen  
  float B,Z,C; // Vi tuyen
  long int N; // Vi tuyen 
  Lo=longitude1.substring(0,3); // tach 2 so dau tu chuoi ra
  C=Lo.toInt(); // doi tu chuoi sang gia tri số vê độ

  L1=longitude1.substring(3,5); // tach 2 so tiep theo truoc dau cham
  O=longitude1.substring(6); // tach các so con lại sau dấu chấm
  Lo1 = L1 + O  ; // noi 2 chuoi lại
  N=Lo1.toInt(); // doi sang gia tri so
  B=N/(6000000.0); // doi sang phut
  Z=C+B; // cong thuc tinh GPS
  longitude2=Z;
  longitude=String(Z,6); // hien 6 so sau dau phay

  // hien thi GPS len tren man hinh
  GPSdata.println("Toa do GPS: ");
  GPSdata.println("Kinh do :");
  GPSdata.println(M,6); // hien 6 so sau dau phay
  GPSdata.println("Vi do :");
  GPSdata.println(Z,6); // hien 6 so sau dau phay
   
}

 void JsonWrap(){                       // đóng gói dữ liệu lại theo chuẩn
  StaticJsonBuffer<400> jsonBuffer;
  JsonObject& root = jsonBuffer.createObject();
  JsonArray& data = root.createNestedArray("GPS");
  String kinhdo,vido;
  kinhdo=latitude;
  vido=longitude;
  data.add(kinhdo);  // thêm 2 tọa độ vào Data
  data.add(vido);
 // root["sensor"] = "gps";             // giả lập tên sensor là gps
 // root["rpm"] = random(1000,10000);   // giả lập tốc độ rpm từ 1000-10000 rpm
  int adc; adc = analogRead(A0);
  root["a1"]= adc;                      // các du lieu dc truyen vao
  root["a2"]= adc +5;
  root["a3"]= adc +10;
  root["io12"]= digitalRead(12);
  root["io13"]= digitalRead(13);
  root["io14"]= digitalRead(14);
  root["io15"]= digitalRead(15);
  root["io16"]= digitalRead(16);
  root["d1"]= random(100);
  root["d2"]= random(100);
  root["d3"]= random(100);
  root["d4"]= random(100);
  root["d5"]= random(100);
  
  char Jsonstring[200];  // Tạo một chuỗi tên là Jsonstring
  root.printTo(Jsonstring); // lưu chuối Json vừa tạo vào chuỗi Jsonstring
  // đén bước này đã đóng gói thành công chuỗi JSON cần gửi
  sendTCP(Jsonstring); // gửi lên server chuỗi Jsonstring với mã là customEvent
  //root.prettyPrintTo(Serial);
  delay(1000);
};

void sendTCP(String KDVD){ // gửi dữ liệu trong ham JsonWrap
  sendData("AT+CIPSTART=\"TCP\",\"hoangvandung.cf\",1334",300);
  delay(4000);
  sendData("AT+CIPSEND",200);
  delay(2000);
  sendData(KDVD,200);
  delay(2000);
  Serial.write(0x1A);
  delay(2000);  
 }

 void sendTCP1(){ // gui du lieu len server ( Test dữ liệu gửi đúng hông?)
  sendData("AT+CIPSTART=\"TCP\",\"hoangvandung.cf\",1334",300); // thiet lap kieu gui, dia chi, port 
  delay(3000);
  sendData("AT+CIPSEND",200); // lệnh gữi 
  sendData(latitude,200); // nội dung gữi
  sendData(longitude,200);
  Serial.write(0x1A);  // lệnh kết thúc gữi
  delay(2000);
 }

  ///aaaa
