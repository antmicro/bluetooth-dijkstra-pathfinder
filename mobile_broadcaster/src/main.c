/* main.c - Application main entry point */

/*
 * Copyright (c) 2015-2016 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/types.h>
#include <stddef.h>
#include <sys/printk.h>
#include <sys/util.h>

#include <bluetooth/bluetooth.h>
#include <bluetooth/hci.h>
#include <timing/timing.h>

uint8_t mfg_data2[] = {0x01, 0x01, 0x01, 0x01, 0x01 ,0x01, 0x01, 0x01}; 
const struct bt_data ad2[] = {BT_DATA(0xff, mfg_data2, 8)
};


void main(void)
{
    /* 
    // Timing setup 
    timing_t start_time, end_time;
    uint64_t time_in_ns;
    uint64_t cycles; 
    
    // start time measurement
    timing_init();
    timing_start();
    start_time = timing_counter_get();
    */
	int err;

	printk("Starting Broadcaster\n");

    // Initialize BLE 
    err = bt_enable(NULL);
	if (err) {
		printk("Bluetooth init failed (err %d)\n", err);
		return;
	}

    // Receiver address 
    bt_addr_le_t receiver_addr = {
        .type = BT_ADDR_LE_RANDOM,
        .a = { // set it to receiver addr 
            //      LSB                           MSB
			.val = {0x01, 0x00, 0x00, 0x00, 0x00 ,0xC0} 
			//.val = {0x22, 0x00, 0x00, 0x00, 0x00 ,0xC0} 
        }
    };
    char r_identity[129];
    bt_addr_le_to_str(&receiver_addr, r_identity, sizeof(r_identity)); 
    printk("Sending to node with identity: %s\n", r_identity); 
    printk("Sending to node with mesh id: 2\n");

    // Advertisement setup 
    uint32_t ext_adv_aptions = 
        BT_LE_ADV_OPT_EXT_ADV
        + BT_LE_ADV_OPT_DIR_MODE_LOW_DUTY;  // TODO:remove that ??
        //+ BT_LE_ADV_OPT_NOTIFY_SCAN_REQ;
        //+ BT_LE_ADV_OPT_CONNECTABLE;
        //+ BT_LE_ADV_OPT_DIR_MODE_LOW_DUTY;

    struct bt_le_adv_param params = {
        .id = 0x0,
        .options = ext_adv_aptions,
        .peer = &receiver_addr,
        .interval_min = BT_GAP_ADV_FAST_INT_MIN_1, //TODO: advertising interval bigger?
        .interval_max = BT_GAP_ADV_FAST_INT_MAX_1 
    };

    // create advertising set
	struct bt_le_ext_adv *adv;
	err = bt_le_ext_adv_create(
            &params, NULL, &adv);
    if(err){
        printk("Failed to create advertising set (err %d)\n",
                err); 
        return;
    }
    
    // get time in ns as 64b int 
    /*
    end_time = timing_counter_get(); 
    cycles = timing_cycles_get(&start_time, &end_time);
    time_in_ns = timing_cycles_to_ns(cycles); 
    */
    //printk("Measured start time: %llu, cycles: %llu\n", time_in_ns, cycles); 

    // Data setup
    // First byte is ***mesh address*** of final destination 
    // Giving time stamp to the packet 
    uint8_t mfg_data[9]; 
    memcpy(&mfg_data[1], mfg_data2, sizeof(mfg_data2));
    mfg_data[0] = 0x00; 
    const struct bt_data ad[] = {BT_DATA(0xff, mfg_data, 9)
    };

    // set advertising set ad and sd 
    err = bt_le_ext_adv_set_data(adv, ad, ARRAY_SIZE(ad),
           NULL, 0);
    if(err){
        printk("Failed to set adv data (err %d)\n",
                err); 
        return;
    }

    // start advertising
    err = bt_le_ext_adv_start(adv,
        BT_LE_EXT_ADV_START_PARAM(0, 20));
        //BT_LE_EXT_ADV_START_DEFAULT);
    if (err) {
        printk("Advertising failed to start (err %d)\n", err);
        return;
    }

    printk("MOBILE BROADCASTER ADV START\n");

    // wait some time to finish 	
    k_msleep(100);
    
    // stop it
    /*err = bt_le_adv_stop();
    if (err) {
        printk("Advertising failed to stop (err %d)\n", err);
        return;
    }*/
    /* 
    end_time = timing_counter_get(); 
    cycles = timing_cycles_get(&start_time, &end_time);
    time_in_ns = timing_cycles_to_ns(cycles); 
    //printk("Measured start time: %llu, cycles: %llu \n", time_in_ns, cycles); 
    
    uint8_t mfg_data2[9]; 
    memcpy(&mfg_data2[1], &time_in_ns, sizeof(time_in_ns));
    mfg_data2[0] = 0x00; 
    const struct bt_data ad2[] = {BT_DATA(0xff, mfg_data2, 9)
    };
    */ 
    // set new data to adv set 
    // 
    /*
    err = bt_le_ext_adv_set_data(adv, ad2, ARRAY_SIZE(ad2),
            NULL, 0);
    if(err){
        printk("Failed to set data for adv (err %d)\n",
                err); 
        return;
    }
    */
    
    //printk("Second message advertising...\n");

    /*
    do{
        err = bt_le_ext_adv_start(adv,
            //BT_LE_EXT_ADV_START_DEFAULT); 
            BT_LE_EXT_ADV_START_PARAM(0, 15));
    }while(err);
    
    if(err){
        printk("Failed to start adv (err %d)\n",
                err); 
        return;
    }
    */
	
	do {
		k_msleep(100);

	} while (1);
}
