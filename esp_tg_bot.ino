/*******************************************************************
    A telegram bot for your ESP8266 that responds
    with whatever message you send it.

    Parts:
    D1 Mini ESP8266 * - http://s.click.aliexpress.com/e/uzFUnIe
    (or any ESP8266 board)

      = Affilate

    If you find what I do useful and would like to support me,
    please consider becoming a sponsor on Github
    https://github.com/sponsors/witnessmenow/


    Written by Brian Lough
    YouTube: https://www.youtube.com/brianlough
    Tindie: https://www.tindie.com/stores/brianlough/
    Twitter: https://twitter.com/witnessmenow
 *******************************************************************/

#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>

// Wifi network station credentials
#define WIFI_SSID "C's Galaxy S21 FE"
#define WIFI_PASSWORD "XXXXXXXXXXX"
// Telegram BOT Token (Get from Botfather)
#define BOT_TOKEN "XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX"
#define USER_ID "XXXXXXX"

const unsigned long BOT_MTBS = 1000; // mean time between scan messages

X509List cert(TELEGRAM_CERTIFICATE_ROOT);
WiFiClientSecure secured_client;
UniversalTelegramBot bot(BOT_TOKEN, secured_client);
unsigned long bot_lasttime; // last time messages' scan has been done

void ask_for_stats(){
  Serial.println("mcu_send_stats");
}

void handleNewMessages(int numNewMessages)
{
  for (int i = 0; i < numNewMessages; i++)
  {
    if(bot.messages[i].text.indexOf("stats")>=0){
      ask_for_stats();
    }
    else bot.sendMessage(bot.messages[i].chat_id, bot.messages[i].text, "");
  }
}

void setup()
{
  Serial.begin(115200);
  Serial.println();

  // attempt to connect to Wifi network:
  Serial.print("Connecting to Wifi SSID ");
  Serial.print(WIFI_SSID);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  secured_client.setTrustAnchors(&cert); // Add root certificate for api.telegram.org
  
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(500);
  }
  Serial.print("\nWiFi connected. IP address: ");
  Serial.println(WiFi.localIP());

  Serial.print("Retrieving time: ");
  configTime(0, 0, "pool.ntp.org"); // get UTC time via NTP
  time_t now = time(nullptr);
  while (now < 24 * 3600)
  {
    Serial.print(".");
    delay(100);
    now = time(nullptr);
  }
  Serial.println(now);
  bot.sendMessage(USER_ID, "Dustbin booted up.");
}

void loop()
{
  if (Serial.available() > 0){
    // read the incoming string from the Arduino
    String incomingString = Serial.readString();

    // If the incoming string from the Arduino equals "change" then we toggle the LED
    if (incomingString.indexOf("send_string")>=0){
      Serial.println("Sending to tg...");
      String message = incomingString.substring(12);
      bot.sendMessage(USER_ID, message);                    
    }
  }

  if (millis() - bot_lasttime > BOT_MTBS)
  {
    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);

    while (numNewMessages)
    {
      Serial.println("got response");
      handleNewMessages(numNewMessages);
      numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    }

    bot_lasttime = millis();
  }
}