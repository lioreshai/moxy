import os
import configparser
config = configparser.ConfigParser()
config.read(os.path.dirname(os.path.realpath(__file__)) + '/config.ini')

from concurrent.futures import ThreadPoolExecutor
import time
import smbus
import paho.mqtt.client as mqtt
import asyncio
import sys
import asyncio
from driver_encoder import Encoder
import RPi.GPIO as GPIO

GPIO.setmode(GPIO.BCM)

bus= smbus.SMBus(1)
picoADDR = 0x08

meterUpdate = [0,0,0,0,0,0,0,0]

def run_interface():
    import driver_lcd
    mylcd = driver_lcd.lcd()
    mylcd.lcd_display_string("Hello World!", 1)

    def valueChanged(value, direction):
        mylcd.lcd_display_string(str(value) + " " + direction, 2)
        # print("* New value: {}, Direction: {}".format(value, direction))
    e1 = Encoder(5, 26, valueChanged)

def update_meters():
    while True:
        # print("Writing update")
        # print(meterUpdate)
        # sys.stdout.flush()
        bus.write_i2c_block_data(picoADDR, 100, meterUpdate)
        time.sleep(1)

def on_connect(client, flags, rc, properties):
    print('Panel service connected to MQTT')    
    sys.stdout.flush()
    client.subscribe('moxy/meters/#')

def on_message(client, userdata, message):
    meterId = message.topic.split('/')[2]
    # print(message.payload)
    # sys.stdout.flush()
    payload = message.payload.decode("utf-8").split(':')
    levelIndex = (int(meterId)-1)*2
    hgIndex = levelIndex+1
    meterUpdate[levelIndex] = int(payload[0])
    meterUpdate[hgIndex] = int(payload[1])

def run_mqtt():
    client = mqtt.Client()
    client.on_connect = on_connect
    client.on_message = on_message
    client.connect(config['MQTT']['ServerHost'], int(config['MQTT']['ServerPort']))
    client.loop_forever()

async def main():
    print('Starting panel interface service.')    
    sys.stdout.flush()
    loop.run_in_executor(p, run_mqtt)
    loop.run_in_executor(p, update_meters)
    loop.run_in_executor(p, run_interface)

loop = asyncio.get_event_loop()

p = ThreadPoolExecutor(3)
loop.run_until_complete(main())