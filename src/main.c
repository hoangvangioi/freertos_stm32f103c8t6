#include "defines.h"

static void prvSetupHardware(void);
void		GPIO_Configuration(void);
void		vDisplayTask(void *pvParameters);
void		vButtonTask(void *pvParameters);
extern void SystemInit(void);
extern void SystemCoreClockUpdate(void);

const char *pvButtonTask  = "Task Button is running.";
const char *pvDisplayTask = "Task Display is running.";

#define DISPLAY_SIZE 6

SemaphoreHandle_t xSemaphore = NULL;
QueueHandle_t	  xQueue;

int main(void)
{
	SystemInit();
	SystemCoreClockUpdate();
	prvSetupHardware();
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOC, ENABLE);

	GPIO_Configuration();
	SetupUSART();
	USART_PutStr("\n");
	USART_PutStr("Initialization is done.\r\n");

	BH1750_Init();
	LcdInitialize();

	LcdClear();
	LcdStringX2("Hoang", 0, 15);
	LcdStringX2("Van", 2, 22);
	LcdStringX2("Gioi", 4, 19);

	xTaskCreate(vButtonTask, "Task Button", configMINIMAL_STACK_SIZE, (void *)pvButtonTask, 1, NULL);
	xTaskCreate(vDisplayTask, "Task Dispaly Nokia 5110", configMINIMAL_STACK_SIZE, (void *)pvDisplayTask, 1, NULL);
	// do uu tien cac task
	xSemaphore = xSemaphoreCreateMutex();
	xQueue	   = xQueueCreate(DISPLAY_SIZE, sizeof(uint16_t));

	vTaskStartScheduler();
}

static void prvSetupHardware(void)
{
	/* Start with the clocks in their expected state. */
	RCC_DeInit();

	/* Enable HSE (high speed external clock). */
	RCC_HSEConfig(RCC_HSE_ON);

	/* Wait till HSE is ready. */
	while (RCC_GetFlagStatus(RCC_FLAG_HSERDY) == RESET)
	{
	}

	/* 2 wait states required on the flash. */
	*((unsigned long *)0x40022000) = 0x02;

	/* HCLK = SYSCLK */
	RCC_HCLKConfig(RCC_SYSCLK_Div1);

	/* PCLK2 = HCLK */
	RCC_PCLK2Config(RCC_HCLK_Div1);

	/* PCLK1 = HCLK/2 */
	RCC_PCLK1Config(RCC_HCLK_Div2);

	/* PLLCLK = 12MHz * 6 = 72 MHz. */
	RCC_PLLConfig(RCC_PLLSource_HSE_Div1, RCC_PLLMul_6);

	/* Enable PLL. */
	RCC_PLLCmd(ENABLE);

	/* Wait till PLL is ready. */
	while (RCC_GetFlagStatus(RCC_FLAG_PLLRDY) == RESET)
	{
	}

	/* Select PLL as system clock source. */
	RCC_SYSCLKConfig(RCC_SYSCLKSource_PLLCLK);

	/* Wait till PLL is used as system clock source. */
	while (RCC_GetSYSCLKSource() != 0x08)
	{
	}

	/* Enable GPIOA, GPIOB, GPIOC, GPIOD, GPIOE and AFIO clocks */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB | RCC_APB2Periph_GPIOC | RCC_APB2Periph_GPIOD |
							   RCC_APB2Periph_GPIOE | RCC_APB2Periph_AFIO,
						   ENABLE);

	/* SPI2 Periph clock enable */
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI2, ENABLE);

	/* Set the Vector Table base address at 0x08000000 */
	NVIC_SetVectorTable(NVIC_VectTab_FLASH, 0x0);

	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);

	/* Configure HCLK clock as SysTick clock source. */
	SysTick_CLKSourceConfig(SysTick_CLKSource_HCLK);
}

void GPIO_Configuration(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Pin	  = GPIO_Pin_6 | GPIO_Pin_7;
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_IPU;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
}

void vButtonTask(void *pvParameters)
{
	while (1)
	{
		if (GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_6) == RESET)
		{
			xSemaphoreTake(xSemaphore, portMAX_DELAY);
			LcdClear();
			xSemaphoreGive(xSemaphore);
		}
		else if (GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_7) == RESET)
		{
			xSemaphoreTake(xSemaphore, portMAX_DELAY);
			uint16_t luxValue = BH1750_ReadLux();
			xQueueSend(xQueue, &luxValue, portMAX_DELAY);
			xSemaphoreGive(xSemaphore);
			vTaskDelay(pdMS_TO_TICKS(200));
		}
		vTaskDelay(pdMS_TO_TICKS(100));
	}
}

void vDisplayTask(void *pvParameters)
{
	uint16_t luxData[DISPLAY_SIZE] = {0};
	while (1)
	{
		xSemaphoreTake(xSemaphore, portMAX_DELAY);

		uint16_t luxValue;
		if (xQueueReceive(xQueue, &luxValue, 0) == pdPASS)
		{
			for (int i = DISPLAY_SIZE - 1; i > 0; i--)
			{
				luxData[i] = luxData[i - 1];
			}
			luxData[0] = luxValue;
			for (int i = 0; i < DISPLAY_SIZE; i++)
			{
				char xBuffer[8];
				citoa(luxData[i], xBuffer, 10);
				LcdClearRow(i);
				LCD_GotoXY(0, i);
				char si[1];
				citoa(i + 1, si, 10);
				LcdString(si);
				LcdString(": ");
				LcdString(xBuffer);
				LcdString(" lux");
			}
		}
		xSemaphoreGive(xSemaphore);
		vTaskDelay(pdMS_TO_TICKS(1000));
	}
}

void vApplicationStackOverflowHook(TaskHandle_t pxTask, char *pcTaskName)
{
	(void)pxTask;
	(void)pcTaskName;

	for (;;)
		;
}
