/**
******************************************************************************
// Startup code for STM32F4 Discovery Board
// 1. Creates a vector table
// 2. Initializes .data and .bss section in SRAM
// 3. Calls main()
******************************************************************************
*/

#include "stm32_startup.h"

// Variables from linker scripts: to keep track of section boundaries
extern uint32_t _la_data;
extern uint32_t _sdata;
extern uint32_t _edata;
extern uint32_t _ebss;
extern uint32_t _sbss;

// Vector table
uint32_t vectors[] __attribute__((section (".isr_vector"))) = 
{
    STACK_START,
    (uint32_t)&Reset_Handler,
    (uint32_t)&NMI_Handler,
    (uint32_t)&HardFault_Handler,
    (uint32_t)&MemManage_Handler,
    (uint32_t)&BusFault_Handler,				
    (uint32_t)&UsageFault_Handler,
    0,
    0,
    0,
    0,	
    (uint32_t)&SVC_Handler,			
    (uint32_t)&DebugMon_Handler,	
    0,			
    (uint32_t)&PendSV_Handler,
    (uint32_t)&SysTick_Handler,
    (uint32_t)&WWDG_IRQHandler,
    (uint32_t)&PVD_IRQHandler,             
    (uint32_t)&TAMP_STAMP_IRQHandler,      
    (uint32_t)&RTC_WKUP_IRQHandler,                               
    (uint32_t)&RCC_IRQHandler,             
    (uint32_t)&EXTI0_IRQHandler,           
    (uint32_t)&EXTI1_IRQHandler,           
    (uint32_t)&EXTI2_IRQHandler,           
    (uint32_t)&EXTI3_IRQHandler,           
    (uint32_t)&EXTI4_IRQHandler,           
    (uint32_t)&DMA1_Stream0_IRQHandler,    
    (uint32_t)&DMA1_Stream1_IRQHandler,    
    (uint32_t)&DMA1_Stream2_IRQHandler,    
    (uint32_t)&DMA1_Stream3_IRQHandler,    
    (uint32_t)&DMA1_Stream4_IRQHandler,    
    (uint32_t)&DMA1_Stream5_IRQHandler,    
    (uint32_t)&DMA1_Stream6_IRQHandler,    
    (uint32_t)&ADC_IRQHandler,             
    (uint32_t)&CAN1_TX_IRQHandler,         
    (uint32_t)&CAN1_RX0_IRQHandler,        
    (uint32_t)&CAN1_RX1_IRQHandler,        
    (uint32_t)&CAN1_SCE_IRQHandler,        
    (uint32_t)&EXTI9_5_IRQHandler,         
    (uint32_t)&TIM1_BRK_TIM9_IRQHandler,   
    (uint32_t)&TIM1_UP_TIM10_IRQHandler,   
    (uint32_t)&TIM1_TRG_COM_TIM11_IRQHandler,
    (uint32_t)&TIM1_CC_IRQHandler,         
    (uint32_t)&TIM2_IRQHandler,            
    (uint32_t)&TIM3_IRQHandler,            
    (uint32_t)&TIM4_IRQHandler,            
    (uint32_t)&I2C1_EV_IRQHandler,         
    (uint32_t)&I2C1_ER_IRQHandler,         
    (uint32_t)&I2C2_EV_IRQHandler,         
    (uint32_t)&I2C2_ER_IRQHandler,         
    (uint32_t)&SPI1_IRQHandler,           
    (uint32_t)&SPI2_IRQHandler,            
    (uint32_t)&USART1_IRQHandler,         
    (uint32_t)&USART2_IRQHandler,        
    (uint32_t)&USART3_IRQHandler,        
    (uint32_t)&EXTI15_10_IRQHandler,     
    (uint32_t)&RTC_Alarm_IRQHandler,    
    (uint32_t)&OTG_FS_WKUP_IRQHandler, 
    (uint32_t)&TIM8_BRK_TIM12_IRQHandler,
    (uint32_t)&TIM8_UP_TIM13_IRQHandler,
    (uint32_t)&TIM8_TRG_COM_TIM14_IRQHandler,
    (uint32_t)&TIM8_CC_IRQHandler,
    (uint32_t)&DMA1_Stream7_IRQHandler,
    (uint32_t)&FSMC_IRQHandler,
    (uint32_t)&SDIO_IRQHandler,
    (uint32_t)&TIM5_IRQHandler,
    (uint32_t)&SPI3_IRQHandler,
    (uint32_t)&UART4_IRQHandler,
    (uint32_t)&UART5_IRQHandler,
    (uint32_t)&TIM6_DAC_IRQHandler,
    (uint32_t)&TIM7_IRQHandler,
    (uint32_t)&DMA2_Stream0_IRQHandler,
    (uint32_t)&DMA2_Stream1_IRQHandler,
    (uint32_t)&DMA2_Stream2_IRQHandler,
    (uint32_t)&DMA2_Stream3_IRQHandler,
    (uint32_t)&DMA2_Stream4_IRQHandler,
    (uint32_t)&ETH_IRQHandler,
    (uint32_t)&ETH_WKUP_IRQHandler,
    (uint32_t)&CAN2_TX_IRQHandler,
    (uint32_t)&CAN2_RX0_IRQHandler,
    (uint32_t)&CAN2_RX1_IRQHandler,
    (uint32_t)&CAN2_SCE_IRQHandler,
    (uint32_t)&OTG_FS_IRQHandler,
    (uint32_t)&DMA2_Stream5_IRQHandler,
    (uint32_t)&DMA2_Stream6_IRQHandler,
    (uint32_t)&DMA2_Stream7_IRQHandler,
    (uint32_t)&USART6_IRQHandler,
    (uint32_t)&I2C3_EV_IRQHandler,
    (uint32_t)&I2C3_ER_IRQHandler,
    (uint32_t)&OTG_HS_EP1_OUT_IRQHandler,
    (uint32_t)&OTG_HS_EP1_IN_IRQHandler,
    (uint32_t)&OTG_HS_WKUP_IRQHandler,
    (uint32_t)&OTG_HS_IRQHandler,
    (uint32_t)&DCMI_IRQHandler,
    (uint32_t)&CRYP_IRQHandler,
    (uint32_t)&HASH_RNG_IRQHandler,
    (uint32_t)&FPU_IRQHandler
};

void Reset_Handler(void) 
{
    // copy .data section in Flash to SRAM
    uint32_t data_size = (uint32_t)&_edata - (uint32_t)&_sdata; // get the addrs & subtract
    uint8_t *pDDst = (uint8_t*) &_sdata; // sram
    uint8_t *pDSrc = (uint8_t*) &_la_data; // flash

    for (uint32_t i = 0; i < data_size; i++) 
    {
        *pDDst++ = *pDSrc++;
    }

    // init .bss section to zero in SRAM
    uint32_t bss_size = (uint32_t)&_ebss - (uint32_t)&_sbss;
    uint8_t *pBDst = (uint8_t*) &_sbss;
    for (uint32_t i = 0; i < bss_size; i++) 
    {
        *pBDst++ = 0;
    }

    // init the c standard lib
    __libc_init_array();

    // call main()
    main();
    
};

void Default_Handler(void) 
{
    while(1);
};