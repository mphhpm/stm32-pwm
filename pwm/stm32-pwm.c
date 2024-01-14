//
//    Copyright (c) 2024 mphhpm.
//    Permission is hereby granted, free of charge, to any person obtaining
//    a copy of this software and associated documentation files (the "Software"),
//    to deal in the Software without restriction, including without limitation
//    the rights to use, copy, modify, merge, publish, distribute, sublicense,
//    and/or sell copies of the Software, and to permit persons to whom the Software
//    is furnished to do so, subject to the following conditions:
//
//    The above copyright notice and this permission notice shall be included in
//    all copies or substantial portions of the Software.
//
//    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
//    EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
//    OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
//    IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
//    CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
//    TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE
//    OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
//
//    Changes
//    1-1-2024: mphhpm: initial version

#include <libopencm3/cm3/nvic.h>
#include <libopencm3/cm3/systick.h>

#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/timer.h>
#include <libopencm3/stm32/dma.h>

void setup_clocks(void);
void setup_dma(uint32_t dmaUnit, uint8_t stream, uint32_t channel);
void setup_gpio(void);
void setup_timer(void);
void setup_systick(void);

#ifndef ARRAY_LEN
#define ARRAY_LEN(array) (sizeof((array))/sizeof((array)[0]))
#endif

volatile uint32_t systicks = 0;
void sys_tick_handler(void)
{
	systicks++;
}

volatile uint32_t dma_stream_complete = 0;
uint8_t  dmaStream  = DMA_STREAM1;
uint32_t dmaChannel = DMA_SxCR_CHSEL_6;
uint32_t dmaUnit    = DMA2;
//
// sequence of values easily recognizable on a scope
const uint16_t dma_buffer[] = { 100, 350, 500, 350 };
//
// save the data being transferred to the timer for debugging purposes
uint8_t  idx = 0;
uint16_t dma_tim1_ccr1[ARRAY_LEN(dma_buffer)];

void dma2_stream1_isr()
{
	dma_stream_complete |= 1 << 9;
	dma_clear_interrupt_flags(DMA2, 1, DMA_TCIF);
	dma_tim1_ccr1[idx]=TIM_CCR1(TIM1);
	idx = ((idx+1) % ARRAY_LEN(dma_tim1_ccr1));
}

void setup_systick(void) {
	//
	// use internal clock source
	systick_set_clocksource(STK_CSR_CLKSOURCE);
	//
	// ticks count miliseconds
	systick_set_reload(84*1000-1);
	systick_interrupt_enable();
	systick_counter_enable();
}

void setup_clocks(void) {
    rcc_clock_setup_pll(&rcc_hse_25mhz_3v3[RCC_CLOCK_3V3_84MHZ]);
}

void setup_gpio(void) {
	//
	// supply clock to peripheral unit
    rcc_periph_clock_enable(RCC_GPIOA);
    rcc_periph_clock_enable(RCC_GPIOC);
    //
    // timer output compare connects to GPIO A
    gpio_mode_setup(GPIOA, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO8);
    //
    // set alternate function according to reference manual
    gpio_set_af(GPIOA, GPIO_AF1, GPIO8);
    //
    // assume the led is at this pin
    gpio_mode_setup(GPIOC, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO13);
}


void setup_timer(void) {
	//
	// supply clock to peripheral units
    rcc_periph_clock_enable(RCC_TIM1);
    rcc_periph_reset_pulse(RST_TIM1);
    //
    // stop timer during initialisation
    timer_disable_counter(TIM1);
    //
    // internal clock with divisor 1, edge aligned, up counting
    timer_set_mode(TIM1, TIM_CR1_CKD_CK_INT, TIM_CR1_CMS_EDGE, TIM_CR1_DIR_UP);
    //
    // assuming 84 Mhz
    timer_set_prescaler(TIM1, 83); // Timer clock is 1 MHz
    //
    // enale preload
    timer_enable_preload(TIM1);
    //
    // let it run forever
    timer_continuous_mode(TIM1);
    //
    // PWM period: 1 kHz
    timer_set_period(TIM1, 1000);
    //
    // Configure channel 1 as PWM output
    timer_set_oc_mode(TIM1, TIM_OC1, TIM_OCM_PWM2);
    //
    // configure as output compare
    timer_enable_oc_output(TIM1, TIM_OC1);
    //
    // use preload
    timer_enable_oc_preload(TIM1, TIM_OC1);
    //
    // set initial duty cycle
    timer_set_oc_value(TIM1, TIM_OC1, dma_buffer[0]);
    //
    // enble generation of dma requets
    timer_enable_irq(TIM1, TIM_DIER_CC1DE);
    //
    // connect timer to output pin
    timer_enable_break_main_output(TIM1);
    //
    // define when to request dma
    timer_set_dma_on_compare_event(TIM1);
    //
    // let it run
    timer_enable_counter(TIM1);
}

void setup_dma(uint32_t dmaController, uint8_t stream, uint32_t channel) {
	//
	// stop controller during configuration
    dma_disable_stream(dmaController, stream);
    //
    // all values to reset state
    dma_stream_reset(dmaController, stream);
    //
    // use double buffered mode
    dma_enable_fifo_mode(dmaController, stream);
    //
    // data is being copied from memory to peripheral
	dma_set_transfer_mode(dmaController, stream, DMA_SxCR_DIR_MEM_TO_PERIPHERAL);
	//
	// destination is tim1 ccr1
	dma_set_peripheral_address(dmaController, stream, (uint32_t)&TIM1_CCR1);
	//
	// source is a memory buffer
    dma_set_memory_address(dmaController, stream, (uint32_t)dma_buffer);
    //
    // 16-bit (2 bytes) items will be copied to the destnation
    dma_set_memory_size(dmaController, stream, DMA_SxCR_MSIZE_16BIT);
    //
    // continue, don't stop after first cycle
    dma_enable_circular_mode(dmaController, stream);
    //
    // determine channel to be used between timer channel and dma controller
    dma_channel_select(dmaController, stream, channel);
    //
    // determine the number of data items are to be transferred
    dma_set_number_of_data(dmaController, stream, ARRAY_LEN(dma_buffer));
    //
    // increment memory pointer after data transfer has been completed
    dma_enable_memory_increment_mode(dmaController, stream);
    //
    // 16-bit (2 bytes) items will be copied from the source
    dma_set_peripheral_size(dmaController, stream, DMA_SxCR_PSIZE_16BIT);
    //
    // destination address needs to be unchanged
    dma_disable_peripheral_increment_mode(dmaController, stream);
    //
    // report back when a transfer has been completed
    dma_enable_transfer_complete_interrupt(dmaController, stream);
    //
    // let it run
    dma_enable_stream(dmaController, stream);
}

int main(void) {
	//
	// initialise peripherals
    rcc_periph_clock_enable(RCC_DMA2);
    setup_clocks();
    setup_systick();
    setup_gpio();
    setup_timer();
    setup_dma(dmaUnit, dmaStream, DMA_SxCR_CHSEL_6);

    int flashes = 1;
    int totalFlashCount = 0;
    uint32_t  onTime = 50;
    uint32_t  shortOffTime = 5*onTime;
    uint32_t  offTime = 3000 - (flashes*(onTime+shortOffTime));
    uint32_t  duration = onTime;
    uint32_t  ticks = systicks;
    int flashCount = 0;
    //
    // enable dma interrupt
    nvic_enable_irq(NVIC_DMA2_STREAM1_IRQ);

    while(1) {
    	//
    	// report back that dma is working by generating 5 flashes
    	flashes = dma_stream_complete > 0 ? 5:1;
        offTime = 3000 - (flashes*(onTime+shortOffTime));
        if (systicks > ticks) {
        	//
        	// period elapsed
			if (duration == shortOffTime && ++flashCount == flashes) {
				//
				// number of short flashes reached, start over with long time off
				duration = offTime;
				gpio_set(GPIOC, GPIO13);
				flashCount = 0;
			}
			else if (duration == onTime) {
				//
				// end of short flash reached, start over with short time off
				duration = shortOffTime;
				gpio_set(GPIOC, GPIO13);
			}
			else if (duration == shortOffTime) {
				//
				// end of short time off reached, tart over with short time on
				duration = onTime;
				gpio_clear(GPIOC, GPIO13);
			}
			else {
				duration = onTime;
				gpio_clear(GPIOC, GPIO13);
			}
        	ticks = systicks+duration;
        }
    }
}
