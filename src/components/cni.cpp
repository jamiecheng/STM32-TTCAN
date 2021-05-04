//
// Created by Jamie on 1-5-2021.
//

#include <cassert>
#include "cni.h"
#include "stm32f0xx.h"

using namespace cni;

CommNetworkInterface CommNetworkInterface::m_inst;

CommNetworkInterface::CommNetworkInterface() = default;

CommNetworkInterface *CommNetworkInterface::get_instance()
{
    return &m_inst;
}

void CommNetworkInterface::set_master(bool master)
{
    m_is_master = master;
}

void CommNetworkInterface::set_schedule_matrix(ScheduleMatrix *matrix)
{
    m_matrix = matrix;
}
void CommNetworkInterface::attach_rx_cb(CommNetworkInterface::rx_cb cb)
{
    m_rx_cb = cb;
}

void CommNetworkInterface::schedule_tx_trigger(schedule_tx_t *tx_trigger)
{
    // only masters are allowed to send at column 0
    assert(tx_trigger->column != 0);

    for (int i = 0; i < sizeof(m_scheduled_tx); i++)
    {
        if (m_scheduled_tx[i] == nullptr)
        {
            m_scheduled_tx[i] = tx_trigger;
            break;
        }
    }
}

void CommNetworkInterface::start()
{
    _config_can();
    _config_timer();

    TIM2->CR1 |= TIM_CR1_CEN;
}

void CommNetworkInterface::tim_isr()
{
    // every timeout makes the matrix shift to the next column
    if ((TIM2->SR & TIM_SR_CC1IF) != 0)
    {
        TIM2->SR &= ~TIM_SR_CC1IF;

        auto res = m_matrix->step_column();

        if (res == 1)
        {
            // end of basic cycle
            TIM2->DIER &= ~TIM_DIER_CC1IE;
        } else
        {
            // schedule end of column
            TIM2->CCR1 += m_matrix->get_column_duration();
        }

        // check if the current node is scheduled for transmission
        for (auto tx : m_scheduled_tx)
        {
            if (tx->column == m_matrix->get_column() && tx->basic_cycle == m_matrix->get_basic_cycle())
            {
                CAN->MCR |= CAN_MCR_NART;

                _transmit_message(tx->id, tx->data);

                break;
            }
        }
    }

    // only the master has the functionality to send the reference message
    if ((TIM2->SR & TIM_SR_UIF) != 0)
    {
        TIM2->SR &= ~TIM_SR_UIF;

        if (m_is_master)
        {
            m_matrix->step_basix_cycle();

            // Reference window is an exclusive window
            // Enable non automatic retransmission mode
            CAN->MCR |= CAN_MCR_NART;

            // Send reference message
            uint16_t data[8] = {0};
            data[0] = m_matrix->get_basic_cycle();
            _transmit_message(MSG_ID_REF, (uint64_t *) data);

            // Schedule the timeout for the first transmission column
            // and enable interrupt for transmission column timeout
            TIM2->CNT = 0;
            TIM2->CCR1 = m_matrix->get_column_duration(0) - 1;
            TIM2->DIER |= TIM_DIER_CC1IE;
        }
    }
}

void CommNetworkInterface::can_isr()
{
    if ((CAN->RF0R & CAN_RF0R_FMP0) != 0)
    {
        uint16_t identifier = CAN->sFIFOMailBox[0].RIR >> 21;

        if(!m_is_master)
        {
            TIM2->CNT = 0;
            TIM2->CCR1 = m_matrix->get_column_duration(0) - 1;
            TIM2->DIER |= TIM_DIER_CC1IE;
        }

        if(m_rx_cb)
            m_rx_cb(static_cast<msg_id_t>(identifier), CAN->sFIFOMailBox[0].RDHR, CAN->sFIFOMailBox[0].RDLR);

        CAN->RF0R |= CAN_RF0R_RFOM0;
    }
}

uint8_t CommNetworkInterface::_transmit_message(const uint32_t id, const uint64_t *data)
{
    // 1. Check mailbox 0 is empty
    if ((CAN->TSR & CAN_TSR_TME0) == CAN_TSR_TME0)
    {
        // 2. Set data length
        // 3. Set data low
        // 4. Set data high
        // 5. Set id and request a transmission
        CAN->sTxMailBox[0].TDTR = 8;
        CAN->sTxMailBox[0].TDLR = (uint32_t) *data;
        CAN->sTxMailBox[0].TDHR = (uint32_t) (*data >> 32);
        CAN->sTxMailBox[0].TIR = (uint32_t) (id << 21 | CAN_TI0R_TXRQ);

        return 1;
    } else
    {
        // 2. Abort transmission if not empty
        CAN->TSR |= CAN_TSR_ABRQ0;

        return 0;
    }
}

void CommNetworkInterface::_config_can()
{
    // Enable the peripheral clock of GPIOB
    RCC->AHBENR |= RCC_AHBENR_GPIOBEN;

    // GPIO configuration for CAN signals
    // 1. Select AF mode (10) on PB8 and PB9
    // 2. AF4 for CAN signals
    GPIOB->MODER = (GPIOB->MODER & ~(GPIO_MODER_MODER8 | GPIO_MODER_MODER9))
        | (GPIO_MODER_MODER8_1 | GPIO_MODER_MODER9_1);
    GPIOB->AFR[1] = (GPIOA->AFR[1] & ~(GPIO_AFRH_AFRH0 | GPIO_AFRH_AFRH1))
        | (4 << (0 * 4)) | (4 << (1 * 4));

    // Enable the CAN peripheral clock
    RCC->APB1ENR |= RCC_APB1ENR_CANEN;

    // Reset the CAN module
    CAN->MCR |= CAN_MCR_INRQ;
    while ((CAN->MSR & CAN_MSR_INAK) != CAN_MSR_INAK)
    {
        // Add time out here for a robust application
    }
    CAN->MCR &= ~CAN_MCR_SLEEP;
    CAN->MCR |= CAN_MCR_RESET;

    // Configure CAN
    // 1. Enter CAN init mode to write the configuration
    // 2. Wait the init mode entering
    CAN->MCR |= CAN_MCR_INRQ;
    while ((CAN->MSR & CAN_MSR_INAK) != CAN_MSR_INAK)
    {
        // Add time out here for a robust application
    }

    /* BaudRate settings calculations
     *
     * t_q = (BRP[9:0] + 1) * t_PCLK
     *     = (    5    + 1) * 1/48 MHz
     *     = 6/48e6
     *
     * t_BS1 =    t_q   * (TS1[2:0] + 1)
     *       = 6/48e6 * (  3      + 1)
     *       = 24/48e6
     *
     * t_BS2 =    t_q   * (TS2[2:0] + 1)
     *       = 6/48e6 * (  2      + 1)
     *       = 18/48e6
     *
     * NominalBitTime = 1 * t_q + t_BS1 + t_BS2
     *                = 1 * 6/48e6 + 24/48e6 + 18/48e6
     *                = 48/48e6
     *                = 1 us
     *
     * BaudRate = 1 / NominalBitTime
     *          = 1 / 1 us
     *          = 1 Mbit/s
     */

    // 3. Exit sleep mode
    // 4. Disable automatic retransmission
    //    A message will be transmitted only once, independently of the
    //    transmission result (successful, error or arbitration lost).
    // 5. Set timing to 1 Mbit/s: BS1 = 2, BS2 = 3, prescaler = 5
    // 6. Leave init mode
    // 7. Wait the init mode leaving
    CAN->MCR &= ~CAN_MCR_SLEEP;
    CAN->MCR |= CAN_MCR_NART;
    CAN->BTR |= (2 << 20) | (3 << 16) | (5 << 0);
    CAN->MCR &= ~CAN_MCR_INRQ;
    while ((CAN->MSR & CAN_MSR_INAK) == CAN_MSR_INAK)
    {
        // Add time out here for a robust application
    }

    // 8. Enter filter init mode, (16-bit + mask, filter 0 for FIFO 0)
    CAN->FMR = CAN_FMR_FINIT;

    /* CAN_FiRx - FB[31:0]: Filter bits
     *
     * Identifier
     *   Each bit of the register specifies the level of the corresponding bit
     *   of the expected identifier.
     *   0: Dominant bit is expected
     *   1: Recessive bit is expected
     *
     * Mask
     *   Each bit of the register specifies whether the bit of the associated
     *   identifier register must match with the corresponding bit of the
     *   expected identifier or not.
     *   0: Do not care, the bit is not used for the comparison
     *   1: Must match, the bit of the incoming identifier must have the same
     *   level has specified in the corresponding identifier register of the
     *   filter.
     */

    // 9. Activate filters in identifier mask mode
    // 10. Set identifiers and the masks
    CAN->FA1R |= CAN_FA1R_FACT0;
    CAN->sFilterRegister[0].FR1 = (MSG_ID_REF << 5) | (0xFF00U << 16);
    CAN->FA1R |= CAN_FA1R_FACT1;
    CAN->sFilterRegister[1].FR1 = (MSG_ID_ENGINE_TEMP << 5) | (0xFFE0U << 16);
    CAN->FA1R |= CAN_FA1R_FACT2;
    CAN->sFilterRegister[2].FR1 = (MSG_ID_RPM << 5) | (0xFFE0U << 16);
    CAN->FA1R |= CAN_FA1R_FACT3;
    CAN->sFilterRegister[3].FR1 = (MSG_ID_BRAKE << 5) | (0xFFE0U << 16);
    CAN->FA1R |= CAN_FA1R_FACT4;
    CAN->sFilterRegister[4].FR1 = (MSG_ID_GAS << 5) | (0xFFE0U << 16);

    // 11. Leave filter init
    CAN->FMR &= ~CAN_FMR_FINIT;

    // 12. Set FIFO 0 message pending IT enable
    CAN->IER |= CAN_IER_FMPIE0;

    // Configure IT
    // 13. Set priority for CAN_IRQn
    // 14. Enable CAN_IRQn
    NVIC_SetPriority(CEC_CAN_IRQn, 0);
    NVIC_EnableIRQ(CEC_CAN_IRQn);
}

void CommNetworkInterface::_config_timer()
{
    // 1. Enable the peripheral clock of Timer 2
    RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;

    // 2. Set PSC
    TIM2->PSC = 48 - 1;

    // 3. Set ARR
    TIM2->ARR = m_matrix->get_basic_cycle_duration() - 1;

    if (m_is_master)
    {
        // 4. Set compare value
        // 5. Capture/compare and update interrupt enable
        TIM2->CCR1 = m_matrix->get_column_duration(0) - 1;
        TIM2->DIER |= TIM_DIER_CC1IE | TIM_DIER_UIE;
    }

    // Configure IT
    // 7. Set priority for TIM2_IRQn
    // 8. Enable TIM2_IRQn
    NVIC_SetPriority(TIM2_IRQn, 0);
    NVIC_EnableIRQ(TIM2_IRQn);
}
