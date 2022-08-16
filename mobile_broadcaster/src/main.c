#include <zephyr/types.h>
#include <stddef.h>
#include <sys/printk.h>
#include <sys/util.h>

#include <bluetooth/bluetooth.h>
#include <bluetooth/hci.h>
#include <timing/timing.h>

#define SENDER_ID_IDX 0
#define MSG_TYPE_IDX 1
#define DST_ADDR_IDX 2
#define RCV_ADDR_IDX 3
#define TIME_STAMP_MSB_IDX 4
#define TIME_STAMP_LSB_IDX 5
#define BROADCAST_ADDR 0x7F // 127

static uint8_t mfg_data[] = {0x01, 0x01, 0x01, 0x01, 0x01 ,0x01, 0x01, 0x01}; 

void main(void)
{
    int err;

	printk("Starting Broadcaster\n");
    
    // Initialize BLE 
    err = bt_enable(NULL);
	if (err) {
		printk("Bluetooth init failed (err %d)\n", err);
		return;
	}

    mfg_data[SENDER_ID_IDX] = 0xFF;
    mfg_data[RCV_ADDR_IDX] = BROADCAST_ADDR; // Broadcast addr 
    mfg_data[DST_ADDR_IDX] = 0x00; // Destination addr
    mfg_data[MSG_TYPE_IDX] = 0x1; // MSG_TYPE_DATA 
    
    static const struct bt_data ad[] = {
	    BT_DATA(BT_DATA_NAME_COMPLETE, mfg_data, 8)};
   
    do{
        uint8_t timestamp_lower, timestamp_upper;
        uint32_t cycles32 = k_cycle_get_32();

        // Get only lower 16 bits
        timestamp_lower = 0x00FF & cycles32;
        timestamp_upper = (0xFF00 & cycles32) >> 8;
        mfg_data[TIME_STAMP_MSB_IDX] = timestamp_upper;
        mfg_data[TIME_STAMP_LSB_IDX] = timestamp_lower;

        err = bt_le_adv_start(BT_LE_ADV_NCONN_IDENTITY, 
                ad, ARRAY_SIZE(ad),
                NULL, 0);

        if (err) {
            printk("Advertising failed to start (err %d)\n", err);
            return;
        }

        k_msleep(100);

        err = bt_le_adv_stop();
        if (err) {
            printk("Advertising failed to stop (err %d)\n", err);
            return;
        }
        printk("Advertised\n");
        
        k_msleep(100);
        //mfg_data[5]++;
    }while(true);

    printk("MOBILE BROADCASTER ADV START\n");
}

