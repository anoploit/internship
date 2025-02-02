#include <zephyr/bluetooth/addr.h>
#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/conn.h>
#include <zephyr/bluetooth/gap.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>


LOG_MODULE_REGISTER(bt_scan_by_name);
const struct bt_le_oob_sc_data *oobdLocal;
const struct bt_le_oob_sc_data *oobdRemote;
static struct bt_conn *mConn = nullptr;
static char wantedName[] = "SEN-GS-1-INTEGRATION-DUT"; // Replace with the name you're looking for
static struct bt_conn_le_create_param *create_param = BT_CONN_LE_CREATE_CONN;
static struct bt_le_conn_param *param = BT_LE_CONN_PARAM_DEFAULT;

static void passkeyDisplay(struct bt_conn *conn, unsigned int passkey) { LOG_INF("Passkey: %06u", passkey); }

static void passkeyEntry(struct bt_conn *conn)
{
    unsigned int passkey = 130000;
    LOG_INF("Please enter passkey...");
    bt_conn_auth_passkey_entry(conn, passkey);
}

static void pairingConfirm(struct bt_conn *conn) { LOG_INF("Pairing Confirmed"); }

static void securityChanged(struct bt_conn *conn, bt_security_t level, enum bt_security_err err)
{
    if (err)
    {
        LOG_ERR("Security error: %d", err);
    }
    else
    {
        LOG_INF("Security level changed to %d", level);
    }
}

static void pairingComplete(struct bt_conn *conn, bool bonded) { LOG_INF("Pairing complete, bonded: %d", bonded); }

static void pairingFailed(struct bt_conn *conn, enum bt_security_err reason) { LOG_ERR("Pairing failed: %d", reason); }

static void authCancel(struct bt_conn *conn) { LOG_INF("Pairing cancelled"); }

static void connected(struct bt_conn *conn, uint8_t err)
{
    if (err)
    {
        LOG_ERR("Failed to connect (err %u)", err);
        return;
    }
    mConn = bt_conn_ref(conn);
    LOG_INF("Connected to device");

    int err_pair =
        bt_conn_set_security(conn, BT_SECURITY_L4); // Start pairing with security level 2 (authenticated pairing)
    if (err_pair)
    {
        LOG_ERR("Failed to start pairing (err %d)", err_pair);
    }
}

static void disconnected(struct bt_conn *conn, uint8_t reason)
{
    LOG_INF("Disconnected (reason %u)", reason);
    if (mConn)
    {
        bt_conn_unref(mConn);
        mConn = nullptr;
    }
}

static struct bt_conn_auth_info_cb connAuthInfoCallbacks = {
    .pairing_complete = pairingComplete,
    .pairing_failed = pairingFailed,
};

static const struct bt_conn_auth_cb authCb = {
    .passkey_display = passkeyDisplay, // Display passkey for user
    .passkey_entry = passkeyEntry,     // Handle passkey entry from user
    .cancel = authCancel,
    .pairing_confirm = pairingConfirm, // Confirm the pairing process
};

static struct bt_conn_cb connCallbacks = {
    .connected = connected,
    .disconnected = disconnected,
    .security_changed = securityChanged,
};

// Callback function to parse advertisement data and match the target name
static bool bt_scan_find_name_cb_data_cb(struct bt_data *data, void *user_data)
{
    if (user_data == nullptr) return false;

    bool *deviceFound = (bool *)user_data;
    if (data->type == BT_DATA_NAME_COMPLETE || data->type == BT_DATA_NAME_SHORTENED)
    {
        if (memcmp(data->data, wantedName, data->data_len) == 0)
        {
            LOG_INF("Target device found: %s", wantedName);
            *deviceFound = true;
            return false; // Stop parsing once the name is found
        }
    }
    return true; // Continue parsing if name not found
}

// Scan callback to handle each advertisement packet
static void bt_scan_find_name_cb(const bt_addr_le_t *addr, int8_t rssi, uint8_t adv_type, struct net_buf_simple *buf)
{
    // Parse the advertisement data to look for the device name
    bool targetFound = false;
    bt_data_parse(buf, bt_scan_find_name_cb_data_cb, &targetFound);
    if (targetFound)
    {
        int err = bt_le_scan_stop();
        if (err)
        {
            LOG_ERR("Failed to stop scanning (err %d)", err);
        }

        // Initiate connection
        err = bt_conn_le_create(addr, create_param, param, &mConn);
        if (err)
        {
            LOG_ERR("Failed to create connection (err %d)", err);
        }
    }
}

// Main function to initialize Bluetooth and start scanning
int main()
{
    int err;

    LOG_INF("Starting Bluetooth scan by name example");

    bt_conn_cb_register(&connCallbacks);

    err = bt_conn_auth_cb_register(&authCb);
    if (err)
    {
        LOG_ERR("Failed to register auth callback: %d", err);
        return err;
    }

    err = bt_conn_auth_info_cb_register(&connAuthInfoCallbacks);
    if (err)
    {
        LOG_ERR("Failed to register auth info callback: %d", err);
        return err;
    }
    
    err = bt_enable(nullptr);
    if (err)
    {
        LOG_ERR("Bluetooth init failed (err %d)", err);
        return err;
    }
    
    k_sleep(K_MSEC(10));

    struct bt_le_scan_param scanParams = {
        .type = BT_LE_SCAN_TYPE_ACTIVE,
        .options = BT_LE_SCAN_OPT_NONE,
        .interval = BT_GAP_SCAN_FAST_INTERVAL,
        .window = BT_GAP_SCAN_FAST_WINDOW,
    };

    err = bt_le_scan_start(&scanParams, bt_scan_find_name_cb);
    if (err)
    {
        LOG_ERR("Scanning failed to start (err %d)", err);
        return err;
    }

    LOG_INF("Scanning for devices...");

    return 0;
}
