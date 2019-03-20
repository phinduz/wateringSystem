#!/usr/bin/python3

import logging
from modules.io import IO
from modules.io import Pump
from modules.io import State
from modules.io import PumpMode
from modules.io import init_serial_bus
from modules.plant import Plant



import yaml


def main():

    logging.basicConfig(format='%(levelname)s %(asctime)s %(message)s',
                        level=logging.INFO)

    config_file = 'plant_configuration.yaml'
    configuration = load_configuration(config_file)

    # Init serial bus
    sb = init_serial_bus()
    plants = setup_configuration(configuration, sb)

    plants.get('Grape').read_moist_sensor()
    plants.get('Grape').read_light_sensor()
    plants.get('Grape').read_temperature_sensor()
    plants.get('Cactus').read_humidity_sensor()
    plants.get('Grape').run_pump(PumpMode.TIME, 10)
    plants.get('Palm').run_pump(PumpMode.CENTILITER, 42)


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

    return plant_object_dict


def load_configuration(config_file):
    '''Return configuration dict from config file'''
    with open(config_file, 'r') as yamlfile:
        cfg = yaml.load(yamlfile, Loader=yaml.SafeLoader)
    return cfg


if __name__ == "__main__":
    main()
