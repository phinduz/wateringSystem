#!/usr/bin/python3

import argparse
import datetime
import logging
import os
import time
import yaml
from multiprocessing import Process, Manager, Queue
from modules.io import IO
from modules.io import Pump
from modules.plant import Plant
from modules.control_monitor import control_monitor
from modules.log_data_handler import log_data


datetime_string = datetime.datetime.today().strftime('%Y-%m-%dT%H-%M-%S')

# Argument parser options
parser = argparse.ArgumentParser(
    description='Run water control application.')
parser.add_argument('-c', '--configfile', type=str,
                    default='plant_configuration.yaml', dest='configfile',
                    help='set plant configuration file')
parser.add_argument('--com', '--comport', type=int, default=3, dest='comport',
                    help='set which com port to be used towards arduino')
parser.add_argument('-d', '--debugfile', type=str,
                    default=datetime_string + '_debug.log',
                    help='set debug log file name')
parser.add_argument('-dl', '--debuglevel', type=int,
                    default=logging.DEBUG,
                    help='set debug log level')
parser.add_argument('-l', '--logfile', type=str,
                    default=datetime_string,
                    help='set signal output log file name')
args = parser.parse_args()

logger = logging.getLogger()
logger.setLevel(logging.DEBUG)
# Create file handler to log debug messages
fh = logging.FileHandler(os.path.join('data', args.debugfile))
fh.setLevel(logging.DEBUG)
# Create console handler to log info messages
ch = logging.StreamHandler()
ch.setLevel(logging.INFO)
# Create file and console formatter and add it to the handlers
formatter_console = logging.Formatter(
    '%(module)-25s | %(levelname)-8s | %(message)s')
formatter_file = logging.Formatter(
    '%(levelname)s | %(module)s | %(asctime)s | %(message)s')
ch.setFormatter(formatter_console)
fh.setFormatter(formatter_file)
# Add the handlers to logger
logger.addHandler(ch)
logger.addHandler(fh)


def main():
    """Main process application."""

    s = 'Main process started with argument vector: {}'
    logger.info(s.format(vars(args)))
    configuration = load_configuration(args.configfile)

    with Manager() as manager:
        process_data = manager.dict()
        plant_data = manager.dict()
        meas_log_q = Queue()
        water_log_q = Queue()

        process_data['file_prefix'] = args.logfile
        process_data['com_port'] = 'com' + str(args.comport)
        process_data['abort'] = False


        plants = setup_configuration(configuration)

        plant_data.update(plants)

        log_data_handler_p = Process(target=log_data,
                                     args=(process_data, meas_log_q,
                                           water_log_q))
        log_data_handler_p.start()

        control_monitor_handler_p = Process(target=control_monitor,
                                            args=(process_data, plant_data,
                                                  meas_log_q, water_log_q))
        control_monitor_handler_p.start()

        while not process_data['abort']:
            pass

        log_data_handler_p.join()
        control_monitor_handler_p.join()


def setup_configuration(cfg):
    '''Setup IO and plants according to configuration file'''

    # Create dict containing all IO objects
    io_object_dict = dict()
    for io_name, specification in cfg.get('io').items():
        io = IO(io_name, specification)
        io_object_dict.update({io_name: io})

    for pump_name, specification in cfg.get('pump').items():
        pump = Pump(pump_name, specification, io_object_dict)
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
    logging.info('Loaded configuration file: {}'.format(config_file))
    logging.debug('Loaded configuration: {}'.format(cfg))
    return cfg


if __name__ == "__main__":
    main()
