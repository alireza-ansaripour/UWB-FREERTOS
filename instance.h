#include "deca_types.h"
#include "deca_device_api.h"

static void time_sync_task (void * pvParameter);

enum Role {
  ROLE_NONE,
  ROLE_RX,
  ROLE_TX,
  ROLE_TS,
  ROLE_TS_MASTER,
}; 

typedef struct{
  uint32_t LQ_CHECK_PERIOD;
  uint32_t tx_enable;
  uint32_t rx_enable;
  uint8_t ID;
  uint16_t dst;
  enum Role role;
  

  struct{
    uint32_t tx_num_per_round;
    uint64_t tx_sequence_number;
    uint32_t tx_delay_dw;
    uint32_t tx_delay_rtos;
    size_t   packet_size;
    uint32_t cca_timeout;
    uint32_t IPI;
  }tx;
  struct{
    uint32_t rx_ok;
    uint32_t rx_self;
    uint32_t rx_other;
    uint32_t rx_err;
  }rx; 
  dwt_config_t config;
  dwt_txconfig_t tx_config;

} instance_info_t; 




extern instance_info_t instance_info; 

#define LED_RX GPIO_DIR_GDP2_BIT_MASK
#define LED_TX GPIO_DIR_GDP3_BIT_MASK
#define PORT_DE GPIO_DIR_GDP1_BIT_MASK
#define SET_OUPUT_GPIOs 0xFFFF & ~(GPIO_DIR_GDP1_BIT_MASK | GPIO_DIR_GDP2_BIT_MASK | GPIO_DIR_GDP3_BIT_MASK)
#define ENABLE_ALL_GPIOS_MASK 0x200000

void instance_init();

extern dwt_txconfig_t tx_pwr_array[];