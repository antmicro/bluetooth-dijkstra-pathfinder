#include <zephyr.h>
//#include <device.h>
//#include <devicetree.h>
//#include <drivers/gpio.h>
#include <sys/printk.h>
//#include <usb/usb_device.h>
#include <drivers/uart.h>
//#include <string.h>

/* 1000 msec = 1 sec */
#define SLEEP_TIME_MS   1000

/* The devicetree node identifier for the "led0" alias. */

void main(void)
{
    int counter = 0;
	while (1) {
        const struct device *dev;
        dev = DEVICE_DT_GET(DT_CHOSEN(zephyr_console));
        uart_poll_out(dev, 'A');
        
        printk("Hello, world %d\n", counter);
        counter++;
		k_msleep(SLEEP_TIME_MS);
	}
}

