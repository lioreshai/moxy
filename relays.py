import asyncio
import os
import configparser
config = configparser.ConfigParser()
config.read(os.path.dirname(os.path.realpath(__file__)) + '/config.ini')

from concurrent.futures import ThreadPoolExecutor
import paho.mqtt.client as mqtt
import paho.mqtt.subscribe as subscribe
import RPi.GPIO as GPIO
import time
import sys

GPIO.setmode(GPIO.BCM)
GPIO.setwarnings(False)
switchPins = [17,27,22,23]
GPIO.setup(switchPins[0], GPIO.OUT)
GPIO.setup(switchPins[1], GPIO.OUT)
GPIO.setup(switchPins[2], GPIO.OUT)
GPIO.setup(switchPins[3], GPIO.OUT)

def on_connect(client, flags, rc, properties):
    print('Relay service connected to MQTT for switch subscription')    
    sys.stdout.flush()
    client.subscribe("moxy/power/switch/+/set")

def on_message(client, userdata, message):
    circuitId = int(message.topic.split("/")[3])
    print(switchPins[circuitId-1])
    if message.payload == b'ON':
        GPIO.output(switchPins[circuitId-1],True)
        client.publish(f"moxy/power/switch/{circuitId}", "ON")
    elif message.payload == b'OFF':
        print(message)
        GPIO.output(switchPins[circuitId-1],False)
        client.publish(f"moxy/power/switch/{circuitId}", "OFF")

def run_subscription_mqtt():
    client = mqtt.Client()
    client.on_connect = on_connect
    client.on_message = on_message
    client.connect(config['MQTT']['ServerHost'], int(config['MQTT']['ServerPort']))
    client.loop_forever()

def run_status_publish_mqtt():
    client = mqtt.Client()
    client.connect(config['MQTT']['ServerHost'], int(config['MQTT']['ServerPort']))
    print('Relay service connected to MQTT for switch status publishing') 
    while True:
        client.publish("moxy/power/switch/1", "ON" if GPIO.input(17) else "OFF")
        client.publish("moxy/power/switch/2", "ON" if GPIO.input(27) else "OFF")
        client.publish("moxy/power/switch/3", "ON" if GPIO.input(22) else "OFF")
        client.publish("moxy/power/switch/4", "ON" if GPIO.input(23) else "OFF")
        time.sleep(5)

async def main():
    print('Starting relay switching service.')    
    sys.stdout.flush()
    loop.run_in_executor(p, run_subscription_mqtt)
    loop.run_in_executor(p, run_status_publish_mqtt)

loop = asyncio.get_event_loop()

p = ThreadPoolExecutor(2)
loop.run_until_complete(main())