# Watering system
It is intended to automate the irrigation process of your plants


## Short description



## Roadmap
* [ ] 0.01 - Read configuration file and set-up objects
* [ ] 0.02 - Add logging


## Tasks

### Documentation
* [ ] Document where to start when starting to use the system.
* [ ] Document serial protocol with examples

### Python

* [ ] Implement serial protocol in python environment

### MCU
* [ ] Implement serial protocol in arduino environment


### Known issues, decide on actions
* [ ]



## System overview
The system consists of:

* Onboard environment
    * Arduino
    * Temperature and humidity sensor
    * Ampient light sensor
    * Soil moisture sensors
	* Power supply unit
	* Water pump
	* Relays controlling ball valves

* PC/Raspberry PI, Offboard environment
    *

### Onboard environment

#### Arduino
Arduino Mega 2560 Rev3, code: A000067, [arduino-mega-2560][arduino-mega-2560].

The MCU (Arduino) is connected to peripherals with 4 different electrical interfaces, I2C, PWM, “Digital & Analog” and Serial. Design wise the MCU is aimed to be as simple as possible offering sensor data and executing watering commands. Needed configuration, calibration and transformation are targeted for implementation in off board environment (PC/Raspberry PI).




### Interface Onboard - Offboard

| Signal name       | range          | Comment                         |
| :---              | :----:         | :---                            |
| ["a0"]            | [0 1023]       | Analog input 0                  |
| ["a1"]            | [0 1023]       | Analog input 1                  |
| ["d0"]            | [0 1]          | Digital input 0                 |
| ["i2c_0x75_0x04"] | [-]            | I2C address 0x75, register 0x04 |


Example of raw_data:
```json
{
  "a0": 345,
  "a1": 94,
  "d0": 0,
  "i2c_0x75_0x04": 4621
}
```


### PC, Offboard environment



## Getting Started

#### Install Python with modules

Install python 3:

```sh
$ sudo apt-get install python3
```

Go to the repo root and install the required packages:
```sh
$ python3 -m pip install -r requirements.txt
```
If python is installed for all users in Users/Program Files/ then the command above needs to be executed in a shell that has been started with "Run as administrator".

### Python Environment

Go to Python folder and run main_process.py
```sh
$ cd Python
$ python3 main_process.py --help
```

## Coding style tests
For python code [PEP 8][python-pep8] is followed. Look at the example [How to Write Beautiful Python Code With PEP 8][real-python-pep8].

Enforce your coding style using eg. [Flake8][flake8].

```sh
$ python -m pip install flake8
```

and run:

```sh
$ flake8 path/to/code/
```

## Generate documentation
To generate this documentation in html, install [grip][grip]

```sh
$ python -m pip install grip
```

To export to html run:

```sh
$ python -m grip README.md --export README.html
```

To render directly:

```sh
$ grip -b README.md
```


[//]: # (Markdown comments)

[arduino-mega-2560]: https://store.arduino.cc/mega-2560-r3
[grip]: https://github.com/joeyespo/grip
[python-pep8]: https://www.python.org/dev/peps/pep-0008/
[real-python-pep8]: https://realpython.com/python-pep8/
[flake8]: http://flake8.pycqa.org/en/latest/
