#ifndef VWTP_H
#define VWTP_H

typedef enum
{
  VWTP_CAN_TX_ERROR,
  VWTP_CAN_RX_TIMEOUT,
  VWTP_ERROR,
  VWTP_FRAME_ERROR,
  VWTP_MSG_TOO_LONG,
  VWTP_TX_MSG_TOO_LONG,
  VWTP_OK = 255
} VWTP_Result_t;

VWTP_Result_t VWTP_Connect();
VWTP_Result_t VWTP_Disconnect();
VWTP_Result_t VWTP_ACK();
VWTP_Result_t VWTP_KWP2000Message(u8 SID, u8 parameter, u8 * kwpMessage);


#endif //VWTP_H
