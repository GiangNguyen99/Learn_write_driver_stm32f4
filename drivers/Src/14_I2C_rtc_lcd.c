/*
 * 14_I2C_rtc_lcd.c
 *
 *  Created on: Aug 23, 2023
 *      Author: hoang
 */

#include<stdio.h>
#include<string.h>
#include "stm32f411xx.h"

#define SYSTICK_TIM_CLK   16000000UL

/* Enable this macro if you want to test RTC on LCD */
#define PRINT_LCD

void init_systick_timer(uint32_t tick_hz)
{
	uint32_t *pSRVR = (uint32_t*) 0xE000E014;
	uint32_t *pSCSR = (uint32_t*) 0xE000E010;

	/* calculation of reload value */
	uint32_t count_value = (SYSTICK_TIM_CLK / tick_hz) - 1;

	//Clear the value of SVR
	*pSRVR &= ~(0x00FFFFFFFF);

	//load the value in to SVR
	*pSRVR |= count_value;

	//do some settings
	*pSCSR |= (1 << 1); //Enables SysTick exception request:
	*pSCSR |= (1 << 2);  //Indicates the clock source, processor clock source

	//enable the systick
	*pSCSR |= (1 << 0); //enables the counter

}

char* get_day_of_week(uint8_t i)
{
	char *days[] =
	{ "Sun", "Mon", "Tues", "Wed", "Thurs", "Fri",
			"Sat" };

	return days[i - 1];
}

void number_to_string(uint8_t num, char *buf)
{

	if (num < 10)
	{
		buf[0] = '0';
		buf[1] = num + 48;
	}
	else if (num >= 10 && num < 99)
	{
		buf[0] = (num / 10) + 48;
		buf[1] = (num % 10) + 48;
	}
}

//hh:mm:ss
char* time_to_string(RTC_time_t *rtc_time)
{
	static char buf[9];

	buf[2] = ':';
	buf[5] = ':';

	number_to_string(rtc_time->hours, buf);
	number_to_string(rtc_time->minutes, &buf[3]);
	number_to_string(rtc_time->seconds, &buf[6]);

	buf[8] = '\0';

	return buf;

}

//dd/mm/yy
char* date_to_string(RTC_date_t *rtc_date)
{
	static char buf[9];

	buf[2] = '/';
	buf[5] = '/';

	number_to_string(rtc_date->date, buf);
	number_to_string(rtc_date->month, &buf[3]);
	number_to_string(rtc_date->year, &buf[6]);

	buf[8] = '\0';

	return buf;

}

static void mdelay(uint32_t cnt)
{
	for (uint32_t i = 0; i < (cnt * 1000); i++)
		;
}

void GPIO_ButtonInit(void)
{
	GPIO_Handle_t GPIOBtn;

	//this is btn gpio configuration
	GPIOBtn.pGPIOx = GPIOA;
	GPIOBtn.GPIO_PinConfig.GPIO_PinNumber = GPIO_PIN_NO_1;
	GPIOBtn.GPIO_PinConfig.GPIO_PinMode = GPIO_MODE_IN;
	GPIOBtn.GPIO_PinConfig.GPIO_PinSpeed = GPIO_SPEED_FAST;
	GPIOBtn.GPIO_PinConfig.GPIO_PinPuPdControl = GPIO_PIN_PD;


	GPIO_Init(&GPIOBtn);

}

uint8_t state = 0;
int main(void)
{

	RTC_time_t current_time;
	RTC_date_t current_date;

//	GPIO_ButtonInit();

//	while (1)
//	{
//		//wait till button is pressed
//		while (!GPIO_ReadFromInputPin(GPIOA, GPIO_PIN_NO_1))
//			;
//		mdelay(200);
	lcd_init();


#ifndef PRINT_LCD
	printf("RTC test\n");
#else

#endif

	lcd_display_clear();
	lcd_display_return_home();
	if (ds3231_init())
	{
		printf("RTC init has failed\n");
		while (1)
			;
	}

	if (state == 0)
	{
		init_systick_timer(1);
		state = 1;
	}

	current_date.day = WEDNESDAY
	;
	current_date.date = 15;
	current_date.month = 10;
	current_date.year = 21;

	current_time.hours = 11;
	current_time.minutes = 59;
	current_time.seconds = 30;
	current_time.time_format = TIME_FORMAT_12HRS_PM;

	ds3231_set_current_date(&current_date);
	ds3231_set_current_time(&current_time);

	ds3231_get_current_time(&current_time);
	ds3231_get_current_date(&current_date);

	char *am_pm;
	if (current_time.time_format != TIME_FORMAT_24HRS)
	{
		am_pm = (current_time.time_format) ? "PM" : "AM";
#ifndef PRINT_LCD
		printf("Current time = %s %s\n",time_to_string(&current_time),am_pm); // 04:25:41 PM
#else
		lcd_print_string(time_to_string(&current_time));
		lcd_print_string(am_pm);
#endif
	}
	else
	{
#ifndef PRINT_LCD
		printf("Current time = %s\n",time_to_string(&current_time)); // 04:25:41
#else
		lcd_print_string(time_to_string(&current_time));
#endif
	}

#ifndef PRINT_LCD
	printf("Current date = %s <%s>\n",date_to_string(&current_date), get_day_of_week(current_date.day));
#else
	lcd_set_cursor(2, 1);
	lcd_print_string(date_to_string(&current_date));
#endif
//	}
	while (1)
		;

	return 0;
}

void SysTick_Handler(void)
{
	RTC_time_t current_time;
	RTC_date_t current_date;

	ds3231_get_current_time(&current_time);

	char *am_pm;
	if (current_time.time_format != TIME_FORMAT_24HRS)
	{
		am_pm = (current_time.time_format) ? "PM" : "AM";
#ifndef PRINT_LCD
		printf("Current time = %s %s\n",time_to_string(&current_time),am_pm); // 04:25:41 PM
#else
		lcd_set_cursor(1, 1);
		lcd_print_string(time_to_string(&current_time));
		lcd_print_string(am_pm);
#endif

	}
	else
	{
#ifndef PRINT_LCD
		printf("Current time = %s\n",time_to_string(&current_time)); // 04:25:41
#else
		lcd_set_cursor(1, 1);
		lcd_print_string(time_to_string(&current_time));
#endif
	}

	ds3231_get_current_date(&current_date);

#ifndef PRINT_LCD
	printf("Current date = %s <%s>\n",date_to_string(&current_date), get_day_of_week(current_date.day));
#else
	lcd_set_cursor(2, 1);
	lcd_print_string(date_to_string(&current_date));
	lcd_print_char('<');
	lcd_print_string(get_day_of_week(current_date.day));
	lcd_print_char('>');
#endif

}
