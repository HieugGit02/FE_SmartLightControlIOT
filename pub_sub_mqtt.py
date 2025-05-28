import paho.mqtt.client as mqtt
import ssl

# MQTT Broker Info
MQTT_BROKER = "21583ec947f6453f8e3793f0b34dde0b.s1.eu.hivemq.cloud"
MQTT_PORT = 8883  # TLS port
MQTT_USER = "testmse21"
MQTT_PASS = "Testmse21dn!"
MQTT_TOPIC = "lamp/state"

# ---- Subscriber Function ----
def test_mqtt_subscribe():
    def on_connect(client, userdata, flags, rc):
        print("Connected with result code", rc)
        client.subscribe(MQTT_TOPIC)

    def on_message(client, userdata, msg):
        print(f"Received on {msg.topic}: {msg.payload.decode()}")

    client = mqtt.Client()
    client.username_pw_set(MQTT_USER, MQTT_PASS)
    client.tls_set(cert_reqs=ssl.CERT_NONE)
    client.tls_insecure_set(True)  # Accept self-signed certs for testing

    client.on_connect = on_connect
    client.on_message = on_message

    client.connect(MQTT_BROKER, MQTT_PORT, 60)
    client.loop_forever()


# ---- Publisher Function ----
def test_mqtt_publish(value):
    assert value in (0, 1), "Value must be 0 or 1"
    client = mqtt.Client()
    client.username_pw_set(MQTT_USER, MQTT_PASS)
    client.tls_set(cert_reqs=ssl.CERT_NONE)
    client.tls_insecure_set(True)

    client.connect(MQTT_BROKER, MQTT_PORT, 60)
    client.publish(MQTT_TOPIC, str(value))
    client.disconnect()
    print(f"Published: {value} to topic {MQTT_TOPIC}")

test_mqtt_subscribe()
