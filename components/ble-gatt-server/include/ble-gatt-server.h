/**
 * @file ble-controller.h
 * @author Pedro Luis Dionísio Fraga (pedrodfraga@hotmail.com)
 * @brief Bluetooth Low Energy GATT server controller for ESP32
 * @version 0.1
 * @date 2025-05-31
 *
 * @copyright Copyright (c) 2025
 *
 */

#ifndef BLE_GATT_SERVER_H
#define BLE_GATT_SERVER_H

#include <esp_err.h>
#include <esp_gatt_defs.h>
#include <esp_gatts_api.h>
#include <stdint.h>

typedef void(ble_gatts_cb_t)(esp_gatts_cb_event_t event, esp_gatt_if_t p_gatts_if, esp_ble_gatts_cb_param_t *param);

/**
 * @brief Configuration structure for BLE GATT server
 *
 * Contains all necessary parameters to initialize the BLE GATT server
 * with custom settings for advertising and device information.
 */
typedef struct
{
  char *device_name;     /**< Name of the BLE device shown during discovery */
  uint32_t service_uuid; /**< UUID of the primary service */
  uint16_t bonding_mode; /**< Enable/disable bonding (0=disabled, 1=enabled) */
  uint8_t start_flag;    /**< Custom start flag for advertising */
  uint8_t end_flag;      /**< Custom end flag for advertising */
} ble_gatt_server_config_t;

typedef struct
{
  esp_gatts_cb_t profile_cb;
  esp_gatt_if_t gatts_if;
  esp_gatt_srvc_id_t service_id_info;
  uint16_t service_handle;
  uint16_t app_id;
  uint16_t conn_id;
  uint16_t char_handle;
  esp_bt_uuid_t char_uuid;
  esp_gatt_char_prop_t property;
  uint16_t descr_handle;
  esp_bt_uuid_t descr_uuid;
  char *profile_name;
  esp_gatts_attr_db_t gatt_db[2];
  esp_gatt_perm_t perm;
} ble_profile_t;

/**
 * @brief Initialize the BLE GATT server
 *
 * Sets up the BLE stack, configures GAP, GATT, and GATTS components
 * and starts advertising based on the provided configuration.
 *
 * @param[in] c Pointer to configuration structure
 * @return void
 */
void ble_gatt_server_init(ble_gatt_server_config_t *c);

/**
 * @brief Deinitialize the BLE GATT server
 *
 * Stops advertising, closes all active connections and releases BLE resources.
 *
 * @return void
 */
void ble_gatt_server_deinit();

/**
 * @brief Add a BLE profile to the controller
 *
 * Registers a new profile with the BLE controller and assigns it an app_id.
 * If BLE is already initialized, the profile will be registered immediately.
 *
 * @param[in] profile Pointer to the profile structure to add
 * @return esp_err_t ESP_OK on success, error code otherwise
 */
esp_err_t ble_gatt_server_add_profile(ble_profile_t *profile);

#endif  // BLE_GATT_SERVER_H
