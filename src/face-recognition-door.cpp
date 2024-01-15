#include <ESP32Servo.h>
#include <PubSubClient.h>
#include <WiFi.h>

#define LED_PIN 26
#define TRIG_PIN 27
#define ECHO_PIN 14
#define SERVO_PIN 12

Servo myservo;
WiFiClient espClient;
PubSubClient client(espClient);

const char *mqtt_broker = "broker.emqx.io";
const char *topic_mqtt = "MSIB5/Bangkit/face_recognition";
const char *mqtt_username = "emqx";
const char *mqtt_password = "public";
const int mqtt_port = 1883;

bool gate_opened = false;
bool on_passing = false;

void open_gate()
{
  gate_opened = true;
  Serial.println("Gate Opened!");
  myservo.write(0);
  client.publish("MSIB5/Bangkit/gate_status", "True");
}

void close_gate()
{
  gate_opened = false;
  Serial.println("Gate Closed!");
  myservo.write(90);
  client.publish("MSIB5/Bangkit/gate_status", "False");
}

void callback(char *topic, byte *payload, unsigned int length)
{
  Serial.print("Pesan baru dari topik: ");
  Serial.println(topic);

  if (strcmp(topic, "MSIB5/Bangkit/face_recognition") == 0)
  {
    payload[length] = '\0'; // Tambahkan null terminator untuk mengonversi ke string

    // degree = atoi((char *)payload);  // Konversi string menjadi integer
    open_gate();
  }
}

void setup()
{
  Serial.begin(115200);

  myservo.attach(SERVO_PIN);
  myservo.write(90);
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(LED_PIN, OUTPUT);

  WiFi.begin("Wokwi-GUEST", "");
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(100);
    Serial.print(".");
  }
  Serial.println(" Connected!");
  client.setServer(mqtt_broker, mqtt_port);
  client.setCallback(callback);
  while (!client.connected())
  {
    String client_id = "esp32-client-";
    client_id += String(WiFi.macAddress());
    Serial.print("The client ");
    Serial.print(client_id);
    Serial.println(" connects to the public mqtt broker");
    if (client.connect(client_id.c_str(), mqtt_username, mqtt_password))
    {
      Serial.println("Public emqx mqtt broker connected");
    }
    else
    {
      Serial.print("failed with state ");
      Serial.println(client.state());
      delay(2000);
    }
  }

  client.subscribe("MSIB5/Bangkit/face_recognition");
}

void loop()
{
  if (gate_opened)
  {
    long duration, distance;
    // Mengirim sinyal pulse ke sensor ultrasonik
    digitalWrite(TRIG_PIN, LOW);
    delayMicroseconds(2);
    digitalWrite(TRIG_PIN, HIGH);
    delayMicroseconds(10);
    digitalWrite(TRIG_PIN, LOW);

    // Membaca durasi sinyal echo
    duration = pulseIn(ECHO_PIN, HIGH);

    // Menghitung jarak berdasarkan durasi echo
    distance = (duration / 2) * 0.0343;

    // Menampilkan hasil ke Serial Monitor
    Serial.print("Jarak: ");
    Serial.print(distance);
    Serial.println(" cm");
    if (distance < 50)
    {
      on_passing = true;
      digitalWrite(LED_PIN, HIGH);
      // tone(BUZZER_PIN, 1000);
    }
    else if (on_passing)
    {
      on_passing = false;
      digitalWrite(LED_PIN, LOW);
      // noTone(BUZZER_PIN);
      close_gate();
    }
    delay(500);
  }
  else
  {
    client.loop();
  }
}