import logging
import serial
from enum import Enum
import random


class ClassIO(Enum):
    def __str__(self):
        return str(self.value)
    ANALOG = 'analog'
    DIGITAL = 'digital'
    I2C = 'i2c'
    UNINITIALIZED_CLASS = 'uninitialized_class'


class TypeIO(Enum):
    def __str__(self):
        return str(self.value)
    READ = 'read'
    WRITE = 'write'
    UNINITIALIZED_STATE = 'uninitialized_type'


class State(Enum):
    def __str__(self):
        return str(self.value)
    OFF = 0
    ON = 1
    UNINITIALIZED = 'uninitialized'


class IO:
    '''Basic class for all IO devices.'''

    # Keep track of all IO devices
    instances = []

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

        s = 'Added IO: {} on {}:{} as {}'
        logging.debug(s.format(self.get_name(), self._get_class_io(),
                               self.get_address_io(), self.get_type_io()))
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

    def get_value(self):
        '''Get value from IO device'''

        # Warn about faulty usage
        if self.get_type_io() is not TypeIO.READ:
            s = '{}, tried to get_value, but is not configured as {}'
            logging.warning(s.format(self.get_name(), TypeIO.READ))
            return
        pin = self.get_address_io()
        class_io = self._get_class_io()
        serial_command = 'Read {0}_{1}\n'.format(class_io, pin)
        if class_io is ClassIO.I2C:
            serial_command = '{}_{}\n'.format(serial_command[:-1],
                                              self._get_register_io())

        status, message = send_serial_command(self._serial_bus, serial_command)
        if status:
            s = '{}; Cmd: {}, returned status: {}, with message: {}'
            logging.debug(s.format(self.get_name(), serial_command[:-1],
                                   status, message))
        return status, message

    def set_value(self, value):
        '''Set value for IO device'''

        # Warn about faulty usage
        if self.get_type_io() is not TypeIO.WRITE:
            s = '{}, tried to set_value, but is not configured as {}'
            logging.warning(s.format(self.get_name(), TypeIO.WRITE))
            return

        pin = self.get_address_io()
        class_io = self._get_class_io()
        serial_command = 'Write {0}_{1} {2}\n'.format(class_io, pin, value)
        status, message = send_serial_command(self._serial_bus, serial_command)
        if status:
            s = '{}; Cmd: {}, returned status: {}, with message: {}'
            logging.debug(s.format(self.get_name(), serial_command[:-1],
                                   status, message))
        return status, message


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
        # Add to list of class instances

        Pump.instances.append(self)


def send_serial_command(sb, command):
    '''Send serial command and return response.
    Available commands and syntax:
        Assign analog/digital input/output pin [number]\n
        Read analog/digital pin [number]\n
        Write analog/digital pin [number]\n
    '''

    logging.debug('Sending cmd on serial bus: {}'.format(command[:-1]))
    status, message = recieve_serial_command(command)

    s = 'Recieved status: {} with message: {}'
    logging.debug(s.format(status, message))
    return status, message
    '''
    if not sb.is_open():
        sb.open()
    sb.write(command)
    return sb.readline()
    '''


def recieve_serial_command(command):
    '''Emulates a simple response from serial command.'''

    message = 'Someting went wrong'
    status = 1
    if command[:4] == 'Read':
        number = random.randint(0, 255)
        if command[5:11] == 'analog':
            value = number
            status = 0
        elif command[5:12] == 'digital':
            if number > 127:
                value = 1
            else:
                value = 0
            status = 0
        elif command[5:8] == 'i2c':
            value = number*4
            status = 0
    if command[:5] == 'Write':
        status = 0
        value = None

    if status == 0:
        return status, value
    else:
        return status, message


def init_serial_bus():
    '''Setup serial bus.'''

    # https://pyserial.readthedocs.io/en/latest/shortintro.html

    sb = serial.Serial(
        port='/dev/ttyUSB0',
        baudrate=9600,
        parity=serial.PARITY_NONE,
        stopbits=serial.STOPBITS_ONE,
        bytesize=serial.EIGHTBITS,
        timeout=1)

    test_serial_bus(sb)
    return sb


def test_serial_bus(sb):
    '''Verify connection to other unit on bus.
       Send init command and get confirmation as response.
    '''

    if not sb.is_open():
        sb.open()

    sb.write('init')
    response = sb.readline()
    if response is not 'init complete':
        logging.warning('Failed to get response from unit')
    sb.close()
    return
