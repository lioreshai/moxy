import os
import configparser
config = configparser.ConfigParser()
config.read(os.path.dirname(os.path.realpath(__file__)) + '/config.ini')

import smbus
import time
import paho.mqtt.client as mqtt

mqttc = mqtt.Client()
mqttc.connect(config['MQTT']['ServerHost'], int(config['MQTT']['ServerPort']), int(config['MQTT']['KeepAlive']))
mqttc.loop_start()

bus= smbus.SMBus(1)
picoADDR = 0x08
xiaoADDR = 0x09

def getReadingFromBytes(data, sensorId):
    highIndex = (sensorId-1)*2
    lowIndex = highIndex+1
    reading = data[lowIndex] | ( data[highIndex] << 8 )
    return round(max(0,((reading*-1)+3153)/130),2)

while True:
    response = bus.read_i2c_block_data(xiaoADDR,0x00,8)
    mqttc.publish("pumanage/current/1", getReadingFromBytes(response, 1))
    mqttc.publish("pumanage/current/2", getReadingFromBytes(response, 2))
    mqttc.publish("pumanage/current/3", getReadingFromBytes(response, 3))
    mqttc.publish("pumanage/current/4", getReadingFromBytes(response, 4))
    time.sleep(.4)
    response = bus.read_i2c_block_data(picoADDR,12,4)
    # print("".join(map(chr, response)))
    # print(response)
    # # sys.stdout.flush()
    mqttc.publish("pumanage/temp", response[0])
    mqttc.publish("pumanage/humidity", response[1]+21)
    mqttc.publish("pumanage/gas", response[2]*5)
    time.sleep(.4)

