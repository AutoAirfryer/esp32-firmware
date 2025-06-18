/**
 * @file ble-table.h
 * @author Pedro Luis Dionísio Fraga (pedrodfraga@hotmail.com)
 * @brief
 * @version 0.1
 * @date 2025-06-09
 *
 * @copyright Copyright (c) 2025
 *
 */

#ifndef BLE_TABLE_H
#define BLE_TABLE_H

#include <esp_gatts_api.h>  // Implements GATT Server configuration such as creating services and characteristics.

#define SERVICE_UUID 0xED58

//                ADVERTISEMENT DATA                                |      SCAN RESPONSE DATA
// 0x|020106|020AEB|0303ED58|021980|0E09'4175746F2D4169726672796572'|020106020AEB0303FF00
// 000000000000000000000000000000000000000000000000

// TODO: Implement start, end flags and service UUID for advertising data
static uint8_t raw_adv_data[] = {
  //* Flags */
  0x02,  // Length (1 byte for flag type + 1 byte for flag value)
  ESP_BLE_AD_TYPE_FLAG,
  (ESP_BLE_ADV_FLAG_GEN_DISC | ESP_BLE_ADV_FLAG_BREDR_NOT_SPT),  // General discoverable mode, BR/EDR not
                                                                 // supported

  //* TX Power Level */
  0x02,  // Length (1 byte for flag type + 1 byte for flag value)
  ESP_BLE_AD_TYPE_TX_PWR,
  0xEB,  // TX power level

  //* Service UUID (16 bit) */
  0x03,                        // Length (1 byte for type + 2 bytes UUID)
  ESP_BLE_AD_TYPE_16SRV_CMPL,  // AD type: Complete list of 16-bit UUIDs
  (SERVICE_UUID >> 8) & 0xFF,  // UUID MSB (0xAB)
  (SERVICE_UUID) & 0xFF,       // UUID LSB (0xCD)

  //* Appearance */
  0x02,
  ESP_BLE_AD_TYPE_APPEARANCE,
  ESP_BLE_APPEARANCE_GENERIC_COMPUTER,

  //* Complete name */
  0x0E,                       // Length of name + 1
  ESP_BLE_AD_TYPE_NAME_CMPL,  // Complete local name
  'A',
  'u',
  't',
  'o',
  '-',
  'A',
  'i',
  'r',
  'f',
  'r',
  'y',
  'e',
  'r',
};

// TODO: Study the ESP-IDF documentation to understand it
// `rsp` is the scan response data that will be sent when a device scans for this BLE device.
static uint8_t raw_scan_rsp_data[] = {
  /* Flags */
  0x02,
  ESP_BLE_AD_TYPE_FLAG,
  0x06,
  /* TX Power Level */
  0x02,
  ESP_BLE_AD_TYPE_TX_PWR,
  0xEB,
  /* Complete 16-bit Service UUIDs */
  0x03,
  ESP_BLE_AD_TYPE_16SRV_CMPL,
  0xFF,
  0x00,
};

// TODO: Study the ESP-IDF documentation to understand it
static esp_ble_adv_params_t adv_params = {
  .adv_int_min = 0x20,
  .adv_int_max = 0x40,
  .adv_type = ADV_TYPE_IND,
  .own_addr_type = BLE_ADDR_TYPE_PUBLIC,
  .channel_map = ADV_CHNL_ALL,
  .adv_filter_policy = ADV_FILTER_ALLOW_SCAN_ANY_CON_ANY,
};

// `2` is the number of attributes in the GATT database.
// Example: Simple GATT DB with one service and one characteristic
#define GATTS_NUM_HANDLES 3

static uint16_t primary_service_uuid = ESP_GATT_UUID_PRI_SERVICE; // 0x2800
static uint16_t my_service_uuid = 0x180A; // Example service UUID
static uint16_t char_decl_uuid = ESP_GATT_UUID_CHAR_DECLARE; // 0x2803
static uint16_t my_char_uuid = 0x2A57; // Example characteristic UUID

static uint8_t char_property = ESP_GATT_CHAR_PROP_BIT_READ | ESP_GATT_CHAR_PROP_BIT_WRITE;
static uint8_t char_value[20] = {0}; // Initial value

static const esp_gatts_attr_db_t gatt_db[GATTS_NUM_HANDLES] = {
    // Service Declaration
    [0] = {
        .attr_control = { .auto_rsp = ESP_GATT_AUTO_RSP },
        .att_desc = {
            .uuid_length = ESP_UUID_LEN_16,
            .uuid_p = (uint8_t *)&primary_service_uuid,
            .perm = ESP_GATT_PERM_READ,
            .max_length = sizeof(uint16_t),
            .length = sizeof(my_service_uuid),
            .value = (uint8_t *)&my_service_uuid,
        },
    },
    // Characteristic Declaration
    [1] = {
        .attr_control = { .auto_rsp = ESP_GATT_AUTO_RSP },
        .att_desc = {
            .uuid_length = ESP_UUID_LEN_16,
            .uuid_p = (uint8_t *)&char_decl_uuid,
            .perm = ESP_GATT_PERM_READ,
            .max_length = sizeof(uint8_t),
            .length = sizeof(uint8_t),
            .value = &char_property,
        },
    },
    // Characteristic Value
    [2] = {
        .attr_control = { .auto_rsp = ESP_GATT_AUTO_RSP },
        .att_desc = {
            .uuid_length = ESP_UUID_LEN_16,
            .uuid_p = (uint8_t *)&my_char_uuid,
            .perm = ESP_GATT_PERM_READ | ESP_GATT_PERM_WRITE,
            .max_length = sizeof(char_value),
            .length = sizeof(char_value),
            .value = char_value,
        },
    },
};

// TODO: Implement characteristic UUIDs and properties

#endif  // BLE_TABLE_H
