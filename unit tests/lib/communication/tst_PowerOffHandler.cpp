#include <gtest/gtest.h>
#include <zephyr/fff.h>
#include <zephyr/kernel.h>

#include "../mocks/errorHandler/MockErrorHandler.h"
#include "../mocks/nrfPower/MockNrf_power.h"
#include "../mocks/poweroff/MockPoweroff.h"
#include <communication/PowerOffHandler.h>
#include <events/EventDispatcher.h>

using namespace ::testing;

class PowerOffHandlerTest : public ::testing::Test
{
protected:
    EventDispatcher mEventDispatcher;

    virtual ~PowerOffHandlerTest() {}

    virtual void SetUp() override
    {
        MockNrf::setup();
        MockPowerOff::setup();
    }
};

TEST_F(PowerOffHandlerTest, On_Init_Should_Call_PowerOff_On_NFC)
{
    // Arrange
    nrf_power_resetreas_get_fake.return_val = NRF_POWER_RESETREAS_NFC_MASK;

    // Act
    PowerOffHandler::init();
    k_sleep(K_SECONDS(130));

    // Assert
    ASSERT_EQ(sys_poweroff_fake.call_count, 1);
}

TEST_F(PowerOffHandlerTest, On_Init_Should_Call_PowerOff_On_SoftwareReset)
{
    // Arrange
    nrf_power_resetreas_get_fake.return_val = NRF_POWER_RESETREAS_SREQ_MASK;
    ErrorHandler__getErrorInfo_fake.return_val = {ErrorHandler::RESET_REQUEST};

    // Act
    PowerOffHandler::init();
    k_sleep(K_SECONDS(130));

    // Assert
    ASSERT_EQ(sys_poweroff_fake.call_count, 1);
}

TEST_F(PowerOffHandlerTest, On_Init_Should_Call_PowerOff_After_There_Is_No_Session_Detected)
{
    // Arrange
    nrf_power_resetreas_get_fake.return_val = NRF_POWER_RESETREAS_SREQ_MASK;
    ErrorHandler__getErrorInfo_fake.return_val = {ErrorHandler::NONE};

    // Act
    PowerOffHandler::init();
    k_sleep(K_SECONDS(1000));

    // Assert
    ASSERT_EQ(sys_poweroff_fake.call_count, 1);
}

TEST_F(PowerOffHandlerTest, On_Init_Should_Call_PowerOff_On_Other_Instances)
{
    // Arrange
    nrf_power_resetreas_get_fake.return_val =
        NRF_POWER_RESETREAS_LOCKUP_MASK; // Could also be left empty and not done anything with it

    // Act
    PowerOffHandler::init();
    k_sleep(K_SECONDS(120));

    // Assert
    ASSERT_EQ(sys_poweroff_fake.call_count, 1);
}
