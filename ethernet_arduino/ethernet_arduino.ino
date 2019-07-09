/*
 * Руи Сантос (Rui Santos)
 * Более подробно о проекте здесь:
 * http://randomnerdtutorials.com
*/
 
#include <SPI.h>
#include <Ethernet.h>
 
// введите ниже MAC-адрес и IP-адрес вашего контроллера;
// IP-адрес будет зависеть от вашей локальной сети:
byte mac[] = {
  0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
IPAddress ip(192,168,1,111);
 
// инициализируем библиотеку Ethernet Server, указывая нужный порт
// (по умолчанию порт для HTTP – это «80»):
EthernetServer server(80);
 
// задаем контакт и начальное состояние для реле:
String relay1State = "Off";
const int relay = 2;
 
// задаем переменные для клиента:
char linebuf[80];
int charcount=0;
 
void setup() {
  // подготавливаем реле-модуль:
  pinMode(relay, OUTPUT);
  digitalWrite(relay, HIGH);
 
  // открываем последовательную коммуникацию на скорости 9600 бод:
  Serial.begin(9600);
 
  // запускаем Ethernet-коммуникацию и сервер:
  Ethernet.begin(mac, ip);
  server.begin();
  Serial.print("server is at ");  //  "сервер на "
  Serial.println(Ethernet.localIP());
}
 
// Показываем веб-страницу с кнопкой «вкл/выкл» для реле:
void dashboardPage(EthernetClient &client) {
  client.println("<!DOCTYPE HTML><html><head>");
  client.println("<meta name=\"viewport\" content=\"width=device-width, initial-scale=1\"></head><body>");                                                            
  client.println("<h3>Arduino Web Server - <a href=\"/\">Refresh</a></h3>");
  // генерируем кнопку для управления реле:
  client.println("<h4>Relay 1 - State: " + relay1State + "</h4>");
  // если реле выключено, показываем кнопку «вкл»:          
  if(relay1State == "Off"){
    client.println("<a href=\"/relay1on\"><button>ON</button></a>");
  }
  // если реле включено, показываем кнопку «выкл»:
  else if(relay1State == "On"){
    client.println("<a href=\"/relay1off\"><button>OFF</button></a>");                                                                    
  }
  client.println("</body></html>");
}
 
 
void loop() {
  // прослушиваем входящих клиентов:
  EthernetClient client = server.available();
  if (client) {
    Serial.println("new client");  //  "новый клиент"
    memset(linebuf,0,sizeof(linebuf));
    charcount=0;
    // HTTP-запрос заканчивается пустой строкой:
    boolean currentLineIsBlank = true;
    while (client.connected()) {
      if (client.available()) {
       char c = client.read();
        // считываем HTTP-запрос, символ за символом:
        linebuf[charcount]=c;
        if (charcount<sizeof(linebuf)-1) charcount++;
        // если вы дошли до конца строки (т.е. если получили
        // символ новой строки), это значит,
        // что HTTP-запрос завершен, и вы можете отправить ответ:
        if (c == '\n' && currentLineIsBlank) {
          dashboardPage(client);
          break;
        }
        if (c == '\n') {
          if (strstr(linebuf,"GET /relay1off") > 0){
            digitalWrite(relay, HIGH);
            relay1State = "Off";
          }
          else if (strstr(linebuf,"GET /relay1on") > 0){
            digitalWrite(relay, LOW);
            relay1State = "On";
          }
          // если получили символ новой строки...
          currentLineIsBlank = true;
          memset(linebuf,0,sizeof(linebuf));
          charcount=0;          
        }
        else if (c != '\r') {
          // если получили какой-то другой символ...
          currentLineIsBlank = false;
        }
      }
    }
    // даем веб-браузеру время на получение данных:
    delay(1);
    // закрываем соединение:
    client.stop();
    Serial.println("client disonnected");  //  "Клиент отключен"
  }
}
