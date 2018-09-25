#ifndef PLANT_H
#define PLANT_H

#include "BasicIO.h"
#include <string>

class Plant
{
private:
    Pump* m_pump = nullptr;
    // This value impacts the flow rate of watering. 8-bit value between 0-255 corresponding to min-max output voltage
    int8_t unsigned m_pumpValue = 0xFF; // Default to max
    Relay m_relay = Relay();
    const Sensor* m_moist = nullptr;
    const Sensor* m_temperature = nullptr;
    const Sensor* m_light = nullptr;
    std::string m_name = "";

    // Some schedule for when to water or threshold value etc..
    // or this logic is probably handled on higher level
    void startWater();
    void stopWater();
public:
    /** Default Plant constructor
     *
     */
    Plant()
    {
    }

    /** Plant constructor
     *  Initialize Plant with different accessories
     */
    Plant(Pump* pump, Relay relay, const Sensor* moist, const Sensor* temperature, const Sensor* light, std::string name = "");

    /** Water plant
     *
     */
    void waterSeconds(int seconds);
    void waterAmount(int centiliters);

    /** Set pump value.
     *  This value impacts the flow rate of watering
     */
    void setPumpValue(int8_t pumpValue);

    /** Set name of Plant
     *
     */
    void setName(std::string name) { m_name = name; }

    /** Getters for sensors and Plant name
     *
     */
    int getTemperature() const { return m_temperature->getValue(); }
    int getMoist() const { return m_moist->getValue(); }
    int getLight() const { return m_light->getValue(); }
    const Sensor* getTemperatureSensor() const;
    const Sensor* getMoistSensor() const;
    const Sensor* getLightSensor() const;
    std::string getName() const { return m_name; }
};

std::ostream& operator<<(std::ostream &out, Plant &p);

#endif // PLANT_H
