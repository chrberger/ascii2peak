/*
 * Copyright (C) 2023  Christian Berger
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "cluon-complete.hpp"
#include "opendlv-standard-message-set.hpp"
#include "peak_can.hpp"

#include <unistd.h>

#include <cerrno>
#include <cmath>
#include <cstdint>
#include <cstring>

#include <iostream>
#include <iomanip>
#include <string>
#include <sstream>

int32_t main(int32_t argc, char **argv) {
    int32_t retCode{1};
    auto commandlineArguments = cluon::getCommandlineArguments(argc, argv);
    if ( (1 == commandlineArguments.count("help")) ) {
        std::cerr << argv[0] << " translates messages from CAN to ODVD messages for PEAK CAN GPS." << std::endl;
        std::cerr << "Usage:   " << argv[0] << " [--id=ID] [--verbose]" << std::endl;
        std::cerr << "         --id:     ID to use as senderStamp for encoding" << std::endl;
        std::cerr << "Example: " << argv[0] << " --id=1 --verbose" << std::endl;
    }
    else {
        const uint32_t ID{(commandlineArguments["id"].size() != 0) ? static_cast<uint32_t>(std::stoi(commandlineArguments["id"])) : 0};
        const bool VERBOSE{commandlineArguments.count("verbose") != 0};

        // Delegate to convert incoming CAN frames into ODVD messages that are broadcast into the OD4Session.
        std::mutex msgAVRMutex;
        opendlv::proxy::AngularVelocityReading msgAVR;

        std::mutex msgWGS84RMutex;
        opendlv::proxy::GeodeticWgs84Reading msgWGS84R;

        std::mutex msgDelusionMutex;
        opendlv::device::gps::peak::Delusion msgDelusion;

        auto decode = [ID, VERBOSE, &msgAVRMutex, &msgAVR, &msgWGS84RMutex, &msgDelusionMutex, &msgDelusion, &msgWGS84R](cluon::data::TimeStamp ts, uint16_t canFrameID, uint8_t *src, uint8_t len) {
            if ( (nullptr == src) || (0 == len) ) return;
            if (PEAK_CAN_GPS_COURSE_SPEED_FRAME_ID == canFrameID) {
                peak_can_gps_course_speed_t tmp;
                if (0 == peak_can_gps_course_speed_unpack(&tmp, src, len)) {
                    {
                        opendlv::proxy::GroundSpeedReading msg;
                        msg.groundSpeed(static_cast<float>(peak_can_gps_course_speed_gps_speed_decode(tmp.gps_speed)/3.6f));
                        if (VERBOSE) {
                            std::stringstream sstr;
                            msg.accept([](uint32_t, const std::string &, const std::string &) {},
                                       [&sstr](uint32_t, std::string &&, std::string &&n, auto v) { sstr << n << " = " << v << '\n'; },
                                       []() {});
                            std::clog << sstr.str() << std::endl;
                        }
                        //od4.send(msg, ts, ID);
                        cluon::data::Envelope env;
                        env.senderStamp(ID)
                           .sent(ts)
                           .received(ts)
                           .sampleTimeStamp(ts);
                        cluon::ToProtoVisitor proto;
            
                        env.dataType(msg.ID());
                        msg.accept(proto);
                        env.serializedData(proto.encodedData());
                        std::cout << cluon::serializeEnvelope(std::move(env));
                        std::cout.flush();
                    }

                    {
                        opendlv::proxy::GeodeticHeadingReading msg;
                        msg.northHeading(static_cast<float>(peak_can_gps_course_speed_gps_course_decode(tmp.gps_course)/180.0f * M_PI));
                        if (VERBOSE) {
                            std::stringstream sstr;
                            msg.accept([](uint32_t, const std::string &, const std::string &) {},
                                       [&sstr](uint32_t, std::string &&, std::string &&n, auto v) { sstr << n << " = " << v << '\n'; },
                                       []() {});
                            std::clog << sstr.str() << std::endl;
                        }
                        //od4.send(msg, ts, ID);
                        cluon::data::Envelope env;
                        env.senderStamp(ID)
                           .sent(ts)
                           .received(ts)
                           .sampleTimeStamp(ts);
                        cluon::ToProtoVisitor proto;
            
                        env.dataType(msg.ID());
                        msg.accept(proto);
                        env.serializedData(proto.encodedData());
                        std::cout << cluon::serializeEnvelope(std::move(env));
                        std::cout.flush();
                    }
                }
            }
            else if (PEAK_CAN_BMC_ACCELERATION_FRAME_ID == canFrameID) {
                peak_can_bmc_acceleration_t tmp;
                if (0 == peak_can_bmc_acceleration_unpack(&tmp, src, len)) {
                    {
                        opendlv::proxy::TemperatureReading msg;
                        msg.temperature(static_cast<float>(peak_can_bmc_acceleration_temperature_decode(tmp.temperature)));
                        if (VERBOSE) {
                            std::stringstream sstr;
                            msg.accept([](uint32_t, const std::string &, const std::string &) {},
                                       [&sstr](uint32_t, std::string &&, std::string &&n, auto v) { sstr << n << " = " << v << '\n'; },
                                       []() {});
                            std::clog << sstr.str() << std::endl;
                        }
                        //od4.send(msg, ts, ID);
                        cluon::data::Envelope env;
                        env.senderStamp(ID)
                           .sent(ts)
                           .received(ts)
                           .sampleTimeStamp(ts);
                        cluon::ToProtoVisitor proto;
            
                        env.dataType(msg.ID());
                        msg.accept(proto);
                        env.serializedData(proto.encodedData());
                        std::cout << cluon::serializeEnvelope(std::move(env));
                        std::cout.flush();
                    }

                    const float mG_to_mps2{9.80665f/1000.f};
                    {
                        opendlv::device::gps::peak::Acceleration msg;
                        msg.accelerationX(static_cast<float>(peak_can_bmc_acceleration_acceleration_x_decode(tmp.acceleration_x)*mG_to_mps2));
                        msg.accelerationY(static_cast<float>(peak_can_bmc_acceleration_acceleration_y_decode(tmp.acceleration_y)*mG_to_mps2));
                        msg.accelerationZ(static_cast<float>(peak_can_bmc_acceleration_acceleration_z_decode(tmp.acceleration_z)*mG_to_mps2));
                        msg.verticalAxis(static_cast<uint8_t>(peak_can_bmc_acceleration_vertical_axis_decode(tmp.vertical_axis)));
                        msg.orientation(static_cast<uint8_t>(peak_can_bmc_acceleration_orientation_decode(tmp.orientation)));
                        if (VERBOSE) {
                            std::stringstream sstr;
                            msg.accept([](uint32_t, const std::string &, const std::string &) {},
                                       [&sstr](uint32_t, std::string &&, std::string &&n, auto v) { sstr << n << " = " << v << '\n'; },
                                       []() {});
                            std::clog << sstr.str() << std::endl;
                        }
                        //od4.send(msg, ts, ID);
                        cluon::data::Envelope env;
                        env.senderStamp(ID)
                           .sent(ts)
                           .received(ts)
                           .sampleTimeStamp(ts);
                        cluon::ToProtoVisitor proto;
            
                        env.dataType(msg.ID());
                        msg.accept(proto);
                        env.serializedData(proto.encodedData());
                        std::cout << cluon::serializeEnvelope(std::move(env));
                        std::cout.flush();
                    }

                    {
                        opendlv::proxy::AccelerationReading msg;
                        msg.accelerationX(static_cast<float>(peak_can_bmc_acceleration_acceleration_x_decode(tmp.acceleration_x)*mG_to_mps2));
                        msg.accelerationY(static_cast<float>(peak_can_bmc_acceleration_acceleration_y_decode(tmp.acceleration_y)*mG_to_mps2));
                        msg.accelerationZ(static_cast<float>(peak_can_bmc_acceleration_acceleration_z_decode(tmp.acceleration_z)*mG_to_mps2));
                        if (VERBOSE) {
                            std::stringstream sstr;
                            msg.accept([](uint32_t, const std::string &, const std::string &) {},
                                       [&sstr](uint32_t, std::string &&, std::string &&n, auto v) { sstr << n << " = " << v << '\n'; },
                                       []() {});
                            std::clog << sstr.str() << std::endl;
                        }
                        //od4.send(msg, ts, ID);
                        cluon::data::Envelope env;
                        env.senderStamp(ID)
                           .sent(ts)
                           .received(ts)
                           .sampleTimeStamp(ts);
                        cluon::ToProtoVisitor proto;
            
                        env.dataType(msg.ID());
                        msg.accept(proto);
                        env.serializedData(proto.encodedData());
                        std::cout << cluon::serializeEnvelope(std::move(env));
                        std::cout.flush();
                    }
                }
            }
            else if (PEAK_CAN_BMC_MAGNETIC_FIELD_FRAME_ID == canFrameID) {
                peak_can_bmc_magnetic_field_t tmp;
                if (0 == peak_can_bmc_magnetic_field_unpack(&tmp, src, len)) {
                    const float mT_to_T{1e-6f};
                    opendlv::proxy::MagneticFieldReading msg;
                    msg.magneticFieldX(static_cast<float>(peak_can_bmc_magnetic_field_magnetic_field_x_decode(tmp.magnetic_field_x)*mT_to_T));
                    msg.magneticFieldY(static_cast<float>(peak_can_bmc_magnetic_field_magnetic_field_y_decode(tmp.magnetic_field_y)*mT_to_T));
                    msg.magneticFieldZ(static_cast<float>(peak_can_bmc_magnetic_field_magnetic_field_z_decode(tmp.magnetic_field_z)*mT_to_T));
                    if (VERBOSE) {
                        std::stringstream sstr;
                        msg.accept([](uint32_t, const std::string &, const std::string &) {},
                                   [&sstr](uint32_t, std::string &&, std::string &&n, auto v) { sstr << n << " = " << v << '\n'; },
                                   []() {});
                        std::clog << sstr.str() << std::endl;
                    }
                    //od4.send(msg, ts, ID);
                    cluon::data::Envelope env;
                    env.senderStamp(ID)
                       .sent(ts)
                       .received(ts)
                       .sampleTimeStamp(ts);
                    cluon::ToProtoVisitor proto;
            
                    env.dataType(msg.ID());
                    msg.accept(proto);
                    env.serializedData(proto.encodedData());
                    std::cout << cluon::serializeEnvelope(std::move(env));
                    std::cout.flush();
                }
            }
            else if (PEAK_CAN_L3_GD20_ROTATION_A_FRAME_ID == canFrameID) {
                peak_can_l3_gd20_rotation_a_t tmp;
                if (0 == peak_can_l3_gd20_rotation_a_unpack(&tmp, src, len)) {
                    std::lock_guard<std::mutex> lck(msgAVRMutex);
                    msgAVR.angularVelocityX(static_cast<float>(peak_can_l3_gd20_rotation_a_rotation_x_decode(tmp.rotation_x)/180.0f * M_PI));
                    msgAVR.angularVelocityY(static_cast<float>(peak_can_l3_gd20_rotation_a_rotation_y_decode(tmp.rotation_y)/180.0f * M_PI));
                    if (VERBOSE) {
                        std::stringstream sstr;
                        msgAVR.accept([](uint32_t, const std::string &, const std::string &) {},
                                   [&sstr](uint32_t, std::string &&, std::string &&n, auto v) { sstr << n << " = " << v << '\n'; },
                                   []() {});
                        std::clog << sstr.str() << std::endl;
                    }
                    // Will be sent when Z is in.
                    //od4.send(msgAVR, ts, ID);
                }
            }
            else if (PEAK_CAN_L3_GD20_ROTATION_B_FRAME_ID == canFrameID) {
                peak_can_l3_gd20_rotation_b_t tmp;
                if (0 == peak_can_l3_gd20_rotation_b_unpack(&tmp, src, len)) {
                    std::lock_guard<std::mutex> lck(msgAVRMutex);
                    msgAVR.angularVelocityZ(static_cast<float>(peak_can_l3_gd20_rotation_b_rotation_z_decode(tmp.rotation_z)/180.0f * M_PI));
                    if (VERBOSE) {
                        std::stringstream sstr;
                        msgAVR.accept([](uint32_t, const std::string &, const std::string &) {},
                                   [&sstr](uint32_t, std::string &&, std::string &&n, auto v) { sstr << n << " = " << v << '\n'; },
                                   []() {});
                        std::clog << sstr.str() << std::endl;
                    }
                    //od4.send(msgAVR, ts, ID);
                    cluon::data::Envelope env;
                    env.senderStamp(ID)
                       .sent(ts)
                       .received(ts)
                       .sampleTimeStamp(ts);
                    cluon::ToProtoVisitor proto;
            
                    env.dataType(msgAVR.ID());
                    msgAVR.accept(proto);
                    env.serializedData(proto.encodedData());
                    std::cout << cluon::serializeEnvelope(std::move(env));
                    std::cout.flush();
                }
            }
            else if (PEAK_CAN_GPS_POSITION_LATITUDE_FRAME_ID == canFrameID) {
                peak_can_gps_position_latitude_t tmp;
                if (0 == peak_can_gps_position_latitude_unpack(&tmp, src, len)) {
                    std::lock_guard<std::mutex> lck(msgWGS84RMutex);
                    float latitude{0.0f};
                    latitude = static_cast<float>(peak_can_gps_position_latitude_gps_latitude_minutes_decode(tmp.gps_latitude_minutes))/60.0f;
                    latitude += static_cast<float>(peak_can_gps_position_latitude_gps_latitude_degree_decode(tmp.gps_latitude_degree));
                    int sign = static_cast<int>(peak_can_gps_position_latitude_gps_indicator_ns_decode(tmp.gps_indicator_ns));
                    if (83 == sign) {
                        latitude *= -1.0f;
                    }
                    msgWGS84R.latitude(latitude);

                    if (VERBOSE) {
                        std::stringstream sstr;
                        msgWGS84R.accept([](uint32_t, const std::string &, const std::string &) {},
                                   [&sstr](uint32_t, std::string &&, std::string &&n, auto v) { sstr << n << " = " << v << '\n'; },
                                   []() {});
                        std::clog << sstr.str() << std::endl;
                    }
                    // Will be sent when longitude is in.
                    //od4.send(msgWGS84R, ts, ID);
                }
            }
            else if (PEAK_CAN_GPS_POSITION_LONGITUDE_FRAME_ID == canFrameID) {
                peak_can_gps_position_longitude_t tmp;
                if (0 == peak_can_gps_position_longitude_unpack(&tmp, src, len)) {
                    std::lock_guard<std::mutex> lck(msgWGS84RMutex);
                    float longitude{0.0f};
                    longitude = static_cast<float>(peak_can_gps_position_longitude_gps_longitude_minutes_decode(tmp.gps_longitude_minutes))/60.0f;
                    longitude += static_cast<float>(peak_can_gps_position_longitude_gps_longitude_degree_decode(tmp.gps_longitude_degree));
                    int sign = static_cast<int>(peak_can_gps_position_longitude_gps_indicator_ew_decode(tmp.gps_indicator_ew));
                    if (87 == sign) {
                        longitude *= -1.0f;
                    }

                    msgWGS84R.longitude(longitude);
                    if (VERBOSE) {
                        std::stringstream sstr;
                        msgWGS84R.accept([](uint32_t, const std::string &, const std::string &) {},
                                   [&sstr](uint32_t, std::string &&, std::string &&n, auto v) { sstr << n << " = " << v << '\n'; },
                                   []() {});
                        std::clog << sstr.str() << std::endl;
                    }
                    //od4.send(msgWGS84R, ts, ID);
                    cluon::data::Envelope env;
                    env.senderStamp(ID)
                       .sent(ts)
                       .received(ts)
                       .sampleTimeStamp(ts);
                    cluon::ToProtoVisitor proto;
            
                    env.dataType(msgWGS84R.ID());
                    msgWGS84R.accept(proto);
                    env.serializedData(proto.encodedData());
                    std::cout << cluon::serializeEnvelope(std::move(env));
                    std::cout.flush();
                }
            }
            else if (PEAK_CAN_GPS_POSITION_ALTITUDE_FRAME_ID == canFrameID) {
                peak_can_gps_position_altitude_t tmp;
                if (0 == peak_can_gps_position_altitude_unpack(&tmp, src, len)) {
                    opendlv::proxy::AltitudeReading msg;
                    msg.altitude(static_cast<float>(peak_can_gps_position_altitude_gps_altitude_decode(tmp.gps_altitude)));
                    if (VERBOSE) {
                        std::stringstream sstr;
                        msg.accept([](uint32_t, const std::string &, const std::string &) {},
                                   [&sstr](uint32_t, std::string &&, std::string &&n, auto v) { sstr << n << " = " << v << '\n'; },
                                   []() {});
                        std::clog << sstr.str() << std::endl;
                    }
                    //od4.send(msg, ts, ID);
                    cluon::data::Envelope env;
                    env.senderStamp(ID)
                       .sent(ts)
                       .received(ts)
                       .sampleTimeStamp(ts);
                    cluon::ToProtoVisitor proto;
            
                    env.dataType(msg.ID());
                    msg.accept(proto);
                    env.serializedData(proto.encodedData());
                    std::cout << cluon::serializeEnvelope(std::move(env));
                    std::cout.flush();
                }
            }
            else if (PEAK_CAN_GPS_DELUSIONS_A_FRAME_ID == canFrameID) {
                peak_can_gps_delusions_a_t tmp;
                if (0 == peak_can_gps_delusions_a_unpack(&tmp, src, len)) {
                    std::lock_guard<std::mutex> lck(msgDelusionMutex);
                    msgDelusion.GPS_PDOP(static_cast<float>(peak_can_gps_delusions_a_gps_pdop_decode(tmp.gps_pdop)));
                    msgDelusion.GPS_HDOP(static_cast<float>(peak_can_gps_delusions_a_gps_hdop_decode(tmp.gps_hdop)));
                    if (VERBOSE) {
                        std::stringstream sstr;
                        msgDelusion.accept([](uint32_t, const std::string &, const std::string &) {},
                                   [&sstr](uint32_t, std::string &&, std::string &&n, auto v) { sstr << n << " = " << v << '\n'; },
                                   []() {});
                        std::clog << sstr.str() << std::endl;
                    }
                    // Will be sent when delusion frame B is in.
                    //od4.send(msgDelusion, ts, ID);
                }
            }
            else if (PEAK_CAN_GPS_DELUSIONS_B_FRAME_ID == canFrameID) {
                peak_can_gps_delusions_b_t tmp;
                if (0 == peak_can_gps_delusions_b_unpack(&tmp, src, len)) {
                    std::lock_guard<std::mutex> lck(msgDelusionMutex);
                    msgDelusion.GPS_VDOP(static_cast<float>(peak_can_gps_delusions_b_gps_vdop_decode(tmp.gps_vdop)));
                    if (VERBOSE) {
                        std::stringstream sstr;
                        msgDelusion.accept([](uint32_t, const std::string &, const std::string &) {},
                                   [&sstr](uint32_t, std::string &&, std::string &&n, auto v) { sstr << n << " = " << v << '\n'; },
                                   []() {});
                        std::clog << sstr.str() << std::endl;
                    }
                    //od4.send(msgDelusion, ts, ID);
                    cluon::data::Envelope env;
                    env.senderStamp(ID)
                       .sent(ts)
                       .received(ts)
                       .sampleTimeStamp(ts);
                    cluon::ToProtoVisitor proto;
            
                    env.dataType(msgDelusion.ID());
                    msgDelusion.accept(proto);
                    env.serializedData(proto.encodedData());
                    std::cout << cluon::serializeEnvelope(std::move(env));
                    std::cout.flush();
                }
            }
            else if (PEAK_CAN_GPS_STATUS_FRAME_ID == canFrameID) {
                peak_can_gps_status_t tmp;
                if (0 == peak_can_gps_status_unpack(&tmp, src, len)) {
                    opendlv::device::gps::peak::GPSStatus msg;
                    msg.antennaStatus(static_cast<uint8_t>(peak_can_gps_status_gps_antenna_status_decode(tmp.gps_antenna_status)));
                    msg.numberOfSatellites(static_cast<uint8_t>(peak_can_gps_status_gps_num_satellites_decode(tmp.gps_num_satellites)));
                    msg.navigationMethod(static_cast<uint8_t>(peak_can_gps_status_gps_navigation_method_decode(tmp.gps_navigation_method)));
                    if (VERBOSE) {
                        std::stringstream sstr;
                        msg.accept([](uint32_t, const std::string &, const std::string &) {},
                                   [&sstr](uint32_t, std::string &&, std::string &&n, auto v) { sstr << n << " = " << v << '\n'; },
                                   []() {});
                        std::clog << sstr.str() << std::endl;
                    }
                    //od4.send(msg, ts, ID);
                    cluon::data::Envelope env;
                    env.senderStamp(ID)
                       .sent(ts)
                       .received(ts)
                       .sampleTimeStamp(ts);
                    cluon::ToProtoVisitor proto;
            
                    env.dataType(msg.ID());
                    msg.accept(proto);
                    env.serializedData(proto.encodedData());
                    std::cout << cluon::serializeEnvelope(std::move(env));
                    std::cout.flush();
                }
            }
            else if (PEAK_CAN_IO_FRAME_ID == canFrameID) {
                peak_can_io_t tmp;
                if (0 == peak_can_io_unpack(&tmp, src, len)) {
                    opendlv::device::gps::peak::IO msg;
                    msg.din1Status(0 < std::fabs(peak_can_io_din1_status_decode(tmp.din1_status)));
                    msg.din2Status(0 < std::fabs(peak_can_io_din2_status_decode(tmp.din2_status)));
                    msg.doutStatus(0 < std::fabs(peak_can_io_dout_status_decode(tmp.dout_status)));
                    msg.SDPresent(0 < std::fabs(peak_can_io_sd_present_decode(tmp.sd_present)));
                    msg.GPSPowerStatus(0 < std::fabs(peak_can_io_gps_power_status_decode(tmp.gps_power_status)));
                    msg.deviceID(static_cast<uint8_t>(peak_can_io_device_id_decode(tmp.device_id)));
                    if (VERBOSE) {
                        std::stringstream sstr;
                        msg.accept([](uint32_t, const std::string &, const std::string &) {},
                                   [&sstr](uint32_t, std::string &&, std::string &&n, auto v) { sstr << n << " = " << v << '\n'; },
                                   []() {});
                        std::clog << sstr.str() << std::endl;
                    }
                    //od4.send(msg, ts, ID);
                    cluon::data::Envelope env;
                    env.senderStamp(ID)
                       .sent(ts)
                       .received(ts)
                       .sampleTimeStamp(ts);
                    cluon::ToProtoVisitor proto;
            
                    env.dataType(msg.ID());
                    msg.accept(proto);
                    env.serializedData(proto.encodedData());
                    std::cout << cluon::serializeEnvelope(std::move(env));
                    std::cout.flush();
                }
            }
            else if (PEAK_CAN_RTC_DATE_TIME_FRAME_ID == canFrameID) {
                peak_can_rtc_date_time_t tmp;
                if (0 == peak_can_rtc_date_time_unpack(&tmp, src, len)) {
                    opendlv::device::gps::peak::RTCDateTime msg;
                    msg.RTCSec(static_cast<uint8_t>(peak_can_rtc_date_time_rtc_sec_decode(tmp.rtc_sec)));
                    msg.RTCMin(static_cast<uint8_t>(peak_can_rtc_date_time_rtc_min_decode(tmp.rtc_min)));
                    msg.RTCHour(static_cast<uint8_t>(peak_can_rtc_date_time_rtc_hour_decode(tmp.rtc_hour)));
                    msg.RTCDayOfWeek(static_cast<uint8_t>(peak_can_rtc_date_time_rtc_day_of_week_decode(tmp.rtc_day_of_week)));
                    msg.RTCDayOfMonth(static_cast<uint8_t>(peak_can_rtc_date_time_rtc_day_of_month_decode(tmp.rtc_day_of_month)));
                    msg.RTCMonth(static_cast<uint8_t>(peak_can_rtc_date_time_rtc_month_decode(tmp.rtc_month)));
                    msg.RTCYear(static_cast<uint16_t>(peak_can_rtc_date_time_rtc_year_decode(tmp.rtc_year)));
                    if (VERBOSE) {
                        std::stringstream sstr;
                        msg.accept([](uint32_t, const std::string &, const std::string &) {},
                                   [&sstr](uint32_t, std::string &&, std::string &&n, auto v) { sstr << n << " = " << v << '\n'; },
                                   []() {});
                        std::clog << sstr.str() << std::endl;
                    }
                    //od4.send(msg, ts, ID);
                    cluon::data::Envelope env;
                    env.senderStamp(ID)
                       .sent(ts)
                       .received(ts)
                       .sampleTimeStamp(ts);
                    cluon::ToProtoVisitor proto;
            
                    env.dataType(msg.ID());
                    msg.accept(proto);
                    env.serializedData(proto.encodedData());
                    std::cout << cluon::serializeEnvelope(std::move(env));
                    std::cout.flush();
                }
            }
            else if (PEAK_CAN_GPS_DATE_TIME_FRAME_ID == canFrameID) {
                peak_can_gps_date_time_t tmp;
                if (0 == peak_can_gps_date_time_unpack(&tmp, src, len)) {
                    opendlv::device::gps::peak::UTCDateTime msg;
                    msg.UTCYear(static_cast<uint8_t>(peak_can_gps_date_time_utc_year_decode(tmp.utc_year)));
                    msg.UTCMonth(static_cast<uint8_t>(peak_can_gps_date_time_utc_month_decode(tmp.utc_month)));
                    msg.UTCDayOfMonth(static_cast<uint8_t>(peak_can_gps_date_time_utc_day_of_month_decode(tmp.utc_day_of_month)));
                    msg.UTCHour(static_cast<uint8_t>(peak_can_gps_date_time_utc_hour_decode(tmp.utc_hour)));
                    msg.UTCMinute(static_cast<uint8_t>(peak_can_gps_date_time_utc_minute_decode(tmp.utc_minute)));
                    msg.UTCSecond(static_cast<uint8_t>(peak_can_gps_date_time_utc_second_decode(tmp.utc_second)));
                    if (VERBOSE) {
                        std::stringstream sstr;
                        msg.accept([](uint32_t, const std::string &, const std::string &) {},
                                   [&sstr](uint32_t, std::string &&, std::string &&n, auto v) { sstr << n << " = " << v << '\n'; },
                                   []() {});
                        std::clog << sstr.str() << std::endl;
                    }
                    //od4.send(msg, ts, ID);
                    cluon::data::Envelope env;
                    env.senderStamp(ID)
                       .sent(ts)
                       .received(ts)
                       .sampleTimeStamp(ts);
                    cluon::ToProtoVisitor proto;
            
                    env.dataType(msg.ID());
                    msg.accept(proto);
                    env.serializedData(proto.encodedData());
                    std::cout << cluon::serializeEnvelope(std::move(env));
                    std::cout.flush();
                }
            }

        };

        // Read from STDIN
        do {
            std::string line;
            std::getline(std::cin, line);
            if (line.length() > 0) {
                std::vector<std::string> tokens = stringtoolbox::split(line, ' ');

                // Timestamp.
                cluon::data::TimeStamp ts;
                {
                    std::string time = stringtoolbox::split(tokens.at(0).substr(1), ')').at(0);
                    std::string time_s = stringtoolbox::split(time, '.').at(0);
                    std::string time_mics = stringtoolbox::split(time, '.').at(1);
                    int64_t s = std::stoll(time_s);
                    int64_t mics = std::stoll(time_mics);
                    ts = cluon::time::fromMicroseconds(s * static_cast<int64_t>(1000*1000) + mics);
                }

                // FrameID.
                uint16_t ui_frameID = 0;
                {
                    std::string frameID = stringtoolbox::split(tokens.at(2), '#').at(0);
                    std::stringstream sstrFrameID;
                    sstrFrameID << std::hex << frameID;
                    sstrFrameID >> ui_frameID;
                }

                // Flip CAN data.
                uint8_t len = 0;
                union _data {
                  uint64_t u64 = 0;
                  uint8_t b8[sizeof(uint64_t)];
                } ui_data;
                {
                    std::string rdata = stringtoolbox::split(tokens.at(2), '#').at(1);
                    len = rdata.length();

                    std::vector<char> __data;
                    __data.resize(len);
                    int i = len;
                    int j = 0;
                    for(; i >= 2; i-=2) {
                      __data[j] = rdata[i-2];
                      __data[j+1] = rdata[i-1];
                      j+=2;
                    }
                    const std::string data(__data.begin(), __data.end());

                    std::stringstream sstrData;
                    sstrData << std::hex << data;

                    sstrData >> ui_data.u64;
                }

                //std::clog << ts.seconds() << "/" << ts.microseconds() << ", ID = " << ui_frameID << ":" << ui_data.u64 << ", " << +len << std::endl;

                decode(ts, ui_frameID, ui_data.b8, len/2);
            }
        } while (std::cin.good());
        retCode = 0;
    }
    return retCode;
}

