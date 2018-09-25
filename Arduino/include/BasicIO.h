#ifndef BASICIO_H
#define BASICIO_H
#include <cstdint>
#include <cstdlib> // for rand() and srand()

/** Random number generator, which is used to simulate measurement values
 *
 */
int getRandomNumber(int min, int max);

/** Definition of state
 *
 */
enum State
{
    OFF,
    ON,
};

/** Definition of IO type
 *
 */
enum TypeIO
{
    ANALOG,
    DIGITAL,
    UNITIALIZED_TYPE,
};

/** Definition of IO state
 *
 */
enum StateIO
{
    READ,
    WRITE,
    UNITIALIZED_STATE,
};

/** Base class for all IO classes
 *
 */
class BasicIO
{
    private:
        // Keep track of IO devices maybe?
    protected:
        TypeIO m_typeIO;
        StateIO m_stateIO;
        int m_pin;
    public:
        BasicIO(TypeIO typeIO = UNITIALIZED_TYPE, StateIO stateIO = UNITIALIZED_STATE, int pin = -1);
        int getPin()         const { return m_pin; }
        TypeIO getTypeIO()   const { return m_typeIO; }
        StateIO getStateIO() const { return m_stateIO; }
};

/** Definition of Sensor class, which inherits from IO base class
 *
 */
class Sensor: public BasicIO
{
public:
    Sensor();
    Sensor(TypeIO typeIO, StateIO stateIO, int pin);
    int getValue() const { return getRandomNumber(0, 100); }
};

/** Definition of Relay class, which inherits from IO base class
 *
 */
class Relay: public BasicIO
{
    private:
        State m_state = OFF;
    public:
        Relay(TypeIO typeIO = UNITIALIZED_TYPE, StateIO stateIO = UNITIALIZED_STATE, int pin = -1);
        State getState() const     { return m_state; };
        void setState(State state) { m_state = state; };
};

// Two dummy functions that exists in Arduino.h
void analogWrite(int pin, int value);
int millis();

/** Definition of Pump class, which inherits from IO base class
 *
 */
class Pump: public BasicIO
{
private:
    State m_state = OFF;
    const Sensor* m_flow;
    int m_counter;
    const double m_countsPerCentiliter = 16.53;

    // ISR Interrupt routine calls this function and increments counter
    void countInterrupt() { ++m_counter; }

public:
    Pump(TypeIO typeIO, StateIO stateIO, int pin, const Sensor *flow);
    State getState() const  { return m_state; }
    void resetCounter()     { m_counter = 0; }
    int getAmountCentiliter() const;
    void start(int8_t value);
    void stop();
};

#endif // BASICIO_H
