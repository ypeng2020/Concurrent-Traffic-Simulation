#include <iostream>
#include <random>
#include "TrafficLight.h"

/* Implementation of class "MessageQueue" */

 
template <typename T>
T MessageQueue<T>::receive()
{
    // FP.5a : The method receive should use std::unique_lock<std::mutex> and _condition.wait() 
    // to wait for and receive new messages and pull them from the queue using move semantics. 
    // The received object should then be returned by the receive function.
  
    // lock
    std::unique_lock<std::mutex> uLock(_mutex);
    _cond.wait(uLock, [this](){return !_queue.empty();});

    // dequeue
    T msg = std::move(_queue.back());
    _queue.pop_back();
    return msg;
  
}

template <typename T>
void MessageQueue<T>::send(T &&msg)
{
    // FP.4a : The method send should use the mechanisms std::lock_guard<std::mutex> 
    // as well as _condition.notify_one() to add a new message to the queue and afterwards send a notification.
    std::lock_guard<std::mutex> uLock(_mutex);
    _queue.emplace_back(std::move(msg));
    _cond.notify_one();
}


/* Implementation of class "TrafficLight" */


TrafficLight::TrafficLight()
{
    _current_phase = TrafficLightPhase::red;
}

void TrafficLight::waitForGreen()
{
    // FP.5b : add the implementation of the method waitForGreen, in which an infinite while-loop 
    // runs and repeatedly calls the receive function on the message queue. 
    // Once it receives TrafficLightPhase::green, the method returns.
  
    while(true) {
        TrafficLightPhase phase = _message_queue.receive();
        if (phase == TrafficLightPhase::green) return;
    }

}

TrafficLightPhase TrafficLight::getCurrentPhase()
{
    return _current_phase;
}

void TrafficLight::simulate()
{
    // FP.2b : Finally, the private method „cycleThroughPhases“ should be started in a thread when the public method „simulate“ is called. To do this, use the thread queue in the base class. 
    threads.emplace_back(std::thread(&TrafficLight::cycleThroughPhases, this)); 
}

// virtual function which is executed in a thread
void TrafficLight::cycleThroughPhases()
{
    // FP.2a : Implement the function with an infinite loop that measures the time between two loop cycles 
    // and toggles the current phase of the traffic light between red and green and sends an update method 
    // to the message queue using move semantics. The cycle duration should be a random value between 4 and 6 seconds. 
    // Also, the while-loop should use std::this_thread::sleep_for to wait 1ms between two cycles. 
  
    std::vector<int> random_seconds {4, 5, 6};
    srand(std::time(0)); // use current time as seed for random generator
    int waiting_duration_second = random_seconds[std::rand() % random_seconds.size()];
    // std::cout << waiting_duration_second << std::endl;
    auto previous_time = std::chrono::system_clock::now();
    // infinite loop 
    while(true) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
        auto current_time = std::chrono::system_clock::now();
        auto elapsed_time = std::chrono::duration_cast<std::chrono::seconds> (current_time - previous_time).count();  

        if(elapsed_time >= waiting_duration_second) {
            _current_phase = _current_phase == TrafficLightPhase::green ? TrafficLightPhase::red : TrafficLightPhase::green;
            _message_queue.send(std::move(_current_phase));
            // reset the time and duration
            srand(std::time(NULL));
            waiting_duration_second = random_seconds[std::rand() % random_seconds.size()];
            previous_time = std::chrono::system_clock::now();
            
        }
    }
  
}
