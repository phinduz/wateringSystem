#!/usr/bin/python3

import logging
import serial
from modules.io import IO
from modules.io import Pump

import yaml


def main():

    logging.basicConfig(format='%(levelname)s %(asctime)s %(message)s',
                        level=logging.DEBUG)

    config_file = 'plant_configuration.yaml'
    configuration = load_configuration(config_file)

    # Dummy serial bus
    sb = 'Serial bus object'
    setup_configuration(configuration, sb)


def setup_configuration(cfg, sb):
    '''Setup IO and plants according to configuration file'''

    # Create dict containing all IO objects
    io_object_dict = dict()
    for io_name, specification in cfg.get('io').items():
        io = IO(io_name, specification, sb)
        io_object_dict.update({io_name: io})

    for pump_name, specification in cfg.get('pump').items():
        pump = Pump(pump_name, specification, sb, io_object_dict)
        io_object_dict.update({pump_name: pump})

    # Create dict containing all plant objects
    plant_object_dict = dict()
    for plant_name, specification in cfg.get('plants').items():
        plant = Plant(plant_name, specification, io_object_dict)
        plant_object_dict.update({plant_name: plant})


def load_configuration(config_file):
    '''Return configuration dict from config file'''
    with open(config_file, 'r') as yamlfile:
        cfg = yaml.load(yamlfile, Loader=yaml.SafeLoader)
    return cfg


class Plant:

    # Keep track of all plants
    instances = []

    def __init__(self, name, spec, io_object_dict):
        '''Plant Constructor'''

        # Retrieve plant data from configuration
        self._data = dict({'name': name})

        for sensor_type, sensor_name in spec.get('sensors').items():
            if sensor_name in io_object_dict.keys():
                if sensor_type == 'moist':
                    self._data.update({'moist_sensor':
                                      io_object_dict.get(sensor_name)})
                if sensor_type == 'temperature':
                    self._data.update({'temperature_sensor':
                                      io_object_dict.get(sensor_name)})
                if sensor_type == 'light':
                    self._data.update({'light_sensor':
                                      io_object_dict.get(sensor_name)})
            else:
                s = 'Sensor: {} in plant: {} not defined in io list'
                logging.warning(s.format(sensor_name, name))
        for actuator_type, actuator_name in spec.get(
                'actuators').items():
            if actuator_name in io_object_dict.keys():
                if actuator_type == 'relay':
                    self._data.update({'relay':
                                      io_object_dict.get(actuator_name)})
                if actuator_type == 'pump':
                    self._data.update({'pump':
                                      io_object_dict.get(actuator_name)})
            else:
                s = 'Actuator: {} in plant: {} not defined in io list'
                logging.warning(s.format(actuator_name, name))
        s = 'Added Plant: {} with {}'
        logging.debug(s.format(self.get_name(), self.get_IO_string()))
        # Add to list of class instances
        Plant.instances.append(self)

    def __del__(self):
        '''Destructor, being called at deletion of plant.'''
        # Make sure all IO is turned of here
        print('Destructor Deleting object - {}'.format(self.get_name()))

    def get_IO_string(self):
        '''Return all plant IO as a readable string'''
        s = ''
        if self.get_pump():
            s += self.get_pump().get_name()
        if self.get_relay():
            s += ', ' + self.get_relay().get_name()
        if self.get_moist_sensor():
            s += ', ' + self.get_moist_sensor().get_name()
        if self.get_temperature_sensor():
            s += ', ' + self.get_temperature_sensor().get_name()
        if self.get_light_sensor():
            s += ', ' + self.get_light_sensor().get_name()
        return s

    def get_name(self):
        '''Return nae of plant'''
        return self._data.get('name')

    def get_moist_sensor(self):
        '''Return moist sensor object'''
        return self._data.get('moist_sensor')

    def get_temperature_sensor(self):
        '''Return temperature sensor object'''
        return self._data.get('temperature_sensor')

    def get_light_sensor(self):
        '''Return light sensor object'''
        return self._data.get('light_sensor')

    def get_relay(self):
        '''Return relay object'''
        return self._data.get('relay')

    def get_pump(self):
        '''Return pump object'''
        return self._data.get('pump')

    def read_sensor_value(self, sensor):
        '''Default method for reading sensor values'''
        if sensor:
            status, value = sensor.get_value()
            if status:
                logging.warning('Failed to read sensor value.')
                return None
            return value
        else:
            s = 'Tried to read sensor, but {} has no such sensor.'
            logging.warning(s.format(self.get_name()))
            return None

    def read_moist_sensor(self):
        '''Read value from moist sensor.'''
        return self.read_sensor_value(self.get_moist_sensor())

    def read_light_sensor(self):
        '''Read value from moist sensor.'''
        return self.read_sensor_value(self.get_light_sensor())

    def read_temperature_sensor(self):
        '''Read value from moist sensor.'''
        return self.read_sensor_value(self.get_temperature_sensor())

if __name__ == "__main__":
    main()
