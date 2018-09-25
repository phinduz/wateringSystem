#include <cassert>
#include <iostream>
#include "Plant.h"

/** Plant constructor
 *  Initialize Plant with different accessories
 */
Plant::Plant(Pump* pump, Relay relay, const Sensor* moist, const Sensor* temperature, const Sensor* light, std::string name):
    m_pump(pump), m_relay(relay), m_moist(moist), m_temperature(temperature), m_light(light), m_name(name)
{
}
void Plant::startWater()
{
    m_pump->resetCounter();
    m_relay.setState(ON);
    m_pump->start(m_pumpValue);
}
void Plant::stopWater()
{
    m_relay.setState(OFF);
    m_pump->stop();
}

/** Water plant
 *
 */
void Plant::waterSeconds(int seconds)
{
    int currentTime = millis();
    m_pump->start(m_pumpValue);
    while (currentTime - millis() < seconds * 1000)
    {
    }
    m_pump->stop();
}

void Plant::waterAmount(int centiliters)
{
    // Use timer as a safety backup
    int currentTime = millis();
    int maxTime = 10;
    m_pump->start(m_pumpValue);
    while ( m_pump->getAmountCentiliter() < centiliters && (currentTime - millis()) < maxTime * 1000)
    {
    }
    m_pump->stop();
}

/** Set pump value.
 *  This value impacts the flow rate of watering
 */
void Plant::setPumpValue(int8_t pumpValue)
{
    // Set minimum value to at least half of maximum
    m_pumpValue = (pumpValue >= 0x80 ? pumpValue : 0x80);
}

const Sensor* Plant::getTemperatureSensor() const
{
    assert(m_temperature);
    return m_temperature;
}
const Sensor* Plant::getMoistSensor() const
{
    assert(m_moist);
    return m_moist;
}
const Sensor* Plant::getLightSensor() const
{
    assert(m_light);
    return m_light;
}

/** Overload Plant output information
 *
 */
std::ostream& operator<<(std::ostream &out, Plant &p)
{
    out << "Name:  " << p.getName()         << "\n";
    out << "Light: " << p.getLight()        << "\n";
    out << "Moist: " << p.getMoist()        << "\n";
    out << "Temp:  " << p.getTemperature()  << "\n";
    return out;
}
