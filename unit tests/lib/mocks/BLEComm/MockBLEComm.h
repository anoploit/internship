#include <zephyr/fff.h>

#include "MockBLEComm.h"

extern "C" {
DEFINE_FAKE_VALUE_FUNC(int, bt_passkey_set, unsigned int);
DEFINE_FAKE_VALUE_FUNC(int, bt_conn_auth_cb_register, const struct bt_conn_auth_cb *);
DEFINE_FAKE_VALUE_FUNC(int, bt_set_name, const char *);
DEFINE_FAKE_VALUE_FUNC(int, bt_conn_auth_info_cb_register, struct bt_conn_auth_info_cb *);
DEFINE_FAKE_VALUE_FUNC(int, bt_enable, bt_ready_cb_t);
DEFINE_FAKE_VALUE_FUNC(int, bt_unpair, uint8_t, const bt_addr_le_t*);
DEFINE_FAKE_VOID_FUNC(bt_conn_cb_register, struct bt_conn_cb *);
DEFINE_FAKE_VALUE_FUNC(int, bt_le_adv_start, const struct bt_le_adv_param *, const struct bt_data *, size_t,const struct bt_data *, size_t);
DEFINE_FAKE_VALUE_FUNC(int, bt_le_oob_get_local, uint8_t, struct bt_le_oob *);
DEFINE_FAKE_VALUE_FUNC(int, bt_rand, void *, size_t);
DEFINE_FAKE_VALUE_FUNC(int, bt_conn_le_param_update, struct bt_conn *, const struct bt_le_conn_param *);
DEFINE_FAKE_VALUE_FUNC(int, bt_gatt_exchange_mtu, struct bt_conn *, struct bt_gatt_exchange_params *);
DEFINE_FAKE_VALUE_FUNC(int, bt_conn_disconnect, struct bt_conn *, uint8_t);
DEFINE_FAKE_VALUE_FUNC(uint16_t, bt_gatt_get_mtu, struct bt_conn *);
DEFINE_FAKE_VALUE_FUNC(int, bt_conn_get_info, const struct bt_conn *, struct bt_conn_info *);
DEFINE_FAKE_VALUE_FUNC(int, bt_conn_auth_cancel, struct bt_conn *);
DEFINE_FAKE_VALUE_FUNC(int, bt_le_oob_set_sc_data, struct bt_conn *, const struct bt_le_oob_sc_data *, const struct bt_le_oob_sc_data *);
DEFINE_FAKE_VALUE_FUNC(const char*, bt_get_name);
DEFINE_FAKE_VALUE_FUNC(const bt_addr_le_t *, bt_conn_get_dst, const struct bt_conn *);
DEFINE_FAKE_VOID_FUNC(bt_conn_security_changed, struct bt_conn *, bt_security_t, enum bt_security_err);
}

struct bt_conn_cb *MockBLEComm::callbacks = nullptr;
