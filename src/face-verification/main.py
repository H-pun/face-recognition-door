from face_recognition import analysis
import paho.mqtt.client as mqtt

# Konfigurasi koneksi ke broker
broker_address = "broker.emqx.io"
port = 1883
username = "emqx"  # Ganti dengan username broker Anda jika diperlukan
password = "public"  # Ganti dengan password broker Anda jika diperlukan
gate_status = False

# Fungsi yang dipanggil ketika koneksi ke broker berhasil
def on_connect(client, userdata, flags, rc):
    print("Connected with result code "+str(rc))
    client.subscribe("MSIB5/Bangkit/gate_status")

# Fungsi yang dipanggil setelah publish berhasil
def on_publish(client, userdata, mid):
    print("Message Published")

# Fungsi yang dipanggil setiap kali pesan diterima dari broker
def on_message(client, userdata, msg):
    global gate_status
    gate_status = msg.payload.decode() == "True"
    print(f"Received message on topic {msg.topic}: {msg.payload.decode()}")

# Inisialisasi klien MQTT
client = mqtt.Client()
client.username_pw_set(username, password)
client.on_connect = on_connect
client.on_publish = on_publish
client.on_message = on_message

# Melakukan koneksi ke broker
client.connect(broker_address, port, 60)
client.loop_start()

def publish(label):
    print("Gate status", gate_status)
    if not gate_status:
        client.publish("MSIB5/Bangkit/face_recognition", True)

analysis("database", frame_threshold=30, time_threshold=3, callback=publish)

# Menutup koneksi ke broker setelah keluar dari loop
client.disconnect()
print("Koneksi ke broker ditutup.")
