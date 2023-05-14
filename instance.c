 #include "FreeRTOS.h"
#include "task.h"
#include "timers.h"
#include "stdio.h"
#include "bsp.h"
#include "nordic_common.h"
#include "nrf_drv_clock.h"
#include "deca_device_api.h"
#include "instance.h"
#include "deca_regs.h"
//#include "nrf_drv_uart.h"
#include <sdk_config.h>
#include "radio_config.h"
#include "nrfx_uart.h"
#include <math.h>
#include "queue.h"
#include "message_foramat.h"
#include "shared_defines.h"
#include "roles_task.h"

#include "usb.h"

#include "app_error.h"
#include "app_util.h"
#include "app_usbd_core.h"
#include "app_usbd.h"
#include "app_usbd_string_desc.h"
#include "app_usbd_cdc_acm.h"
#include "app_usbd_serial_num.h"
#include "app_usbd_cdc_acm.h"


#define SET_OUPUT_GPIOs 0xFFFF & ~(GPIO_DIR_GDP1_BIT_MASK | GPIO_DIR_GDP2_BIT_MASK | GPIO_DIR_GDP3_BIT_MASK)
#define ENABLE_ALL_GPIOS_MASK 0x200000
#define SWITCH_CONF_INDEX 10



#define TASK_DELAY        100           /**< Task delay. Delays a LED0 task for 200 ms */
#define TIMER_PERIOD      500          /**< Timer period. LED1 timer will expire after 1000 ms */
#define WAIT              60000
#define IPI_TS            50
#define USB_QUEUE_SIZE    10

uint8_t stat;

int rx_flag = 0;
int tx_flag = 0;

TaskHandle_t  t2, t3;   /**< Reference to LED0 toggling FreeRTOS task. */
TimerHandle_t change_conf;  /**< Reference to LED1 toggling FreeRTOS timer. */


xQueueHandle dataQueue = NULL;
xQueueHandle txQueue = NULL;
xQueueHandle usbQueue = NULL;
xQueueHandle usbMSGQueue = NULL;


xTimerHandle tx_handler = NULL;
TimerHandle_t hb_handler = NULL;
TimerHandle_t Q_handler = NULL;

instance_info_t instance_info;
mac_frame tx_frame, rx_frame;
uint32_t tx_num;
int tx_size = 10;
int rx_size = 5;
uint64_t seq_num = 0;
int usb_queue_size = USB_QUEUE_SIZE;

uint64_t get_rx_timestamp_u64(void)
{
    uint8_t ts_tab[5];
    uint64_t ts = 0;
    int8_t i;
    dwt_readrxtimestamp(ts_tab);
    for (i = 4; i >= 0; i--)
    {
        ts <<= 8;
        ts |= ts_tab[i];
    }
    return ts;
}

uint64_t get_tx_timestamp_u64(void)
{
    uint8_t ts_tab[5];
    uint64_t ts = 0;
    int8_t i;
    dwt_readtxtimestamp(ts_tab);
    for (i = 4; i >= 0; i--)
    {
        ts <<= 8;
        ts |= ts_tab[i];
    }
    return ts;
}


void gpio_set(uint16_t port){
	dwt_or16bitoffsetreg(GPIO_OUT_ID, 0, (port));
}

void gpio_reset(uint16_t port){
	dwt_and16bitoffsetreg(GPIO_OUT_ID, 0, (uint16_t)(~(port)));
}

void init_LEDs(){
	dwt_enablegpioclocks();
	dwt_write32bitoffsetreg(GPIO_MODE_ID, 0, ENABLE_ALL_GPIOS_MASK);
	dwt_write16bitoffsetreg(GPIO_OUT_ID, 0, 0x0);
	dwt_write16bitoffsetreg(GPIO_DIR_ID, 0, SET_OUPUT_GPIOs);

}



uint8_t output_msges[10][200];
usb_msg_t usb_msg_arr[120];
uint8_t p;
void send_USB_msg(uint8_t *msg, size_t size){
  uint8_t p;
  if (usb_queue_size == 0){
    return;
  }
  usb_queue_size--;
  if (xQueueReceive(usbQueue, &p, 100)){
    memcpy(usb_msg_arr[p].msg, msg, size);
    usb_msg_arr[p].size = size;    
    long ret = xQueueSend(usbMSGQueue, &p, 100);
  }
}

static void task_None(void *  params){
  dwt_forcetrxoff();
  dwt_configure(&ts_config);
  dwt_configuretxrf(&ts_tx_config);
  //dwt_forcetrxoff();
  dwt_rxenable(DWT_START_RX_IMMEDIATE);
  uint32_t dev_id = NRF_FICR->DEVICEADDR[0];
  dwt_forcetrxoff();
  size_t size;
  int i = 0;
  dwt_cb_data_t cb_data;
  while (true){
    vTaskDelay(2000);
    size = sprintf(output_msges[0], "Device ID:%d, 0x%x\r\n", i, dev_id);
    send_USB_msg(output_msges[0], size);
    i++;
  }
}

size_t size;

static void switch_conf_ts(void * params){
  dwt_forcetrxoff();
  dwt_configure(&ts_config);
  dwt_configuretxrf(&ts_tx_config);
  dwt_rxenable(DWT_START_RX_IMMEDIATE);
  //size = sprintf(output_msges[0], "conf change\n");
  //send_USB_msg(output_msges[0], size);
}
 xTimerHandle ts_conf_handler;
int pwr_ind = 0;
int conf_ind = 0;
uint8_t senders[] = {0, 202, 111, 119, 133, 114, 56, 121, 225};
uint32_t config_counter = 0;
uint32_t pwr_counter = 0;

#define NUM_EXP_ROUND 1
int turn = 0;
int exp_round = 0;
int warmUp = 1;


static void task_TS_master(void * params){
  dwt_cb_data_t cb_data;
  packet_t *payload = tx_frame.payload;
  ts_msg *ts = payload->payload;
  size = sprintf(output_msges[1], "APP START\n");
  send_USB_msg(output_msges[1], size);
  //vTaskDelay(120000);
  seq_num = 0;
  while (1){
    LEDS_INVERT(BSP_LED_1_MASK);
    vTaskDelay(WAIT);
    dwt_configure(&ts_config);
    dwt_configuretxrf(&ts_tx_config);
    dwt_forcetrxoff();
    tx_frame.msg_type = MSG_TS;
    tx_frame.sequence_number = (uint8_t) instance_info.tx.tx_sequence_number;
    payload->config_ID  = config_counter;
    payload->seq = instance_info.tx.tx_sequence_number;
    ts->sender = senders[turn] ;
    ts->config_counter = config_counter;
    vTaskDelay(2*instance_info.ID % 10);
    dwt_writetxdata(sizeof(tx_frame), (uint8_t *) &tx_frame, 0);
    dwt_writetxfctrl(sizeof(tx_frame), 0, 0);
    int ret = dwt_starttx(DWT_START_TX_IMMEDIATE);
    if (ret == DWT_SUCCESS){
    
      gpio_set(LED_TX);
      while(xQueueReceive(txQueue, &stat, 10) == pdFALSE){
           
      }
      gpio_reset(LED_TX);
      size_t size = sprintf(output_msges[0], "TS MSG: SN: %u, Sender:%d, config: %d", (uint16_t)instance_info.tx.tx_sequence_number, senders[turn], config_counter);
      send_USB_msg(output_msges[0], size);
  
      pwr_ind = (config_counter/ (sizeof(conf_array)/sizeof(dwt_config_t))) % (sizeof(tx_pwr_array)/sizeof(dwt_txconfig_t));
      conf_ind = config_counter % (sizeof(conf_array)/sizeof(dwt_config_t));
      //conf_ind = (config_counter / (sizeof(tx_pwr_array)/sizeof(dwt_txconfig_t))) % (sizeof(conf_array)/sizeof(dwt_config_t));
      vTaskDelay(500);      
      size = sprintf(output_msges[1], "CONF:{PC: %u, DR:%u, PLEN:%u, PAC:%u},{TXP:0x%x}",conf_array[conf_ind].rxCode,conf_array[conf_ind].dataRate,conf_array[conf_ind].txPreambLength,conf_array[conf_ind].rxPAC, tx_pwr_array[pwr_ind].power);
      send_USB_msg(output_msges[1], size);
      
      instance_info.tx.tx_sequence_number ++;
      if (warmUp > 0){
        warmUp--;
        turn += 1;
        continue;
      }else
        config_counter++;
      
      if (config_counter % (sizeof(conf_array)/sizeof(dwt_config_t)) == 0){
        pwr_counter++;
        if (pwr_counter % (sizeof(tx_pwr_array)/sizeof(dwt_txconfig_t)) == 0){
          turn += 1;
          config_counter = 0;
          pwr_counter = 0;
        }
        
      }
      if (turn == sizeof(senders)){
        turn = 0;
      }
    }else{
    }
  }
  while(1){}

}


uint16_t looking;
dwt_rxdiag_t diag_info;

uint32_t ip_diag_1;
uint16_t ip_diag_12;
uint32_t dgc;
uint32_t power;






static void task_TS(void * params){
  dwt_cb_data_t cb_data;
  looking = 0;
  uint64_t seq_num = 0;  
  dwt_configure(&ts_config);
  dwt_configuretxrf(&ts_tx_config);
  dwt_forcetrxoff();
  dwt_rxenable(DWT_START_RX_IMMEDIATE);
  packet_t *payload = rx_frame.payload;
  packet_t *tx_payload = tx_frame.payload;
  ts_msg *ts = payload->payload;
  int cnt = -1;
  uint8_t sender;

  size_t size = sprintf(output_msges[0], "start PROCESS");  
  send_USB_msg(output_msges[0], size);
  uint32_t trx_timestamp = 0;
 
  
  while(1){
    
    if (xQueueReceive(dataQueue, &stat, 10)){
      LEDS_INVERT(BSP_LED_1_MASK);
      dwt_readrxdata((uint8_t *) &rx_frame, sizeof(rx_frame), 0);
      dwt_readdiagnostics(&diag_info);
      if (rx_frame.msg_type == MSG_TS){
        if (payload->seq >= looking){
          looking = payload->seq + 1;
          dwt_forcetrxoff();
  
          pwr_ind = (ts->config_counter/ (sizeof(conf_array)/sizeof(dwt_config_t))) % (sizeof(tx_pwr_array)/sizeof(dwt_txconfig_t));
          conf_ind = ts->config_counter % (sizeof(conf_array)/sizeof(dwt_config_t));
        
          cnt ++;
          size_t size = sprintf(output_msges[0], "TS MSG:%u,%u,%d,%d,%d", rx_frame.src, (uint16_t)rx_frame.sequence_number, ts->sender, conf_ind, pwr_ind);
          send_USB_msg(output_msges[0], size);
        
          rx_frame.src = instance_info.ID;        
          dwt_writetxdata(sizeof(rx_frame), (uint8_t *) &rx_frame, 0);
          dwt_writetxfctrl(sizeof(rx_frame), 0, 0);
          int ret = dwt_starttx(DWT_START_TX_IMMEDIATE);
          memcpy((uint8_t *) &sender, ts->sender, 1);
          gpio_set(LED_TX);
          while(xQueueReceive(txQueue, &stat, 10) == pdFALSE){
          }
          gpio_reset(LED_TX);
          dwt_forcetrxoff();
          if(dwt_configure(&conf_array[conf_ind]) == DWT_SUCCESS){
          
          }else{
          }

          dwt_configuretxrf(&tx_pwr_array[pwr_ind]);
        
          vTaskDelay(500);
          size = sprintf(output_msges[1], "CONF:{PC: %u, DR:%u, PLEN:%u, PAC:%u},{TXP:0x%x}",conf_array[conf_ind].rxCode,conf_array[conf_ind].dataRate,conf_array[conf_ind].txPreambLength,conf_array[conf_ind].rxPAC, tx_pwr_array[pwr_ind].power);
           send_USB_msg(output_msges[1], size);
          xTimerStart(ts_conf_handler, 10);
        
          if (instance_info.ID == ts->sender){
            dwt_forcetrxoff();
            vTaskDelay(1000);
            size = sprintf(output_msges[2], "Start TX");
            send_USB_msg(output_msges[2], size);
            vTaskDelay(1000);
            tx_frame.msg_type = MSG_DATA;
            for (int i = 0; i < 1000 ; i+= 1){
              tx_payload->seq = seq_num;
              seq_num++;
              dwt_writetxdata(sizeof(tx_frame), (uint8_t *) &tx_frame, 0);
              dwt_writetxfctrl(sizeof(tx_frame), 0, 0);
              int ret = dwt_starttx(DWT_START_TX_IMMEDIATE);
              if (ret != DWT_SUCCESS){
              }
              gpio_set(LED_TX);
              while(xQueueReceive(txQueue, &stat, 10) == pdFALSE){
           
              }
            
              gpio_reset(LED_TX);
              vTaskDelay(IPI_TS);
            }
          }
        }
        dwt_forcetrxoff();
        dwt_rxenable(DWT_START_RX_IMMEDIATE);
      }
      if (rx_frame.msg_type != MSG_TS){
        size_t size = sprintf(output_msges[3], "DATA MSG:%u,%u,%u\n", 
        rx_frame.src, 
        (uint32_t) payload->seq,
        diag_info.ipatovPower);
        send_USB_msg(output_msges[3], size);
        dwt_forcetrxoff();
        dwt_rxenable(DWT_START_RX_IMMEDIATE);
      }
    

    }

  }
}


uint32_t tx_time, curr, rx_time;
static void task_node(void * params){
  
}


uint16_t acc_arr2[100];
double rx_power[200];
int cnt2 = 0;
int rx_count_start;




static void start_rx_session(void * params){
  //dwt_rxenable(DWT_START_RX_IMMEDIATE);
  //rx_count_start = 0;
  //dwt_cb_data_t cb_data;
  //packet_t *payload = (packet_t *) rx_frame.payload;
  //uint8_t num;
  //while(1){
  //  if (xQueueReceive(dataQueue, &num, 10)){
  //    LEDS_INVERT(BSP_LED_1_MASK);
  //    dwt_readrxdata((uint8_t *) &rx_frame, 20, 0);
  //    //acc_arr2[cnt2++] = ip_diag_12;
  //    //printf("%d,%d,%d,%d\n",cntr,ip_diag_1,ip_diag_12,dgc);
  //    instance_info.rx.rx_ok++;
  //    if (rx_frame.msg_type == MSG_DATA){
  //      instance_info.rx.rx_other ++;
  //      double pwr = 10 * log10(2097152 * ip_diag_1 / (ip_diag_12 * ip_diag_12)) - 121.7 + (6 * dgc) ;
  //      rx_power[cnt2++] = pwr;        
  //      if (rx_frame.dst == instance_info.ID){
  //        //gpio_set(LED_RX);
  //        instance_info.rx.rx_self++;
  //        //gpio_reset(LED_RX);
  //        //printf("src: %d, with seq: %d\n", recv_frame.src, payload->seq );
  //      }
  //    }
  //  }
  //}
}


dwt_cb_data_t tx_cb;
void tx_handle(const dwt_cb_data_t *cb_data){
  //dwt_forcetrxoff();
  //dwt_rxenable(DWT_START_RX_IMMEDIATE);
  gpio_reset(LED_TX);
  tx_cb.status = 0;
  long ok = xQueueSend(txQueue, &stat, 2);
  if (ok != pdTRUE){
  }
  tx_flag = 1;
  

}


void rx_handle(const dwt_cb_data_t *cb_data){
  
  gpio_set(LED_RX);
  //dwt_rxdiag_t diag;
  //dwt_readdiagnostics(&diag);
  ////dwt_forcetrxoff();
  //power = diag.ipatovPower;
  ip_diag_1= dwt_read32bitoffsetreg(0xc0000,0x2C)& 0x1FFFF;//C
  ip_diag_12= dwt_read16bitoffsetreg(0xc0000,0x58)& 0xFFF;//N
  dgc=(dwt_read32bitoffsetreg(0x30000,0x60))>>28;//
  dwt_readdiagnostics(&diag_info);
  //dwt_rxenable(DWT_START_RX_IMMEDIATE);
  instance_info.rx.rx_ok++;
  gpio_reset(LED_RX);
  
  //dwt_readrxdata((uint8_t *) &rx_frame, 100, 0);
  long ok = xQueueSend(dataQueue, &stat, 20);
  if (ok != pdTRUE){
    //printf("RX buffer failed\n");
  }

}

void rx_to(const dwt_cb_data_t *cb_data){

}

int cnt = 0;
uint16_t acc_arr[100];
int err_cnt2 = 0;
void err_handle_cb(const dwt_cb_data_t *cb_data){
  size_t size;
  dwt_readdiagnostics(&diag_info);
  size = sprintf(output_msges[0], "RX ERR 0x%x, %d\n", cb_data->status, diag_info.ipatovPower);
  send_USB_msg(output_msges[0], size);
  dwt_forcetrxoff();
  dwt_rxenable(DWT_START_RX_IMMEDIATE);
}


void init_tx_frame(){
  tx_frame.packet_id = 0x8841;
  tx_frame.src = instance_info.ID;
  tx_frame.dst = instance_info.dst;
  memset(tx_frame.payload, instance_info.ID, 100);

}

void instance_config(){
  uint32_t dev_id = NRF_FICR->DEVICEADDR[0];
  instance_info.tx.tx_sequence_number = 0;
  instance_info.role = ROLE_NONE;
  
  switch(dev_id){
    case 0xe8d125b7: //235
      instance_info.role = ROLE_TS_MASTER;
      instance_info.ID = 235;
    break;
    case 0xf01065cf:
      instance_info.role = ROLE_TS;
      instance_info.ID = 12;
    break;
    case 0xccd1a0cf:
      instance_info.role = ROLE_TS;
      instance_info.ID = 13;
    break;

    case 0x920b25f: //56
      instance_info.role = ROLE_TS;
      instance_info.ID = 56;
    break;
    case 0xf5330684://107
      instance_info.role = ROLE_TS;
      instance_info.ID = 107;
    break;
    case 0x36ee80d0://201
      instance_info.role = ROLE_TS;
      instance_info.ID = 201;
    break;
    case 0xc6d8395a://114
      instance_info.role = ROLE_TS;
      instance_info.ID = 114;
    break;
    case 0xa2950321://111
      instance_info.role = ROLE_TS;
      instance_info.ID = 111;
    break;
    case 0xf965bd2e://133
      instance_info.role = ROLE_TS;
      instance_info.ID = 133;
    break;
    case  0x625880e5://225
      instance_info.role = ROLE_TS;
      instance_info.ID = 225;
    break;
    case 0x2e0bc9bb://202
      instance_info.role = ROLE_TS;
      instance_info.ID = 202;
    break;
    case 0x6355b620://121
      instance_info.role = ROLE_TS;
      instance_info.ID = 121;
    break;
    case 0xce95d98d://119
      instance_info.role = ROLE_TS;
      instance_info.ID = 119;
    break;
    
    

  };
  //instance_info.role = ROLE_NONE;
  dwt_configure(&ts_config);
  dwt_configuretxrf(&ts_tx_config);
  dwt_settxantennadelay(16385);
  init_tx_frame();
  
}

static void hear_beat_task(void * param){
  size_t size = sprintf(output_msges[0], "HeartBeat 2");
  send_USB_msg(output_msges[0], size);
}

uint8_t pointer;
TimerHandle_t usb_handler = NULL;

static void usb_handler_task(void * params){
  while(1){
    if(xQueueReceive(usbMSGQueue, &pointer, 100)){
      usb_queue_size++;
      USB_send_msg(usb_msg_arr[pointer].msg, usb_msg_arr[pointer].size);
      xQueueSend(usbQueue, &pointer, 100);
    }
    vTaskDelay(10);
  }
}

uint8_t arr2[100];



void instance_init(){
  int i;
  init_LEDs();
  instance_config();
  //dwt_setrxaftertxdelay(0);
  dwt_setcallbacks(&tx_handle, &rx_handle, &rx_to, &err_handle_cb, NULL, NULL);
  //ali_setcallbacks(&cca_failed_cb);
  //xTaskCreate(test_task, "REG", configMINIMAL_STACK_SIZE + 200, NULL, 2, &task_handle);
  dwt_setinterrupt(SYS_ENABLE_LO_TXFRS_ENABLE_BIT_MASK |
                   SYS_ENABLE_LO_ARFE_ENABLE_BIT_MASK  |
                   SYS_ENABLE_LO_RXFTO_ENABLE_BIT_MASK |
                   SYS_ENABLE_LO_RXPTO_ENABLE_BIT_MASK |
                   SYS_ENABLE_LO_RXPHE_ENABLE_BIT_MASK |
                   //SYS_ENABLE_LO_RXPRD_ENABLE_BIT_MASK |
                   //SYS_ENABLE_LO_RXPHD_ENABLE_BIT_MASK |
                   //SYS_ENABLE_LO_TXFRB_ENABLE_BIT_MASK |
                   //SYS_ENABLE_LO_TXPRS_ENABLE_BIT_MASK |
                   //SYS_ENABLE_LO_TXPHS_ENABLE_BIT_MASK |
                   SYS_ENABLE_LO_RXFCE_ENABLE_BIT_MASK |
                   SYS_ENABLE_LO_RXFSL_ENABLE_BIT_MASK |
                   SYS_ENABLE_LO_RXSTO_ENABLE_BIT_MASK |
                   SYS_ENABLE_LO_RXFCG_ENABLE_BIT_MASK,
                   0,
                   //SYS_STATUS_HI_CCA_FAIL_BIT_MASK,
                   DWT_ENABLE_INT_ONLY);

  /* Clearing the SPI ready interrupt */
  dwt_write32bitreg(SYS_STATUS_ID, SYS_STATUS_RCINIT_BIT_MASK | SYS_STATUS_SPIRDY_BIT_MASK);
  port_set_dwic_isr(dwt_isr);



  txQueue = xQueueCreate(1000, sizeof(uint8_t));
  dataQueue = xQueueCreate(1000, sizeof(uint8_t));
  usbQueue = xQueueCreate(500, sizeof(uint8_t));
  usbMSGQueue = xQueueCreate(500, sizeof(uint8_t));
  
  for (i = 0; i < 10; i++){
    arr2[i] = i;
    xQueueSend(usbQueue, &arr2[i], 100);
  }
  //usb_handler =  xTimerCreate( "gholi", 10, pdTRUE, NULL, usb_handler_task);
  //xTimerStart(usb_handler, 0);


  //hb_handler =  xTimerCreate( "LED1", 2003, pdTRUE, NULL, hear_beat_task);
  //xTimerStart(hb_handler, 0);
  ts_conf_handler =  xTimerCreate( "LED1", WAIT - 1000, pdFALSE, NULL, switch_conf_ts);
  xTaskCreate(usb_handler_task, "LED1", configMINIMAL_STACK_SIZE + 20, NULL, 1, &tx_handler);


  if (instance_info.role == ROLE_TS_MASTER)
    xTaskCreate(task_TS_master, "LED0", configMINIMAL_STACK_SIZE + 100, NULL, 1, &tx_handler);
  
  if (instance_info.role == ROLE_TS)
    xTaskCreate(task_TS, "LED0", configMINIMAL_STACK_SIZE + 100, NULL, 1, &tx_handler);

  if (instance_info.role == ROLE_TX)
    xTaskCreate(task_node, "LED0", configMINIMAL_STACK_SIZE + 100, NULL, 1, &tx_handler);

  
  if (instance_info.role == ROLE_NONE)
    xTaskCreate(task_None, "LED0", configMINIMAL_STACK_SIZE + 100, NULL, 1, &tx_handler);
}




