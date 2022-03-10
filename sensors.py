import os
import configparser
config = configparser.ConfigParser()
config.read(os.path.dirname(os.path.realpath(__file__)) + '/config.ini')

import smbus
import time
import paho.mqtt.client as mqtt

#Initialize MQTT client for publishing metrics
mqttc = mqtt.Client()
mqttc.connect(config['MQTT']['ServerHost'], int(config['MQTT']['ServerPort']), int(config['MQTT']['KeepAlive']))
mqttc.loop_start()

#Initialize i2c bus and define MCU addresses
bus= smbus.SMBus(1)
picoADDR = 0x08
xiaoADDR = 0x09

#Reconstruct current ADC reading and convert to Amps
def getCurrentReadingFromBytes(data, sensorId):
    highIndex = (sensorId-1)*2
    lowIndex = highIndex+1
    reading = data[lowIndex] | ( data[highIndex] << 8 )
    #Normalize current reading from ADC
    return round(max(0,((reading*-1)+3153)/130),2)

while True:
    #Read current levels from xiao MCU
    response = bus.read_i2c_block_data(xiaoADDR,0x00,8)
    mqttc.publish("moxy/current/1", getCurrentReadingFromBytes(response, 1))
    mqttc.publish("moxy/current/2", getCurrentReadingFromBytes(response, 2))
    mqttc.publish("moxy/current/3", getCurrentReadingFromBytes(response, 3))
    mqttc.publish("moxy/current/4", getCurrentReadingFromBytes(response, 4))
    time.sleep(.4)
    #Get sensor readings from pico MCU
    response = bus.read_i2c_block_data(picoADDR,12,4)
    # print(response)
    # # sys.stdout.flush()
    mqttc.publish("moxy/temp", response[0])
    #Calibrated sensor values
    mqttc.publish("moxy/humidity", response[1]+21)
    mqttc.publish("moxy/gas", response[2]*5)
    time.sleep(.4)

