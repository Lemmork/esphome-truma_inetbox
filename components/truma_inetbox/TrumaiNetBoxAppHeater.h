#pragma once

#include "TrumaStausFrameResponseStorage.h"
#include "TrumaStructs.h"

namespace esphome {
namespace truma_inetbox {

class TrumaiNetBoxAppHeater : public TrumaStausFrameResponseStorage<StatusFrameHeater, StatusFrameHeaterResponse> {
 public:
  StatusFrameHeaterResponse *update_prepare() override;
  void create_update_data(StatusFrame *response, uint8_t *response_len, uint8_t command_counter) override;
  void dump_data() const override;
  bool can_update() override;

  float get_current_room_temperature() const { return (float)this->status_.current_temp_room / 10.0f; }
  float get_current_water_temperature() const { return (float)this->status_.current_temp_water / 10.0f; }
  float get_target_room_temperature() const { return (float)this->status_.target_temp_room / 10.0f; }
  uint8_t get_target_water_temperature() const { return this->status_.target_temp_water; }
  uint8_t get_heating_mode() const { return (uint8_t)this->status_.heating_mode; }
  uint8_t get_operating_status() const { return (uint8_t)this->status_.operating_status; }
  uint16_t get_error_code() const { return this->status_.error_code; }
  uint8_t get_electric_power_level() const { return (uint8_t)this->status_.electric_power_level; }
  uint8_t get_energy_mix() const { return (uint8_t)this->status_.energy_mix; }

  bool action_heater_room(uint8_t temperature, HeatingMode mode = HeatingMode::HEATING_MODE_OFF);
  bool action_heater_water(uint8_t temperature);
  bool action_heater_water(TargetTemp temperature);
  bool action_heater_electric_power_level(u_int16_t value);
  bool action_heater_energy_mix(EnergyMix energy_mix,
                                ElectricPowerLevel el_power_level = ElectricPowerLevel::ELECTRIC_POWER_LEVEL_0);
};

}  // namespace truma_inetbox
}  // namespace esphome
