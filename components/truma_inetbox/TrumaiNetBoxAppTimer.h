#pragma once

#include "TrumaStausFrameResponseStorage.h"
#include "TrumaStructs.h"

namespace esphome {
namespace truma_inetbox {

class TrumaiNetBoxAppTimer : public TrumaStausFrameResponseStorage<StatusFrameTimer, StatusFrameTimerResponse> {
 public:
  StatusFrameTimerResponse *update_prepare() override;
  void create_update_data(StatusFrame *response, uint8_t *response_len, uint8_t command_counter) override;
  void dump_data() const override;
  
  uint16_t get_start_time() const { return this->status_.start_time; }
  uint16_t get_stop_time() const { return this->status_.stop_time; }
  float get_room_temperature() const { return (float)this->status_.target_temp_room / 10.0f; }
  uint8_t get_water_temperature() const { return this->status_.target_temp_water; }

  bool action_timer_disable();
  bool action_timer_activate(u_int16_t start, u_int16_t stop, uint8_t room_temperature,
                             HeatingMode mode = HeatingMode::HEATING_MODE_OFF, uint8_t water_temperature = 0,
                             EnergyMix energy_mix = EnergyMix::ENERGY_MIX_NONE,
                             ElectricPowerLevel el_power_level = ElectricPowerLevel::ELECTRIC_POWER_LEVEL_0);
};

}  // namespace truma_inetbox
}  // namespace esphome