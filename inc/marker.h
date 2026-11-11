#ifndef _MARKER_H
#define _MARKER_H

// The markers use the SWD header, so a debug probe must stay disconnected
#define MARKER_Port GPIOA
#define MARKER_RX_Pin GPIO_PIN_14 // SWCLK
#define MARKER_TX_Pin GPIO_PIN_13 // SWDIO

void marker_init(void);
void marker_set_filter(uint32_t ide, uint32_t id, uint32_t mask);
void marker_frame_received(uint32_t ide, uint32_t id);
void marker_frame_sent(uint32_t ide, uint32_t id);

#endif // _MARKER_H
