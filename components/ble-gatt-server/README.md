# BLE GATT Server
Bluetooth Low Energy (BLE) stacks (like Android’s BlueDroid) are organized in layers, each handling a specific function. At the top are GATT and ATT, which manage how data is structured and exchanged as attributes. GAP handles how devices advertise, discover, and connect. Lower down, L2CAP multiplexes data channels and handles packet size (MTU), and HCI is the interface to the radio hardware.

## UUID (Universally Unique Identifier)
A 128-bit value used to uniquely identify information in computer systems for distinguishing services and characteristics, ensuring that each can be uniquely identified across devices.

## Bluedroid
A Google's Bluetooth protocol stack implementation originally created for Android. The Bluedroid Stack is a layered architecture handling everything from low-level radio operations to high-level application interfaces.

## Service and Characteristic
In BLE, a **service** is a collection of data and associated behaviors that encapsulate a specific functionality (e.g., heart rate monitoring). A **characteristic** is a single piece of data or a property within a service, which can be read, written, or notified. Each characteristic has a unique UUID and may include properties like read, write, notify, etc.

## Profile
A collection of services that form a specific use case, each profile contains:
- A single service.
- Multiple characteristics under that service.
- A state machine (callback) to handle BLE events.

### Define Your Profile's Purpose
For our example, let's create an "Environmental Sensor" profile.
1. Define the characteristics:
    - Temperature (read, notify)
    - Humidity (read)
    - Control (write)
2. Define each service UUID and characteristic:
    ```c
    // Service and characteristic UUIDs
    #define ENV_SERVICE_UUID           0x181A  // Environmental Sensing service (standard UUID)
    #define TEMP_CHAR_UUID             0x2A6E  // Temperature characteristic (standard UUID)
    #define HUMID_CHAR_UUID            0x2A6F  // Humidity characteristic (standard UUID)
    #define CONTROL_CHAR_UUID          0xABCD  // Custom control characteristic

    // Descriptor UUIDs
    #define CHAR_CLIENT_CONFIG_UUID    0x2902  // Client Characteristic Configuration

    void env_sensor_profile_init(void);
    void env_sensor_update_temperature(float temp);
    void env_sensor_update_humidity(float humidity);
    ```
3. Create profile implementation handlers:
    ```c
    static void env_sensor_profile_event_handler(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param) {
        switch (event) {
            case ESP_GATTS_REG_EVT:
                // Register service and characteristics
                break;
            case ESP_GATTS_CREATE_EVT:
                // Create service and characteristics
                break;
            case ESP_GATTS_READ_EVT:
                // Handle read requests for temperature or humidity
                break;
            case ESP_GATTS_WRITE_EVT:
                // Handle write requests to control characteristic
                break;
            case ESP_GATTS_NOTIFY_EVT:
                // Notify clients of temperature or humidity changes
                break;
            default:
                break;
        }
    }
    ```

## GAP (Generic Access Profile)
GAP defines how BLE devices make themselves known and connect to each other. It specifies roles and procedures for advertising, scanning, connection establishment, and basic security.

## GATT (Generic Attribute Profile)
GATT establishes how data is organized and exchanged between two connected BLE devices. It manages reading, writing, notifying, and discovering attribute values between a GATT client (usually a smartphone) and a GATT server (the BLE peripheral).

### ATT (Attribute Protocol)
ATT is the protocol that underlies GATT. It defines how a GATT server exposes a table of attributes and how a GATT client accesses them. ATT is responsible for the low-level mechanics of reading and writing attribute values on the server.

## GATTS (GATT Server)
The GATT Server is the device (or role) that hosts the BLE service and characteristic data. It manages connections and disconnections and the local database of attributes that other devices can read or write. A device acting as a peripheral (e.g. a fitness tracker or sensor) typically runs a GATT server.

## Advertisement
An advertising payload can be up to 31 bytes of data.\
Possibles advertising type flags:
| Flag Value | ESP-IDF Constant                    | Description                             |
|------------|-------------------------------------|-----------------------------------------|
| 0x01       | ESP_BLE_ADV_FLAG_LIMIT_DISC         | Limited Discoverable Mode               |
| 0x02       | ESP_BLE_ADV_FLAG_GEN_DISC           | General Discoverable Mode               |
| 0x04       | ESP_BLE_ADV_FLAG_BREDR_NOT_SPT      | BR/EDR Not Supported (BLE only)         |
| 0x08       | ESP_BLE_ADV_FLAG_DMT_CONTROLLER_SPT | Simultaneous LE and BR/EDR (Controller) |
| 0x10       | ESP_BLE_ADV_FLAG_DMT_HOST_SPT       | 	Simultaneous LE and BR/EDR (Host)      |
| 0x00       | ESP_BLE_ADV_FLAG_NON_LIMIT_DISC     | Unlimited Discoverable Mode             |

## MTU (Maximum Transmission Unit)
The maximum size of data packets that can be sent in a single BLE data exchange. Default is 23 bytes, but can be negotiated to be larger.

## Piconet
A piconet is a set of devices connected wirelessly in an ad-hoc manner. This consists of one primary device and up to seven secondary devices.

## Scatternet
This is a bluetooth network consisting of two or more piconets that support communication between more than eight devices.

## Dictionary
- **bluedroid**: Google's Bluetooth protocol stack implementation originally created for Android.
- bluedroid stack?
- `GAP`, `GATT` and `GATTS`?
- the difference between `GATT` and `GAP`?
- `MTU`?
- `ATT`?
- `UUID`?
- `service` and `characteristic`?

## References
- [ESP-IDF GATT Server Walkthrough Documentation](https://github.com/espressif/esp-idf/blob/master/examples/bluetooth/bluedroid/ble/gatt_server/tutorial/Gatt_Server_Example_Walkthrough.md)
- [Advertising flags](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/bluetooth/esp_gap_ble.html#macros)
- [Bluetooth (BlueDroid) Stack Layers](docs/Bluetooth%20(BlueDroid)%20Stack%20Layers.pdf)
- [Sample BLE code](C:/Users/pedro/esp/v5.4.1/esp-idf/components/protocomm/src/simple_ble/simple_ble.c)
