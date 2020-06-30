import json
import numpy as np
import logging
import os
import time


logger = logging.getLogger()

def current_time_ms():
    """Return unix time in int ms"""
    return int(round(time.time() * 1000))


def queue_to_file_and_list(q, f, first):
    """Function will empty queue q into file f
    also make a copy into list lis. Items in queue must be a dict.
    """
    s = ''
    while not q.empty():
        if first:
            tmp_d = q.get()
            s = (str(tmp_d))
            first = False
        else:
            tmp_d = q.get()
            s = s + ',\n' + (str(tmp_d))
    f.write(s)
    return first


def log_data(process_data, meas_log_q, water_log_q):
    """
    """

    # initialize process local variables
    meas_first = True
    water_first = True
    time_old = current_time_ms()

    # ensure folder path exists and create log files for current execution
    try:
        folder = '../data'
        parent_dir = os.path.dirname(os.path.abspath(__file__))
        path = os.path.join(parent_dir, folder)
        if not os.path.exists(path):
            print('Successfully created the directory %s ' % path)
            os.mkdir(path)
    except OSError:
        logger.error('Creation of the directory {} failed'.format(path))
    else:
        lf = open(path + '/' + process_data['file_prefix']
                  + '_log' + '.json', 'w+')
        wf = open(path + '/' + process_data['file_prefix']
                  + '_water' + '.json', 'w+')
        logging.info('meas_log_file open, control_log_file open')

        # Initialize files to .jason compatible format
        lf.write('[' + '\n')
        wf.write('[' + '\n')

    logger.info('run loop')
    while not (process_data['abort']):
        # running log and live visualization main loop

        # data from other processes are fetched from queues and stored to file
        # also added to live plot data arrays of fixed size's
        meas_first = queue_to_file_and_list(meas_log_q, lf, meas_first)
        water_first = queue_to_file_and_list(water_log_q, wf, water_first)

    logger.info('loop terminated')

    # Wait to let other process(MCU handler and plate controller) close/stop
    # adding data to queue
    time.sleep(3)

    # Final data on queue's written to file,
    # close IPC queues and files then complete process function
    queue_to_file_and_list(meas_log_q, lf, meas_first)
    queue_to_file_and_list(water_log_q, wf, water_first)
    meas_log_q.close()
    water_log_q.close()
    water_log_q.join_thread()
    meas_log_q.join_thread()
    lf.write('\n' + ']')
    wf.write('\n' + ']')
    lf.close()
    wf.close()
    logger.info('closed')
