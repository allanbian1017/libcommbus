#ifndef USB_H
#define USB_H

#ifdef __cplusplus
extern "C" {
#endif

extern int usb_open(uint16_t vid, uint16_t pid, int iface);
extern int usb_ctrl(unsigned char req_type, unsigned char req, uint32_t val,
                        uint32_t idx, unsigned char *data, uint16_t len);
extern int usb_bulk(unsigned char ep, unsigned char *data, int len);
extern int usb_close(int iface);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif
