import datetime
import json
import logging
import os
import time

from modules.io import init_serial_bus
from modules.io import IO
from modules.io import PumpMode


logger = logging.getLogger()


def control_monitor(process_data, plant_data, meas_log_q, water_log_q):
    """
    """

    # Init serial bus
    sb = init_serial_bus(port=process_data['com_port'], simulated=False)

    data = plant_data.get('Plant_0').read_moist_sensor(sb)
    meas_log_q.put(data)
    data = plant_data.get('Plant_0').read_light_sensor(sb)
    meas_log_q.put(data)
    data = plant_data.get('Plant_0').read_temperature_sensor(sb)
    meas_log_q.put(data)
    data = plant_data.get('Plant_0').read_humidity_sensor(sb)
    meas_log_q.put(data)

    data = plant_data.get('Plant_2').run_pump(PumpMode.TIME, 3, sb)
    water_log_q.put(data)
    time.sleep(1)
    data = plant_data.get('Plant_2').run_pump(PumpMode.CENTILITER, 2, sb)
    time.sleep(1)
    water_log_q.put(data)
    data = plant_data.get('Plant_3').run_pump(PumpMode.TIME, 8, sb)
    time.sleep(1)
    water_log_q.put(data)
    data = plant_data.get('Plant_4').run_pump(PumpMode.TIME, 2, sb)
    time.sleep(1)
    water_log_q.put(data)

    process_data['abort'] = True

    water_log_q.close()
    water_log_q.join_thread()
    meas_log_q.close()
    meas_log_q.join_thread()