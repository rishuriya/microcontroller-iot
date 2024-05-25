#include <Arduino.h>
#include <WiFi.h>

#include <FirebaseClient.h>
#include <ArduinoJson.h>
#define WIFI_SSID "Wokwi-GUEST"
#define WIFI_PASSWORD ""

// The API key can be obtained from Firebase console > Project Overview > Project settings.
#define API_KEY "<SECRET KEY>"
#define DATABASE_URL "<db_URL>"

void asyncCB(AsyncResult &aResult);

void printResult(AsyncResult &aResult);

void handlePatchEvent(const char* result);

void handlePutEvent(const char* result);

DefaultNetwork network;

NoAuth no_auth;


FirebaseApp app;
#include <WiFiClientSecure.h>
WiFiClientSecure ssl_client;

using AsyncClient = AsyncClientClass;

AsyncClient aClient(ssl_client, getNetwork(network));

RealtimeDatabase Database;

bool taskComplete = false;

void setup()
{

  Serial.begin(115200);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  pinMode(2, OUTPUT);
  pinMode(0, OUTPUT);
  pinMode(4, OUTPUT);
  pinMode(15, OUTPUT);
  digitalWrite(2, LOW);
  digitalWrite(0, HIGH);
  digitalWrite(4, LOW);
  digitalWrite(15, LOW);

  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(300);
  }
  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();

  Firebase.printf("Firebase Client v%s\n", FIREBASE_CLIENT_VERSION);

  Serial.println("Initializing app...");

  ssl_client.setInsecure();

  initializeApp(aClient, app, getAuth(no_auth), asyncCB, "authTask");
  Serial.println("Initalized");
  app.getApp<RealtimeDatabase>(Database);

  Database.url(DATABASE_URL);
}

void loop()
{
  // The async task handler should run inside the main loop
  // without blocking delay or bypassing with millis code blocks.

  app.loop();

  Database.loop();

  if (app.ready() && !taskComplete)
  {
    taskComplete = true;
    Database.setSSEFilters("get,put,patch,keep-alive,cancel,auth_revoked");
    Database.get(aClient, "/test", asyncCB, true, "getTask2");

  }
}

void asyncCB(AsyncResult &aResult)
{

  printResult(aResult);
}

void printResult(AsyncResult &aResult)
{
  if (aResult.isEvent())
  {
    Firebase.printf("Event task: %s, msg: %s, code: %d\n", aResult.uid().c_str(), aResult.appEvent().message().c_str(), aResult.appEvent().code());
  }

  if (aResult.isDebug())
  {
    Firebase.printf("Debug task: %s, msg: %s\n", aResult.uid().c_str(), aResult.debug().c_str());
  }

  if (aResult.isError())
  {
    Firebase.printf("Error task: %s, msg: %s, code: %d\n", aResult.uid().c_str(), aResult.error().message().c_str(), aResult.error().code());
  }

  if (aResult.available())
  {

    const char* result = aResult.c_str();
    Serial.print("Raw result: ");
    Serial.println(aResult.c_str());


    const char* eventKey = "event:";
    char* eventStart = strstr(result, eventKey);
    if (eventStart) {
      eventStart += strlen(eventKey); // Move past "event\":"

      // Find the end of the event type string
      char* eventEnd = strstr(eventStart, "\n");
      if (eventEnd) {
        // Extract the event type
        String event = String(eventStart).substring(0, eventEnd - eventStart);
        event.trim(); // Remove leading/trailing spaces

        if (event == "put") {
          // Handle "put" event
          handlePutEvent(result);
        } else if (event == "patch") {
          // Handle "patch" event
          handlePatchEvent(result);
        } else {
          Serial.println("Error: Unknown event type");
        }
      } else {
        Serial.println("Error: End of event type not found");
      }
    } else {
      Serial.println("Error: Event key not found");
    }
}
}

void handlePutEvent(const char* result) {
  // Find the index of "data:" and extract the data portion
  const char* dataKey = "\"data\":";
  char* dataStart = strstr(result, dataKey);
  if (dataStart)
  {
    dataStart += strlen(dataKey); // Move past "data\":"

    // Find the correct ending of the JSON object
    int braceCount = 0;
    char* dataEnd = dataStart;
    while (*dataEnd != '\0')
    {
      if (*dataEnd == '{')
        braceCount++;
      else if (*dataEnd == '}')
        braceCount--;

      dataEnd++;

      if (braceCount == 0)
        break;
    }

    if (braceCount == 0)
    {
      String data = String(dataStart).substring(0, dataEnd - dataStart);
      Serial.print("Extracted data: ");
      Serial.println(data);

      // Parse the JSON data
      StaticJsonDocument<400> doc; // Adjust size as necessary
      DeserializationError error = deserializeJson(doc, data);
      if (error)
      {
        Serial.print("deserializeJson() failed: ");
        Serial.println(error.c_str());
        return;
      }

      // Iterate over the JSON object and set pin states accordingly
      JsonObject dataObj = doc.as<JsonObject>();
      for (JsonPair kv : dataObj)
      {
        const char* key = kv.key().c_str();
        JsonObject pinObj = kv.value().as<JsonObject>();

        // Check if the pin key is numeric
        if (isDigit(key[0]))
        {
          int pin = atoi(key);
          const char* state = pinObj["state"];
          const char* name = pinObj["name"];

          if (state && name)
          {
            Serial.print("Pin: ");
            Serial.print(pin);
            Serial.print(", Name: ");
            Serial.print(name);
            Serial.print(", State: ");
            Serial.println(state);

            if (strcmp(state, "LOW") == 0)
            {
              digitalWrite(pin, LOW);
            }
            else if (strcmp(state, "HIGH") == 0)
            {
              digitalWrite(pin, HIGH);
            }
            else
            {
              Serial.print("Error: Invalid state for pin ");
              Serial.println(pin);
            }
          }
          else
          {
            Serial.print("Error: State or name for pin ");
            Serial.print(pin);
            Serial.println(" is missing or invalid");
          }
        }
        else
        {
          Serial.print("Error: Invalid pin number ");
          Serial.println(key);
        }
      }
    }
    else
    {
      Serial.println("Error: Mismatched braces in JSON data");
    }
  }
  else
  {
    Serial.println("Error: Data key not found in result");
  }
}


void handlePatchEvent(const char* result) {
  Serial.print("Raw result: ");
  Serial.println(result);

 const char* dataKey = "data: ";
char* dataStart = strstr(result, dataKey);
if (dataStart) {
  dataStart += strlen(dataKey); // Move past "data\":"

  // Find the correct ending of the JSON object (considering nested braces)
  int braceCount = 0;
  char* dataEnd = dataStart;
  while (*dataEnd != '\0') {
    if (*dataEnd == '{' || *dataEnd == '[') { 
      braceCount++;
    } else if (*dataEnd == '}' || *dataEnd == ']') {
      braceCount--;
    }

    dataEnd++;

    if (braceCount == 0) {
      break;
    }
  }

  if (braceCount == 0) { 
    // Extract the data portion
    String data = String(dataStart).substring(0, dataEnd - dataStart);
    Serial.print("Extracted data: ");
    Serial.println(data);

    // Parse the JSON data
    StaticJsonDocument<200> doc; // Adjust size as necessary
    DeserializationError error = deserializeJson(doc, data);

    if (error) {
      Serial.print("deserializeJson() failed: ");
      Serial.println(error.c_str());
      return;
    }

    // Extract the path and state (accessing "state" correctly)
    const char* path = doc["path"];
    const char* state = doc["data"]["state"]; 
    String pinString = String(path).substring(1);
    int pin = pinString.toInt();
      if (path && state)
      {
        if (strcmp(state, "LOW") == 0)
            {
              digitalWrite(pin, LOW);
            }
            else if (strcmp(state, "HIGH") == 0)
            {
              digitalWrite(pin, HIGH);
            }
            else
            {
              Serial.print("Error: Invalid state for pin ");
              Serial.println(pin);
            }
      }
      else
      {
        Serial.println("Error: Path or state missing or invalid");
      }
    }
    else
    {
      Serial.println("Error: Mismatched braces in JSON data");
    }
  }
  else
  {
    Serial.println("Error: Data key not found in result");
  }
}