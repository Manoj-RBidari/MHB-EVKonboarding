/***************************************************************************//**
 * @file
 * @brief Core application logic.
 *******************************************************************************
 * # License
 * <b>Copyright 2020 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * SPDX-License-Identifier: Zlib
 *
 * The licensor of this software is Silicon Laboratories Inc.
 *
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 *
 ******************************************************************************/
#include "em_common.h"
#include "app_assert.h"
#include "sl_bluetooth.h"
#include "app.h"
#include "gatt_db.h"
#include "stdio.h"


// The advertising set handle allocated from Bluetooth stack.
static uint8_t advertising_set_handle = 0xff;
sl_status_t sc;
bd_addr BT_Address;
uint8_t System_ID[8];
static uint8_t m_deviceName[10];
uint8_t BT_Address_type;
static const uint8_t HH_Onboard_Service[16] = {0x20,0x24,0x3e,0x4e,0x14,0x56,0x27,0xb7,0xc2,0x49,0xbf,0x7f,0xba,0x37,0x5f,0xbb};
static const uint8_t HH_Onboard_Characteristics[16] = {0x8b, 0x1f, 0xba, 0xae, 0x2b, 0x77, 0x71, 0x87, 0x39, 0x45, 0xab, 0x21, 0xf9, 0x51, 0xa0, 0x66};

__uint32_t service_handle_HH;
static uint8_t connection_IDHomeHub;
uint16_t chararcteristic_handleHH_Onboard;
typedef enum
{
  discover_HH_service,
  discover_HH_Onboard_Characteristics,
  HH_Onboard_Characteristic_discovered,
} conn_state_t;
static conn_state_t connection_state_HH;


typedef enum{
  Mobile_HH_Wifi_SSID              = 0xB1,
  Mobile_HH_Wifi_Password          = 0xB2,
  HH_Mobile_Wifi_Connection_State  = 0xB3,
  Mobile_HH_Tuya_Credentials       = 0xB4,
  HH_Mobile_Tuya_Device_ID         = 0xB5,
  Mobile_HH_Serial_Number          = 0xB6,
  Beacon_Mobile_HH_DeviceID        = 0xB7,
  Garage_Mobile_HH_DeviceID        = 0xB8,
}HH_Onboarding;

typedef enum {
  Wifi_Connection_Successful = 0x01,
  Wifi_Connection_Not_Successful = 0x02,
}Wifi_State;

static Wifi_State Wifi_Connection_State;

typedef enum {
  Tuya_Connection_Successful = 0x01,
  Tuya_Connection_Not_Successful = 0x02,
}Tuya_State;

typedef enum {
  HH_Serial_Number_Reception_Successful = 0x01,
  HH_Serial_Number_Reception_Not_Successful = 0x02,
}HH_Serial_Number_State;

static Tuya_State Tuya_Connection_State;
uint8_t Wifi_SSID[3] = {0x01,0x02,0x03};
uint8_t Wifi_Password[3] = {0x01,0x01,0x2};
uint8_t Tuya_Credentials[5] = {0x01,0x01,0x01,0x03,0x02};
static uint8_t Received_Wifi_SSID[3];

/**************************************************************************//**
 * Application Init.
 *****************************************************************************/
SL_WEAK void app_init(void)
{
  /////////////////////////////////////////////////////////////////////////////
  // Put your additional application init code here!                         //
  // This is called once during start-up.                                    //
  /////////////////////////////////////////////////////////////////////////////
}

/**************************************************************************//**
 * Application Process Action.
 *****************************************************************************/
SL_WEAK void app_process_action(void)
{
  /////////////////////////////////////////////////////////////////////////////
  // Put your additional application code here!                              //
  // This is called infinitely.                                              //
  // Do not call blocking functions from here!                               //
  /////////////////////////////////////////////////////////////////////////////
}

/**************************************************************************//**
 * Bluetooth stack event handler.
 * This overrides the dummy weak implementation.
 *
 * @param[in] evt Event coming from the Bluetooth stack.
 *****************************************************************************/
void sl_bt_on_event(sl_bt_msg_t *evt)
{

  printf("THE EVENT HEADER IS %x\n",evt->header);
  switch (SL_BT_MSG_ID(evt->header)) {
    // -------------------------------
    // This event indicates the device has started and the radio is ready.
    // Do not call any stack command before receiving this boot event!
    case sl_bt_evt_system_boot_id:

      //Extract unique ID from BT address
      sc = sl_bt_system_get_identity_address(&BT_Address, &BT_Address_type);
      app_assert_status(sc);

      // Pad and reverse unique ID to get System ID.


      System_ID[7] = BT_Address.addr[5];
      System_ID[6] = BT_Address.addr[4];
      System_ID[5] = BT_Address.addr[3];
      System_ID[4] = 0xFE;
      System_ID[3] = 0xFF;
      System_ID[2] = BT_Address.addr[2];
      System_ID[1] = BT_Address.addr[1];
      System_ID[0] = BT_Address.addr[0];

      app_log_append_debug(" THE SYSTEM ID IS %02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X\n ",
                           System_ID[0],
                           System_ID[1],
                           System_ID[2],
                           System_ID[3],
                           System_ID[4],
                           System_ID[5],
                           System_ID[6],
                           System_ID[7]);

      //Storing the System ID in an attribute called gattdb_System_ID
      sc = sl_bt_gatt_server_write_attribute_value(gattdb_System_ID,
                                                   0,
                                                   sizeof(System_ID),
                                                   System_ID);

      //GIVE THE NAME TO YOUR DEVICE
      m_deviceName[0]=0x4D;//M
      m_deviceName[1]=0x79;//y
      m_deviceName[2]=0x48;//H
      m_deviceName[3]=0x6F;//o
      m_deviceName[4]=0x6D;//m
      m_deviceName[5]=0x65;//e
      m_deviceName[6]=0x20;//space
      m_deviceName[7]=0x48;//H
      m_deviceName[8]=0x75;//u
      m_deviceName[9]=0x62;//b

      app_log_append_debug("THE DEVICE WHICH IS SCANNING IS %c%c%c%c%c%c:%c%c:%c%c\n ",
                           m_deviceName[0],//M
                           m_deviceName[1],//y
                           m_deviceName[2],//H
                           m_deviceName[3],//o
                           m_deviceName[4],//m
                           m_deviceName[5],//e
                           m_deviceName[6],//space
                           m_deviceName[7],//H
                           m_deviceName[8],//u
                           m_deviceName[9]);//b

      //Storing the device name in an attribute called gattdb_device_name
      sc = sl_bt_gatt_server_write_attribute_value(gattdb_device_name ,
                                                   0,
                                                   sizeof(m_deviceName),
                                                   m_deviceName);
      app_assert_status(sc);

      // Create an advertising set.
      sc = sl_bt_advertiser_create_set(&advertising_set_handle);
      app_assert_status(sc);
      // Set advertising interval to 100ms.
      sc = sl_bt_advertiser_set_timing(
          advertising_set_handle,
          160, // min. adv. interval (milliseconds * 1.6)
          160, // max. adv. interval (milliseconds * 1.6)
          0,   // adv. duration
          0);  // max. num. adv. events
      app_assert_status(sc);

      //Create a custom advertising packet
      demo_setup_adv(advertising_set_handle,m_deviceName,System_ID);

      //Set the physical layer as coded for the our advertising set
      sc = sl_bt_extended_advertiser_set_phy(advertising_set_handle, sl_bt_gap_phy_coded, sl_bt_gap_phy_coded);
      app_assert_status(sc);

      //Start the advertising
      sc = sl_bt_extended_advertiser_start(advertising_set_handle,
                                           sl_bt_extended_advertiser_connectable,
                                           0);

      if(sc==SL_STATUS_OK) {
          app_log_append_info("Home Hub is Advertising\n");
      }
      else {
          app_log_append_info("Home Hub failed to Advertise\n");
      }
      break;

      // -------------------------------
      // This event indicates that a new connection was opened.
    case sl_bt_evt_connection_opened_id:

      app_log_append_info(" Connection Phase \n ");
      connection_IDHomeHub = evt->data.evt_connection_opened.connection;
      printf("The connection id is %02x\n",connection_IDHomeHub);
      sc = sl_bt_advertiser_stop(advertising_set_handle);
      app_assert_status(sc);
      // Discover Home Beacon service on the responder device
      sc = sl_bt_gatt_discover_primary_services_by_uuid(evt->data.evt_connection_opened.connection,
                                                        sizeof(HH_Onboard_Service),
                                                        (const uint8_t*)HH_Onboard_Service);
      app_assert_status(sc);
      break;

    case sl_bt_evt_gatt_service_id:
      app_log_append_info("THE SERVICE IS DISCOVERED FOR MOBILE\n");
      app_log_append_debug("THE CONNECTION HANDLE OF THE DEVICES WHCIH ARE CONNECTED IS %02X\n",evt->data.evt_gatt_service.connection);
      app_log_append_info("THE NEW SERVICE IS DISCOVERED\n");
      app_log_append_debug("THE SERVICE HANDLE IS %4X \n", evt->data.evt_gatt_service.service);
      service_handle_HH = evt->data.evt_gatt_service.service;
      app_log_append_debug("THE SERVICE HANDLE IS %4X \n",service_handle_HH);
      app_log_append_debug("ServiceUUID address : %02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X \r\n",
                           evt->data.evt_gatt_service.uuid.data[15],
                           evt->data.evt_gatt_service.uuid.data[14],
                           evt->data.evt_gatt_service.uuid.data[13],
                           evt->data.evt_gatt_service.uuid.data[12],
                           evt->data.evt_gatt_service.uuid.data[11],
                           evt->data.evt_gatt_service.uuid.data[10],
                           evt->data.evt_gatt_service.uuid.data[9],
                           evt->data.evt_gatt_service.uuid.data[8],
                           evt->data.evt_gatt_service.uuid.data[7],
                           evt->data.evt_gatt_service.uuid.data[6],
                           evt->data.evt_gatt_service.uuid.data[5],
                           evt->data.evt_gatt_service.uuid.data[4],
                           evt->data.evt_gatt_service.uuid.data[3],
                           evt->data.evt_gatt_service.uuid.data[2],
                           evt->data.evt_gatt_service.uuid.data[1],
                           evt->data.evt_gatt_service.uuid.data[0]);
      connection_state_HH = discover_HH_service;
      break;


    case sl_bt_evt_gatt_characteristic_id:
      if(connection_state_HH ==discover_HH_Onboard_Characteristics ) {
          app_log_append_info("A NEW CONNECTION HAS BEEN DISCOVERED\n");
          app_log_append_debug("THE UUID OF THE ONBOARDING CHARACTERISTIC DISCOVERED IS:"
              " %02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X \r\n",
              evt->data.evt_gatt_characteristic.uuid.data[15],
              evt->data.evt_gatt_characteristic.uuid.data[14],
              evt->data.evt_gatt_characteristic.uuid.data[13],
              evt->data.evt_gatt_characteristic.uuid.data[12],
              evt->data.evt_gatt_characteristic.uuid.data[11],
              evt->data.evt_gatt_characteristic.uuid.data[10],
              evt->data.evt_gatt_characteristic.uuid.data[9],
              evt->data.evt_gatt_characteristic.uuid.data[8],
              evt->data.evt_gatt_characteristic.uuid.data[7],
              evt->data.evt_gatt_characteristic.uuid.data[6],
              evt->data.evt_gatt_characteristic.uuid.data[5],
              evt->data.evt_gatt_characteristic.uuid.data[4],
              evt->data.evt_gatt_characteristic.uuid.data[3],
              evt->data.evt_gatt_characteristic.uuid.data[2],
              evt->data.evt_gatt_characteristic.uuid.data[1],
              evt->data.evt_gatt_characteristic.uuid.data[0]);
          app_log_append_debug("THE CHARACTERISTIC HANDLE IS:%X\n",evt->data.evt_gatt_characteristic.characteristic);
          chararcteristic_handleHH_Onboard=evt->data.evt_gatt_characteristic.characteristic;
          connection_state_HH=HH_Onboard_Characteristic_discovered;
      }
      break;

    case sl_bt_evt_gatt_procedure_completed_id:
      if(connection_state_HH==discover_HH_service) {
          app_log_append_info("Onboarding Service has been discovered!\r\n");
          app_log_append_debug("THE SERVICE HANDLE IS %4X \n",service_handle_HH);
          sc = sl_bt_gatt_discover_characteristics_by_uuid(evt->data.evt_gatt_procedure_completed.connection,
                                                           service_handle_HH,
                                                           sizeof( HH_Onboard_Characteristics),
                                                           (const uint8_t*) HH_Onboard_Characteristics);
          if(sc == SL_STATUS_OK) {
              connection_state_HH = discover_HH_Onboard_Characteristics;
          }
      }
      else if(connection_state_HH==HH_Onboard_Characteristic_discovered) {
          app_log_append_info("Onboarding Characteristic has been discovered!\r\n");
          // connection_state_HH = discover_HH_Onboard_Characteristics;
      }
      break;

    case sl_bt_evt_gatt_server_user_write_request_id:
      printf("HI\n");
      sc = sl_bt_gatt_server_send_user_write_response(connection_IDHomeHub, chararcteristic_handleHH_Onboard, 0);
      app_assert_status(sc);
      printf("The data which is received is\n");
      for(int i=0 ; i < evt->data.evt_gatt_server_user_write_request.value.len;i++)
        {
          printf("%02x:",evt->data.evt_gatt_server_user_write_request.value.data[i]);
        }
      printf("\n");
      uint8_t payload_length =  evt->data.evt_gatt_server_user_write_request.value.data[1];
      uint8_t *data = &(evt->data.evt_gatt_server_user_write_request.value.data[2]);
      uint8_t CRC   = calculateChecksum(data, payload_length);
       //printf("The length is %02x\n",payload_length);
      //printf("The CRC is %02x : the data is %02x :\n",CRC,data[payload_length]);
      if(CRC == data[payload_length]) {
          if(evt->data.evt_gatt_server_user_write_request.value.data[0] == Mobile_HH_Wifi_SSID) {
              Mobile_HH_BLE_Wifi_SSID(data);
          }
          if(evt->data.evt_gatt_server_user_write_request.value.data[0] == Mobile_HH_Wifi_Password) {
              Mobile_HH_BLE_Wifi_Password(data);
          }
          if(evt->data.evt_gatt_server_user_write_request.value.data[0] == Mobile_HH_Tuya_Credentials) {
              Mobile_HH_BLE_Tuya_Credentials(data);
          }
          if(evt->data.evt_gatt_server_user_write_request.value.data[0] == Mobile_HH_Serial_Number) {
              Mobile_HH_BLE_Serial_Number(data);
          }
      }
      else {
          printf("Wrong CRC has been received for Received packet\n");
          WrongCRCReceived_Over_BT(evt->data.evt_gatt_server_user_write_request.value.data[0] ,Wrong_CRC, sizeof(uint8_t));
      }
      break;

      // -------------------------------
      // This event indicates that a connection was closed.
    case sl_bt_evt_connection_closed_id:
      // Generate data for advertising
      sc = sl_bt_legacy_advertiser_generate_data(advertising_set_handle,
                                                 sl_bt_advertiser_general_discoverable);
      app_assert_status(sc);

      // Restart advertising after client has disconnected.
      sc = sl_bt_legacy_advertiser_start(advertising_set_handle,
                                         sl_bt_advertiser_connectable_scannable);
      app_assert_status(sc);
      break;

      ///////////////////////////////////////////////////////////////////////////
      // Add additional event handlers here as your application requires!      //
      ///////////////////////////////////////////////////////////////////////////

      // -------------------------------
      // Default event handler.
    default:
      break;
  }
}

/******************************************************************
 * Advertisement constructor
 * ***************************************************************/
void demo_setup_adv(uint8_t handle, uint8_t *device_name, uint8_t *DeviceID) {
  const uint8_t flag_data = 0x6;
  uint8_t local_name[11];
  uint8_t Device_ID[8];

  /*Copy the DeivceID passed as a function call to the demo_setup_adv to a
   *variable named Device_ID declared locally to the demo_setup_adv function
   */
  memcpy(Device_ID,DeviceID,8);

  /*Copy the device name passed as a function call to the demo_setup_adv to a
   *variable named local_name declared locally to the demo_setup_adv function
   */
  memcpy(local_name,device_name,10);

  ad_element_t ad_elements[] = {
      /* Element 0 */
      {
          .ad_type = flags,
          .len = 1,
          .data = &flag_data
      },
      /* Element 1 */
      {
          .ad_type = complete_local_name,
          .len = sizeof(local_name) - 1,
          .data = local_name
      },
      /* Element 2 */
      {
          .ad_type = extended_inquiry_response_record,
          .len = sizeof(Device_ID),
          .data = Device_ID
      }
  };

  /* Set up advertisement payload with the 3 elements */
  adv_t adv = {
      .adv_handle = handle,
      .adv_packet_type = adv_packet,
      .ele_num = 3,
      .p_element = ad_elements
  };
  sc = construct_adv(&adv);
  if (sc != SL_STATUS_OK) {
      app_log("Check error here [%s:%u]\n", __FILE__, __LINE__);
  }
}


sl_status_t construct_adv(const adv_t *adv)
{
  uint8_t amout_bytes = 0, i;
  uint8_t buf[MAX_EXTENDED_ADV_LENGTH] = { 0 };

  if (!adv) {
      app_log("input param null, aborting.\n");
      return SL_STATUS_NULL_POINTER;
  }

  for (i = 0; i < adv->ele_num; i++) {
      amout_bytes += adv->p_element[i].len + 2;
      if (!adv->p_element[i].data) {
          app_log("adv unit payload data null, aborting.\n");
          return SL_STATUS_NULL_POINTER;
      }
  }
  if (((amout_bytes > MAX_ADV_DATA_LENGTH))
      || ((amout_bytes > MAX_EXTENDED_ADV_LENGTH))) {
      app_log("Adv data too long [length = %d], aborting.\n", amout_bytes);
      return SL_STATUS_BT_CTRL_PACKET_TOO_LONG;
  }

  amout_bytes = 0;
  for (i = 0; i < adv->ele_num; i++) {
      buf[amout_bytes++] = adv->p_element[i].len + 1;
      buf[amout_bytes++] = adv->p_element[i].ad_type;
      memcpy(buf + amout_bytes, adv->p_element[i].data, adv->p_element[i].len);
      amout_bytes += adv->p_element[i].len;
  }
  app_log_append_info("The advertising data for Home Hub is :\n");
  //The advertising packet being advertised will be displayed over the terminal
  /*The advertising data is of 25 bytes
   * The first advertising type is the flag "ad_type = 0x06"
   * The second avertising type is the complete local name "ad_type = 0x09"
   * The third advertising type is the device ID "ad_type = 0x10"
   */
  app_log_append_debug("%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:"
      "%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:\n",buf[0],buf[1],buf[2],buf[3],buf[4],buf[5]
      ,buf[6],buf[7],buf[8],buf[9],buf[10],buf[11],buf[12],buf[13],buf[14],buf[15],buf[16],buf[17],buf[18],buf[19],buf[20],buf[21],buf[22],buf[23]
      ,buf[24],buf[25],buf[26],buf[27],buf[28],buf[29],buf[30],buf[31]);

  //Set the advertising packet for the advertising set
  sc = sl_bt_extended_advertiser_set_data(adv->adv_handle, amout_bytes, buf);

  return SL_STATUS_OK;
}



void Mobile_HH_BLE_Wifi_SSID(uint8_t *payload) {
  for(int i = 0 ; i < 3 ; i++) {
      //This is dummy implementation as the Received Wifi SSID is not being sent to Tuya over UART
      Received_Wifi_SSID[i] = payload[i];
  }
  printf("The SSID is received\n");
}

/**
 * @brief Calculates the checksum for the given data array.
 *
 * This function calculates the checksum by summing up all the bytes in the
 * data array.
 * It iterates through the data array and accumulates the sum of each byte to
 * compute the checksum.
 * The result is a byte containing the calculated checksum.
 *
 * @param data Pointer to the data array for which the checksum is calculated.
 * @param packetlen The length of the data array.
 *
 * @return The calculated checksum.
 */

uint8_t calculateChecksum(uint8_t *data , uint8_t packetlen) {
  uint8_t checksum = 0;
  uint16_t i;

  // Calculate Checksum
  for(i=0; i  < packetlen; i++) {
      checksum += data[i];
  }
  return(checksum);
}




void Mobile_HH_BLE_Wifi_Password(uint8_t *payload) {
  /*This is not the actual implementation as in the actual scenario the Wifi SSID
  * and the Wifi Password will be shared to Tuya which will try to connect to Home Wifi
  * As tuya
  */
  if(memcmp(Received_Wifi_SSID,Wifi_SSID,3)==0) {
      if(memcmp(payload,Wifi_Password,3) == 0) {
          Wifi_Connection_State = Wifi_Connection_Successful;
          app_log_append_info("The Wifi credentials are correct and Tuya is connected to Wifi\n");
          Wifi_Credentials_Response_Packet(HH_Mobile_Wifi_Connection_State,Wifi_Connection_Successful,sizeof(uint8_t));
      }
      else {
          Wifi_Connection_State = Wifi_Connection_Not_Successful;
          app_log_append_info("The Wifi credentials are not correct and Tuya is not connected to Wifi\n");
          Wifi_Credentials_Response_Packet(HH_Mobile_Wifi_Connection_State,Wifi_Connection_Not_Successful,sizeof(uint8_t));

      }
  }
}

typedef struct {
  uint8_t WifiResponseCommand;
  uint8_t WifiResponseLength;
  uint8_t WifiResponsePayload;
  uint8_t WifiResponseCRC;
}WifiCredentials;

void Wifi_Credentials_Response_Packet(uint8_t command,uint8_t payload,uint8_t payloadlength) {
  WifiCredentials WifiResponse;
  WifiResponse.WifiResponseCommand = command;
  WifiResponse.WifiResponseLength = payloadlength;
  WifiResponse.WifiResponsePayload = payload;
  WifiResponse.WifiResponseCRC = calculateChecksum(&(WifiResponse.WifiResponsePayload),WifiResponse.WifiResponseLength);


  sc = sl_bt_gatt_server_send_notification(connection_IDHomeHub, chararcteristic_handleHH_Onboard, sizeof(WifiResponse),
                                           (const uint8_t *)&WifiResponse);
  if(sc == SL_STATUS_OK) {
      app_log_append_info("Response is sent back to Mobile  \n");
  }
}

void Mobile_HH_BLE_Tuya_Credentials(uint8_t *payload) {
  if(memcmp(payload,Tuya_Credentials,5) == 0) {
      Tuya_Connection_State = Tuya_Connection_Successful;
      app_log_append_info("The Tuya credentials are correct and Tuya is connected to Cloud\n");
      Tuya_Credentials_Response_Packet(Mobile_HH_Tuya_Credentials,Tuya_Connection_Successful,sizeof(uint8_t));
  }
  else {
      Tuya_Connection_State = Tuya_Connection_Not_Successful;
      app_log_append_info("The Tuya credentials are not correct and Tuya is not connected to Cloud\n");
      Tuya_Credentials_Response_Packet(Mobile_HH_Tuya_Credentials,Tuya_Connection_Not_Successful,sizeof(uint8_t));

  }
}

void Mobile_HH_BLE_Serial_Number(uint8_t *payload) {
  printf("The HH Serial NUmber has been received from Mobile \n");
  // A function call has to be implemented here to store the information in the flash.
  app_log_append_info("Storing the HH_Serial_Number,Device ID,Wifi Credentials in the flash \n");
  app_log_append_info("Send the confirmation update to Mobile\n");
  Serial_Number_Response_Packet(Mobile_HH_Serial_Number, HH_Serial_Number_Reception_Successful,sizeof(uint8_t));
}

typedef struct {
  uint8_t Serial_NumberResponseCommand;
  uint8_t Serial_NumberResponseLength;
  uint8_t Serial_NumberResponsePayload;
  uint8_t Serial_NumberResponseCRC;
}HH_Serial_Number;

void Serial_Number_Response_Packet(uint8_t command,uint8_t payload,uint8_t payloadlength) {
  HH_Serial_Number Serial_NumberResponse;
  Serial_NumberResponse.Serial_NumberResponseCommand = command;
  Serial_NumberResponse.Serial_NumberResponseLength = payloadlength;
  Serial_NumberResponse.Serial_NumberResponsePayload = payload;
  Serial_NumberResponse.Serial_NumberResponseCRC = calculateChecksum(&(Serial_NumberResponse.Serial_NumberResponsePayload),Serial_NumberResponse.Serial_NumberResponseLength);


  sc = sl_bt_gatt_server_send_notification(connection_IDHomeHub, chararcteristic_handleHH_Onboard, sizeof(Serial_NumberResponse),
                                           (const uint8_t *)&Serial_NumberResponse);
  if(sc == SL_STATUS_OK) {
      app_log_append_info("Home Hub onboarding is successfully completed \n");
  }
}

typedef struct {
  uint8_t TuyaResponseCommand;
  uint8_t TuyaResponseLength;
  uint8_t TuyaResponsePayload;
  uint8_t TuyaResponseCRC;
}TuyaCredentials;

void Tuya_Credentials_Response_Packet(uint8_t command,uint8_t payload,uint8_t payloadlength) {
  TuyaCredentials TuyaResponse;
  TuyaResponse.TuyaResponseCommand = command;
  TuyaResponse.TuyaResponseLength = payloadlength;
  TuyaResponse.TuyaResponsePayload = payload;
  TuyaResponse.TuyaResponseCRC = calculateChecksum(&(TuyaResponse.TuyaResponsePayload),TuyaResponse.TuyaResponseLength);


  sc = sl_bt_gatt_server_send_notification(connection_IDHomeHub, chararcteristic_handleHH_Onboard, sizeof(TuyaResponse),
                                           (const uint8_t *)&TuyaResponse);
  if(sc == SL_STATUS_OK) {
      uint8_t DeviceID[4] = {0x01,0x01,0x02,0x03};
      app_log_append_info("Response for Tuya connection is sent back to Mobile  \n");
      if(TuyaResponse.TuyaResponsePayload == Tuya_Connection_Successful) {
          Tuya_Device_ID_HH_Mobile(HH_Mobile_Tuya_Device_ID,DeviceID,sizeof(DeviceID));
      }
  }
}


typedef struct {
  uint8_t Tuya_Device_IDCommand;
  uint8_t Tuya_Device_IDLength;
  uint8_t Tuya_Device_IDPayload[4];
  uint8_t Tuya_Device_IDCRC;
}Tuya_Device_ID;

void Tuya_Device_ID_HH_Mobile(uint8_t command,uint8_t payload,uint8_t payloadlength) {
  Tuya_Device_ID Tuya_Device_ID;
  Tuya_Device_ID.Tuya_Device_IDCommand = command;
  Tuya_Device_ID.Tuya_Device_IDLength = payloadlength;
  memcpy(&Tuya_Device_ID.Tuya_Device_IDPayload,(uint8_t *)payload,4);
  Tuya_Device_ID.Tuya_Device_IDCRC = calculateChecksum(&(Tuya_Device_ID.Tuya_Device_IDPayload),Tuya_Device_ID.Tuya_Device_IDLength);


  sc = sl_bt_gatt_server_send_notification(connection_IDHomeHub, chararcteristic_handleHH_Onboard, sizeof(Tuya_Device_ID),
                                           (const uint8_t *)&Tuya_Device_ID);
  if(sc == SL_STATUS_OK) {
      app_log_append_info("The Device ID of Tuya from Tuya cloud has been sent to Mobile \n");

  }
}


typedef struct {
  uint8_t WrongCRCReceived_IDCommand;
  uint8_t WrongCRCReceived_IDLength;
  uint8_t WrongCRCReceived_IDPayload;
  uint8_t WrongCRCReceived_IDCRC;
}WrongCRCReceived_ID;

void WrongCRCReceived_Over_BT(uint8_t command, uint8_t payload, uint8_t payloadlength) {

  WrongCRCReceived_ID WrongCRCReceived_ID;
  WrongCRCReceived_ID.WrongCRCReceived_IDCommand = command;
  WrongCRCReceived_ID.WrongCRCReceived_IDLength = payloadlength;
  WrongCRCReceived_ID.WrongCRCReceived_IDPayload = payload;
  WrongCRCReceived_ID.WrongCRCReceived_IDCRC = calculateChecksum(&(WrongCRCReceived_ID.WrongCRCReceived_IDPayload),WrongCRCReceived_ID.WrongCRCReceived_IDLength);


  sc = sl_bt_gatt_server_send_notification(connection_IDHomeHub, chararcteristic_handleHH_Onboard, sizeof(WrongCRCReceived_ID),
                                           (const uint8_t *)&WrongCRCReceived_ID);
  if(sc == SL_STATUS_OK) {
      app_log_append_info("The Wrong CRC update has been sent to Mobile\n");

  }
}


