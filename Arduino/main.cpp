#include <iostream>
#include <ctime> // for time()
#include "BasicIO.h"
#include "Plant.h"

int main()
{
    srand(static_cast<unsigned int>(time(0))); // set initial seed value to system clock
	rand(); // get rid of first result

    /*
    Plant is the highest level and has one of each member:
        Pump is shared between all plants, it also has a flow sensor attached
        Relay is specific for each plant
        Moist sensor is specific for each plant
        Temperature sensor and light sensor is shared between all plants
    */

    int numberOfPlants = 8;

    // Setup common devices
    const Sensor temperature(ANALOG, READ, 9);
    const Sensor light(ANALOG, READ, 10);
    const Sensor flow(DIGITAL, READ, 11);
    Pump pump(ANALOG, WRITE, 6, &flow);
    int moistSensorAddress[numberOfPlants] = {10, 11, 12, 13, 14, 15, 16, 17};
    Sensor moist[numberOfPlants];
    for (int k = 0; k < static_cast<int>(sizeof(moistSensorAddress) / sizeof(moistSensorAddress[0])); ++k)
    {
        moist[k] = Sensor(DIGITAL, READ, moistSensorAddress[k]);
    }

    // Setup plants
    std::string plantName[numberOfPlants] = {"Cactus", "Banana", "Tomato", "Grape", "Physalis", "Basil", "Palm", "Rosemary"};
    int relayAddress[numberOfPlants] = {1, 3, 5, 6, 8, 1, 2, 3};

    Plant plantArray[numberOfPlants];

    for (int k = 0; k < numberOfPlants; ++k)
    {
        plantArray[k] = Plant(&pump, Relay(DIGITAL, WRITE, relayAddress[k]), &moist[k], &temperature, &light, plantName[k]);
    }

    plantArray[7].setName("Thyme");

    // Run this pump on a lower setting
	plantArray[3].setPumpValue(0xF0);

    for (int k = 0; k < numberOfPlants; ++k)
    {
        //plantArray[k].getMoistSensor()->getValue();
        std::cout << plantArray[k];
    }
    return 0;
}