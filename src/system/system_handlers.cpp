//
// Created by Jamie on 1-5-2021.
//

#include "local_tick.h"
#include "cni.h"

/******************************************************************************/
/*            Cortex-M0 Processor Exceptions Handlers                         */
/******************************************************************************/
extern "C" {
/**
  * Brief  This function handles NMI exception.
  * Param  None
  * Return None
  */
__attribute__((unused)) void NMI_Handler()
{
}

/**
  * Brief  This function handles Hard Fault exception.
  * Param  None
  * Return None
  */
__attribute__((unused)) void HardFault_Handler()
{
    // Go to infinite loop when Hard Fault exception occurs
    while (true)
    {
    }
}

/**
  * Brief  This function handles SVCall exception.
  * Param  None
  * Return None
  */
__attribute__((unused)) void SVC_Handler()
{
}

/**
  * Brief  This function handles PendSVC exception.
  * Param  None
  * Return None
  */
__attribute__((unused)) void PendSV_Handler()
{
}

/**
  * Brief  This function handles SysTick Handler.
  * Param  None
  * Return None
  */
__attribute__((unused)) void SysTick_Handler()
{
    // Increment local clock every 1ms
    local_tick_increment();
}

/******************************************************************************/
/*                 STM32F0xx Peripherals Interrupt Handlers                   */
/*  Add here the Interrupt Handler for the used peripheral(s) (PPP), for the  */
/*  available peripheral interrupt handler's name please refer to the startup */
/*  file (startup_stm32f072xb.s).                                             */
/******************************************************************************/
__attribute__((unused)) void CEC_CAN_IRQHandler()
{
    auto comm = cni::CommNetworkInterface::get_instance();

    comm->can_isr();
}

__attribute__((unused)) void TIM2_IRQHandler()
{
    auto comm = cni::CommNetworkInterface::get_instance();

    comm->tim_isr();
//    if ((TIM2->SR & TIM_SR_UIF) != 0)
//    {
//        TIM2->SR &= ~TIM_SR_UIF;
//    }
//
//    if ((TIM2->SR & TIM_SR_CC1IF) != 0)
//    {
//        TIM2->SR &= ~TIM_SR_CC1IF;
//    }
}

}
