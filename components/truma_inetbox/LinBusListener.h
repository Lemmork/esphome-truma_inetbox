#pragma once

#include "LinBusLog.h"
#include "esphome/core/component.h"
#include "esphome/components/uart/uart.h"

#ifdef USE_ESP32
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#endif  // USE_ESP32
#ifdef USE_RP2040
#include <hardware/uart.h>
#include <FreeRTOS.h>
#include <semphr.h>
#include <queue.h>
#endif  // USE_RP2040

#ifndef  TRUMA_MSG_QUEUE_LENGTH
#define TRUMA_MSG_QUEUE_LENGTH 6
#endif
#ifndef  TRUMA_LOG_QUEUE_LENGTH
#define TRUMA_LOG_QUEUE_LENGTH 6
#endif

namespace esphome {
namespace truma_inetbox {

/// LIN protocol checksum versions (LIN 1.X vs LIN 2.X Enhanced)
enum class LIN_CHECKSUM { LIN_CHECKSUM_VERSION_1, LIN_CHECKSUM_VERSION_2 };

/// Structure for queued LIN bus messages
struct QUEUE_LIN_MSG {
  uint8_t current_PID;        ///< Protocol Identifier of the message
  uint8_t data[8];            ///< Message data payload (up to 8 bytes)
  uint8_t len;                ///< Actual length of data in bytes
};

/// Base class for LIN bus communication and protocol handling
/// Manages UART communication, message queuing, and protocol handshake
class LinBusListener : public PollingComponent, public uart::UARTDevice {
 public:
  float get_setup_priority() const override { return setup_priority::DATA; }

  void dump_config() override;
  void setup() override;
  void update() override;

  /// Set the LIN checksum version to use
  void set_lin_checksum(LIN_CHECKSUM val) noexcept { this->lin_checksum_ = val; }
  /// Set the chip select (CS) pin for SPI/bus control
  void set_cs_pin(GPIOPin *pin) noexcept { this->cs_pin_ = pin; }
  /// Set the fault detection pin
  void set_fault_pin(GPIOPin *pin) noexcept { this->fault_pin_ = pin; }
  /// Set observer mode (listen-only, no transmission)
  void set_observer_mode(bool val) noexcept { this->observer_mode_ = val; }
  /// Check if LIN bus fault has been detected (persistent over 3 readings)
  bool get_lin_bus_fault() const noexcept { return fault_on_lin_bus_reported_ > 3; }

  /// Process all queued LIN messages with timeout
  void process_lin_msg_queue(TickType_t xTicksToWait);
  /// Process all queued debug log entries with timeout
  void process_log_queue(TickType_t xTicksToWait);

#ifdef USE_RP2040
  /// RP2040: Handle serial data and return recommended wait time until next check
  uint32_t onSerialEvent();
#endif  // USE_RP2040

 protected:
  LIN_CHECKSUM lin_checksum_{LIN_CHECKSUM::LIN_CHECKSUM_VERSION_2};
  GPIOPin *cs_pin_{nullptr};          ///< Chip select pin for SPI control
  GPIOPin *fault_pin_{nullptr};       ///< Fault detection pin
  bool observer_mode_{false};         ///< If true, only listen without transmitting

  /// Send data response on LIN bus
  void write_lin_answer_(const uint8_t *data, uint8_t len);
  /// Check and report LIN bus fault condition
  bool check_for_lin_fault_();
  /// Process incoming LIN order request and return response
  virtual bool answer_lin_order_(const uint8_t pid) = 0;
  /// Handle received LIN message
  virtual void lin_message_recieved_(const uint8_t pid, const uint8_t *message, uint8_t length) = 0;

 private:
  // Microseconds per UART Baud
  uint32_t time_per_baud_{0};
  // 9.. 15
  static constexpr uint8_t lin_break_length = 13;
  // Microseconds per LIN Break
  uint32_t time_per_lin_break_{0};
  static constexpr uint8_t frame_length_ = (8 /* bits */ + 1 /* Start bit */ + 2 /* Stop bits */);
  // Microseconds per UART Byte (UART Frame)
  uint32_t time_per_pid_{0};
  // Microseconds per UART Byte (UART Frame)
  uint32_t time_per_first_byte_{0};
  // Microseconds per UART Byte (UART Frame)
  uint32_t time_per_byte_{0};

  uint8_t fault_on_lin_bus_reported_{0};
  bool can_write_lin_answer_{false};

  enum class ReadState {
    BREAK,
    SYNC,
    SID,
    DATA,
    ACT,
  };
  ReadState current_state_{ReadState::BREAK};
  uint8_t current_PID_with_parity_{0x00};
  uint8_t current_PID_{0x00};
  bool current_PID_order_answered_{false};
  bool current_data_valid{true};
  uint8_t current_data_count_{0};
  // up to 8 byte data frame + CRC
  std::array<uint8_t, 9> current_data_{};
  // Time when the last LIN data was available.
  uint32_t last_data_recieved_{0};

  void current_state_reset_() noexcept {
    this->current_state_ = ReadState::BREAK;
    this->current_PID_with_parity_ = 0x00;
    this->current_PID_ = 0x00;
    this->current_PID_order_answered_ = false;
    this->current_data_valid = true;
    this->current_data_count_ = 0;
    this->current_data_.fill(0);
  };
  void onReceive_();
  void read_lin_frame_();
  void clear_uart_buffer_();
  void setup_framework();

  uint8_t lin_msg_static_queue_storage[TRUMA_MSG_QUEUE_LENGTH * sizeof(QUEUE_LIN_MSG)];
  StaticQueue_t lin_msg_static_queue_;
  QueueHandle_t lin_msg_queue_ =
      xQueueCreateStatic(/* uxQueueLength */ TRUMA_MSG_QUEUE_LENGTH,
                         /* uxItemSize */ sizeof(QUEUE_LIN_MSG),
                         /* pucQueueStorageBuffer */ lin_msg_static_queue_storage, &lin_msg_static_queue_);

#if ESPHOME_LOG_LEVEL > ESPHOME_LOG_LEVEL_NONE
  uint8_t log_static_queue_storage[TRUMA_LOG_QUEUE_LENGTH * sizeof(QUEUE_LOG_MSG)];
  StaticQueue_t log_static_queue_;
  QueueHandle_t log_queue_ =
      xQueueCreateStatic(/* uxQueueLength */ TRUMA_LOG_QUEUE_LENGTH,
                         /* uxItemSize */ sizeof(QUEUE_LOG_MSG),
                         /* pucQueueStorageBuffer */ log_static_queue_storage, &log_static_queue_);
#endif

#ifdef USE_ESP32
  TaskHandle_t eventTaskHandle_;
  static void eventTask_(void *args);
  TaskHandle_t uartEventTaskHandle_;
  static void uartEventTask_(void *args);
#endif  // USE_ESP32
#ifdef USE_RP2040
  uint8_t uart_number_ = 0;
  uart_inst_t *uart_ = nullptr;
#endif  // USE_RP2040
};

}  // namespace truma_inetbox
}  // namespace esphome
