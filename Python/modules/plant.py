import datetime
import logging
import time
from modules.io import State
from modules.io import CmdIO


class Plant:
    '''Class handling a plant.'''

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
                if sensor_type == 'humidity':
                    self._data.update({'humidity_sensor':
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
        # print('Destructor Deleting object - {}'.format(self.get_name()))

    def get_IO_string(self):
        '''Return all plant IO as a readable string'''
        s = ''
        if self._get_pump():
            s += self._get_pump().get_name()
        if self._get_moist_sensor():
            s += ', ' + self._get_moist_sensor().get_name()
        if self._get_temperature_sensor():
            s += ', ' + self._get_temperature_sensor().get_name()
        if self._get_humidity_sensor():
            s += ', ' + self._get_humidity_sensor().get_name()
        if self._get_light_sensor():
            s += ', ' + self._get_light_sensor().get_name()
        return s

    def get_name(self):
        '''Return name of plant'''
        return self._data.get('name')

    def _get_moist_sensor(self):
        '''Return moist sensor object'''
        return self._data.get('moist_sensor')

    def _get_temperature_sensor(self):
        '''Return temperature sensor object'''
        return self._data.get('temperature_sensor')

    def _get_humidity_sensor(self):
        '''Return humidity sensor object'''
        return self._data.get('humidity_sensor')

    def _get_light_sensor(self):
        '''Return light sensor object'''
        return self._data.get('light_sensor')

    def _get_pump(self):
        '''Return pump object'''
        return self._data.get('pump')

    def _read_sensor_value(self, sensor, serial_bus):
        '''Default method for reading sensor values'''
        if sensor:
            t_start = time.time()
            data = sensor.get_value(serial_bus)
            t_stop = time.time()
            status = data.get('status')
            if status:
                s = '{}; Sensor: {} failed to read value.'
                logging.warning(s.format(self.get_name(), sensor.get_name()))
            value = data.get(str(CmdIO.VALUE))
            time_elapsed = round(t_stop - t_start, 2)
            datetime_string = datetime.datetime.today().strftime('%Y-%m-%dT%H-%M-%S')
            s = 'Plant: {} was measured with: {}: {}'
            logging.info(s.format(self.get_name(), sensor.get_name(), value))
            sensor_data = dict({'plant': self.get_name(), 'time': time_elapsed,
                                'sensor': sensor.get_name(),
                                'value': value, 'date': datetime_string})
            return sensor_data
        else:
            s = 'Tried to read sensor, but {} has no such sensor.'
            logging.warning(s.format(self.get_name()))
            return None

    def read_moist_sensor(self, serial_bus):
        '''Read value from moist sensor.'''
        return self._read_sensor_value(self._get_moist_sensor(), serial_bus)

    def read_light_sensor(self, serial_bus):
        '''Read value from lght sensor.'''
        return self._read_sensor_value(self._get_light_sensor(), serial_bus)

    def read_temperature_sensor(self, serial_bus):
        '''Read value from temperature sensor.'''
        return self._read_sensor_value(self._get_temperature_sensor(), serial_bus)

    def read_humidity_sensor(self, serial_bus):
        '''Read value from humidity sensor.'''
        return self._read_sensor_value(self._get_humidity_sensor(), serial_bus)

    def run_pump(self, mode, value, serial_bus):
        '''Run pump.'''

        pump = self._get_pump()
        t_start = time.time()
        data = pump.run_pump(mode, value, serial_bus)
        t_stop = time.time()
        status = data.get('status')
        if status:
            s = '{}; Pump: {} failed to run, returned message: {}'
            logging.warning(s.format(self.get_name(), pump.get_name(),
                            data))
            return None
        s = 'Plant: {} was watered with: {} pulses for {} seconds'
        pulses = data.get(str(CmdIO.VALUE))
        time_elapsed = round(t_stop - t_start, 2)
        logging.info(s.format(self.get_name(), pulses, time_elapsed))
        datetime_string = datetime.datetime.today().strftime('%Y-%m-%dT%H-%M-%S')
        water_data = dict({'plant': self.get_name(), 'time': time_elapsed,
                           'pump': pump.get_name(), 'pulses': pulses,
                           'date': datetime_string})
        return water_data

    def _set_relay(self, value, serial_bus):
        '''Set relay on or off.'''

        # Warn about faulty usage
        if not isinstance(value, State):
            s = '{}, tried to set_relay with faulty argument: {}'
            logging.warning(s.format(self.get_name(), value))
            return
        if self._get_relay():
            status, message = self._get_relay().set_value(value, serial_bus)
            return status
        else:
            s = '{}, tried to set_relay but has no relay configured.'
            logging.warning(s.format(self.get_name()))
