/*
 ESP401.cpp - ESP3D command class

 Copyright (c) 2014 Luc Lebosse. All rights reserved.

 This code is free software; you can redistribute it and/or
 modify it under the terms of the GNU Lesser General Public
 License as published by the Free Software Foundation; either
 version 2.1 of the License, or (at your option) any later version.

 This code is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 Lesser General Public License for more details.

 You should have received a copy of the GNU Lesser General Public
 License along with This code; if not, write to the Free Software
 Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/
#include "../../include/esp3d_config.h"
#include "../commands.h"
#include "../esp3doutput.h"
#include "../settings_esp3d.h"
#include "../../modules/authentication/authentication_service.h"
#include "../../modules/serial/serial_service.h"
#ifdef SENSOR_DEVICE
#include "../../modules/sensor/sensor.h"
#endif //SENSOR_DEVICE
#ifdef BUZZER_DEVICE
#include "../../modules/buzzer/buzzer.h"
#endif //BUZZER_DEVICE
#ifdef TIMESTAMP_FEATURE
#include "../../modules/time/time_server.h"
#endif //TIMESTAMP_FEATURE
#ifdef NOTIFICATION_FEATURE
#include "../../modules/notifications/notifications_service.h"
#endif //NOTIFICATION_FEATURE
#ifdef SD_DEVICE
#include "../../modules/filesystem/esp_sd.h"
#endif //SD_DEVICE
//Set EEPROM setting
//[ESP401]P=<position> T=<type> V=<value> pwd=<user/admin password>
bool Commands::ESP401(const char* cmd_params, level_authenticate_type auth_type, ESP3DOutput * output)
{
    bool response = true;
    String parameter;
#ifdef AUTHENTICATION_FEATURE
    if (auth_type != LEVEL_ADMIN) {
        output->printERROR("Wrong authentication!", 401);
        return false;
    }
#else
    (void)auth_type;
#endif //AUTHENTICATION_FEATURE
    //check validity of parameters
    String spos = get_param (cmd_params, "P=");
    String styp = get_param (cmd_params, "T=");
    String sval = get_param (cmd_params, "V=");
    if (spos.length() == 0) {
        response = false;
    }
    if (! (styp == "B" || styp == "S" || styp == "A" || styp == "I") ) {
        response = false;
    }

    if (response) {
        //Byte value
        if (styp == "B") {
            if (!Settings_ESP3D::write_byte (spos.toInt(), sval.toInt())) {
                response = false;
            } else {
                //dynamique refresh is better than restart the boards
                switch(spos.toInt()) {
                case ESP_SERIAL_FLAG:
                case ESP_PRINTER_LCD_FLAG:
                case ESP_WEBSOCKET_FLAG:
                case ESP_TELNET_FLAG:
                case ESP_LCD_FLAG:
                case ESP_BT_FLAG:
                    ESP3DOutput::isOutput(ESP_ALL_CLIENTS,true);
                    break;
                case ESP_VERBOSE_BOOT:
                    Settings_ESP3D::isVerboseBoot(true);
                    break;
                case ESP_TARGET_FW:
                    Settings_ESP3D::GetFirmwareTarget(true);
                    break;
                case ESP_SECURE_SERIAL:
                    serial_service.setParameters();
                    break;
#ifdef AUTHENTICATION_FEATURE
                case ESP_SESSION_TIMEOUT:
                    AuthenticationService::setSessionTimeout(1000*60*sval.toInt());
                    break;
#endif //AUTHENTICATION_FEATURE
#ifdef SD_DEVICE
                case ESP_SD_SPEED_DIV:
                    ESP_SD::setSPISpeedDivider(sval.toInt());
                    break;
#endif //SD_DEVICE
#ifdef TIMESTAMP_FEATURE
                case ESP_INTERNET_TIME:
                    timeserver.begin();
                    break;
#endif //TIMESTAMP_FEATURE
#ifdef NOTIFICATION_FEATURE
                case ESP_AUTO_NOTIFICATION:
                    notificationsservice.setAutonotification((sval.toInt() == 0)?false:true);
                    break;
#endif //NOTIFICATION_FEATURE
#ifdef SENSOR_DEVICE
                case ESP_SENSOR_TYPE:
                    esp3d_sensor.begin();
                    break;
#endif //SENSOR_DEVICE
#ifdef BUZZER_DEVICE
                case ESP_BUZZER:
                    if (sval.toInt() == 1) {
                        esp3d_buzzer.begin();
                    } else if (sval.toInt() == 0) {
                        esp3d_buzzer.end();
                    }
                    break;
#endif //BUZZER_DEVICE
                default:
                    break;
                }
            }
        }
        //Integer value
        if (styp == "I") {
            if (!Settings_ESP3D::write_uint32 (spos.toInt(), sval.toInt())) {
                response = false;
            } else {
                //dynamique refresh is better than restart the board
                switch(spos.toInt()) {
#ifdef SENSOR_DEVICE
                case ESP_SENSOR_INTERVAL:
                    esp3d_sensor.setInterval(sval.toInt());
                    break;
#endif //SENSOR_DEVICE
                case ESP_BAUD_RATE:
                    serial_service.updateBaudRate(sval.toInt());
                    break;
                default:
                    break;
                }
            }
        }
        //String value
        if (styp == "S") {
            if (!Settings_ESP3D::write_string (spos.toInt(), sval.c_str())) {
                response = false;
            } else {
                //dynamique refresh is better than restart the board
                switch(spos.toInt()) {
#ifdef AUTHENTICATION_FEATURE
                case ESP_ADMIN_PWD:
                case ESP_USER_PWD:
                    AuthenticationService::update();
                    break;
#endif //AUTHENTICATION_FEATURE 
                default:
                    break;
                }
            }
        }
#if defined (WIFI_FEATURE)
        //IP address
        if (styp == "A") {
            if (!Settings_ESP3D::write_IP_String (spos.toInt(), sval.c_str())) {
                response = false;
            } else {
                //dynamique refresh is better than restart the board
                //TBD
            }
        }
#endif //WIFI_FEATURE
    }
    if (!response) {
        parameter = "error " + spos;
        output->printERROR (parameter.c_str());

    } else {
        parameter = "ok " + spos;
        output->printMSG(parameter.c_str());
    }

    return response;
}
