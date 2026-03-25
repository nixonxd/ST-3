// Copyright 2021 GHA Test Team
#include "TimedDoor.h"
#include <stdexcept>
#include <thread>
#include <chrono>

// DoorTimerAdapter implementation
DoorTimerAdapter::DoorTimerAdapter(TimedDoor& d) : door(d) {}

void DoorTimerAdapter::Timeout() {
    if (door.isDoorOpened()) {
        door.throwState();
    }
}

// TimedDoor implementation
TimedDoor::TimedDoor(int timeout) : iTimeout(timeout), isOpened(false), adapter(new DoorTimerAdapter(*this)) {}

TimedDoor::~TimedDoor() {
    delete adapter;
}

bool TimedDoor::isDoorOpened() {
    return isOpened;
}

void TimedDoor::unlock() {
    isOpened = true;
    timer.tregister(iTimeout, adapter);
}

void TimedDoor::lock() {
    isOpened = false;
}

int TimedDoor::getTimeOut() const {
    return iTimeout;
}

void TimedDoor::throwState() {
    throw std::runtime_error("Door was left open too long");
}

// Timer implementation
void Timer::sleep(int seconds) {
    std::this_thread::sleep_for(std::chrono::seconds(seconds));
}

void Timer::tregister(int timeout, TimerClient* client) {
    this->client = client;
    sleep(timeout);
    client->Timeout();
}
