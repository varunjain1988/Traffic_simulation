#include <iostream>
#include <random>
#include "TrafficLight.h"

/* Implementation of class "MessageQueue" */

// Implement the function receive that should return the TrafficLightPhase 
template <typename T>
T MessageQueue<T>::receive()
{
    // FP.5a : The method receive should use std::unique_lock<std::mutex> and _condition.wait() 
    // to wait for and receive new messages and pull them from the queue using move semantics. 
    // The received object should then be returned by the receive function.
    std::unique_lock<std::mutex> uniqueLck(_mtxMsgQueue);
    
    // Use of lamda function, to check if the queue is empty. It is blocked till the queue is empty,
    // once it is not empty, first message is taken from queue and returned
    // The lock is returned after the queue has some members
    _condVariable.wait(uniqueLck,[this](){return !_queue.empty();});

     //=====================================================================//
     // This returns the oldest value stored in deque, using the lastest 
     // makes more sense (since it gives updated state of traffic light)
     //=====================================================================//
    
    // If the queue has members, return the first element, do not copy, use of move semantics
    // T lightPhase = std::move(_queue.front());

    // Remove it from the list
    // _queue.pop_front();
    
     // If the queue has members, return the last element, do not copy, use of move semantics
     T lightPhase = std::move(_queue.back());

    // Clear the list, since popback would only remove back and the list is then not in sync with phase
    _queue.clear();
    
    // Here also, the return value is not copied using return value optimization in C++
    return lightPhase;
}

template <typename T>
void MessageQueue<T>::send(T &&msg)
{
    // FP.4a : The method send should use the mechanisms std::lock_guard<std::mutex> 
    // as well as _condition.notify_one() to add a new message to the queue and afterwards send a notification. 
    
    // In this task, add the TrafficLightPhase to the messageQueue
    // In order to add the trafficLightphase to the membervariable (shared resource), 
    // use of lock guard to ensure that the member variable does not lead to data race
    std::lock_guard<std::mutex> lck(_mtxMsgQueue);

    // Use Emplaceback and move semantics to add msg
    _queue.emplace_back(std::move(msg));
   
    // After the TrafficLightPhase is added, notify all, that are waiting for conditionVariable
    _condVariable.notify_one();
}


/* Implementation of class "TrafficLight" */
// Destructor has nothing to do, since the threads are from base class traffic object and will be destroyed there
// The constructor has been defined here and assigns the member variable _currentphase to red
TrafficLight::TrafficLight()
{
    _currentPhase = TrafficLightPhase::red;
}


void TrafficLight::waitForGreen()
{
    // FP.5b : add the implementation of the method waitForGreen, in which an infinite while-loop 
    // runs and repeatedly calls the receive function on the message queue. 
    // Once it receives TrafficLightPhase::green, the method returns.

    // Keep checking the returned traffic light phase
    // Use the mutex of cout from traffic object to print to console
    while(true)
    {
        // Use sleep for few milliseconds to reduce the CPU load.
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
        if(TrafficLightQueue.receive() == TrafficLightPhase::green)
        {
            break; // The green phase of traffic light is obtained, return from loop
        }   
    }
}


TrafficLightPhase TrafficLight::getCurrentPhase()
{
    // To protect the shared variables in different threads
    std::unique_lock<std::mutex> lock(_mutex);
    return _currentPhase;
}

void TrafficLight::simulate()
{
    // FP.2b : Finally, the private method cycleThroughPhases should be started in a thread when the public methodsimulate is called. To do this, use the thread queue in the base class. 
    // Start a new thread for the traffic light at this intesection and call the method cycleThrough phases in that thread
    threads.emplace_back(std::thread(&TrafficLight::cycleThroughPhases, this));
}


// virtual function which is executed in a thread
void TrafficLight::cycleThroughPhases()
{
    // FP.2a : Implement the function with an infinite loop that measures the time between two loop cycles 
    // and toggles the current phase of the traffic light between red and green and sends an update method 
    // to the message queue using move semantics. The cycle duration should be a random value between 4 and 6 seconds. 
    // Also, the while-loop should use std::this_thread::sleep_for to wait 1ms between two cycles. 

    // This is relevant for initial
    auto lastUpdate = std::chrono::system_clock::now();

    // Calculate a random duration between 4-6s and check if time is greater than that.
    std::random_device randomDevice;
    std::default_random_engine randomGenerator(randomDevice());
    
    // Distribution between 4s and 6s
    std::uniform_int_distribution<int64_t> distribution(4,6);

    // Generate a random duration
    int64_t durationRequired = distribution(randomGenerator);

    // To protect the shared resource _currentPhase in different threads
    std::unique_lock<std::mutex> lock(_mutex);
    lock.unlock();
    
    while(true)
    {    
        // This thread should sleep for 1ms to avoid excess CPU load
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
                
        // Calculate the difference between last update and time now
        auto timeDifference = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now() - lastUpdate).count();

        // Check if time elapsed is greater than required duration
        if (timeDifference >= durationRequired)
        {    
            // Declare a variable of traffic light type and move it
            TrafficLightPhase TLightPhase;

            // This is used to protect the shared resource _currentPhase from data-race
            lock.lock();    
            // Toggle between red and green after 4-6s
            if (_currentPhase == TrafficLightPhase::red)
            {
                // Use of local variable to push it to the queue
                TLightPhase = TrafficLightPhase::green;
                
                // Save the last value for next loop, if moved then it is invalidated
                _currentPhase = TrafficLightPhase::green;
            }
            else if(_currentPhase== TrafficLightPhase::green)
            {
                // Use of local variable to push it to the queue
                TLightPhase = TrafficLightPhase::red;
                
                // Save the last value for next loop, if moved then it is invalidated
                _currentPhase = TrafficLightPhase::red; 
            }
            
            // Unlock the lock
            lock.unlock();

            // Send the upadte method to the message queue
            TrafficLightQueue.send(std::move(TLightPhase));

            // Update the time counter
            lastUpdate = std::chrono::system_clock::now();

            // Generate a new duration
            durationRequired = distribution(randomGenerator);
        }
    }
}

