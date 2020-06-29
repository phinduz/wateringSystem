import json
import logging
import random
import serial
import time
from enum import Enum


class ClassIO(Enum):
    def __str__(self):
        return str(self.value)
    ANALOG = 'a'
    DIGITAL = 'd'
    I2C = 'i2c'
    UNINITIALIZED_CLASS = 'uninitialized_class'


class TypeIO(Enum):
    def __str__(self):
        return str(self.value)
    READ = 'read'
    WRITE = 'write'
    UNINITIALIZED_TYPE = 'uninitialized_type'


class CmdIO(Enum):
    def __str__(self):
        return str(self.value)
    COMMAND = 'cmd'
    ARGUMENTS = 'args'
    VALUE = 'value'
    AMOUNT = 'amt'
    RELAY = 'relay'
    TACHOMETER = 'tacho'
    PULSES = 'ps'
    TIME = 'time'
    READ = 'read'
    WRITE = 'write'
    PUMP = 'pump'
    INIT = 'init'


class State(Enum):
    def __str__(self):
        return str(self.value)
    OFF = 0
    ON = 1
    UNINITIALIZED = 'uninitialized'


class PumpMode(Enum):
    def __str__(self):
        return str(self.value)
    TIME = 'time'
    CENTILITER = 'centiliter'


class SB:
    def __str__(self):
        return str(self.value)

    def __init__(self, sb):
        self._serial_bus = sb


class IO:
    '''Basic class for all IO devices.'''

    # Keep track of all IO devices
    instances = []
    sb_default_timeout = 1

    def __init__(self, name, spec, sb):
        '''IO Constructor'''

        # Sensor configuration data
        self._serial_bus = sb
        self._data = dict({'name': name})
        for key, value in spec.items():
            if key == 'type':
                if value == 'read':
                    type_io = TypeIO.READ
                if value == 'write':
                    type_io = TypeIO.WRITE
                self._data.update({'type_io': type_io})
            if key == 'class':
                if value == 'analog':
                    class_io = ClassIO.ANALOG
                if value == 'digital':
                    class_io = ClassIO.DIGITAL
                if value == 'i2c':
                    class_io = ClassIO.I2C
                self._data.update({'class_io': class_io})
            if key == 'address' or key == 'pin':
                self._data.update({'address_io': value})
            if key == 'register':
                self._data.update({'register_io': value})
            if key == 'pulse_per_cl':
                self._data.update({'pulse_per_cl': value})
        # Add to list of class instances
        IO.instances.append(self)

    def get_name(self):
        '''Return name of IO device.'''
        return self._data.get('name')

    def get_address_io(self):
        '''Return pin or address of IO device.'''
        return self._data.get('address_io')

    def get_type_io(self):
        '''Return type of io, e.g. read, write.'''
        return self._data.get('type_io')

    def _get_class_io(self):
        '''Return class of io, e.g. analog, digital, i2c'''
        return self._data.get('class_io')

    def _get_register_io(self):
        '''Return register for i2c io'''
        return self._data.get('register_io')

    def _set_serial_bus_timeout(self, timeout):
        '''Set timeout on response from serial bus.'''
        self._serial_bus.timeout = timeout

    def get_value(self):
        '''Get value from IO device'''

        # Set timeout for response on serial bus.
        self._set_serial_bus_timeout(IO.sb_default_timeout)

        # Warn about faulty usage
        if self.get_type_io() is not TypeIO.READ:
            s = '{}, tried to get_value, but is not configured as {}'
            logging.warning(s.format(self.get_name(), TypeIO.READ))
            return
        address = self.get_address_io()
        class_io = self._get_class_io()
        serial_command = {str(CmdIO.COMMAND): str(CmdIO.READ)}
        if class_io is ClassIO.I2C:
            serial_command.update({str(CmdIO.ARGUMENTS): [{str(class_io): [address, self._get_register_io()]}]})
        else:
            serial_command.update({str(CmdIO.ARGUMENTS): [{str(class_io): address}]})
        data = send_serial_command(self._serial_bus, serial_command)
        # Data is always put in a list,
        # but when requesting only one value it is always the first.
        value = data[str(class_io)][0][address]
        status = data.get('status')
        return status, value

    @staticmethod
    def get_all_values(serial_bus):
        sensor_list = []
        for io in IO.instances:
            if io.get_type_io() is TypeIO.READ:
                if io._get_class_io() is ClassIO.I2C:
                    sensor_list.append({str(io._get_class_io()):
                                        [io.get_address_io(),
                                         io._get_register_io()]})
                else:
                    sensor_list.append({str(io._get_class_io()):
                                       io.get_address_io()})
        serial_command = {str(CmdIO.COMMAND): str(CmdIO.READ)}
        serial_command.update({str(CmdIO.ARGUMENTS): sensor_list})
        return send_serial_command(serial_bus, serial_command)

    def set_value(self, value):
        '''Set value for IO device'''

        # Set timeout for response on serial bus.
        self._set_serial_bus_timeout(IO.sb_default_timeout)

        # Warn about faulty usage
        if self.get_type_io() is not TypeIO.WRITE:
            s = '{}, tried to set_value, but is not configured as {}'
            logging.warning(s.format(self.get_name(), TypeIO.WRITE))
            return

        address = self.get_address_io()
        class_io = self._get_class_io()
        serial_command = {str(CmdIO.COMMAND): str(CmdIO.WRITE)}
        if class_io is ClassIO.I2C:
            serial_command.update({str(CmdIO.ARGUMENTS): [{str(class_io): [address, self._get_register_io()], str(CmdIO.VALUE): value}]})
        else:
            serial_command.update({str(CmdIO.ARGUMENTS): [{str(class_io): address, str(CmdIO.VALUE): value}]})
        return send_serial_command(self._serial_bus, serial_command)


class Pump(IO):
    '''Class for all pump devices.'''

    # Keep track of all pump devices
    instances = []

    def __init__(self, name, spec, sb, io_object_dict):
        '''Doc - __init__ Constructor'''

        # Call IO constructor
        super().__init__(name, spec, sb)
        # Pump configuration data
        for key, value in spec.items():
            if key == 'tachometer':
                if value in io_object_dict:
                    self._data.update({'tachometer':
                                      io_object_dict.get(value)})
            if key == 'relay':
                if value in io_object_dict:
                    self._data.update({'relay': io_object_dict.get(value)})
        # Add to list of class instances
        Pump.instances.append(self)

    # Redefine inherited method that will not be used.
    def get_value(*args, **kwargs):
        pass

    # Redefine inherited method that will not be used.
    def set_value(*args, **kwargs):
        pass

    def _get_relay(self):
        '''Return relay object'''
        return self._data.get('relay')

    def _get_tachometer(self):
        '''Return tachometer object'''
        return self._data.get('tachometer')

    def run_pump(self, mode, value):
        '''Run pump with specified mode.
        Supported modes are 'time' and 'centiliter', which
        runs the pump for specified time or amount.
        '''

        # Set timeout for response on serial bus.
        self._set_serial_bus_timeout(Pump.sb_default_timeout)

        if not isinstance(mode, PumpMode):
            s = '{}; Unknown pump mode: {}'
            return 1, s.format(self.get_name(), mode)

        address = self.get_address_io()
        class_io = self._get_class_io()
        relay = self._get_relay()
        relay_address = relay.get_address_io()
        relay_class_io = relay._get_class_io()
        tachometer = self._get_tachometer()
        tachometer_address = tachometer.get_address_io()
        tachometer_class_io = tachometer._get_class_io()
        pulse_per_cl = tachometer._data.get('pulse_per_cl')

        serial_command = {str(CmdIO.COMMAND): str(CmdIO.PUMP)}
        serial_command.update({str(CmdIO.PUMP): {str(class_io): address},
                               str(CmdIO.TACHOMETER): {str(tachometer_class_io): tachometer_address},
                               str(CmdIO.RELAY): {str(relay_class_io): relay_address}})
        if mode is PumpMode.TIME:
            serial_command.update({str(CmdIO.AMOUNT): {str(CmdIO.TIME): value}})
        elif mode is PumpMode.CENTILITER:
            serial_command.update({str(CmdIO.AMOUNT): {str(CmdIO.PULSES): value*pulse_per_cl}})
        data = send_serial_command(self._serial_bus, serial_command)
        status = data.get('status')
        pulses = data.get(str(CmdIO.PULSES))
        message = '{} pulses'.format(pulses)
        return status, message


def send_serial_command(sb, command):
    '''Send serial command and return response.
    '''

    serial_command = json.dumps(command)
    logging.debug('serial bus sent: {}'.format(command))

    if sb._serial_bus.get('simulated'):
        data = recieve_serial_command(serial_command)
    else:
        if not sb.is_open:
            sb.open()

        # Flush buffer on serial bus.
        sb.flushInput()
        time.sleep(0.1)

        # Encode to bytestring
        sb.write(command.encode())
        # Decode from bytestring
        response = sb.readline()

        try:
            serial_data = response.decode()
            data = json.loads(serial_data)
            data.update({'status': 0})
        except:
            s = 'Failed to decode response on serial bus: {}'
            logging.warning(s.format(response))
    logging.debug('serial bus received: {}'.format(data))
    return data


def recieve_serial_command(serial_data):
    '''Emulates a simple response from serial command.'''

    serial_command = json.loads(serial_data)
    command = serial_command.get(str(CmdIO.COMMAND))
    arguments = serial_command.get(str(CmdIO.ARGUMENTS))

    data = dict()
    number = random.randint(0, 255)
    if command == str(CmdIO.READ):
        for argument in arguments:
            if str(ClassIO.ANALOG) in argument:
                io_class_str = str(ClassIO.ANALOG)
                value = number
            elif str(ClassIO.DIGITAL) in argument:
                io_class_str = str(ClassIO.DIGITAL)
                if number > 127:
                    value = 1
                else:
                    value = 0
            elif str(ClassIO.I2C) in argument:
                io_class_str = str(ClassIO.I2C)
                value = number*4
            else:
                s = 'Unknown argument: {} to: {} command.'
                logging.warning(s.format(argument, command))
                return {'status': 3}
            address = argument.get(io_class_str)
            if str(ClassIO.I2C) in argument:
                if io_class_str in data:
                    data[io_class_str].append({address[0]: {address[1]: value}})
                else:
                    data.update({io_class_str: [{address[0]: {address[1]: value}}]})
            else:
                if io_class_str in data:
                    data[io_class_str].append({address: value})
                    pass
                else:
                    data.update({io_class_str: [{address: value}]})
    if command == str(CmdIO.WRITE):
        data.update({str(CmdIO.VALUE): 0})
    if command == str(CmdIO.PUMP):
        data.update({str(CmdIO.PULSES): number*23})
    data.update({'status': 0})
    return data


def init_serial_bus(simulated=False):
    '''Setup serial bus.
    Doc: https://pyserial.readthedocs.io/en/latest/shortintro.html
    '''

    serial_options = dict(port='/dev/ttyUSB0',
                          baudrate=57600,
                          parity=serial.PARITY_EVEN,
                          stopbits=serial.STOPBITS_ONE,
                          bytesize=serial.EIGHTBITS,
                          timeout=1)
    if simulated:
        serial_options.update({'simulated': True})
        return SB(serial_options)

    sb = serial.Serial(**serial_options)

    logging.debug('Init serial bus with options: {}'.format(sb))
    test_serial_bus(sb)
    return sb


def test_serial_bus(sb):
    '''Verify connection to other unit on bus.
       Send init command and get confirmation as response.
    '''

    # Warm up serial bus
    serial_command = {str(CmdIO.COMMAND): str(CmdIO.INIT)}
    for _ in range(5):
        send_serial_command(sb, serial_command)
    return
