#include <Wire.h>
#include "LiquidCrystal_I2C.h"

LiquidCrystal_I2C lcd(0x27, 16, 2); // set the LCD address to 0x27 for a 16 chars and 2 line display

#define FIRST_PIN 2 //Pino onde está o primeiro relê
#define PINS_COUNT 12 //Quantos pinos serão utilizados

//Mantém o estado atual dos pinos (HIGH ou LOW)
int pinsStatus[PINS_COUNT];

void setup()
{
  Serial.begin(115200);
  lcd.init();                      // initialize the lcd
  // Print a message to the LCD.
  lcd.backlight();
  lcd.setCursor(2, 0); // go to start of 2nd line
  lcd.print("Hello, world!");
  lcd.setCursor(4, 1); // go to start of 2nd line
  lcd.print("keywish");

  //Inicializa os pinos
  setupPins();
}

void setupPins()
{
  //Coloca os pinos que estão ligados os relês como saída
  for (int i = 0; i < PINS_COUNT; i++)
  {
    pinsStatus[i] = LOW;
    int pinNumber = FIRST_PIN + i;
    pinMode(pinNumber, OUTPUT);
    digitalWrite(pinNumber, pinsStatus[i]);
  }
}

void loop()
{
  //Verifica se há um novo cliente
  if (Serial.available())
  {
    //Faz a leitura da requisição
    String strRequest = readRequest();
    const char* request = strRequest.c_str();

    //Se a requisição não for para o favicon
    if (strstr(request, "favicon") == NULL)
    {
      //Executamos a ação com o valor passado na requisição
      execute(getAction(request), getValue(request));

      //Envia a resposta ao cliente
      sendResponse();
    }
    else
    {
      Serial.print(
        "HTTP/1.1 404 Not Found\r\n"
        "Content-Length: 0\r\n"
        "Connection: close\r\n"
        "\r\n");
    }

    Serial.write(127);
  }
}

//Faz a leitura da primeira linha da requisição
String readRequest()
{
  bool currentLineIsBlank = true;
  String resp = "";
  int i = 0;
  bool firstLine = true;

  while (true) {
    while (!Serial.available());
    char c = Serial.read();

    //Apenas a primeira linha da requisição nos interessa
    if (firstLine) {
      resp += c;
      i++;
    }

    if (c == '\n') {
      //A última linha da requisição é um \r\n sozinho, após o \r\n da linha anterior
      if (currentLineIsBlank) {
        //Se chegou aqui é porque a requisição foi lida por completo
        break;
      }

      currentLineIsBlank = true;
      firstLine = false;
    }
    else if (c != '\r') {
      //Se leu qualquer caracter que não for \n e \r significa que a linha não está em branco
      currentLineIsBlank = false;
    }
  }

  return resp;
}

//Envia o HTML para o cliente
void sendResponse()
{
  //Envia o cabeçalho HTTP
  Serial.print(
    "HTTP/1.0 200 OK\r\n"
    "Content-Type: text/html; charset=UTF-8\r\n"
    "Connection: close\r\n"
    "\r\n");

  Serial.println("<!DOCTYPE HTML>");
  Serial.println("<html>");
  head();//Envia o cabeçalho do HTML
  body();//Envia o corpo do HTML
  Serial.println("</html>");
}

//Envia o CSS para modificar a aparência da página
void head()
{
  Serial.println(F("<head>"
                   "<meta name='viewport' content='width=device-width, initial-scale=1.0'>"
                   "<style>"
                   "body{"
                   "text-align: center;"
                   "font-family: sans-serif;"
                   "font-size: 14px;"
                   "}"
                   "p{"
                   "color:#555;"
                   "font-size: 12px;"
                   "}"
                   ".button{"
                   "outline: none;"
                   "display: block;"
                   "border: 1px solid #555;"
                   "border-radius:18px;"
                   "width: 150px;"
                   "height: 30px;"
                   "margin: 10px;"
                   "margin-left: auto;"
                   "margin-right: auto;"
                   "cursor: pointer;"
                   "}"
                   ".button_off{"
                   "background-color:#FFF;"
                   "color: #555;"
                   "}"
                   ".button_on{"
                   "background-color:#2C5;"
                   "color: #fff;"
                   "}"
                   "</style>"
                   "</head>"));
}

//Exibe os dados dos sensores e cria os botões
void body()
{
  Serial.println("<body>");

  String buttons = "";

  //Cria um botão para cada pino que possui um relê
  for (int i = 0; i < PINS_COUNT; i++)
  {
    buttons.concat(button(i));
  }

  Serial.println(buttons);
  Serial.println(F(
                   "<div>"
                   "<input id=\"text_1\" />"
                   "<button onclick=\"location.href='?changeText='+(text_1.value)\">Enviar</button>"
                   "</div>"
                 ));
  Serial.println("</body>");
}

//Cria um botão com a aparência e ação correspondente ao estado atual do relê
String button(int number)
{
  String label = String(number + 1);
  String className = "button ";
  className += pinsStatus[number] == 1 ? "button_on" : "button_off";
  String action = pinsStatus[number] == 1 ? "off" : "on";
  return "<button class=\"" + className + "\"onclick=\"location.href='?" + action + "=" + label + "'\">" + label + "</button>";
}

//Retorna a ação que o cliente deseja executar (on off)
String getAction(char *request)
{
  String resp = getStringBetween(request, '?', '=');
  return resp;
}

//Retorna o valor (numero do relê) que a ação será executada
String getValue(char *request)
{
  String resp = getStringBetween(request, '=', ' ');
  return resp;
}

//Retorna a string que fica entre o primeiro caractere 'start' e o primeiro caractere 'end'
String getStringBetween(char* input, char start, char end)
{
  String str = "";
  //retorna o endereço de memória do caractere 'start'
  char* c = strchr(input, start);

  //Se não achou o caractere
  if (c == NULL)
  {
    return "";
  }

  //Vai para o próximo caractere
  c++;

  //Enquanto não chegar ao caractere 'end' ou ao final da string
  while (*c != end && *c != '\0')
  {
    str += *c;
    c++;
  }

  return str;
}

//Executada a ação junto ao valor (número do relê)
void execute(String action, String value)
{
  //Se é uma das duas ações que esperamos
  if (action == "on" || action == "off")
  {
    //Os relês são numerados a partir do 1, max o array começa do 0
    //então tiramos 1
    int index = value.toInt() - 1;
    //O número do pino será o índice mais o número do pino onde os relês
    //começam. Os relês devem estar em sequência a partir do pino inicial (FIRST_PIN)
    int pinNumber = FIRST_PIN + index;
    int status = action == "on" ? 1 : 0;

    if (action == "on") {
      digitalWrite(pinNumber, HIGH);
    } else {
      digitalWrite(pinNumber, LOW);
    }
    pinsStatus[index] = status;
  }
  if (action == "changeText") {
    String fixed_value = hex2ascii(value);

    String first_line = fixed_value;
    String second_line = "";

    if (fixed_value.length() > 15) {
      first_line = fixed_value.substring(0, 16);
      second_line = fixed_value.substring(16);
    }

    lcd.clear();
    lcd.setCursor(0, 0); // go to start of 2nd line
    lcd.print(first_line);
    lcd.setCursor(0, 1);
    lcd.print(second_line);
  }
}


String hex2ascii(String string)
{
  string.replace("%21", "!");
  string.replace("%23", "#");
  string.replace("%24", "$");
  string.replace("%25", "%");
  string.replace("%26", "&");
  string.replace("%27", "'");
  string.replace("%28", "(");
  string.replace("%29", ")");
  string.replace("%2A", "*");
  string.replace("%2B", "+");
  string.replace("%2C", ",");
  string.replace("%2F", "/");
  string.replace("%3A", ":");
  string.replace("%3B", ";");
  string.replace("%3D", "=");
  string.replace("%3F", "?");
  string.replace("%40", "@");
  string.replace("%5B", "[");
  string.replace("%5D", "]");

  string.replace("%20", " ");
  string.replace("%22", """");
  string.replace("%2D", "-");
  string.replace("%2E", ".");
  string.replace("%3C", "<");
  string.replace("%3E", ">");
  string.replace("%5C", "\\");
  string.replace("%5E", "^");
  string.replace("%5F", "_");
  string.replace("%60", "`");
  string.replace("%7B", "{");
  string.replace("%7C", "|");
  string.replace("%7D", "}");
  string.replace("%7E", "~");
  string.replace("%C2%A3", "£");

  return string;
}
