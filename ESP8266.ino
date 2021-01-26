#include <ESP8266WiFi.h>

//Troque pelos dados da sua rede
const char* ssid = "@allisonverdam";
const char* password = "10203040";

//Timeout da conexão
#define TIMEOUT 1000
#define MAX_BUFFER 300

//Server na porta 80 (padrão http)
WiFiServer server(80);
//Buffer onde serão gravados os bytes da comunicação
uint8_t buffer[MAX_BUFFER];

void setup() {
 Serial.begin(115200);

 //Envia a informação da rede para conectar
 WiFi.disconnect();
 WiFi.mode(WIFI_STA);
 WiFi.begin(ssid, password);

 //Espera a conexão com o access point
 while (WiFi.status() != WL_CONNECTED) {
   delay(500);
 }

 Serial.println(ssid);
 Serial.print("IP address: ");
 Serial.println(WiFi.localIP());  //IP address assigned to your ESP

 //Inicializa o server
 server.begin();
}

void loop() {
 //Verifica se alguem se conectou
 WiFiClient client = server.available();
 if (!client) {
   //Se ninguém conectou apenas retorna sem fazer nada
   return;
 }

 //Marca o tempo que o cliente se conectou e a quantidade
 //de bytes lidos
 uint32_t connectedTime = millis();
 int bytesRead = 0;

 //Enquanto o cliente estiver conectado
 while (client.connected())
 {
   //Tempo agora
   uint32_t now = millis();
   //Quanto tempo passou desde a conexão com o cliente
   uint32_t ellapsed = now - connectedTime;

   //Se o tempo passou do tempo máximo e não leu nenhum byte
   if (ellapsed > TIMEOUT && bytesRead == 0)
   {
     //Fecha a conexão com o cliente
     client.stop();
     break;
   }

   int available = client.available();

   //Se o cliente possui bytes a serem lidos
   if (available)
   {
     int bufferSize = available < MAX_BUFFER ? available : MAX_BUFFER;
     int readCount = client.read(buffer, bufferSize);
     //Envia os bytes pela serial e aumenta o contador de bytes lidos
     Serial.write(buffer, readCount);
     Serial.flush();
     bytesRead += readCount;
   }

   available = Serial.available();

   //Se a serial possui bytes a serem lidos
   if (available)
   {
     int bufferSize = available < MAX_BUFFER ? available : MAX_BUFFER;
     //Lê os bytes
     Serial.readBytes(buffer, bufferSize);

     //Se for o byte que define a finalização da conexão
     if (buffer[bufferSize - 1] == 127)
     {
       client.write(buffer, bufferSize - 1);
       //Envia o que ainda não tenha sido enviado
       client.flush();
       //Espera um tempo para o cliente receber
       delay(100);
       //Fecha a conexão com o cliente e sai do 'while'
       client.stop();
       break;
     }

     //Envia os bytes para o cliente
     client.write(buffer, bufferSize);
   }
 }//while(client.connected())
}//loop
