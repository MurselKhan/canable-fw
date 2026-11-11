//
// can: initializes and provides methods to interact with the CAN peripheral
//

#include "stm32f0xx_hal.h"
#include <string.h>
#include "slcan.h"
#include "usbd_cdc_if.h"
#include "can.h"
#include "led.h"
#include "error.h"
#include "marker.h"


// Private variables
static CAN_HandleTypeDef can_handle;
static CAN_FilterTypeDef filter;
static uint32_t prescaler;
static can_bus_state_t bus_state = OFF_BUS;
static uint8_t can_autoretransmit = ENABLE;
static can_txbuf_t txqueue = {0};
static can_rxbuf_t rxqueue = {0};


// Initialize CAN peripheral settings, but don't actually start the peripheral
void can_init(void)
{
    // Initialize GPIO for CAN transceiver 
    GPIO_InitTypeDef GPIO_InitStruct;
    __HAL_RCC_CAN1_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();

    //PB8     ------> CAN_RX
    //PB9     ------> CAN_TX
    GPIO_InitStruct.Pin = GPIO_PIN_8|GPIO_PIN_9;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF4_CAN;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);


    // Initialize default CAN filter configuration
    filter.FilterIdHigh = 0;
    filter.FilterIdLow = 0;
    filter.FilterMaskIdHigh = 0;
    filter.FilterMaskIdLow = 0;
    filter.FilterFIFOAssignment = CAN_RX_FIFO0;
    filter.FilterBank = 0;
    filter.FilterMode = CAN_FILTERMODE_IDMASK;
    filter.FilterScale = CAN_FILTERSCALE_32BIT;
    filter.FilterActivation = ENABLE;


    // default to 125 kbit/s
    prescaler = 48;
    can_handle.Instance = CAN;
    bus_state = OFF_BUS;

    HAL_NVIC_SetPriority(CEC_CAN_IRQn, 1, 0);
    HAL_NVIC_EnableIRQ(CEC_CAN_IRQn);

}


// Start the CAN peripheral
void can_enable(void)
{
    if (bus_state == OFF_BUS)
    {
    	can_handle.Init.Prescaler = prescaler;
    	can_handle.Init.Mode = CAN_MODE_NORMAL;

    	can_handle.Init.SyncJumpWidth = CAN_SJW_1TQ;
    	can_handle.Init.TimeSeg1 = CAN_BS1_4TQ;
    	can_handle.Init.TimeSeg2 = CAN_BS2_3TQ;
    	can_handle.Init.TimeTriggeredMode = DISABLE;
    	can_handle.Init.AutoBusOff = ENABLE;
    	can_handle.Init.AutoWakeUp = DISABLE;
    	can_handle.Init.AutoRetransmission = can_autoretransmit;
    	can_handle.Init.ReceiveFifoLocked = DISABLE;
    	can_handle.Init.TransmitFifoPriority = ENABLE;
        HAL_CAN_Init(&can_handle);

        HAL_CAN_ConfigFilter(&can_handle, &filter);

        rxqueue.head = 0;
        rxqueue.tail = 0;

        HAL_CAN_Start(&can_handle);
        HAL_CAN_ActivateNotification(&can_handle, CAN_IT_RX_FIFO0_MSG_PENDING | CAN_IT_TX_MAILBOX_EMPTY);
        bus_state = ON_BUS;

        led_blue_on();
    }
}


// Disable the CAN peripheral and go off-bus
void can_disable(void)
{
    if (bus_state == ON_BUS)
    {
        // Do a bxCAN reset (set RESET bit to 1)
    	can_handle.Instance->MCR |= CAN_MCR_RESET;
        bus_state = OFF_BUS;

        led_green_on();
    }
}


// Set the bitrate of the CAN peripheral
void can_set_bitrate(enum can_bitrate bitrate)
{
    if (bus_state == ON_BUS)
    {
        // cannot set bitrate while on bus
        return;
    }

    switch (bitrate)
    {
        case CAN_BITRATE_10K:
        	prescaler = 600;
            break;
        case CAN_BITRATE_20K:
        	prescaler = 300;
            break;
        case CAN_BITRATE_50K:
        	prescaler = 120;
            break;
        case CAN_BITRATE_100K:
            prescaler = 60;
            break;
        case CAN_BITRATE_125K:
            prescaler = 48;
            break;
        case CAN_BITRATE_250K:
            prescaler = 24;
            break;
        case CAN_BITRATE_500K:
            prescaler = 12;
            break;
        case CAN_BITRATE_750K:
            prescaler = 8;
            break;
        case CAN_BITRATE_1000K:
            prescaler = 6;
            break;
        case CAN_BITRATE_INVALID:
        default:
            prescaler = 6;
            break;
    }

    led_green_on();
}


// Set CAN peripheral to silent mode
void can_set_silent(uint8_t silent)
{
    if (bus_state == ON_BUS)
    {
        // cannot set silent mode while on bus
        return;
    }
    if (silent)
    {
    	can_handle.Init.Mode = CAN_MODE_SILENT;
    } else {
    	can_handle.Init.Mode = CAN_MODE_NORMAL;
    }

    led_green_on();
}


// Enable/disable auto-retransmission
void can_set_autoretransmit(uint8_t autoretransmit)
{
    if (bus_state == ON_BUS)
    {
        // Cannot set autoretransmission while on bus
        return;
    }
    if (autoretransmit)
    {
    	can_autoretransmit = ENABLE;
    } else {
    	can_autoretransmit = DISABLE;
    }

    led_green_on();
}


// Send a message on the CAN bus
uint32_t can_tx(CAN_TxHeaderTypeDef *tx_msg_header, uint8_t* tx_msg_data)
{
	// Check if space available in the buffer (FIXME: wastes 1 item)
	if( ((txqueue.head + 1) % TXQUEUE_LEN) == txqueue.tail)
	{
		error_assert(ERR_FULLBUF_CANTX);
		return HAL_ERROR;
	}

	// Copy header struct into array
	txqueue.header[txqueue.head] = *tx_msg_header;

	// Copy data into array
	for(uint8_t i=0; i<tx_msg_header->DLC; i++)
	{
		txqueue.data[txqueue.head][i] = tx_msg_data[i];
	}

	// Increment the head pointer
	txqueue.head = (txqueue.head + 1) % TXQUEUE_LEN;

	return HAL_OK;
}


// Process messages in the TX output queue
void can_process(void)
{
    if((txqueue.tail != txqueue.head) && (HAL_CAN_GetTxMailboxesFreeLevel(&can_handle) > 0))
	{
		// Transmit can frame
		uint32_t mailbox_txed = 0;
		uint32_t status = HAL_CAN_AddTxMessage(&can_handle, &txqueue.header[txqueue.tail], txqueue.data[txqueue.tail], &mailbox_txed);
		txqueue.tail = (txqueue.tail + 1) % TXQUEUE_LEN;

		led_green_on();

		// This drops the packet if it fails (no retry). Failure is unlikely
		// since we check if there is a TX mailbox free.
		if(status != HAL_OK)
		{
			error_assert(ERR_CAN_TXFAIL);
		}
	}
}


// Take the oldest received message out of the RX queue
uint32_t can_rx(CAN_RxHeaderTypeDef *rx_msg_header, uint8_t* rx_msg_data)
{
	if (rxqueue.tail == rxqueue.head)
	{
		return HAL_ERROR;
	}

	*rx_msg_header = rxqueue.header[rxqueue.tail];
	memcpy(rx_msg_data, rxqueue.data[rxqueue.tail], RXQUEUE_DATALEN);
	rxqueue.tail = (rxqueue.tail + 1) % RXQUEUE_LEN;

	led_blue_on();
	return HAL_OK;
}


// Check if a CAN message has been received and is waiting in the RX queue
uint8_t is_can_msg_pending(uint8_t fifo)
{
    if (bus_state == OFF_BUS)
    {
        return 0;
    }
    return(rxqueue.tail != rxqueue.head);
}


// Return reference to CAN handle
CAN_HandleTypeDef* can_gethandle(void)
{
	return &can_handle;
}


// Callback for FIFO0 full
void HAL_CAN_RxFifo0FullCallback(CAN_HandleTypeDef *hcan)
{
	error_assert(ERR_CANRXFIFO_OVERFLOW);
}


// Callback for a message in FIFO0. Moves it into the RX queue and marks it.
void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan)
{
	CAN_RxHeaderTypeDef *header = &rxqueue.header[rxqueue.head];

	if (HAL_CAN_GetRxMessage(hcan, CAN_RX_FIFO0, header, rxqueue.data[rxqueue.head]) != HAL_OK)
	{
		return;
	}

	marker_frame_received(header->IDE, (header->IDE == CAN_ID_EXT) ? header->ExtId : header->StdId);

	// The head slot stays free, so a full queue drops the newest message
	uint8_t next = (rxqueue.head + 1) % RXQUEUE_LEN;
	if (next == rxqueue.tail)
	{
		error_assert(ERR_CANRXFIFO_OVERFLOW);
		return;
	}
	rxqueue.head = next;
}


// Mark a message that left a TX mailbox and was acknowledged on the bus
static void can_tx_complete(CAN_HandleTypeDef *hcan, uint8_t mailbox)
{
	uint32_t tir = hcan->Instance->sTxMailBox[mailbox].TIR;

	if (tir & CAN_TI0R_IDE)
	{
		marker_frame_sent(CAN_ID_EXT, tir >> CAN_TI0R_EXID_Pos);
	}
	else
	{
		marker_frame_sent(CAN_ID_STD, tir >> CAN_TI0R_STID_Pos);
	}
}


// Callbacks for a completed transmission
void HAL_CAN_TxMailbox0CompleteCallback(CAN_HandleTypeDef *hcan)
{
	can_tx_complete(hcan, 0);
}

void HAL_CAN_TxMailbox1CompleteCallback(CAN_HandleTypeDef *hcan)
{
	can_tx_complete(hcan, 1);
}

void HAL_CAN_TxMailbox2CompleteCallback(CAN_HandleTypeDef *hcan)
{
	can_tx_complete(hcan, 2);
}

