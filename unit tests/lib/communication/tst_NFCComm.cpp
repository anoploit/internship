#include <gtest/gtest.h>
#include <zephyr/fff.h>
#include <zephyr/kernel.h>

#include "../mocks/NFCComm/MockNFCComm.h"
#include "../mocks/errorHandler/MockErrorHandler.h"
#include "../mocks/poweroff/MockPoweroff.h"

#include <communication/NFCComm.h>
#include <communication/PowerOffHandler.h>

using namespace ::testing;

class NfcCommunicationTest : public ::testing::Test
{
protected:
    NfcCommunicationTest() { ; }

    virtual ~NfcCommunicationTest() {}

    virtual void SetUp() override
    {
        MockNFCComm::setup();
        NFCComm::init();
        MockPowerOff::setup();
    }
};

TEST_F(NfcCommunicationTest, On_Nfc_Init_Should_Succeed)
{
    // Arrange

    // Act
    NFCComm::init(); // Call init, which will block until btReady is called

    // Assert
    ASSERT_EQ(ErrorHandler__applicationErrorHandler_fake.call_count, 0);
}
