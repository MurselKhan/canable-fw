#ifndef _MARKER_H
#define _MARKER_H

// The markers use the SWD header, so a debug probe must stay disconnected
#define MARKER_Port GPIOA
#define MARKER_1_Pin GPIO_PIN_14 // SWCLK
#define MARKER_2_Pin GPIO_PIN_13 // SWDIO

#define MARKER_COUNT 2

void marker_init(void);
void marker_set_filter(uint8_t marker, uint32_t ide, uint32_t id, uint32_t mask);
void marker_disable(uint8_t marker);
void marker_frame_done(uint32_t ide, uint32_t id);

#endif // _MARKER_H
