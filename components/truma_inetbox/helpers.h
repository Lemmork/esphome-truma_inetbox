#pragma once

#include "TrumaEnums.h"
#include "esphome/core/helpers.h"

namespace esphome {
namespace truma_inetbox {
// First byte is service identifier and to be ignored. Last three bytes can be `xFF` or `x00` (see
// <https://github.com/Fabian-Schmidt/esphome-truma_inetbox/issues/25>).
// `truma_message_header` and `alde_message_header` must have the same size!

/// LIN message header identifier for Truma devices
const std::array<uint8_t, 11> truma_message_header = {0x00, 0x00, 0x1F, 0x00, 0x1E, 0x00,
                                                       0x00, 0x22, 0xFF, 0xFF, 0xFF};

/// LIN message header identifier for Alde devices
const std::array<uint8_t, 11> alde_message_header = {0x00, 0x00, 0x1F, 0x00, 0x1A, 0x00, 0x00, 0x22, 0xFF, 0xFF, 0xFF};

/// Calculate address parity bits for LIN protocol (PID)
/// @param pid Protocol Identifier byte
/// @return 2-bit parity value
uint8_t addr_parity(const uint8_t pid);

/// Calculate checksum for LIN message data
/// @param message Pointer to message data
/// @param length Message length in bytes
/// @param sum Initial sum value (0 for LIN 1.X, PID for LIN 2.X)
/// @return Calculated checksum value
uint8_t data_checksum(const uint8_t *message, uint8_t length, uint16_t sum);

/// Convert temperature code (raw sensor value) to decimal Celsius
/// @param val Raw temperature code
/// @param zero Default value if temperature code is zero (default: NAN)
/// @return Temperature in Celsius
float temp_code_to_decimal(u_int16_t val, float zero = NAN);

/// Convert TargetTemp enum to decimal Celsius
/// @param val TargetTemp enum value
/// @param zero Default value if temperature code is zero (default: NAN)
/// @return Temperature in Celsius
float temp_code_to_decimal(TargetTemp val, float zero = NAN);

/// Fix water temperature reading for 200°C anomaly
/// @param val Raw water temperature value
/// @return Corrected temperature (80°C if input is 200, otherwise unchanged)
float water_temp_200_fix(float val);

/// Convert decimal Celsius temperature to TargetTemp enum format
/// @param val Temperature in Celsius as uint8_t
/// @return TargetTemp encoded value
TargetTemp decimal_to_temp(uint8_t val);

/// Convert decimal Celsius temperature to TargetTemp enum format
/// @param val Temperature in Celsius as float
/// @return TargetTemp encoded value
TargetTemp decimal_to_temp(float val);

/// Convert decimal to room temperature with validation (5-30°C range)
/// @param val Temperature in Celsius as uint8_t
/// @return TargetTemp with validated room temperature range
TargetTemp decimal_to_room_temp(uint8_t val);

/// Convert decimal to room temperature with validation (5-30°C range)
/// @param val Temperature in Celsius as float
/// @return TargetTemp with validated room temperature range
TargetTemp decimal_to_room_temp(float val);

/// Convert decimal to air conditioning manual temperature with validation (16-31°C range)
/// @param val Temperature in Celsius as uint8_t
/// @return TargetTemp with validated AC manual range
TargetTemp decimal_to_aircon_manual_temp(uint8_t val);

/// Convert decimal to air conditioning manual temperature with validation (16-31°C range)
/// @param val Temperature in Celsius as float
/// @return TargetTemp with validated AC manual range
TargetTemp decimal_to_aircon_manual_temp(float val);

/// Convert decimal to air conditioning auto temperature with validation (16-31°C range)
/// @param val Temperature in Celsius as uint8_t
/// @return TargetTemp with validated AC auto range
TargetTemp decimal_to_aircon_auto_temp(uint8_t val);

/// Convert decimal to air conditioning auto temperature with validation (16-31°C range)
/// @param val Temperature in Celsius as float
/// @return TargetTemp with validated AC auto range
TargetTemp decimal_to_aircon_auto_temp(float val);

/// Convert decimal to water heater temperature with validation
/// @param val Temperature in Celsius as uint8_t
/// @return TargetTemp with validated water temperature range
TargetTemp decimal_to_water_temp(uint8_t val);

/// Convert decimal to water heater temperature with validation
/// @param val Temperature in Celsius as float
/// @return TargetTemp with validated water temperature range
TargetTemp decimal_to_water_temp(float val);

/// Convert operating status enum to human-readable string
/// @param val OperatingStatus enum value
/// @return String representation of operating status
const std::string operating_status_to_str(OperatingStatus val);

/// Convert electric power level code to ElectricPowerLevel enum
/// @param val Raw power level code
/// @return ElectricPowerLevel enum value
ElectricPowerLevel decimal_to_el_power_level(u_int16_t val);

}  // namespace truma_inetbox
}  // namespace esphome
