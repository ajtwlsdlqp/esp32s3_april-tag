/*
 * ap_spiffs.c
 *
 *  Created on: Sep 28, 2021
 *      Author: Jason
 */




#include <string.h>
#include "esp_spiffs.h"
#include "esp_log.h"
#include "hw.h"



const char *TAG="spiffs";


void cliSpiffs(cli_args_t *args);

esp_err_t init_spiffs(void)
{
    ESP_LOGI(TAG, "Initializing SPIFFS");

    esp_vfs_spiffs_conf_t conf = {
      .base_path = "/spiffs",
      .partition_label = "spiffs",
      .max_files = 5,   // This decides the maximum number of files that can be created on the storage
      .format_if_mount_failed = true
    };

    esp_err_t ret = esp_vfs_spiffs_register(&conf);
    if (ret != ESP_OK) {
        if (ret == ESP_FAIL) {
            ESP_LOGE(TAG, "Failed to mount or format filesystem");
        } else if (ret == ESP_ERR_NOT_FOUND) {
            ESP_LOGE(TAG, "Failed to find SPIFFS partition");
        } else {
            ESP_LOGE(TAG, "Failed to initialize SPIFFS (%s)", esp_err_to_name(ret));
        }
        return ESP_FAIL;
    }

    size_t total = 0, used = 0;
    ret = esp_spiffs_info("spiffs", &total, &used);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to get SPIFFS partition information (%s)", esp_err_to_name(ret));
        return ESP_FAIL;
    }

    ESP_LOGI(TAG, "Partition size: total: %d, used: %d", total, used);

#ifdef _USE_HW_CLI
    cliAdd("spiffs", cliSpiffs);
#endif


    // Open renamed file for reading
    ESP_LOGI(TAG, "Reading file");
    FILE* f = fopen("/spiffs/index.htm", "r");
    if (f == NULL) {
        ESP_LOGE(TAG, "Failed to open file for reading");
        return ESP_FAIL;
    }
    char line[200];
    for (int i = 0; i < sizeof(line); i++) line[i] = 0x30;
    fseek(f, 0L, SEEK_END);
    int file_len = ftell(f);
    rewind(f);

    fread(line, sizeof(line), 1, f);
    fclose(f);
    cliPrintf("file length is %d.\n", file_len);
    cliPrintf("-----------------------\n");
//    cliPrintf("%s", line);

//    // All done, unmount partition and disable SPIFFS
//    esp_vfs_spiffs_unregister(conf.partition_label);
//    ESP_LOGI(TAG, "SPIFFS unmounted");

    return ESP_OK;
}





#ifdef _USE_HW_CLI
void cliSpiffs(cli_args_t *args)
{
  bool ret = false;


  if (args->argc == 1 && args->isStr(0, "info") == true)
  {
    while(cliKeepLoop())
    {
      size_t total = 0, used = 0;
      ret = esp_spiffs_info(NULL, &total, &used);
      if (ret != ESP_OK) {
        cliPrintf("Failed to get SPIFFS partition information (%s)", esp_err_to_name(ret));
      }
      cliPrintf("Partition size: total: %d, used: %d", total, used);
      delay(100);
    }
    ret = true;
  }

  if (args->argc == 1 && args->isStr(0, "list") == true)
  {
    while(cliKeepLoop())
    {
      delay(100);
    }

    ret = true;
  }

  if (ret != true)
  {
    cliPrintf("spiffs info\n");
    cliPrintf("spiffs list\n");
  }
}
#endif

