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

#define STACK_SIZE 500
#define USB_SEND_PRIORITY 5

void usb_send(){
    printk("Hello, world %s\n", CONFIG_BOARD);
}

//k_tid_t usb_send_data_tid;
K_THREAD_DEFINE(usb_send_data_tid, STACK_SIZE, usb_send, 
        NULL, NULL, NULL,USB_SEND_PRIORITY, 0, 0); 



void main(void)
{
    /* start led config */
    const struct device *led;
	bool led_is_on = true;
	int ret;

	led = device_get_binding(LED0);
	if (led == NULL) {
		return;
	}

	ret = gpio_pin_configure(led, PIN, GPIO_OUTPUT_ACTIVE | FLAGS);
	if (ret < 0) {
		return;
	}
    /* end led config */ 

    /* start usb cdc acm comm */
    const struct device *dev = DEVICE_DT_GET(DT_CHOSEN(zephyr_console));
	uint32_t dtr = 0;

    if (usb_enable(NULL)) {
		return;
	}
    
	uart_line_ctrl_get(dev, UART_LINE_CTRL_DTR, &dtr);
	k_msleep(2000);
    /* end usb cdc acm comm */

    /* start data through usb sending thread */
        /* end data through usb sending thread */

	while (1) {
		gpio_pin_set(led, PIN, (int)led_is_on);
        //printk("Hello, world %s\n", CONFIG_BOARD);
		led_is_on = !led_is_on;
		k_msleep(SLEEP_TIME_MS);
	}
}

