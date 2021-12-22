#include <zephyr.h>
#include <device.h>
#include <devicetree.h>
#include <drivers/gpio.h>
#include <sys/printk.h>
#include <usb/usb_device.h>
#include <drivers/uart.h>

/* 1000 msec = 1 sec */
#define SLEEP_TIME_MS   1000

/* The devicetree node identifier for the "led0" alias. */
#define LED0_NODE DT_ALIAS(led0)

#if DT_NODE_HAS_STATUS(LED0_NODE, okay)
#define LED0	DT_GPIO_LABEL(LED0_NODE, gpios)
#define PIN	DT_GPIO_PIN(LED0_NODE, gpios)
#define FLAGS	DT_GPIO_FLAGS(LED0_NODE, gpios)
#else
/* A build error here means your board isn't set up to blink an LED. */
#error "Unsupported board: led0 devicetree alias is not defined"
#define LED0	""
#define PIN	0
#define FLAGS	0
#endif

BUILD_ASSERT(DT_NODE_HAS_COMPAT(DT_CHOSEN(zephyr_console), zephyr_cdc_acm_uart),
	     "Console device is not ACM CDC UART device");


void main(void)
{
    /* start led config */
    const struct device *led;
	bool led_is_on = true;
	int ret;

	led = device_get_binding(LED0);
	if (led == NULL) {
		//gpio_pin_set(led, PIN, true);
		return;
	}

	ret = gpio_pin_configure(led, PIN, GPIO_OUTPUT_ACTIVE | FLAGS);
	if (ret < 0) {
		//gpio_pin_set(led, PIN, true);
		return;
	}
    /* end led config */ 

    /* start usb cdc acm comm */
    const struct device *dev = DEVICE_DT_GET(DT_CHOSEN(zephyr_console));
	uint32_t dtr = 0;

    if (usb_enable(NULL)) {
		//gpio_pin_set(led, PIN, true);
		return;
	}

    /* Poll if the DTR flag was set */
	uart_line_ctrl_get(dev, UART_LINE_CTRL_DTR, &dtr);
	while (false) {
		uart_line_ctrl_get(dev, UART_LINE_CTRL_DTR, &dtr);
		gpio_pin_set(led, PIN, led_is_on);
		led_is_on = !led_is_on;
		/* Give CPU resources to low priority threads. */
		k_sleep(K_MSEC(100));
    }
    /* end usb cdc acm comm */

	while (1) {
		gpio_pin_set(led, PIN, (int)led_is_on);
        printk("Hello, world %s\n", CONFIG_BOARD);
		led_is_on = !led_is_on;
		k_msleep(SLEEP_TIME_MS);
	}
}

