/**
 * @file ble-gatt-server.c
 * @author Pedro Luis Dionísio Fraga (pedrodfraga@hotmail.com)
 * @brief
 * @version 0.1
 * @date 2025-05-31
 *
 * @copyright Copyright (c) 2025
 *
 */

// Implements BT controller and VHCI configuration procedures from the host side.
#include <esp_bt.h>
#include <esp_bt_device.h>
// Implements initialization and enabling of the Bluedroid stack.
#include <esp_bt_main.h>
#include <esp_gap_ble_api.h>  // Implements GAP configuration such as advertising and connection parameters.
#include <esp_gatt_common_api.h>
#include <esp_gatt_defs.h>  // Implements GATT server API definitions.
#include <esp_log.h>
#include <string.h>

#include "ble-gatt-server.h"
#include "ble-table.h"
#include "nvm_driver.h"

// An user-assigned number to identify each profile
#define ESP_APP_ID   0x55
#define MAX_MTU_SIZE 500
#define MAX_PROFILES 5

#define DEBUG 1

// Macros
#ifdef DEBUG
#define DEBUG_PRINT_ERR(fmt, ...) ESP_LOGE(TAG, fmt, ##__VA_ARGS__)
#else
#define DEBUG_PRINT_ERR(fmt, ...) \
  do                              \
  {                               \
  } while (0)
#endif

#define UNUSED(x) (void)(x)

#define PROFILE_1_APP_ID 0

static ble_profile_t *g_ble_profile;

static const char *TAG = "BLE_GATT_SERVER";
static const char *TAG_GATT = "GATT";
static const char *TAG_GATTS = "GATTS";
static const char *TAG_GAP = "GAP";

static void gatts_profile_event_handler(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if,
                                        esp_ble_gatts_cb_param_t *param);

// Array to store profiles
static ble_profile_t s_profiles[MAX_PROFILES] = {
  [PROFILE_1_APP_ID] =
    {
      .profile_cb = gatts_profile_event_handler,
      .gatts_if = ESP_GATT_IF_NONE,
      .service_id_info =
        {
          .is_primary = true,
          .id =
            {
              .uuid.uuid.uuid16 = ESP_APP_ID,
              .inst_id = 0,
            },
        },
      .service_handle = 0,
      .profile_name = "Default Profile",
      .perm = ESP_GATT_PERM_READ | ESP_GATT_PERM_WRITE,
    },
};

static uint8_t s_num_profiles = 0;

// Forward declarations
static void gap_event_handler(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t *param);
static esp_err_t ble_gap_init(const char *device_name);
static esp_err_t ble_gatt_init();
static esp_err_t ble_gatts_init();

static void gatts_profile_event_handler(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if,
                                        esp_ble_gatts_cb_param_t *param)
{
  esp_err_t ret = ESP_FAIL;
  switch (event)
  {
    case ESP_GATTS_REG_EVT:  // Called when the GATT server is registered
    {
      ESP_LOGI(TAG_GATTS, "GATTS registered, app_id: %d, gatts_if: %d", param->reg.app_id, gatts_if);

      // Find the profile that matches this app_id
      ble_profile_t *current_profile = NULL;
      uint16_t app_id = param->reg.app_id;

      for (int i = 0; i < s_num_profiles; i++)
      {
        if (s_profiles[i].app_id == app_id)
        {
          ESP_LOGW(TAG_GATTS, "Found profiledffasdfasdfdsa '%s' with app_id: %d", s_profiles[i].profile_name, app_id);
          current_profile = &s_profiles[i];
          break;
        }
      }

      // Create attribute table for the service
      esp_err_t ret = esp_ble_gatts_create_attr_tab(gatt_db,            // your attribute table
                                                    gatts_if,           // interface
                                                    GATTS_NUM_HANDLES,  // number of handles
                                                    0                   // service instance id
      );
      if (ret != ESP_OK)
      {
        ESP_LOGE(TAG_GATTS, "Failed to create attribute table, error code = %d", ret);
      }

      break;
    }
    case ESP_GATTS_CREAT_ATTR_TAB_EVT:
    {
      ESP_LOGI(TAG_GATTS, "GATTS attribute table created, status: %d", param->add_attr_tab.status);

      if (param->add_attr_tab.status != ESP_GATT_OK)
      {
        ESP_LOGE(TAG_GATTS, "Failed to create attribute table, status: %d", param->add_attr_tab.status);
        return;
      }
      break;
    }
    default:
    {
      ESP_LOGI(TAG_GATTS, "Unhandled GATTS event: %d", event);
      break;
    }
  }
}

// Central GATTS event handler that dispatches events to profile callbacks
static void gatts_event_handler(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param)
{
  // Handle common GATTS events
  switch (event)
  {
    case ESP_GATTS_REG_EVT:  // Setups upon startup
    {
      ESP_LOGI(TAG_GATTS, "GATTS registered, app_id: %d, gatts_if: %d", param->reg.app_id, gatts_if);

      // Store the gatts_if for each registered profile
      uint16_t app_id = param->reg.app_id;

      // Find the profile with this app_id
      ESP_LOGI(TAG_GATTS, "s_num_profiles: %d", s_num_profiles);
      for (int i = 0; i < s_num_profiles; i++)
      {

        if (s_profiles[i].app_id == app_id)
        {
          ESP_LOGI(TAG_GATTS, "Found fdfdprofile '%s' with app_id: %d", s_profiles[i].profile_name, app_id);
          if (param->reg.status == ESP_GATT_OK)
          {
            ESP_LOGI(TAG_GATTS, "Profile '%s' registered, gatts_if: %d", s_profiles[i].profile_name, gatts_if);
          }

          // Call the profile's callback
          if (s_profiles[i].profile_cb != NULL)
          {
            ESP_LOGI(TAG_GATTS, "Calling profile callback for '%s'", s_profiles[i].profile_name);
            s_profiles[i].profile_cb(event, gatts_if, param);
          }

          break;
        }
      }

      break;
    }
    case ESP_GATTS_CREAT_ATTR_TAB_EVT:
    {
      ESP_LOGI(TAG_GATTS, "GATTS attribute table created, status: %d", param->add_attr_tab.status);

      if (param->add_attr_tab.status != ESP_GATT_OK)
      {
        ESP_LOGE(TAG_GATTS, "Failed to create attribute table, status: %d", param->add_attr_tab.status);
        return;
      }

      // Start the service using the handle from the table
      esp_ble_gatts_start_service(param->add_attr_tab.handles[0]);
      ESP_LOGI(TAG_GATTS, "Service started, handle: %d", param->add_attr_tab.handles[0]);
      break;
    }
    // In the ESP_GATTS_CREATE_EVT handler, modify the characteristic creation code:
    case ESP_GATTS_CREATE_EVT:  // After a service is created
    {
      uint16_t service_handle = param->create.service_handle;
      ESP_LOGI(TAG_GATTS, "GATTS service created, service_handle: %d", service_handle);

      break;
    }
    case ESP_GATTS_ADD_CHAR_EVT:
    {
      ESP_LOGI(TAG_GATTS, "GATTS characteristic added, handle: %d", param->add_char.attr_handle);

      break;
    }
    case ESP_GATTS_READ_EVT:
    {
      ESP_LOGI(TAG_GATTS, "GATTS read event, handle: %d", param->read.handle);
      // If you need custom handling for reads, add it here
      break;
    }

    case ESP_GATTS_WRITE_EVT:
    {
      ESP_LOGI(TAG_GATTS, "GATTS write event, handle: %d, value len: %d", param->write.handle, param->write.len);

      // Log the written value
      if (param->write.len > 0)
      {
        ESP_LOGI(TAG_GATTS, "Written value: ");
        esp_log_buffer_hex(TAG_GATTS, param->write.value, param->write.len);
      }
      break;
    }
    case ESP_GATTS_CONNECT_EVT:
    {
      ESP_LOGI(TAG_GATTS, "GATTS connection established, conn_id: %d", param->connect.conn_id);
      break;
    }
    case ESP_GATTS_DISCONNECT_EVT:
    {
      ESP_LOGI(TAG_GATTS, "GATTS disconnected, restart advertising");
      esp_ble_gap_start_advertising(&adv_params);
      break;
    }
    default:
    {
      ESP_LOGI(TAG_GATTS, "Unhandled GATTS event: %d", event);
      break;
    }
  }
}

static void gap_event_handler(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t *param)
{
  // Handle GAP events here
  switch (event)
  {
    case ESP_GAP_BLE_ADV_DATA_RAW_SET_COMPLETE_EVT:
    {
      ESP_LOGI(TAG_GAP, "Advertising data set successfully");
      esp_ble_gap_start_advertising(&adv_params);
      break;
    }
    case ESP_GAP_BLE_SCAN_RSP_DATA_SET_COMPLETE_EVT:
    {
      ESP_LOGI(TAG_GAP, "Scan response data set successfully");

      // esp_ble_gap_start_advertising(&adv_params);
      break;
    }
    case ESP_GAP_BLE_ADV_START_COMPLETE_EVT:
    {
      if (param->adv_start_cmpl.status != ESP_BT_STATUS_SUCCESS)
        ESP_LOGI(TAG_GAP, "Advertising start failed");
      else
        ESP_LOGI(TAG_GAP, "Advertising start successfully");

      break;
    }
    case ESP_GAP_BLE_ADV_STOP_COMPLETE_EVT:
    {
      if (param->adv_stop_cmpl.status != ESP_BT_STATUS_SUCCESS)
        ESP_LOGI(TAG_GAP, "Advertising stop failed");
      else
        ESP_LOGI(TAG_GAP, "Stop adv successfully");

      break;
    }
    case ESP_GAP_BLE_UPDATE_CONN_PARAMS_EVT:
    {
      ESP_LOGI(TAG_GAP,
               "update connection params status = %d, conn_int = %d, latency = %d, "
               "timeout = %d",
               param->update_conn_params.status,
               param->update_conn_params.conn_int,
               param->update_conn_params.latency,
               param->update_conn_params.timeout);
      break;
    }
    case ESP_GAP_BLE_SCAN_RSP_DATA_RAW_SET_COMPLETE_EVT:
    {
      ESP_LOGI(TAG_GAP, "Scan response data raw set successfully");

      // esp_ble_gap_start_advertising(&adv_params);
      break;
    }
    case ESP_GAP_BLE_SET_PKT_LENGTH_COMPLETE_EVT:
    {
      if (param->pkt_data_length_cmpl.status != ESP_BT_STATUS_SUCCESS)
        ESP_LOGI(TAG_GAP, "Set packet length failed");
      else
        ESP_LOGI(TAG_GAP,
                 "Set packet length successfully, tx_len = %d, rx_len = %d",
                 param->pkt_data_length_cmpl.params.tx_len,
                 param->pkt_data_length_cmpl.params.rx_len);

      break;
    }
    case ESP_GAP_BLE_NC_REQ_EVT:
    {
      ESP_LOGI(TAG_GAP, "ESP_GAP_BLE_NC_REQ_EVT");
      /* Call the following function to input the numeric comparison value which is displayed on the remote device */
      break;
    }
    case ESP_GAP_BLE_PASSKEY_REQ_EVT: /* passkey request event */
    {
      ESP_LOGI(TAG, "ESP_GAP_BLE_PASSKEY_REQ_EVT");
      /* Call the following function to input the passkey which is displayed on the remote device */
      break;
    }
    case ESP_GAP_BLE_OOB_REQ_EVT:
    {
      ESP_LOGI(TAG, "ESP_GAP_BLE_OOB_REQ_EVT");
      /* Call the following function to reply the OOB data */
      break;
    }
    default:
    {
      ESP_LOGI(TAG_GAP, "Unhandled GAP event: %d", event);
      break;
    }
  }
}

static esp_err_t ble_gap_init(const char *device_name)
{
  esp_err_t ret;

  // Register GAP callback
  ret = esp_ble_gap_register_callback(gap_event_handler);
  if (ret)
  {
    ESP_LOGI(TAG_GAP, "GAP callback registration failed: %s", esp_err_to_name(ret));
    return ret;
  }

  // Set device name
  ret = esp_ble_gap_set_device_name(device_name);
  if (ret)
  {
    ESP_LOGI(TAG_GAP, "Setting device name failed: %s", esp_err_to_name(ret));
    return ret;
  }

  ret = esp_ble_gap_config_adv_data_raw(raw_adv_data, sizeof(raw_adv_data));
  if (ret)
  {
    ESP_LOGI(TAG_GAP, "Configuring advertising data failed: %s", esp_err_to_name(ret));
    return ret;
  }

  ret = esp_ble_gap_config_scan_rsp_data_raw(raw_scan_rsp_data, sizeof(raw_scan_rsp_data));
  if (ret)
  {
    ESP_LOGI(TAG_GAP, "Configuring scan response data failed: %s", esp_err_to_name(ret));
    return ret;
  }

  ESP_LOGI(TAG_GAP, "GAP initialized successfully with device name: %s", device_name);
  return ESP_OK;
}

static esp_err_t ble_gatt_init(void)
{
  // Set local MTU size
  esp_err_t ret = esp_ble_gatt_set_local_mtu(500);
  if (ret)
  {
    ESP_LOGI(TAG_GATT, "Set local MTU failed: %s", esp_err_to_name(ret));
    return ret;
  }
  return ESP_OK;
}

static esp_err_t ble_gatts_init(void)
{
  esp_err_t ret;

  // Register the central GATTS callback
  ret = esp_ble_gatts_register_callback(gatts_event_handler);
  if (ret)
  {
    ESP_LOGE(TAG_GATTS, "GATTS callback registration failed: %s", esp_err_to_name(ret));
    return ret;
  }

  return ESP_OK;
}

void ble_gatt_server_init(ble_gatt_server_config_t *c)
{
  esp_err_t ret;

  // Initialize NVS
  nvm_init();

  ESP_ERROR_CHECK(esp_bt_controller_mem_release(ESP_BT_MODE_CLASSIC_BT));

  // Initialize Bluetooth controller
  esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
  ret = esp_bt_controller_init(&bt_cfg);
  if (ret)
  {
    DEBUG_PRINT_ERR("Bluetooth controller initialization failed: %s", esp_err_to_name(ret));
    return;
  }

  // Enable Bluetooth controller
  ret = esp_bt_controller_enable(ESP_BT_MODE_BLE);
  if (ret)
  {
    DEBUG_PRINT_ERR("Bluetooth controller enable failed: %s", esp_err_to_name(ret));
    return;
  }

  // Initialize Bluedroid stack
  ret = esp_bluedroid_init();
  if (ret)
  {
    DEBUG_PRINT_ERR("Bluedroid initialization failed: %s", esp_err_to_name(ret));
    return;
  }

  // Enable Bluedroid stack
  ret = esp_bluedroid_enable();
  if (ret)
  {
    DEBUG_PRINT_ERR("Bluedroid enable failed: %s", esp_err_to_name(ret));
    return;
  }

  // Separate GAP, GATT, and GATTS initialization
  if (ble_gap_init(c->device_name) != ESP_OK)
    return;
  if (ble_gatt_init() != ESP_OK)
    return;
  if (ble_gatts_init() != ESP_OK)
    return;

  ESP_LOGI(TAG, "BLE GATT server initialized successfully");

  s_num_profiles++;  // Start with one profile

  memcpy(s_profiles->gatt_db, gatt_db, sizeof(gatt_db));
  ble_gatt_server_add_profile(&s_profiles[PROFILE_1_APP_ID]);  // Add default profile
}

void ble_gatt_server_deinit()
{
  // Disable Bluedroid stack
  esp_bluedroid_disable();

  // Deinitialize Bluedroid stack
  esp_bluedroid_deinit();

  // Disable Bluetooth controller
  esp_bt_controller_disable();

  // Deinitialize Bluetooth controller
  esp_bt_controller_deinit();

  DEBUG_PRINT_ERR("BLE GATT server deinitialized successfully");
}

/**
 * @brief Add a profile to the BLE controller
 */
esp_err_t ble_gatt_server_add_profile(ble_profile_t *profile)
{
  ESP_LOGI(TAG, "Adding profile '%s'", profile->profile_name);
  ESP_LOGI(TAG, "Number of profiles before: %d", s_num_profiles);
  if (s_num_profiles >= MAX_PROFILES)
  {
    ESP_LOGE(TAG, "Maximum number of profiles reached");
    return ESP_ERR_NO_MEM;
  }

  // Assign app_id sequentially starting from ESP_APP_ID
  profile->app_id = ESP_APP_ID + s_num_profiles;
  profile->gatts_if = ESP_GATT_IF_NONE;

  // Copy profile to the array
  memcpy(&s_profiles[s_num_profiles], profile, sizeof(ble_profile_t));

  // Register with GATTS if BLE is already initialized
  if (esp_bluedroid_get_status() != ESP_BLUEDROID_STATUS_ENABLED)
    return ESP_ERR_INVALID_STATE;

  esp_ble_gatts_app_register(profile->app_id);

  s_num_profiles++;
  ESP_LOGI(TAG, "Added profile '%s', app_id: %d", profile->profile_name, profile->app_id);

  g_ble_profile = profile;  // Store the last added profile globally

  return ESP_OK;
}
