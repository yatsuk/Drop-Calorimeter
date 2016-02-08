#ifndef CRC_H_
#define CRC_H_


/*===============================================================================================================*/
extern unsigned short eval_crc16     (unsigned short crc, const unsigned char *msg, unsigned msg_len);
extern unsigned short eval_crc16tbl  (unsigned short crc, const unsigned char *msg, unsigned msg_len);
extern unsigned char  eval_crc8      (unsigned char crc, const unsigned char *msg, unsigned msg_len);
extern unsigned short eval_modbuscrc (const unsigned char *msg, unsigned msg_len);
#ifdef REV_CRC
extern unsigned char  eval_revcrc8   (unsigned char crc, const unsigned char *msg, unsigned msg_len);
#endif
/*===============================================================================================================*/

#endif
