#pragma once

#include "esphome/core/component.h"
#include "LinBusProtocol.h"
#include "TrumaStructs.h"
#include "TrumaiNetBoxAppAirconAuto.h"
#include "TrumaiNetBoxAppAirconManual.h"
#include "TrumaiNetBoxAppClock.h"
#include "TrumaiNetBoxAppConfig.h"
#include "TrumaiNetBoxAppHeater.h"
#include "TrumaiNetBoxAppTimer.h"

#ifdef USE_TIME
#include "esphome/components/time/real_time_clock.h"
#endif  // USE_TIME

namespace esphome {
namespace truma_inetbox {

class TrumaiNetBoxApp; // Forward Declaration

/// LIN Protocol Identifier for Truma iNetBox device
#define LIN_PID_TRUMA_INET_BOX 0x18

/// Main application class for Truma iNetBox communication
/// Manages all device sub-systems (heater, A/C, clock, timer, etc.) and LIN bus communication
class TrumaiNetBoxApp : public LinBusProtocol {
 public:
  TrumaiNetBoxApp();
  /// Update all sub-systems and transmit changes to LIN bus
  void update() override;

  /// Get LIN device identifier (4 bytes) for registration
  const std::array<uint8_t, 4> lin_identifier() override;
  /// Send periodic heartbeat to keep device active
  void lin_heartbeat() override;
  /// Reset device to factory defaults
  void lin_reset_device() override;

  /// Get the main heater device type (Truma, Alde, etc.)
  TRUMA_DEVICE get_heater_device() const { return this->heater_device_; }
  /// Get the air conditioning device type
  TRUMA_DEVICE get_aircon_device() const { return this->aircon_device_; }

  /// Get air conditioning auto mode controller
  TrumaiNetBoxAppAirconAuto *get_aircon_auto() { return &this->airconAuto_; }
  /// Get air conditioning manual mode controller
  TrumaiNetBoxAppAirconManual *get_aircon_manual() { return &this->airconManual_; }
  /// Get clock/time synchronization controller
  TrumaiNetBoxAppClock *get_clock() { return &this->clock_; }
  /// Get device configuration controller
  TrumaiNetBoxAppConfig *get_config() { return &this->config_; }
  /// Get main heating system controller
  TrumaiNetBoxAppHeater *get_heater() { return &this->heater_; }
  /// Get timer/schedule controller
  TrumaiNetBoxAppTimer *get_timer() { return &this->timer_; }

  /// Get timestamp of last CP+ request for diagnostic purposes
  int64_t get_last_cp_plus_request() { return this->device_registered_; }

#ifdef USE_TIME
  /// Set the real-time clock component for time synchronization
  void set_time(time::RealTimeClock *time) { time_ = time; }
  /// Get the real-time clock component
  time::RealTimeClock *get_time() const { return time_; }
#endif  // USE_TIME

 protected:
  uint32_t device_registered_ = 0;     ///< Timestamp when device was registered on bus
  uint32_t init_requested_ = 0;        ///< Timestamp when initialization was requested
  uint32_t init_recieved_ = 0;         ///< Timestamp when initialization response was received
  uint8_t message_counter = 1;         ///< Message sequence counter for LIN protocol

  TRUMA_COMPANY company_ = TRUMA_COMPANY::TRUMA;  ///< Device manufacturer (Truma or Alde)
  TRUMA_DEVICE heater_device_ = TRUMA_DEVICE::UNKNOWN;    ///< Detected heater model
  TRUMA_DEVICE aircon_device_ = TRUMA_DEVICE::UNKNOWN;    ///< Detected air conditioner model

  /// Sub-system controllers
  TrumaiNetBoxAppAirconAuto airconAuto_;
  TrumaiNetBoxAppAirconManual airconManual_;
  TrumaiNetBoxAppClock clock_;
  TrumaiNetBoxAppConfig config_;
  TrumaiNetBoxAppHeater heater_;
  TrumaiNetBoxAppTimer timer_;

  uint32_t update_time_ = 0;  ///< Timestamp of last update

#ifdef USE_TIME
  time::RealTimeClock *time_ = nullptr;       ///< Real-time clock reference
  bool update_status_clock_done = false;      ///< Flag for clock sync completion
#endif  // USE_TIME

  /// Handle incoming LIN order (command) and respond
  bool answer_lin_order_(const uint8_t pid) override;
  /// Read diagnostic information by identifier
  bool lin_read_field_by_identifier_(uint8_t identifier, std::array<uint8_t, 5> *response) override;
  /// Process multi-frame diagnostic message
  const uint8_t *lin_multiframe_recieved(const uint8_t *message, const uint8_t message_len,
                                          uint8_t *return_len) override;
  /// Check if there are pending updates to transmit to the bus
  bool has_update_to_submit_();
};

}  // namespace truma_inetbox
}  // namespace esphome