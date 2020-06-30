#ifndef TRAFFICLIGHT_H
#define TRAFFICLIGHT_H

#include <mutex>
#include <deque>
#include <condition_variable>
#include "TrafficObject.h"

// forward declarations to avoid include cycle
class Vehicle;

// Define the enumeration TrafficLightPhase, having red and green as phases
enum TrafficLightPhase
{
    red,
    green
};

// FP.3 Define a class „MessageQueue“ which has the public methods send and receive. 
// Send should take an rvalue reference of type TrafficLightPhase whereas receive should return this type. 
// Also, the class should define an std::dequeue called _queue, which stores objects of type TrafficLightPhase. 
// Also, there should be an std::condition_variable as well as an std::mutex as private members. 
template <class T>
class MessageQueue
{
public:
    // send method with rvalue reference
    // In this function the traffic light phase is taken as rvalue reference
    void send(T &&msg);
    
    // This function returns the object of type Trafficlight phase (defined as template-> at the time of definition function call/member variable)
    T receive();
private:
    // This is used to store objects of type traffic light phase
    std::deque<TrafficLightPhase> _queue;
    std::mutex _mtxMsgQueue;
    std::condition_variable _condVariable;
};

// FP.1 : Define a class „TrafficLight“ which is a child class of TrafficObject. 
// The class shall have the public methods „void waitForGreen()“ and „void simulate()“ 
// as well as „TrafficLightPhase getCurrentPhase()“, where TrafficLightPhase is an enum that 
// can be either „red“ or „green“. Also, add the private method „void cycleThroughPhases()“. 
// Furthermore, there shall be the private member _currentPhase which can take „red“ or „green“ as its value. 

class TrafficLight: public TrafficObject
{
public:
    // constructor / desctructor
    TrafficLight();
    
    // getters / setters
    TrafficLightPhase getCurrentPhase();

    // typical behaviour methods
    // Wait for green method
    void waitForGreen();

    // Simulate method
    void simulate();    

private:
    // typical behaviour methods
    // declaration for function call cycleThroughPhases which is run in an infinite loop
    void cycleThroughPhases();

    // FP.4b : create a private member of type MessageQueue for messages of type TrafficLightPhase 
    // and use it within the infinite loop to push each new TrafficLightPhase into it by calling 
    // send in conjunction with move semantics.
    TrafficLightPhase _currentPhase;
    std::condition_variable _condition;
    std::mutex _mutex;
    MessageQueue<TrafficLightPhase> TrafficLightQueue;
};

#endif