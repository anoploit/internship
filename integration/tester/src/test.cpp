#include <zephyr/kernel.h>
#include <zephyr/ztest.h>
#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/conn.h>
#include <zephyr/bluetooth/gatt.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(bt_scan_by_name);

const struct bt_le_oob_sc_data *oobdLocal;
const struct bt_le_oob_sc_data *oobdRemote;
static struct bt_conn *mConn = nullptr;
K_SEM_DEFINE(conn_sem, 0, 1);  // Semaphore to synchronize connection

static struct bt_uuid_16 cgm_uuid = BT_UUID_INIT_16(BT_UUID_CGMS_VAL);

static char wantedName[] = "SEN-GS-1-INTEGRATION-DUT"; // Name of dut
static struct bt_conn_le_create_param *create_param = BT_CONN_LE_CREATE_CONN;
static struct bt_le_conn_param *param = BT_LE_CONN_PARAM_DEFAULT;

static void discover_cgm_measurement(struct bt_conn *conn);

static struct k_work discover_work;

static void discover_work_handler(struct k_work *work)
{
    // This is executed in the main context and can safely call discover_cgm_measurement.
    if (mConn != NULL) {
        discover_cgm_measurement(mConn);
    }
}

// Callback for when a connection is established
static void connected(struct bt_conn *conn, uint8_t err)
{
    if (err) {
        LOG_ERR("Failed to connect (err %u)", err);
        return;
    }
    mConn = bt_conn_ref(conn);
    LOG_INF("Connected to device");

    int err_pair = bt_conn_set_security(conn, BT_SECURITY_L4); // Start pairing with security level 4
    if (err_pair) {
        LOG_ERR("Failed to start pairing (err %d)", err_pair);
    }

    // Signal that the connection has been established
    k_sem_give(&conn_sem);
}

// Callback for when the connection is disconnected
static void disconnected(struct bt_conn *conn, uint8_t reason)
{
    LOG_INF("Disconnected (reason %u)", reason);
    if (mConn) {
        bt_conn_unref(mConn);
        mConn = nullptr;
    }
}

// Callback when security level changes
static void securityChanged(struct bt_conn *conn, bt_security_t level, enum bt_security_err err)
{
    if (err) {
        LOG_ERR("Security error: %d", err);
    } else {
        LOG_INF("Security level changed to %d", level);
    }
}

// Callback to display passkey during pairing
static void passkeyDisplay(struct bt_conn *conn, unsigned int passkey)
{
    LOG_INF("Passkey: %06u", passkey);
}

// Callback to handle passkey entry during pairing
static void passkeyEntry(struct bt_conn *conn)
{
    unsigned int passkey = 130000;  // Example passkey
    LOG_INF("Please enter passkey...");
    bt_conn_auth_passkey_entry(conn, passkey);
}

// Callback to handle pairing cancel
static void authCancel(struct bt_conn *conn)
{
    LOG_INF("Pairing cancelled");
}

// Callback to confirm pairing
static void pairingConfirm(struct bt_conn *conn)
{
    LOG_INF("Pairing Confirmed");
}

// Callback when pairing completes
static void pairingComplete(struct bt_conn *conn, bool bonded)
{
    LOG_INF("Pairing complete, bonded: %d", bonded);
    
    // TODO: Fix this
    // k_work_submit(&discover_work);
}

// Callback when pairing fails
static void pairingFailed(struct bt_conn *conn, enum bt_security_err reason)
{
    LOG_ERR("Pairing failed: %d", reason);
}

// Register Bluetooth callbacks
static void bt_register_callbacks(void)
{
    static struct bt_conn_cb connCallbacks = {
        .connected = connected,
        .disconnected = disconnected,
        .security_changed = securityChanged,
    };

    bt_conn_cb_register(&connCallbacks);

    static const struct bt_conn_auth_cb authCb = {
        .passkey_display = passkeyDisplay,
        .passkey_entry = passkeyEntry,
        .cancel = authCancel,
        .pairing_confirm = pairingConfirm,
    };

    int err = bt_conn_auth_cb_register(&authCb);
    if (err) {
        LOG_ERR("Failed to register auth callback: %d", err);
    }

    static struct bt_conn_auth_info_cb connAuthInfoCallbacks = {
        .pairing_complete = pairingComplete,
        .pairing_failed = pairingFailed,
    };

    err = bt_conn_auth_info_cb_register(&connAuthInfoCallbacks);
    if (err) {
        LOG_ERR("Failed to register auth info callback: %d", err);
    }
}

static void work_init(void)
{
    k_work_init(&discover_work, discover_work_handler);  // Initialize the work item
}

// Callback function to parse advertisement data and match the target name
static bool bt_scan_find_name_cb_data_cb(struct bt_data *data, void *user_data)
{
    if (user_data == nullptr) return false;

    bool *deviceFound = (bool *)user_data;
    if (data->type == BT_DATA_NAME_COMPLETE || data->type == BT_DATA_NAME_SHORTENED) {
        if (memcmp(data->data, wantedName, data->data_len) == 0) {
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
    bool targetFound = false;
    bt_data_parse(buf, bt_scan_find_name_cb_data_cb, &targetFound);
    if (targetFound) {
        int err = bt_le_scan_stop();
        if (err) {
            LOG_ERR("Failed to stop scanning (err %d)", err);
        }

        // Initiate connection
        err = bt_conn_le_create(addr, create_param, param, &mConn);
        if (err) {
            LOG_ERR("Failed to create connection (err %d)", err);
        }
    }
}

static bool gatt_discovery_called = false;

static unsigned char discover_cgm_characteristics_cb(struct bt_conn *conn, const struct bt_gatt_attr *attr, 
                                                bt_gatt_discover_params *params)
{
    LOG_INF("Inside discover cgm cb!");    
    static struct bt_gatt_read_params read_params = {
        .func = NULL,
        .handle_count = 1, 
        .single = {
        .handle = attr->handle, // The handle from the discovered attribute
        .offset = 0 // Start reading from offset 0
        }    
    };


    if (bt_uuid_cmp(attr->uuid, &cgm_uuid.uuid) == 0) {
        LOG_INF("CGM Measurement characteristic found!");
        
        int err = bt_gatt_read(conn, &read_params);
        if(err){
            LOG_ERR("Failed to read characteristic (err %d)", err);
        }
        gatt_discovery_called = true;
    }
    return 0;
}


static void discover_cgm_measurement(struct bt_conn *conn)
{
    if (conn == NULL) {
        LOG_ERR("Connection is NULL, cannot proceed with discovery!");
        return;
    }
    LOG_INF("Inside discover cgm measurement!");
    struct bt_gatt_discover_params discover_params = {
        .uuid = &cgm_uuid.uuid,
        .func = discover_cgm_characteristics_cb,
        .start_handle = 0x0001,                     
        .end_handle = 0xFFFF,                       
        .type = BT_GATT_DISCOVER_PRIMARY,           
    };

    int err = bt_gatt_discover(conn, &discover_params);
    if (err) {
        LOG_ERR("Failed to discover services (err %d)", err);
    }
}

// Start scanning for the target device
static void bt_start_scan(void)
{
    struct bt_le_scan_param scanParams = {
        .type = BT_LE_SCAN_TYPE_ACTIVE,
        .options = BT_LE_SCAN_OPT_NONE,
        .interval = BT_GAP_SCAN_FAST_INTERVAL,
        .window = BT_GAP_SCAN_FAST_WINDOW,
    };

    int err = bt_le_scan_start(&scanParams, bt_scan_find_name_cb);
    if (err) {
        LOG_ERR("Scanning failed to start (err %d)", err);
    } else {
        LOG_INF("Scanning for devices...");
    }
}

// Function to initialize Bluetooth and set up necessary callbacks
static int ble_init(void)
{
    int err;

    LOG_INF("Starting Bluetooth scan by name example");

    // Register Bluetooth callbacks
    bt_register_callbacks();
    work_init();
    
    // Enable Bluetooth
    err = bt_enable(NULL);
    if (err) {
        LOG_ERR("Bluetooth init failed (err %d)", err);
        return err;
    }

    // Start scanning for devices
    bt_start_scan();

    return 0; // Success
}

// Function to wait for the connection to be established
static void wait_for_connection(void)
{
    int err = k_sem_take(&conn_sem, K_FOREVER);  // Wait on semaphore
    if (err) {
        LOG_ERR("Failed to wait for connection semaphore (err %d)", err);
    }
}

// Test case to verify BLE initialization
ZTEST(ble_test_suite, test_ble_init)
{
    int err;
    err = ble_init();  // Initialize BLE
    zassert_equal(err, 0, "BLE initialization failed!");  // Assert that initialization succeeded

    // Wait for the Bluetooth connection to be established
    wait_for_connection();

    LOG_INF("BLE scan started successfully");

    // Check that a connection has been established
    zassert_not_null(mConn, "No active connection!");
    // zassert_true(gatt_discovery_called, "No gatt discovery called!");
    // zassert_equal(gatt_discovery_called, 1, "No gatt discovery called!");
    
}

ZTEST_SUITE(ble_test_suite, NULL, NULL, NULL, NULL, NULL);
