// #include "freertos/FreeRTOS.h"
// #include "freertos/task.h"
// #include "esp_bt.h"
// #include "esp_gap_ble_api.h"
// #include "esp_gattc_api.h"
// //#include "include/api/esp_gap_ble_api.h"
// //#include "include/api/esp_gattc_api.h"

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_nimble_hci.h"  // Ensure this header is included
#include "nimble/nimble_port.h"
#include "nimble/nimble_port_freertos.h"
#include "host/ble_hs.h"
#include "host/util/util.h"

static void ble_app_on_sync(void) {
    ble_hs_id_infer_auto(0, NULL);
    printf("BLE: Stack initialized and synced\n");
}

static void host_task(void *param) {
    nimble_port_run();
    nimble_port_freertos_deinit();
}

void app_main(void) {
    // Use the correct initialization function
    esp_err_t ret = esp_nimble_hci_init();  // Corrected function name
    if (ret != ESP_OK) {
        printf("Failed to initialize controller: %d\n", ret);
        return;
    }

    nimble_port_init();
    ble_hs_cfg.sync_cb = ble_app_on_sync;
    nimble_port_freertos_init(host_task);

    printf("BLE (NimBLE): Initialized\n");
}