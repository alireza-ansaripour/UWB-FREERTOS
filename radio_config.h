#include "deca_device_api.h"
#include "deca_regs.h"



dwt_txconfig_t tx_pwr_array[]={
  //{
  //    0x34,           /* PG delay. */
  //    0x03030303,                           /* TX power. */
  //    0x0             /*PG count*/
  //},
  //{
  //    0x34,           /* PG delay. */
  //    0x0f0f0f0f,      /* TX power. */
  //    0x0             /*PG count*/
  //},
  //{
  //    0x34,           /* PG delay. */
  //    0x1c1c1c1c,      /* TX power. */
  //    0x0             /*PG count*/
  //},
  //{
  //    0x34,           /* PG delay. */
  //    0x1f1f1f1f,      /*  */
  //    0x0             /*PG count*/
  //},
  //{
  //    0x34,           /* PG delay. */
  //    0x43434343,                    /* TX power. */
  //    0x0             /*PG count*/
  //},
  {
      0x34,           /* PG delay. */
      0xfdfdfdfd,      /* TX power. */
      0x0             /*PG count*/
  },
};

dwt_config_t ts_config = {
    5,               /* Channel number. */
    DWT_PLEN_2048,    /* Preamble length. Used in TX only. */
    DWT_PAC32,        /* Preamble acquisition chunk size. Used in RX only. */
    9,                      /* TX preamble code. Used in TX only. */
    9,                      /* RX preamble code. Used in RX only. */
    0,               /* 0 to use standard 8 symbol SFD, 1 to use non-standard 8 symbol, 2 for non-standard 16 symbol SFD and 3 for 4z 8 symbol SDF type */
    DWT_BR_850K,      /* Data rate. */
    DWT_PHRMODE_STD, /* PHY header mode. */
    DWT_PHRRATE_STD, /* PHY header rate. */
    (8000 + 8 - 8),   /* SFD timeout (preamble length + 1 + SFD length - PAC size). Used in RX only. */
    DWT_STS_MODE_OFF,
    DWT_STS_LEN_64,  /* STS length, see allowed values in Enum dwt_sts_lengths_e */
    DWT_PDOA_M0      /* PDOA mode off */
};

dwt_txconfig_t ts_tx_config =
{
    0x34,           /* PG delay. */
    0xfdfdfdfd,      /* TX power. */
    0x0             /*PG count*/
};


dwt_config_t conf_array[] = {  
  {
      5,               /* Channel number. */
      DWT_PLEN_64,    /* Preamble length. Used in TX only. */
      DWT_PAC4,        /* Preamble acquisition chunk size. Used in RX only. */
      3,                                                         /* TX preamble code. Used in TX only. */
      3,                                                         /* RX preamble code. Used in RX only. */
      0,               /* 0 to use standard 8 symbol SFD, 1 to use non-standard 8 symbol, 2 for non-standard 16 symbol SFD and 3 for 4z 8 symbol SDF type */
      DWT_BR_6M8,      /* Data rate. */
      DWT_PHRMODE_STD, /* PHY header mode. */
      DWT_PHRRATE_DTA,
      (8000 + 8 - 32),   /* SFD timeout (preamble length + 1 + SFD length - PAC size). Used in RX only. */
      DWT_STS_MODE_OFF, /* STS disabled */
      //DWT_STS_MODE_SDC | DWT_STS_MODE_1, /* STS disabled */
      DWT_STS_LEN_1024,/* STS length see allowed values in Enum dwt_sts_lengths_e */
      DWT_PDOA_M0      /* PDOA mode off */
  },
  {
      9,               /* Channel number. */
      DWT_PLEN_64,    /* Preamble length. Used in TX only. */
      DWT_PAC4,        /* Preamble acquisition chunk size. Used in RX only. */
      3,                                                         /* TX preamble code. Used in TX only. */
      3,                                                         /* RX preamble code. Used in RX only. */
      0,               /* 0 to use standard 8 symbol SFD, 1 to use non-standard 8 symbol, 2 for non-standard 16 symbol SFD and 3 for 4z 8 symbol SDF type */
      DWT_BR_6M8,      /* Data rate. */
      DWT_PHRMODE_STD, /* PHY header mode. */
      DWT_PHRRATE_DTA,
      (8000 + 8 - 32),   /* SFD timeout (preamble length + 1 + SFD length - PAC size). Used in RX only. */
      DWT_STS_MODE_OFF, /* STS disabled */
      //DWT_STS_MODE_SDC | DWT_STS_MODE_1, /* STS disabled */
      DWT_STS_LEN_1024,/* STS length see allowed values in Enum dwt_sts_lengths_e */
      DWT_PDOA_M0      /* PDOA mode off */
  },
};




//dwt_config_t conf_array[] = {  
//  {
//      5,               /* Channel number. */
//      DWT_PLEN_64,    /* Preamble length. Used in TX only. */
//      DWT_PAC4,        /* Preamble acquisition chunk size. Used in RX only. */
//      3,                                                         /* TX preamble code. Used in TX only. */
//      3,                                                         /* RX preamble code. Used in RX only. */
//      0,               /* 0 to use standard 8 symbol SFD, 1 to use non-standard 8 symbol, 2 for non-standard 16 symbol SFD and 3 for 4z 8 symbol SDF type */
//      DWT_BR_6M8,      /* Data rate. */
//      DWT_PHRMODE_STD, /* PHY header mode. */
//      DWT_PHRRATE_DTA,
//      (8000 + 8 - 32),   /* SFD timeout (preamble length + 1 + SFD length - PAC size). Used in RX only. */
//      DWT_STS_MODE_OFF, /* STS disabled */
//      //DWT_STS_MODE_SDC | DWT_STS_MODE_1, /* STS disabled */
//      DWT_STS_LEN_1024,/* STS length see allowed values in Enum dwt_sts_lengths_e */
//      DWT_PDOA_M0      /* PDOA mode off */
//  },
//  {
//      5,               /* Channel number. */
//      DWT_PLEN_64,    /* Preamble length. Used in TX only. */
//      DWT_PAC4,        /* Preamble acquisition chunk size. Used in RX only. */
//      9,                                                         /* TX preamble code. Used in TX only. */
//      9,                                                         /* RX preamble code. Used in RX only. */
//      0,               /* 0 to use standard 8 symbol SFD, 1 to use non-standard 8 symbol, 2 for non-standard 16 symbol SFD and 3 for 4z 8 symbol SDF type */
//      DWT_BR_6M8,      /* Data rate. */
//      DWT_PHRMODE_STD, /* PHY header mode. */
//      DWT_PHRRATE_DTA,
//      (8000 + 8 - 32),   /* SFD timeout (preamble length + 1 + SFD length - PAC size). Used in RX only. */
//      DWT_STS_MODE_OFF, /* STS disabled */
//      //DWT_STS_MODE_SDC | DWT_STS_MODE_1, /* STS disabled */
//      DWT_STS_LEN_1024,/* STS length see allowed values in Enum dwt_sts_lengths_e */
//      DWT_PDOA_M0      /* PDOA mode off */
//  },
//  {
//      5,               /* Channel number. */
//      DWT_PLEN_256,    /* Preamble length. Used in TX only. */
//      DWT_PAC32,        /* Preamble acquisition chunk size. Used in RX only. */
//      9,                                                         /* TX preamble code. Used in TX only. */
//      9,                                                         /* RX preamble code. Used in RX only. */
//      0,               /* 0 to use standard 8 symbol SFD, 1 to use non-standard 8 symbol, 2 for non-standard 16 symbol SFD and 3 for 4z 8 symbol SDF type */
//      DWT_BR_6M8,      /* Data rate. */
//      DWT_PHRMODE_STD, /* PHY header mode. */
//      DWT_PHRRATE_DTA,
//      (8000 + 8 - 32),   /* SFD timeout (preamble length + 1 + SFD length - PAC size). Used in RX only. */
//      DWT_STS_MODE_OFF, /* STS disabled */
//      //DWT_STS_MODE_SDC | DWT_STS_MODE_1, /* STS disabled */
//      DWT_STS_LEN_1024,/* STS length see allowed values in Enum dwt_sts_lengths_e */
//      DWT_PDOA_M0      /* PDOA mode off */
//  },
//  {
//      5,               /* Channel number. */
//      DWT_PLEN_256,    /* Preamble length. Used in TX only. */
//      DWT_PAC32,        /* Preamble acquisition chunk size. Used in RX only. */
//      9,                                                         /* TX preamble code. Used in TX only. */
//      9,                                                         /* RX preamble code. Used in RX only. */
//      0,               /* 0 to use standard 8 symbol SFD, 1 to use non-standard 8 symbol, 2 for non-standard 16 symbol SFD and 3 for 4z 8 symbol SDF type */
//      DWT_BR_850K,      /* Data rate. */
//      DWT_PHRMODE_STD, /* PHY header mode. */
//      DWT_PHRRATE_STD,
//      (8000 + 8 - 32),   /* SFD timeout (preamble length + 1 + SFD length - PAC size). Used in RX only. */
//      DWT_STS_MODE_OFF, /* STS disabled */
//      //DWT_STS_MODE_SDC | DWT_STS_MODE_1, /* STS disabled */
//      DWT_STS_LEN_1024,/* STS length see allowed values in Enum dwt_sts_lengths_e */
//      DWT_PDOA_M0      /* PDOA mode off */
//  },
//  {
//      5,               /* Channel number. */
//      DWT_PLEN_512,    /* Preamble length. Used in TX only. */
//      DWT_PAC32,        /* Preamble acquisition chunk size. Used in RX only. */
//      9,                                                         /* TX preamble code. Used in TX only. */
//      9,                                                         /* RX preamble code. Used in RX only. */
//      0,               /* 0 to use standard 8 symbol SFD, 1 to use non-standard 8 symbol, 2 for non-standard 16 symbol SFD and 3 for 4z 8 symbol SDF type */
//      DWT_BR_850K,      /* Data rate. */
//      DWT_PHRMODE_STD, /* PHY header mode. */
//      DWT_PHRRATE_STD,
//      (8000 + 8 - 32),   /* SFD timeout (preamble length + 1 + SFD length - PAC size). Used in RX only. */
//      DWT_STS_MODE_OFF, /* STS disabled */
//      //DWT_STS_MODE_SDC | DWT_STS_MODE_1, /* STS disabled */
//      DWT_STS_LEN_1024,/* STS length see allowed values in Enum dwt_sts_lengths_e */
//      DWT_PDOA_M0      /* PDOA mode off */
//  },
  
//  {
//      5,               /* Channel number. */
//      DWT_PLEN_1024,    /* Preamble length. Used in TX only. */
//      DWT_PAC32,        /* Preamble acquisition chunk size. Used in RX only. */
//      9,                                                         /* TX preamble code. Used in TX only. */
//      9,                                                         /* RX preamble code. Used in RX only. */
//      0,               /* 0 to use standard 8 symbol SFD, 1 to use non-standard 8 symbol, 2 for non-standard 16 symbol SFD and 3 for 4z 8 symbol SDF type */
//      DWT_BR_850K,      /* Data rate. */
//      DWT_PHRMODE_STD, /* PHY header mode. */
//      DWT_PHRRATE_STD,
//      (8000 + 8 - 32),   /* SFD timeout (preamble length + 1 + SFD length - PAC size). Used in RX only. */
//      DWT_STS_MODE_OFF, /* STS disabled */
//      //DWT_STS_MODE_SDC | DWT_STS_MODE_1, /* STS disabled */
//      DWT_STS_LEN_1024,/* STS length see allowed values in Enum dwt_sts_lengths_e */
//      DWT_PDOA_M0      /* PDOA mode off */
//  },
  
//  {
//      5,               /* Channel number. */
//      DWT_PLEN_2048,    /* Preamble length. Used in TX only. */
//      DWT_PAC32,        /* Preamble acquisition chunk size. Used in RX only. */
//      9,                                                         /* TX preamble code. Used in TX only. */
//      9,                                                         /* RX preamble code. Used in RX only. */
//      0,               /* 0 to use standard 8 symbol SFD, 1 to use non-standard 8 symbol, 2 for non-standard 16 symbol SFD and 3 for 4z 8 symbol SDF type */
//      DWT_BR_850K,      /* Data rate. */
//      DWT_PHRMODE_STD, /* PHY header mode. */
//      DWT_PHRRATE_STD,
//      (8000 + 8 - 32),   /* SFD timeout (preamble length + 1 + SFD length - PAC size). Used in RX only. */
//      DWT_STS_MODE_OFF, /* STS disabled */
//      //DWT_STS_MODE_SDC | DWT_STS_MODE_1, /* STS disabled */
//      DWT_STS_LEN_1024,/* STS length see allowed values in Enum dwt_sts_lengths_e */
//      DWT_PDOA_M0      /* PDOA mode off */
//  }
//};

dwt_txconfig_t node_tx_config = {
    0x34,           /* PG delay. */
    0x0,      /* TX power. */
    0x0             /*PG count*/
};

