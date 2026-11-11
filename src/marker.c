//
// Frame markers for a logic analyzer on the SWD header pins
//

#include "stm32f0xx_hal.h"
#include "marker.h"


typedef struct marker_
{
    uint16_t pin;
    uint8_t enabled;
    uint32_t ide;
    uint32_t id;
    uint32_t mask; // Zero matches every frame
} marker_t;


// Private variables
static marker_t markers[MARKER_COUNT] =
{
    { .pin = MARKER_1_Pin, .enabled = 1 },
    { .pin = MARKER_2_Pin, .enabled = 0 },
};


// Take over the SWD pins as outputs
void marker_init(void)
{
    __HAL_RCC_GPIOA_CLK_ENABLE();
    GPIO_InitTypeDef GPIO_InitStruct;
    GPIO_InitStruct.Pin = MARKER_1_Pin | MARKER_2_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    GPIO_InitStruct.Alternate = 0;
    HAL_GPIO_Init(MARKER_Port, &GPIO_InitStruct);
}


// Let a marker mark frames of this ID type whose masked ID matches. A zero mask marks every frame.
void marker_set_filter(uint8_t marker, uint32_t ide, uint32_t id, uint32_t mask)
{
    if (marker >= MARKER_COUNT)
    {
        return;
    }

    uint32_t primask = __get_PRIMASK();
    __disable_irq();
    markers[marker].ide = ide;
    markers[marker].id = id;
    markers[marker].mask = mask;
    markers[marker].enabled = 1;
    __set_PRIMASK(primask);
}


// Stop a marker from marking frames
void marker_disable(uint8_t marker)
{
    if (marker >= MARKER_COUNT)
    {
        return;
    }

    markers[marker].enabled = 0;
}


static uint8_t matches(const marker_t *marker, uint32_t ide, uint32_t id)
{
    if (!marker->enabled)
    {
        return 0;
    }
    if (marker->mask == 0)
    {
        return 1;
    }
    return (ide == marker->ide) && (((id ^ marker->id) & marker->mask) == 0);
}


// Toggle the pin of every marker that matches a frame completed on the bus
void marker_frame_done(uint32_t ide, uint32_t id)
{
    for (uint8_t i = 0; i < MARKER_COUNT; i++)
    {
        if (matches(&markers[i], ide, id))
        {
            HAL_GPIO_TogglePin(MARKER_Port, markers[i].pin);
        }
    }
}
