// Copyright 2021 GHA Test Team

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <cstdint>
#include <thread>
#include <chrono>
#include <future>
#include "TimedDoor.h"

class MockTimerClient : public TimerClient {
  public:
    MOCK_METHOD(void, Timeout, (), (override));
};

class MockDoor : public Door {
  public:
    MOCK_METHOD(void, lock, (), (override));
    MOCK_METHOD(void, unlock, (), (override));
    MOCK_METHOD(bool, isDoorOpened, (), (override));
};

class TimedDoorTest : public ::testing::Test {
  protected:
    void SetUp() override {
        door = new TimedDoor(1);
    }

    void TearDown() override {
        delete door;
    }

    TimedDoor* door;
};

class DoorTimerAdapterTest : public ::testing::Test {
  protected:
    void SetUp() override {
        door = new TimedDoor(1);
        adapter = new DoorTimerAdapter(*door);
    }

    void TearDown() override {
        delete adapter;
        delete door;
    }

    TimedDoor* door;
    DoorTimerAdapter* adapter;
};

TEST_F(TimedDoorTest, ConstructorSetsTimeout) {
    EXPECT_EQ(door->getTimeOut(), 1);
}

TEST_F(TimedDoorTest, InitiallyClosed) {
    EXPECT_FALSE(door->isDoorOpened());
}

TEST_F(TimedDoorTest, LockClosesDoor) {
    door->unlock();
    EXPECT_TRUE(door->isDoorOpened());
    door->lock();
    EXPECT_FALSE(door->isDoorOpened());
}

TEST_F(TimedDoorTest, UnlockOpensDoor) {
    door->unlock();
    EXPECT_TRUE(door->isDoorOpened());
}

TEST_F(DoorTimerAdapterTest, TimeoutThrowsIfDoorOpen) {
    door->unlock();
    EXPECT_THROW(adapter->Timeout(), std::runtime_error);
}

TEST_F(DoorTimerAdapterTest, TimeoutDoesNotThrowIfDoorClosed) {
    door->lock();
    EXPECT_NO_THROW(adapter->Timeout());
}

TEST_F(TimedDoorTest, ThrowStateThrowsException) {
    EXPECT_THROW(door->throwState(), std::runtime_error);
}

TEST(TimerTest, TregisterCallsTimeout) {
    MockTimerClient mockClient;
    Timer timer;
    std::promise<void> p;
    auto f = p.get_future();

    EXPECT_CALL(mockClient, Timeout())
        .Times(1)
        .WillOnce(testing::InvokeWithoutArgs([&p]() {
            p.set_value();
        }));

    timer.tregister(0, &mockClient);
    EXPECT_EQ(f.wait_for(std::chrono::seconds(1)), std::future_status::ready);
}

TEST_F(TimedDoorTest, UnlockStartsTimer) {
    EXPECT_FALSE(door->isDoorOpened());
    door->unlock();
    EXPECT_TRUE(door->isDoorOpened());
}

TEST_F(TimedDoorTest, LockBeforeTimeoutPreventsException) {
    door->unlock();
    door->lock();
    std::this_thread::sleep_for(std::chrono::milliseconds(1500));
    EXPECT_FALSE(door->isDoorOpened());
}

TEST_F(DoorTimerAdapterTest, AdapterHoldsReference) {
    EXPECT_TRUE(adapter != nullptr);
    door->lock();
    EXPECT_NO_THROW(adapter->Timeout());
}

TEST_F(TimedDoorTest, DestructorDeletesAdapter) {
    TimedDoor* testDoor = new TimedDoor(1);
    delete testDoor;
}
