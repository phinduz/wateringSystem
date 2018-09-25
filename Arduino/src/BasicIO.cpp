#include "BasicIO.h"
//#include "Arduino.h"

BasicIO::BasicIO(TypeIO typeIO, StateIO stateIO, int pin):
    m_typeIO(typeIO), m_stateIO(stateIO), m_pin(pin)
{
    // Some hardware specific setup??
    // Initialize pins as digital/analog input/output probably
}
Sensor::Sensor(): BasicIO()
{
}
Sensor::Sensor(TypeIO typeIO, StateIO stateIO, int pin):
    BasicIO(typeIO, stateIO, pin)
{
}
Relay::Relay(TypeIO typeIO, StateIO stateIO, int pin):
    BasicIO(typeIO, stateIO, pin)
{
}
Pump::Pump(TypeIO typeIO, StateIO stateIO, int pin, const Sensor *flow):
    BasicIO(typeIO, stateIO, pin), m_flow(flow)
{
}

int Pump::getAmountCentiliter() const
{
    return m_counter / m_countsPerCentiliter;
}

void Pump::start(int8_t value)
{
     m_state = ON;
     analogWrite(m_pin, value);
}
void Pump::stop()
{
    m_state = OFF;
    analogWrite(m_pin, 0u);
}

/**
 * Generate a random number between min and max (inclusive) Assumes srand() has already been called
 */
int getRandomNumber(int min, int max)
{
	static const double fraction = 1.0 / (static_cast<double>(RAND_MAX) + 1.0);  // static used for efficiency, so we only calculate this value once
	// evenly distribute the random number across our range
	return static_cast<int>(rand() * fraction * (max - min + 1) + min);
}

/**
 * Dummy implementation of functions that exists in Arduino.h
 */
void analogWrite(int pin, int value)
{
};
int millis()
{
    return getRandomNumber(0x0000, 0x8888);
}
