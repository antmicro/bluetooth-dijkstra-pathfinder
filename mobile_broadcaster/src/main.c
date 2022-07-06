#include <zephyr/types.h>
#include <stddef.h>
#include <sys/printk.h>
#include <sys/util.h>

#include <bluetooth/bluetooth.h>
#include <bluetooth/hci.h>
#include <timing/timing.h>

#define MSG_TYPE_IDX 0
#define DST_ADDR_IDX 1
#define RCV_ADDR_IDX 2
#define BROADCAST_ADDR 0x7F // 127

uint8_t mfg_data[] = {0x01, 0x01, 0x01, 0x01, 0x01 ,0x01, 0x01, 0x01}; 
uint8_t mfg_data2[] = {0x02, 0x02, 0x02, 0x02, 0x02 ,0x02, 0x02, 0x02}; 

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

    // Data setup
    // First byte is ***mesh address*** of final destination 
    //uint8_t mfg_data[9]; TODO: here add some data, for now its whatever
    //memcpy(&mfg_data[1], mfg_data, sizeof(mfg_data));
    
    mfg_data[RCV_ADDR_IDX] = BROADCAST_ADDR; // Broadcast addr 
    mfg_data[DST_ADDR_IDX] = 0x00; // Destination addr
    mfg_data[MSG_TYPE_IDX] = 0x1; // MSG_TYPE_DATA 
    const struct bt_data ad[] = {
	    BT_DATA(BT_DATA_NAME_COMPLETE, mfg_data, 8)};
    const struct bt_data ad2[] = {
	    BT_DATA(BT_DATA_NAME_COMPLETE, mfg_data2, 8)};
        
    do{
        err = bt_le_adv_start(BT_LE_ADV_NCONN_IDENTITY, 
                ad, ARRAY_SIZE(ad),
                NULL, 0);

        if (err) {
            printk("Advertising failed to start (err %d)\n", err);
            return;
        }

        k_msleep(200);

        err = bt_le_adv_stop();
        if (err) {
            printk("Advertising failed to stop (err %d)\n", err);
            return;
        }
        printk("Advertised\n");
        
        k_msleep(1000);
        //mfg_data[5]++;
         
        err = bt_le_adv_start(BT_LE_ADV_NCONN_IDENTITY, ad2, ARRAY_SIZE(ad2), NULL, 0);
        k_msleep(200);

        err = bt_le_adv_stop();
        if (err) {
            printk("Advertising failed to stop (err %d)\n", err);
            return;
        }
        
        k_msleep(1000);
        //mfg_data[5]++;
        

    }while(true);

    printk("MOBILE BROADCASTER ADV START\n");
}

