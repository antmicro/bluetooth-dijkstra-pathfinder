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

uint8_t mfg_data[] = {0x01, 0x01, 0x01, 0x01, 0x01 ,0x01, 0x01, 0x01}; 
const struct bt_data ad[] = {BT_DATA(0xff, mfg_data, 8)//TODO is it overwritten
};


/* Callbacks */
void ble_scanned(struct bt_le_ext_adv *adv,
        struct bt_le_ext_adv_scanned_info *info){
    // adv - advertising set obj
    // info - address adn type
    printk("Scanned \n");
}


void ble_sent(struct bt_le_ext_adv *adv, 
        struct bt_le_ext_adv_sent_info *info){
    printk("Sent adv data \n");
}


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

    // Receiver address 
    bt_addr_le_t receiver_addr = {
        .type = BT_ADDR_LE_RANDOM,
        .a = { // set it to receiver addr 
            //      LSB                           MSB
			//.val = {0x01, 0x00, 0x00, 0x00, 0x00 ,0xC0} 
			.val = {0x02, 0x00, 0x00, 0x00, 0x00 ,0xC0} 
        }
    };
    char r_identity[129];
    bt_addr_le_to_str(&receiver_addr, r_identity, sizeof(r_identity)); 
    printk("Sending to node with identity: %s\n", r_identity); 
    printk("Sending to node with mesh id: 2\n");

    // Advertisement setup 
    uint32_t ext_adv_aptions = 
        BT_LE_ADV_OPT_EXT_ADV
        + BT_LE_ADV_OPT_DIR_MODE_LOW_DUTY
        + BT_LE_ADV_OPT_SCANNABLE
        + BT_LE_ADV_OPT_NOTIFY_SCAN_REQ;

    struct bt_le_adv_param params = {
        .id = 0x0,
        .options = ext_adv_aptions,
        .peer = &receiver_addr,
        .interval_min = BT_GAP_ADV_FAST_INT_MIN_1, 
        .interval_max = BT_GAP_ADV_FAST_INT_MAX_1 
    };

    static struct bt_le_ext_adv_cb adv_callbacks = {
        .sent = ble_sent,
        .scanned = ble_scanned
    };

    // create advertising set
	struct bt_le_ext_adv *adv;
	err = bt_le_ext_adv_create(
            &params, &adv_callbacks, &adv);
    if(err){
        printk("Failed to create advertising set (err %d)\n",
                err); 
        return;
    }
    
    // Data setup
    // First byte is ***mesh address*** of final destination 
    //uint8_t mfg_data[9]; TODO: here add some data, for now its whatever
    //memcpy(&mfg_data[1], mfg_data, sizeof(mfg_data));
    
    mfg_data[0] = 0x00;
    const struct bt_data ad[] = {BT_DATA(0xff, mfg_data, 8)};

    // set advertising set ad and sd 
    err = bt_le_ext_adv_set_data(adv, 
            NULL, 0, // When scannable then ad == NULL
            ad, ARRAY_SIZE(ad));

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
}
