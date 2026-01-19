#include "helpers.h"

namespace esphome {
namespace truma_inetbox {

/// Calculate address parity bits for LIN protocol as per LIN specification
/// Uses P0 and P1 bits based on PID bit positions
uint8_t addr_parity(const uint8_t PID) {
  uint8_t P0 = ((PID >> 0) + (PID >> 1) + (PID >> 2) + (PID >> 4)) & 1;
  uint8_t P1 = ~((PID >> 1) + (PID >> 3) + (PID >> 4) + (PID >> 5)) & 1;
  return (P0 | (P1 << 1));
}

/// Calculate LIN data checksum with wrap-around handling
/// Supports both LIN 1.X (sum=0) and LIN 2.X Enhanced (sum=PID) CRC modes
uint8_t data_checksum(const uint8_t *message, uint8_t length, uint16_t sum) {
  // Add each byte to running sum with 255-based wrap-around
  for (uint8_t i = 0; i < length; i++) {
    sum += message[i];

    // Handle overflow: subtract 255 instead of 256 for LIN protocol
    if (sum >= 256)
      sum -= 255;
  }
  return (~sum);  // Return inverted checksum
}

/// Convert raw temperature sensor code to Celsius
/// Raw value is stored as (Celsius + 273) * 10 in the protocol
float temp_code_to_decimal(u_int16_t val, float zero) {
  if (val == 0) {
    return zero;
  }
  return ((float) val) / 10.0f - 273.0f;
}

/// Fix anomalous water temperature reading of 200°C to correct value of 80°C
float water_temp_200_fix(float val) {
  if (val == 200) {
    return 80;
  }
  return val;
}

/// Convert TargetTemp enum to decimal using generic conversion
float temp_code_to_decimal(TargetTemp val, float zero) { return temp_code_to_decimal((u_int16_t) val, zero); }

/// Convert uint8_t decimal Celsius to TargetTemp encoding
TargetTemp decimal_to_temp(uint8_t val) { return (TargetTemp) ((((u_int16_t) val) + 273) * 10); }

/// Convert float decimal Celsius to TargetTemp encoding
TargetTemp decimal_to_temp(float val) { return (TargetTemp) ((val + 273) * 10); }

/// Convert to room temperature with range validation and OFF state handling
/// Valid range: 5-30°C, below 5°C returns OFF
TargetTemp decimal_to_room_temp(uint8_t val) {
  if (val == 0) {
    return TargetTemp::TARGET_TEMP_OFF;
  }
  if (val < 5) {
    return TargetTemp::TARGET_TEMP_OFF;
  }
  if (val >= 30) {
    return TargetTemp::TARGET_TEMP_ROOM_MAX;
  }
  return decimal_to_temp(val);
}

/// Convert to room temperature with range validation and NaN handling
/// Valid range: 5-30°C, below 5°C or NaN returns OFF
TargetTemp decimal_to_room_temp(float val) {
  if (std::isnan(val)) {
    return TargetTemp::TARGET_TEMP_OFF;
  }
  if (val < 5) {
    return TargetTemp::TARGET_TEMP_OFF;
  }
  if (val >= 30) {
    return TargetTemp::TARGET_TEMP_ROOM_MAX;
  }
  return decimal_to_temp(val);
}

/// Convert to AC manual mode temperature with range validation
/// Valid range: 16-31°C, outside range returns OFF
TargetTemp decimal_to_aircon_manual_temp(uint8_t val) {
  if (val == 0) {
    return TargetTemp::TARGET_TEMP_OFF;
  }
  if (val < 16) {
    return TargetTemp::TARGET_TEMP_OFF;
  }
  if (val >= 31) {
    return TargetTemp::TARGET_TEMP_AIRCON_MAX;
  }
  return decimal_to_temp(val);
}

/// Convert to AC manual mode temperature with range validation and NaN handling
/// Valid range: 16-31°C, outside range or NaN returns OFF
TargetTemp decimal_to_aircon_manual_temp(float val) {
  if (std::isnan(val)) {
    return TargetTemp::TARGET_TEMP_OFF;
  }
  if (val < 16) {
    return TargetTemp::TARGET_TEMP_OFF;
  }
  if (val >= 31) {
    return TargetTemp::TARGET_TEMP_AIRCON_MAX;
  }
  return decimal_to_temp(val);
}

/// Convert to AC auto mode temperature with range validation
/// Valid range: 16-31°C, outside range returns OFF
TargetTemp decimal_to_aircon_auto_temp(uint8_t val) {
  if (val == 0) {
    return TargetTemp::TARGET_TEMP_OFF;
  }
  if (val < 16) {
    return TargetTemp::TARGET_TEMP_OFF;
  }
  if (val >= 31) {
    return TargetTemp::TARGET_TEMP_AIRCON_MAX;
  }
  return decimal_to_temp(val);
}

TargetTemp decimal_to_aircon_auto_temp(float val) {
  if (std::isnan(val)) {
    return TargetTemp::TARGET_TEMP_OFF;
  }
  if (val < 16) {
    return TargetTemp::TARGET_TEMP_OFF;
  }
  if (val >= 31) {
    return TargetTemp::TARGET_TEMP_AIRCON_MAX;
  }
  return decimal_to_temp(val);
}

TargetTemp decimal_to_water_temp(uint8_t val) {
  if (val < 40) {
    return TargetTemp::TARGET_TEMP_OFF;
  } else if (val >= 40 && val < 60) {
    return TargetTemp::TARGET_TEMP_WATER_ECO;
  } else if (val >= 60 && val < 80) {
    return TargetTemp::TARGET_TEMP_WATER_HIGH;
  } else {
    return TargetTemp::TARGET_TEMP_WATER_BOOST;
  }
}

TargetTemp decimal_to_water_temp(float val) {
  if (std::isnan(val) || val < 40) {
    return TargetTemp::TARGET_TEMP_OFF;
  } else if (val >= 40 && val < 60) {
    return TargetTemp::TARGET_TEMP_WATER_ECO;
  } else if (val >= 60 && val < 80) {
    return TargetTemp::TARGET_TEMP_WATER_HIGH;
  } else {
    return TargetTemp::TARGET_TEMP_WATER_BOOST;
  }
}

const std::string operating_status_to_str(OperatingStatus val) {
  if (val == OperatingStatus::OPERATING_STATUS_OFF) {
    return "OFF";
  } else if (val == OperatingStatus::OPERATING_STATUS_WARNING) {
    return "WARNING";
  } else if (val == OperatingStatus::OPERATING_STATUS_START_OR_COOL_DOWN) {
    return "START/COOL DOWN";
  } else if (val == OperatingStatus::OPERATING_STATUS_ON_5) {
    return "ON (5)";
  } else if (val == OperatingStatus::OPERATING_STATUS_ON_6) {
    return "ON (6)";
  } else if (val == OperatingStatus::OPERATING_STATUS_ON_7) {
    return "ON (7)";
  } else if (val == OperatingStatus::OPERATING_STATUS_ON_8) {
    return "ON (8)";
  } else if (val == OperatingStatus::OPERATING_STATUS_ON_9) {
    return "ON (9)";
  } else {
    return esphome::str_snprintf("ON %u", 6, (uint8_t) val);
  }
}

ElectricPowerLevel decimal_to_el_power_level(u_int16_t val) {
  if (val >= 1800) {
    return ElectricPowerLevel::ELECTRIC_POWER_LEVEL_1800;
  } else if (val >= 900) {
    return ElectricPowerLevel::ELECTRIC_POWER_LEVEL_900;
  } else {
    return ElectricPowerLevel::ELECTRIC_POWER_LEVEL_0;
  }
}

}  // namespace truma_inetbox
}  // namespace esphome
