import paho.mqtt.client as mqtt
import json
import time

# --- Setup ---
BROKER = ""
USER = ""
PASS = ""

# Topics for Replication
TOPIC_RAW = "factory/zone_A/po_status"
TOPIC_RT  = "factory/zone_A/realtime"
TOPIC_HIS = "factory/zone_A/history"

def on_message(client, userdata, msg):
    try:
        # Reflection processing
        payload = json.loads(msg.payload.decode())
        payload["master_timestamp"] = time.time() # Contextualization
        
        # Replication: Freeing the PO from multiple requests
        serialized = json.dumps(payload)
        client.publish(TOPIC_RT, serialized, qos=0)
        client.publish(TOPIC_HIS, serialized, qos=1)
        
        print(f"🔄 Replicated PO state at {payload['master_timestamp']}")
    except Exception as e:
        print(f"Error: {e}")

# Client Initialization with v2.0 fix
client = mqtt.Client(mqtt.CallbackAPIVersion.VERSION1, "Master_LO_Orchestrator")
client.username_pw_set(USER, PASS)
client.tls_set()

client.on_message = on_message
client.connect(BROKER, 8883)
client.subscribe(TOPIC_RAW)

print("🚀 Master Logical Object Active...")
client.loop_forever()