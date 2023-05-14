typedef struct __attribute__((packed)){
  size_t size;
  uint8_t msg[200];
} usb_msg_t;

void init_USB(void);
void USB_send_msg(uint8_t * data, size_t size);
