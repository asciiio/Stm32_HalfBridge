/*
 * spwm.c
 *
 *  Created on: May 10, 2026
 *      Author: __vsafonov__
 */

#include <stm32f1xx.h>
#include <math.h>

#define DEADTIME_VAL       					0x24 		// 24 такта по 1/72e6 сек. (500 нс мертвое время)

#define SPWM_POINTS_DEFAULT         1000		// размер буфера для таблицы синуса (от 20 до ... Гц)
#define PWM_FREQUENCY_HZ    				20000		// несущая ШИМ
#define ARR_VALUE           				3600
#define SINE_FREQUENCY_HZ  					50 			// значение при первом старте

#define SOFTSTART_STEP_PERIODS 		  50		  // каждые 50 периодов синуса увеличение амплитуды в SOFTSTART_STEP (для 50 Гц - каждую секунду)
#define SOFTSTART_STEP         			0.1f    // шаг прироста амплитуды

/* таблица синуса */
static volatile uint16_t spwm_points = 400;
static volatile uint16_t sin_table[SPWM_POINTS_DEFAULT];
static volatile uint16_t ind = 0;

/* плавный старт */
static volatile float  softstart_gain        = 0.0f;
static volatile uint8_t softstart_done       = 0;
static volatile uint16_t softstart_period_cnt = 0;

/* сдвиг фаз */
static volatile uint16_t offset_ph_b = 133;
static volatile uint16_t offset_ph_c = 266;

// последние установленные значения
static volatile float amp_ = 1.0;	 // в коэффициенте от максимума (0 - 1), по умолчанию 1.0
static volatile uint16_t freq_ = 50;  // в герцах, по умолчанию 50
static volatile uint16_t ph_ = 120; // в градусах, по умолчанию 120

// spwm using
// CH1 and CH1N of TIM1 on PA8, PA7 (1,2)
// CH2 and CH2N on PA9, PB0					(3,4)
// CH3 and CH3N on PA10, PB1				(5,6)

static void spwm_gpio_init()
{
	// clock PA, PB
    RCC->APB2ENR |= RCC_APB2ENR_IOPAEN;
    RCC->APB2ENR |= RCC_APB2ENR_IOPBEN;

    // port A configuration
    // PA7 clear
    GPIOA->CRL &= ~(GPIO_CRL_MODE7 | GPIO_CRL_CNF7);

    // PA8, PA9, PA10 clear
    GPIOA->CRH &= ~(GPIO_CRH_MODE8 | GPIO_CRH_CNF8 |
    								GPIO_CRH_MODE9 | GPIO_CRH_CNF9 |
										GPIO_CRH_MODE10 | GPIO_CRH_CNF10);

    // PA7 set as out pushpull AF
    GPIOA->CRL |= GPIO_CRL_MODE7 | GPIO_CRL_CNF7_1;

    // PA8, PA9, PA10 set as out PP AF
    GPIOA->CRH |= GPIO_CRH_MODE8 | GPIO_CRH_CNF8_1 |
    							GPIO_CRH_MODE9 | GPIO_CRH_CNF9_1 |
									GPIO_CRH_MODE10 | GPIO_CRH_CNF10_1;

    // port B configuration
    GPIOB->CRL &= ~(GPIO_CRL_MODE0 | GPIO_CRL_CNF0 |
    								GPIO_CRL_MODE1 | GPIO_CRL_CNF1);

    // PB0, PB1 set as out pushpull AF
    GPIOB->CRL |= (GPIO_CRL_MODE0 | GPIO_CRL_CNF0_1 |
    							 GPIO_CRL_MODE1 | GPIO_CRL_CNF1_1);

}

static void spwm_table_rebuild(float gain)
{
    for (uint16_t i = 0; i < spwm_points; i++) {
        float angle = (2.0f * M_PI * i) / spwm_points;

        float duty = 0.5f + gain * (sinf(angle) / 2.0f);
        sin_table[i] = (uint16_t)(10 + duty * (ARR_VALUE - 20));
    }
}

static void spwm_table_init()
{
    spwm_table_rebuild(0.0f);
}

void spwm_init()
{
    spwm_gpio_init();
    spwm_table_init();

    RCC->APB2ENR |= RCC_APB2ENR_TIM1EN;

    AFIO->MAPR |= AFIO_MAPR_TIM1_REMAP_0; // CH1N on PA7, CH2N on PB0, CH3N on PB1

    TIM1->PSC  = 0;
    TIM1->ARR  = ARR_VALUE - 1;

    TIM1->CCR1 = ARR_VALUE / 2;
    TIM1->CCR2 = ARR_VALUE / 2;
    TIM1->CCR3 = ARR_VALUE / 2;

    TIM1->CR2   |= TIM_CR2_OIS1 | TIM_CR2_OIS2 | TIM_CR2_OIS3;

    TIM1->CCMR1 |= TIM_CCMR1_OC1PE | TIM_CCMR1_OC1M | // PWM mode 2 on CH1
    							 TIM_CCMR1_OC2PE | TIM_CCMR1_OC2M; 	// PWM mode 2 on CH2

    TIM1->CCMR2 |= TIM_CCMR2_OC3PE | TIM_CCMR2_OC3M; // PWM mode 2 on CH3

    TIM1->CCER  |= TIM_CCER_CC1E | TIM_CCER_CC1NE | // CH1 enabled
    							 TIM_CCER_CC2E | TIM_CCER_CC2NE |	// CH2 enabled
									 TIM_CCER_CC3E | TIM_CCER_CC3NE;  // CH3 enabled

    TIM1->BDTR  |= (DEADTIME_VAL << TIM_BDTR_DTG_Pos); // set dead-time!

    NVIC_EnableIRQ(TIM1_UP_IRQn);
}

void spwm_start()
{

    softstart_gain       = 0.0f;
    softstart_done       = 0;
    softstart_period_cnt = 0;
    spwm_table_rebuild(0.0f);
    ind = 0;

    TIM1->DIER |= TIM_DIER_UIE;
    TIM1->BDTR |= TIM_BDTR_MOE;
    TIM1->CR1  |= TIM_CR1_CEN;
}

void spwm_stop()
{
    TIM1->DIER &= ~TIM_DIER_UIE;
    TIM1->BDTR &= ~TIM_BDTR_MOE;
    TIM1->CR1  &= ~TIM_CR1_CEN;
}

void spwm_set_amplitude(float amp)
{
		amp_ = amp;
    if (amp < 0.0f) amp = 0.0f;
    if (amp > 1.0f) amp = 1.0f;

    /* проверка на мягкий старт */
    softstart_done = 1;
    softstart_gain = amp;

    spwm_table_rebuild(amp);
}

void spwm_set_phase(uint16_t degree)
{
		ph_ = degree;
		offset_ph_b = (uint16_t)(spwm_points * degree / 360.0);
		offset_ph_c = 2 * offset_ph_b;
}

void spwm_set_freq(uint16_t freq_hz)
{
		freq_ = freq_hz;

		// служебная остановка для пересчета таблицы
		spwm_stop();
		spwm_points = PWM_FREQUENCY_HZ / freq_hz;

		// пересчет значения сдвигов
		spwm_set_phase(ph_);

		// старт spwm
		spwm_start();

		// установка последнего выбранного кф амплитуды
		spwm_set_amplitude(amp_);

}


void TIM1_UP_IRQHandler()
{
    TIM1->SR &= ~TIM_SR_UIF;

    TIM1->CCR1 = sin_table[ind];
    TIM1->CCR2 = sin_table[(ind + offset_ph_b) % spwm_points];
    TIM1->CCR3 = sin_table[(ind + offset_ph_c) % spwm_points];
    ind++;

    if (ind == spwm_points) {
        ind = 0;


        if (!softstart_done) {
            softstart_period_cnt++;

            if (softstart_period_cnt >= SOFTSTART_STEP_PERIODS) {
                softstart_period_cnt = 0;
                softstart_gain += SOFTSTART_STEP;

                if (softstart_gain >= amp_) {
                    softstart_gain = amp_;
                    softstart_done = 1;
                }

                spwm_table_rebuild(softstart_gain);
            }
        }
    }
}
