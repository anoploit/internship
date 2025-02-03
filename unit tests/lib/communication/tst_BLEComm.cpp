#include <gtest/gtest.h>
#include <zephyr/fff.h>
#include <zephyr/kernel.h>

#include "../mocks/BLEComm/MockBLEComm.h"
#include "../mocks/errorHandler/MockErrorHandler.h"
#include "../mocks/poweroff/MockPoweroff.h"

#include <communication/BLEComm.h>
#include <communication/PowerOffHandler.h>
#include <utils/SensibleUICR.h>

using namespace ::testing;

class BLECommunicationTest : public ::testing::Test
{
protected:
    BLECommunicationTest() { ; }

    virtual ~BLECommunicationTest() {}

    virtual void SetUp() override
    {
        MockBLEComm::setup();
        MockPowerOff::setup();
        bt_conn_cb_register(&BLEComm::connCallbacks);
    }
};

TEST_F(BLECommunicationTest, On_Ble_Init_Should_Succeed)
{
    // Arrange
    Uicr::serial = 123456;
    unsigned int expected_passkey = (CONFIG_BT_FIXED_PASSKEY_VALUE + Uicr::serial) % 1000000;

    // Mock behavior for successful calls
    bt_passkey_set_fake.return_val = 0;
    bt_conn_auth_cb_register_fake.return_val = 0;
    bt_conn_auth_info_cb_register_fake.return_val = 0;
    bt_le_adv_start_fake.return_val = 0;

    // Act
    BLEComm::init();

    // Assert
    ASSERT_EQ(bt_passkey_set_fake.call_count, 1);
    ASSERT_EQ(bt_passkey_set_fake.arg0_val, expected_passkey);
    ASSERT_EQ(bt_conn_auth_cb_register_fake.call_count, 1);
    ASSERT_EQ(bt_conn_auth_info_cb_register_fake.call_count, 1);
    ASSERT_EQ(bt_enable_fake.call_count, 1);
    ASSERT_EQ(bt_le_adv_start_fake.call_count, 1);
    ASSERT_EQ(ErrorHandler__applicationErrorHandler_fake.call_count, 0);
}

TEST_F(BLECommunicationTest, On_Ble_Callback_Should_Connect)
{
    // Arrange

    MockBLEComm::custom_bt_conn_cb_register(&BLEComm::connCallbacks);

    struct bt_conn *conn = nullptr;
    uint8_t error_code = 0;

    // Act
    if (MockBLEComm::callbacks && MockBLEComm::callbacks->connected)
    {
        // Call the connected callback
        MockBLEComm::callbacks->connected(conn, error_code);
    }

    // Assert
    EXPECT_EQ(ErrorHandler__applicationErrorHandler_fake.call_count, 0);
    EXPECT_EQ(bt_gatt_exchange_mtu_fake.call_count, 1);
}

TEST_F(BLECommunicationTest, On_Ble_Callback_Change_Security_Should_Fail)
{
    // Arrange
    MockBLEComm::custom_bt_conn_cb_register(&BLEComm::connCallbacks);

    struct bt_conn *conn = nullptr;

    // Act
    if (MockBLEComm::callbacks && MockBLEComm::callbacks->connected)
    {
        // Call the connected callback
        MockBLEComm::callbacks->security_changed(conn, BT_SECURITY_L0, BT_SECURITY_ERR_AUTH_FAIL);
    }

    // Assert
    EXPECT_EQ(bt_conn_disconnect_fake.call_count, 1);
}
