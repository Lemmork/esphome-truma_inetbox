#pragma once

#include <queue>
#include "LinBusListener.h"

namespace esphome {
namespace truma_inetbox {

/// LIN protocol handler for diagnostic and heartbeat messages
/// Implements standard LIN protocol features like multi-frame messages and heartbeat
class LinBusProtocol : public LinBusListener {
 public:
  /// Return LIN identifier (4 bytes) for device identification
  virtual const std::array<uint8_t, 4> lin_identifier() = 0;
  /// Send heartbeat/keep-alive message to indicate device is operational
  virtual void lin_heartbeat() = 0;
  /// Reset device to initial state
  virtual void lin_reset_device();

 protected:
  /// Standard empty LIN response (all 0xFF)
  const std::array<uint8_t, 8> lin_empty_response_ = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

  /// Process incoming LIN order and return response
  bool answer_lin_order_(const uint8_t pid) override;
  /// Handle received LIN message
  void lin_message_recieved_(const uint8_t pid, const uint8_t *message, uint8_t length) override;

  /// Read diagnostic information by identifier
  virtual bool lin_read_field_by_identifier_(uint8_t identifier, std::array<uint8_t, 5> *response) = 0;
  /// Handle multi-frame LIN message reception and return response
  virtual const uint8_t *lin_multiframe_recieved(const uint8_t *message, const uint8_t message_len,
                                                  uint8_t *return_len) = 0;

  /// Queue of update messages to send to the bus
  std::queue<std::array<uint8_t, 8>> updates_to_send_ = {};

 private:
  uint8_t lin_node_address_ = /*LIN initial node address*/ 0x03;

  /// Queue message for transmission on LIN bus
  void prepare_update_msg_(const std::array<uint8_t, 8> message) { this->updates_to_send_.push(std::move(message)); }
  
  /// Check if incoming message matches our device identifier
  bool is_matching_identifier_(const uint8_t *message);

  // Multi-frame PDU (Protocol Data Unit) message state tracking
  u_int16_t multi_pdu_message_expected_size_ = 0;
  uint8_t multi_pdu_message_len_ = 0;
  uint8_t multi_pdu_message_frame_counter_ = 0;
  uint8_t multi_pdu_message_[64];
  
  /// Handle single-frame diagnostic message
  void lin_msg_diag_single_(const uint8_t *message, uint8_t length);
  /// Handle first frame of multi-frame diagnostic message
  void lin_msg_diag_first_(const uint8_t *message, uint8_t length);
  /// Handle consecutive frame of multi-frame diagnostic message
  bool lin_msg_diag_consecutive_(const uint8_t *message, uint8_t length);
  /// Process complete multi-frame diagnostic message
  void lin_msg_diag_multi_();
};

}  // namespace truma_inetbox
}  // namespace esphome
