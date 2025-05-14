/*
 * can_bus.c
 *
 *  Created on: Apr 30, 2025
 *      Author: rolf
 */


#include "can_bus.h"

void CAN_Init(void)
{
    // Start the CAN controller
    HAL_FDCAN_Start(&hfdcan1);

    // Activate notification for RX FIFO 0 new message
    HAL_FDCAN_ActivateNotification(&hfdcan1, FDCAN_IT_RX_FIFO0_NEW_MESSAGE, 0);
}

HAL_StatusTypeDef CAN_Send(uint32_t id, uint8_t *data, uint8_t len)
{
    FDCAN_TxHeaderTypeDef txHeader;
    txHeader.Identifier = id;
    txHeader.IdType = FDCAN_STANDARD_ID;
    txHeader.TxFrameType = FDCAN_DATA_FRAME;
    txHeader.DataLength = (len << 16); // convert to DLC format
    txHeader.ErrorStateIndicator = FDCAN_ESI_ACTIVE;
    txHeader.BitRateSwitch = FDCAN_BRS_OFF;
    txHeader.FDFormat = FDCAN_CLASSIC_CAN;
    txHeader.TxEventFifoControl = FDCAN_NO_TX_EVENTS;
    txHeader.MessageMarker = 0;

    return HAL_FDCAN_AddMessageToTxFifoQ(&hfdcan1, &txHeader, data);
}

HAL_StatusTypeDef CAN_Receive(uint32_t *id, uint8_t *data, uint8_t *len)
{
    FDCAN_RxHeaderTypeDef rxHeader;
    HAL_StatusTypeDef status = HAL_FDCAN_GetRxMessage(&hfdcan1, FDCAN_RX_FIFO0, &rxHeader, data);

    if (status == HAL_OK) {
        *id = rxHeader.Identifier;
        *len = (rxHeader.DataLength >> 16) & 0xF;
    }

    return status;
}
