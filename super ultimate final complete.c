/*Base register adddress header file*/
#include "stm32l1xx.h"
/*Library related header files*/
#include "stm32l1xx_ll_gpio.h"
#include "stm32l1xx_ll_pwr.h"
#include "stm32l1xx_ll_rcc.h"
#include "stm32l1xx_ll_bus.h"
#include "stm32l1xx_ll_utils.h"
#include "stm32l1xx_ll_system.h"
#include "stm32l1xx_ll_tim.h"

#include "dwt_delay.h"

#define TIMx_PSC 3200
#define TIMx_ARR 100

#define DS1820_CONV_TEMP 0x44
#define DS1820_READ_SCTPAD 0xBE

#define OW_IO_PIN 	LL_GPIO_PIN_6
#define OW_IO_PORT 	GPIOB
#define OW_IO_CLK_CMD 	LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOB)

/*already implemented */
void SystemClock_Config(void);
void OW_WriteBit(uint8_t d);
uint8_t OW_ReadBit(void);
void DS1820_GPIO_Configure(void);
uint8_t DS1820_ResetPulse(void);


/*haven't been implemented yet!*/
void OW_Master(void);
void OW_Slave(void);
void OW_WriteByte(uint8_t data);
uint16_t OW_ReadByte(void);


uint32_t CheckDigit(uint32_t);
void segment(uint32_t);
uint32_t CharToUint32_t(char number);
void ltc4727_GPIO_Config(void);


/*motor*/
void TIM_OC_Config(void);
void TIM_OC_GPIO_Config(void);

uint16_t temp;
uint16_t temp_cal;
uint8_t status;
char LOG[10] = "";

uint32_t seg[4] = {LL_GPIO_PIN_2 | LL_GPIO_PIN_10 | LL_GPIO_PIN_11 | LL_GPIO_PIN_12 | LL_GPIO_PIN_13 | LL_GPIO_PIN_14,
									 LL_GPIO_PIN_2 | LL_GPIO_PIN_10 | LL_GPIO_PIN_11 | LL_GPIO_PIN_12 | LL_GPIO_PIN_13 | LL_GPIO_PIN_14,
									 LL_GPIO_PIN_2 | LL_GPIO_PIN_10 | LL_GPIO_PIN_11 | LL_GPIO_PIN_12 | LL_GPIO_PIN_13 | LL_GPIO_PIN_14,
									 LL_GPIO_PIN_2 | LL_GPIO_PIN_10 | LL_GPIO_PIN_11 | LL_GPIO_PIN_12 | LL_GPIO_PIN_13 | LL_GPIO_PIN_14 };
uint32_t digit[2] = {LL_GPIO_PIN_1 , LL_GPIO_PIN_2};

uint32_t test;

uint8_t th,tl,i=0;

int main()
{
	SystemClock_Config();
	DWT_Init();
	DS1820_GPIO_Configure();
	ltc4727_GPIO_Config();
	TIM_OC_Config();
	
	LL_TIM_InitTypeDef timbase_initstructure;
	
	LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_TIM2);
	//Time-base configure
	timbase_initstructure.ClockDivision = LL_TIM_CLOCKDIVISION_DIV1;
	timbase_initstructure.CounterMode = LL_TIM_COUNTERMODE_DOWN;
	timbase_initstructure.Autoreload = 10000 - 1; //10 second
	timbase_initstructure.Prescaler =  32000 - 1;
	LL_TIM_Init(TIM2, &timbase_initstructure);
	LL_TIM_EnableIT_UPDATE(TIM2);
	
	NVIC_SetPriority(TIM2_IRQn,0);
	NVIC_EnableIRQ(TIM2_IRQn);
	
	LL_TIM_EnableCounter(TIM2);
	
	while(1)
	{
		test = LL_TIM_GetCounter(TIM2)/1000;
		
		for(int i=0;i<2;i++){
			LL_GPIO_ResetOutputPin(GPIOC, LL_GPIO_PIN_1 | LL_GPIO_PIN_2);//Write 0 to GPIOC
			LL_GPIO_ResetOutputPin(GPIOB, LL_GPIO_PIN_2 | LL_GPIO_PIN_10 | LL_GPIO_PIN_11 | LL_GPIO_PIN_12 | LL_GPIO_PIN_13 | LL_GPIO_PIN_14 | LL_GPIO_PIN_15);//Reser all segment
			LL_GPIO_SetOutputPin(GPIOC, digit[i]);
			LL_GPIO_SetOutputPin(GPIOB, seg[i]);
		}
	
	
	}
}



void TIM_BASE_Config(void)
{
	LL_TIM_InitTypeDef timbase_initstructure;
	
	LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_TIM3);
	
	timbase_initstructure.ClockDivision = LL_TIM_CLOCKDIVISION_DIV2;
	timbase_initstructure.CounterMode = LL_TIM_COUNTERMODE_UP;
	timbase_initstructure.Autoreload = TIMx_ARR - 1;
	timbase_initstructure.Prescaler =  TIMx_PSC- 1;
	
	LL_TIM_Init(TIM3, &timbase_initstructure);
}

void TIM_OC_GPIO_Config(void)
{
	
	LL_GPIO_InitTypeDef gpio_initstructure;
	
	LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOB);
	
	LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOA); 
	gpio_initstructure.Pull = LL_GPIO_PULL_NO;
	gpio_initstructure.Speed = LL_GPIO_SPEED_FREQ_VERY_HIGH;
	gpio_initstructure.Mode = LL_GPIO_MODE_INPUT;
	gpio_initstructure.Pin = LL_GPIO_PIN_0;
	LL_GPIO_Init(GPIOA, &gpio_initstructure);
	
	LL_GPIO_InitTypeDef l293d_init;
	
	l293d_init.Mode = LL_GPIO_MODE_OUTPUT;
	l293d_init.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
	l293d_init.Pull = LL_GPIO_PULL_NO;
	l293d_init.Pull = LL_GPIO_SPEED_FREQ_VERY_HIGH;
	l293d_init.Pin = LL_GPIO_PIN_4 | LL_GPIO_PIN_7;
	LL_GPIO_Init(GPIOB, &l293d_init);
	
	l293d_init.Pin = LL_GPIO_PIN_5;
	l293d_init.Mode = LL_GPIO_MODE_ALTERNATE;
	l293d_init.Alternate = LL_GPIO_AF_2;
	LL_GPIO_Init(GPIOB, &l293d_init);
	
	LL_GPIO_SetOutputPin(GPIOB, LL_GPIO_PIN_7);
	LL_GPIO_ResetOutputPin(GPIOB, LL_GPIO_PIN_4);
	LL_GPIO_SetOutputPin(GPIOB, LL_GPIO_PIN_5);
	
	SYSCFG->EXTICR[0] &= ~(15<<0);
	EXTI->IMR |= (1<<0);
	EXTI->RTSR |= (1<<0);
	NVIC_EnableIRQ((IRQn_Type) 6);
	NVIC_SetPriority((IRQn_Type)6,0);
}

void TIM_OC_Config(void)
{
	LL_TIM_OC_InitTypeDef tim_oc_initstructure;
	
	TIM_BASE_Config();
	TIM_OC_GPIO_Config();
	
	tim_oc_initstructure.OCState = LL_TIM_OCSTATE_DISABLE;
	tim_oc_initstructure.OCMode = LL_TIM_OCMODE_PWM1;
	tim_oc_initstructure.OCPolarity = LL_TIM_OCPOLARITY_HIGH;
	//tim_oc_initstructure.CompareValue = LL_TIM_GetAutoReload(TIM3); //100% duty
	tim_oc_initstructure.CompareValue = LL_TIM_GetAutoReload(TIM3)/ 100; //0% duty
	LL_TIM_OC_Init(TIM3, LL_TIM_CHANNEL_CH2, &tim_oc_initstructure);
	/*Interrupt Configure*/
	//NVIC_SetPriority(TIM3_IRQn, 1);
	//NVIC_EnableIRQ(TIM3_IRQn);
	//LL_TIM_EnableIT_CC1(TIM3);
	
	/*Start Output Compare in PWM Mode*/
	LL_TIM_CC_EnableChannel(TIM3, LL_TIM_CHANNEL_CH2);
	LL_TIM_EnableCounter(TIM3);
}



void DS1820_GPIO_Configure(void)
{
	LL_GPIO_InitTypeDef ds1820_io;
	
	LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOB);
	
	ds1820_io.Mode = LL_GPIO_MODE_OUTPUT;
	ds1820_io.Pin = OW_IO_PIN;
	ds1820_io.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
	ds1820_io.Pull = LL_GPIO_PULL_NO;
	ds1820_io.Speed = LL_GPIO_SPEED_FREQ_LOW;
	LL_GPIO_Init(GPIOB, &ds1820_io);
}



void SystemClock_Config(void)
{
  /* Enable ACC64 access and set FLASH latency */ 
  LL_FLASH_Enable64bitAccess();; 
  LL_FLASH_SetLatency(LL_FLASH_LATENCY_1);

  /* Set Voltage scale1 as MCU will run at 32MHz */
  LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_PWR);
  LL_PWR_SetRegulVoltageScaling(LL_PWR_REGU_VOLTAGE_SCALE1);
  
  /* Poll VOSF bit of in PWR_CSR. Wait until it is reset to 0 */
  while (LL_PWR_IsActiveFlag_VOSF() != 0)
  {
  };
  
  /* Enable HSI if not already activated*/
  if (LL_RCC_HSI_IsReady() == 0)
  {
    /* HSI configuration and activation */
    LL_RCC_HSI_Enable();
    while(LL_RCC_HSI_IsReady() != 1)
    {
    };
  }
  
	
  /* Main PLL configuration and activation */
  LL_RCC_PLL_ConfigDomain_SYS(LL_RCC_PLLSOURCE_HSI, LL_RCC_PLL_MUL_6, LL_RCC_PLL_DIV_3);

  LL_RCC_PLL_Enable();
  while(LL_RCC_PLL_IsReady() != 1)
  {
  };
  
  /* Sysclk activation on the main PLL */
  LL_RCC_SetAHBPrescaler(LL_RCC_SYSCLK_DIV_1);
  LL_RCC_SetSysClkSource(LL_RCC_SYS_CLKSOURCE_PLL);
  while(LL_RCC_GetSysClkSource() != LL_RCC_SYS_CLKSOURCE_STATUS_PLL)
  {
  };
  
  /* Set APB1 & APB2 prescaler*/
  LL_RCC_SetAPB1Prescaler(LL_RCC_APB1_DIV_1);
  LL_RCC_SetAPB2Prescaler(LL_RCC_APB2_DIV_1);

  /* Set systick to 1ms in using frequency set to 32MHz                             */
  /* This frequency can be calculated through LL RCC macro                          */
  /* ex: __LL_RCC_CALC_PLLCLK_FREQ (HSI_VALUE, LL_RCC_PLL_MUL_6, LL_RCC_PLL_DIV_3); */
  LL_Init1msTick(32000000);
  
  /* Update CMSIS variable (which can be updated also through SystemCoreClockUpdate function) */
  LL_SetSystemCoreClock(32000000);
}

uint8_t DS1820_ResetPulse(void)
{	
	OW_Master();
	LL_GPIO_ResetOutputPin(GPIOB, LL_GPIO_PIN_6);
	DWT_Delay(480);
	OW_Slave();
	DWT_Delay(80);
	
	if(LL_GPIO_IsInputPinSet(OW_IO_PORT, OW_IO_PIN == 0))
	{
		DWT_Delay(400);
		return 0;
	}
	else
	{
		DWT_Delay(400);
		return 1;
	}
}

void OW_Master(void){
	LL_GPIO_SetPinMode(OW_IO_PORT,OW_IO_PIN,LL_GPIO_MODE_OUTPUT);
	LL_GPIO_SetPinPull(OW_IO_PORT,OW_IO_PIN,LL_GPIO_PULL_NO);
}

void OW_Slave(void){
	LL_GPIO_SetPinMode(OW_IO_PORT,OW_IO_PIN,LL_GPIO_MODE_INPUT);
	LL_GPIO_SetPinPull(OW_IO_PORT,OW_IO_PIN,LL_GPIO_PULL_UP);
}

void OW_WriteBit(uint8_t d){
	if(d == 1) //write 1
	{
		OW_Master(); //uC occupires wire system
		LL_GPIO_ResetOutputPin(OW_IO_PORT, OW_IO_PIN);
		DWT_Delay(1);
		OW_Slave(); //uC releases wire system
		DWT_Delay(60);
	}
	else //write 0
	{
		OW_Master(); //uC occupires wire system
		DWT_Delay(60);
		OW_Slave(); //uC releases wire system
	}
}

void OW_WriteByte(uint8_t data)
{
	uint8_t i;
	
	for(i=0; i < 8 ;++i)
	{
		OW_WriteBit(data & 0x01);
		data = (data >> 1 );
	}
}

uint8_t OW_ReadBit(void)
{
	OW_Master();
	LL_GPIO_ResetOutputPin(OW_IO_PORT,OW_IO_PIN);
	DWT_Delay(2);
	OW_Slave();
	
	return LL_GPIO_IsInputPinSet(OW_IO_PORT, OW_IO_PIN);
}

uint16_t OW_ReadByte(void)
{
	uint8_t i, bit;
	uint8_t result = 0;
	for(i = 0 ; i < 8; i++)
	{
		bit = OW_ReadBit();
		if(bit == 1)
		{
			result |= (1<<i);
		}
		DWT_Delay(60);
	}
	return result;
}

//for7 segment

uint32_t CheckDigit(uint32_t number)
{
    int i;
    int digit;
    char seg[4];
    for(i=0;number!=0;i++)
    {
        number = number/10;
    }
    return i;
}

void segment(uint32_t number)
{
    uint32_t digit;
    char numberForShow[2];
    digit = CheckDigit(number);
    switch(digit)
    {
		case 0:
				 numberForShow[0] = '0';
        numberForShow[1] =  '0';
    case 1:
        numberForShow[0] = '0';
        numberForShow[1] = number+'0';
        break;
    case 2:
        numberForShow[0] = (number/10)+'0';
        numberForShow[1] = (number%10)+'0';
        break;
    }
		
		for(int i=0;i<2;i++)
		{
			seg[i] = CharToUint32_t(numberForShow[i]);
		}
}



uint32_t CharToUint32_t(char number)
{
	switch(number)
    {
        case '0':
            return LL_GPIO_PIN_2 | LL_GPIO_PIN_10 | LL_GPIO_PIN_11 | LL_GPIO_PIN_12 | LL_GPIO_PIN_13 | LL_GPIO_PIN_14;
        case '1':
            return LL_GPIO_PIN_10 | LL_GPIO_PIN_11;
        case '2':
            return LL_GPIO_PIN_2 | LL_GPIO_PIN_10 | LL_GPIO_PIN_12 | LL_GPIO_PIN_13 | LL_GPIO_PIN_15;
        case '3':
            return LL_GPIO_PIN_2 | LL_GPIO_PIN_10 | LL_GPIO_PIN_11 | LL_GPIO_PIN_12 | LL_GPIO_PIN_15;
        case '4':
            return LL_GPIO_PIN_10 | LL_GPIO_PIN_11 | LL_GPIO_PIN_14 | LL_GPIO_PIN_15;
        case '5':
            return LL_GPIO_PIN_2 | LL_GPIO_PIN_11 | LL_GPIO_PIN_12 | LL_GPIO_PIN_14 | LL_GPIO_PIN_15;
        case '6':
            return LL_GPIO_PIN_2 | LL_GPIO_PIN_11 | LL_GPIO_PIN_12 | LL_GPIO_PIN_13 | LL_GPIO_PIN_14 | LL_GPIO_PIN_15;
        case '7':
            return LL_GPIO_PIN_2 | LL_GPIO_PIN_10 | LL_GPIO_PIN_11;
        case '8':
            return LL_GPIO_PIN_2 | LL_GPIO_PIN_10 | LL_GPIO_PIN_11 | LL_GPIO_PIN_12 | LL_GPIO_PIN_13 | LL_GPIO_PIN_14 | LL_GPIO_PIN_15;
        case '9':
            return LL_GPIO_PIN_2 | LL_GPIO_PIN_10 | LL_GPIO_PIN_11 | LL_GPIO_PIN_12 | LL_GPIO_PIN_14 | LL_GPIO_PIN_15;
    }
}



void ltc4727_GPIO_Config(void)
{
	LL_GPIO_InitTypeDef ltc4727_init;
	
	//config ltc4727js
	LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOB);
	LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOC);
		
	ltc4727_init.Mode = LL_GPIO_MODE_OUTPUT;
	ltc4727_init.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
	ltc4727_init.Pull = LL_GPIO_PULL_NO;
	ltc4727_init.Speed = LL_GPIO_SPEED_FREQ_VERY_HIGH;
	ltc4727_init.Pin = LL_GPIO_PIN_2 | LL_GPIO_PIN_10 | LL_GPIO_PIN_11 | LL_GPIO_PIN_12 | LL_GPIO_PIN_13 | LL_GPIO_PIN_14 | LL_GPIO_PIN_15;
	LL_GPIO_Init(GPIOB, &ltc4727_init);
		
	ltc4727_init.Pin = LL_GPIO_PIN_1 | LL_GPIO_PIN_2;
	LL_GPIO_Init(GPIOC, &ltc4727_init);
}

void TIM2_IRQHandler(void)
{
	if(LL_TIM_IsActiveFlag_UPDATE(TIM2) == SET)
	{
		LL_TIM_ClearFlag_UPDATE(TIM2);
		
		//Send reset pulse
		DS1820_ResetPulse();
			
		//Send 'Skip Rom (0xCC)' command
		OW_WriteByte(0xCC);
		
		//Send 'Temp Convert (0x44)' command
		OW_WriteByte(0x44);
		
		//Delay at least 200ms (typical conversion time)
		LL_mDelay(200);
		
		//Send reset pulse
		DS1820_ResetPulse();
		
		//Send 'Skip Rom (0xCC)' command
		OW_WriteByte(0xCC);
		
		//Send 'Read Scractpad (0xBE)' command
		OW_WriteByte(0xBE);
		
		//Read byte 1 (Temperature data in LSB)
		//Read byte 2 (Temperature data in MSB)
		
		tl = OW_ReadByte();
		th = OW_ReadByte();
		
		//Convert to readable floating point temperature
		temp = (th << 8) | tl;
		temp_cal = (temp)/16;
		
		segment((uint32_t)temp_cal);
		
		//fan running
		if(temp_cal > 20)
		{
			LL_TIM_OC_SetCompareCH2(TIM3,LL_TIM_GetAutoReload(TIM3));
			
		}
		else
		{
			LL_TIM_OC_SetCompareCH2(TIM3,0);
		}
	}
}