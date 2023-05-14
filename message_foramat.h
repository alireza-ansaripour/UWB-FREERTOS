#include "deca_types.h"


#define MSG_ACK                         0xAC
#define MSG_TS                          0x01
#define MSG_DATA                        0xDA



typedef struct __attribute__((packed)){
  uint64_t seq;
  uint16_t config_ID;
  uint8_t payload[1000];
} packet_t;

typedef struct __attribute__((packed)){
  uint8_t sender;
  uint16_t config_counter;
} ts_msg;

typedef struct __attribute__((packed)){
  uint16_t packet_id; // PACKET_ID
  uint8_t sequence_number __attribute__((packed));
  uint16_t pan_id; __attribute__((packed)); // PAN_ID
  uint16_t dst;
  uint16_t src;
  uint8_t msg_type;
  uint8_t payload[20];
} mac_frame; 