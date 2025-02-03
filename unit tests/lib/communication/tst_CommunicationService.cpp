#include <gtest/gtest.h>
#include <zephyr/fff.h>
#include <zephyr/kernel.h>

#include "../mocks/BLEComm/MockBLEComm.h"
#include "../mocks/poweroff/MockPoweroff.h"
#include <communication/CommunicationService.h>
#include <communication/PowerOffHandler.h>

using namespace ::testing;

class CommunicationServiceTest : public ::testing::Test
{
protected:
    EventDispatcher mEventDispatcher;

    CommunicationService mCommunicationService;

    CommunicationServiceTest() : mCommunicationService(mEventDispatcher) { ; }

    virtual ~CommunicationServiceTest() {}

    virtual void SetUp() override
    {
        MockPowerOff::setup();
        MockBLEComm::setup();

        mCommunicationService.init();
    }
};

TEST_F(CommunicationServiceTest, On_CGMSessionStateChangedEvent_Started_Should_Not_Stop)
{
    // Arrange

    // Act
    mEventDispatcher.dispatch(CGMSessionStateChangedEvent{CGMSessionStateChangedEvent::State::STARTED});
    k_sleep(K_SECONDS(1000));

    // Assert
    ASSERT_EQ(sys_poweroff_fake.call_count, 0);
}

TEST_F(CommunicationServiceTest, On_CGMSessionStateChangedEvent_Stopped_Should_PowerOffAfterDelay)
{
    // Arrange

    // Act
    mEventDispatcher.dispatch(CGMSessionStateChangedEvent{CGMSessionStateChangedEvent::State::STOPPED});
    k_sleep(K_SECONDS(1000));

    // Assert
    ASSERT_EQ(sys_poweroff_fake.call_count, 1);
}

TEST_F(CommunicationServiceTest, On_CGMSessionStateChangedEvent_Stopped_Should_Not_Stop)
{
    // Arrange

    // Act
    mEventDispatcher.dispatch(CGMSessionStateChangedEvent{CGMSessionStateChangedEvent::State::STOPPED});
    k_sleep(K_SECONDS(890)); // Normally it passes 900 seconds as an argument before it powers off, but now its less so
                             // it shouldnt stop

    // Assert
    ASSERT_EQ(sys_poweroff_fake.call_count, 0);
}
