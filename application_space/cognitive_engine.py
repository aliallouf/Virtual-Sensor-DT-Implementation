import paho.mqtt.client as mqtt
import json

# --- Setup ---
BROKER = ""
USER = ""
PASS = ""

TOPIC_SUB = "factory/zone_A/realtime"
TOPIC_CMD = "factory/zone_A/actuator_cmds"

def on_message(client, userdata, msg):
    try:
        data = json.loads(msg.payload.decode())
        temp = data["thermal"]["temp"]
        
        # Cognitive logic for Strong Entanglement
        if temp > 25.0:
            client.publish(TOPIC_CMD, json.dumps({"servo_angle": 90}))
            print(f"⚠️ High Temp ({temp}°C): Cooling ON")
        else:
            client.publish(TOPIC_CMD, json.dumps({"servo_angle": 0}))
            print(f"✅ Temp OK ({temp}°C): Cooling OFF")
            
    except Exception as e:
        print(f"Logic Error: {e}")

app = mqtt.Client(mqtt.CallbackAPIVersion.VERSION1, "App_Cognitive_Engine")
app.username_pw_set(USER, PASS)
app.tls_set()

app.on_message = on_message
app.connect(BROKER, 8883)
app.subscribe(TOPIC_SUB)

print("🧠 Application Space Monitoring Zone A...")
app.loop_forever()