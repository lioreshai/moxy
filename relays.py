import paho.mqtt.client as mqtt
import paho.mqtt.subscribe as subscribe
#import the GPIO and time package
import RPi.GPIO as GPIO
import time
GPIO.setmode(GPIO.BCM)
GPIO.setwarnings(False)
switchPins = [17,27,22,23]
GPIO.setup(switchPins[0], GPIO.OUT)
GPIO.setup(switchPins[1], GPIO.OUT)
GPIO.setup(switchPins[2], GPIO.OUT)
GPIO.setup(switchPins[3], GPIO.OUT)

mqttc = mqtt.Client()
mqttc.connect("127.0.0.1", 1883, 60)
mqttc.loop_start()


def process_power_switches(client, userdata, message):
    circuitId = int(message.topic.split("/")[3])
    print(switchPins[circuitId-1])
    if message.payload == b'ON':
        GPIO.output(switchPins[circuitId-1],True)
        mqttc.publish(f"pumanage/power/switch/{circuitId}", "ON")
    elif message.payload == b'OFF':
        print(message)
        GPIO.output(switchPins[circuitId-1],False)
        mqttc.publish(f"pumanage/power/switch/{circuitId}", "OFF")


def on_message_print(client, userdata, message):
    print("%s %s" % (message.topic, message.payload))

subscribe.callback(process_power_switches, "pumanage/power/switch/+/set", hostname="127.0.0.1")

while True:
    mqttc.publish("pumanage/current/1", "ON" if GPIO.input(17) else "OFF")
    mqttc.publish("pumanage/current/2", "ON" if GPIO.input(27) else "OFF")
    mqttc.publish("pumanage/current/3", "ON" if GPIO.input(22) else "OFF")
    mqttc.publish("pumanage/current/4", "ON" if GPIO.input(23) else "OFF")
    time.sleep(1)