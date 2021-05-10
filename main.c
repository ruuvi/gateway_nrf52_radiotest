#include "app_error.h"
#include "app_uart.h"
#include "bsp.h"
#include "nordic_common.h"
#include "nrf.h"
#include "nrf_cli.h"
#include "nrf_cli_uart.h"
#include "nrf_drv_clock.h"
#include "nrf_delay.h"
#include "nrf_gpio.h"
#include "radio_test.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"

#define LED_PIN 13
#define RB_PA_CSD_PIN 22
#define RB_PA_CRX_PIN 23

static uint8_t m_mode = RADIO_MODE_MODE_Ble_1Mbit;     //<-- Radio mode. Data rate and modulation.
static uint8_t m_txpower = RADIO_TXPOWER_TXPOWER_0dBm; //<-- Radio output power.
static uint8_t m_channel_low    = 0;                   //<-- Radio channel 0 (2402 MHz).
static uint8_t m_channel_medium = 40;                  //<-- Radio channel 40 (2440 MHz).
static uint8_t m_channel_high   = 80;                  //<-- Radio channel 80 (2480 MHz).

static void log_init(void) {
  ret_code_t err_code = NRF_LOG_INIT(NULL);
  APP_ERROR_CHECK(err_code);
  NRF_LOG_DEFAULT_BACKENDS_INIT();
}

static void init(void) {
    nrf_gpio_cfg (LED_PIN,
                NRF_GPIO_PIN_DIR_OUTPUT,
                NRF_GPIO_PIN_INPUT_DISCONNECT,
                NRF_GPIO_PIN_NOPULL,
                NRF_GPIO_PIN_H0H1,
                NRF_GPIO_PIN_NOSENSE);
  nrf_gpio_pin_clear(LED_PIN);

  nrf_gpio_cfg (RB_PA_CSD_PIN,
                NRF_GPIO_PIN_DIR_OUTPUT,
                NRF_GPIO_PIN_INPUT_DISCONNECT,
                NRF_GPIO_PIN_NOPULL,
                NRF_GPIO_PIN_H0H1,
                NRF_GPIO_PIN_NOSENSE);
  nrf_gpio_pin_set(RB_PA_CSD_PIN);

  nrf_gpio_cfg (RB_PA_CRX_PIN,
                NRF_GPIO_PIN_DIR_OUTPUT,
                NRF_GPIO_PIN_INPUT_DISCONNECT,
                NRF_GPIO_PIN_NOPULL,
                NRF_GPIO_PIN_H0H1,
                NRF_GPIO_PIN_NOSENSE);
  NRF_RNG->TASKS_START = 1;

// Cache profiling, disable
#if 0
  NRF_NVMC->ICACHECNF = NVMC_ICACHECNF_CACHEEN_Enabled << NVMC_ICACHECNF_CACHEEN_Pos;
#endif

  // Start 64 MHz crystal oscillator.
  NRF_CLOCK->EVENTS_HFCLKSTARTED = 0;
  NRF_CLOCK->TASKS_HFCLKSTART = 1;

  toggle_dcdc_state(true);

  // Wait for the external oscillator to start up.
  while (NRF_CLOCK->EVENTS_HFCLKSTARTED == 0) {
    // Do nothing.
  }

}

static void test_cycle(void) {
  nrf_gpio_pin_set(RB_PA_CRX_PIN);
  nrf_gpio_pin_clear(LED_PIN);
  radio_unmodulated_tx_carrier(m_txpower, m_mode, m_channel_low);
  nrf_delay_ms(60000);
  radio_unmodulated_tx_carrier(m_txpower, m_mode, m_channel_medium);
  nrf_delay_ms(60000);
  radio_unmodulated_tx_carrier(m_txpower, m_mode, m_channel_high);
  nrf_delay_ms(60000);
  nrf_gpio_pin_clear(RB_PA_CRX_PIN);
  nrf_gpio_pin_set(LED_PIN);
  nrf_delay_ms(60000);
}

void TIMER1_IRQHandler(void) {
  if (NRF_TIMER1->EVENTS_COMPARE[0] == 1) {
    NRF_TIMER1->EVENTS_COMPARE[0] = 0;
  }
}

int main(void) {
  ret_code_t err_code;

  err_code = nrf_drv_clock_init();
  APP_ERROR_CHECK(err_code);
  nrf_drv_clock_lfclk_request(NULL);

  err_code = app_timer_init();
  APP_ERROR_CHECK(err_code);

  init();
  log_init();

  NVIC_EnableIRQ(TIMER0_IRQn);
  NVIC_EnableIRQ(TIMER1_IRQn);
  __enable_irq();

  while (true) {
    test_cycle();
  }
}