//
// Frame markers for a logic analyzer on the SWD header pins
//

#include "stm32f0xx_hal.h"
#include "marker.h"


// Private variables
static uint32_t filter_ide = CAN_ID_STD;
static uint32_t filter_id = 0;
static uint32_t filter_mask = 0;


// Take over the SWD pins as outputs
void marker_init(void)
{
    __HAL_RCC_GPIOA_CLK_ENABLE();
    GPIO_InitTypeDef GPIO_InitStruct;
    GPIO_InitStruct.Pin = MARKER_RX_Pin | MARKER_TX_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    GPIO_InitStruct.Alternate = 0;
    HAL_GPIO_Init(MARKER_Port, &GPIO_InitStruct);
}


// Mark only frames of this ID type whose masked ID matches. A zero mask marks every frame.
void marker_set_filter(uint32_t ide, uint32_t id, uint32_t mask)
{
    uint32_t primask = __get_PRIMASK();
    __disable_irq();
    filter_ide = ide;
    filter_id = id;
    filter_mask = mask;
    __set_PRIMASK(primask);
}


static uint8_t is_marked(uint32_t ide, uint32_t id)
{
    if (filter_mask == 0)
    {
        return 1;
    }
    return (ide == filter_ide) && (((id ^ filter_id) & filter_mask) == 0);
}


// Toggle the RX marker for a frame received from the bus
void marker_frame_received(uint32_t ide, uint32_t id)
{
    if (is_marked(ide, id))
    {
        HAL_GPIO_TogglePin(MARKER_Port, MARKER_RX_Pin);
    }
}


// Toggle the TX marker for a frame the bus acknowledged
void marker_frame_sent(uint32_t ide, uint32_t id)
{
    if (is_marked(ide, id))
    {
        HAL_GPIO_TogglePin(MARKER_Port, MARKER_TX_Pin);
    }
}
