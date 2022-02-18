#include "ota.h"

void otaSetup()
{
	//xTaskCreate(&checkUpdateTask, "checkUpdateTask", 8192, NULL, 5, NULL);
}

void checkUpdateTask(void *pvParameter)
{
	vTaskDelay(pdMS_TO_TICKS(30000));
	while (1)
	{
		otaCheck();

		vTaskDelay(pdMS_TO_TICKS(86400000)); //24h -> 86400000
	}
}

bool otaCheckVersion()
{
	ESP_LOGI(otaTAG, "Looking for a new firmware...");
	char url[100];
	sprintf(url, "%s/BlueRavenGPRS", UPDATE_JSON_URL);

	// configure the esp_http_client
	esp_http_client_config_t config = {
			.url = url,
			.event_handler = _httpEventHandler,
			.timeout_ms = 120000,
	};
	esp_http_client_handle_t client = esp_http_client_init(&config);

	// downloading the json file
	esp_err_t err = esp_http_client_perform(client);
	// cleanup
	esp_http_client_cleanup(client);
	bool newVersion = false;
	if (err == ESP_OK)
	{
		cJSON *json = cJSON_Parse(rcv_buffer);
		if (json == NULL)
			ESP_LOGW(otaTAG, "downloaded file is not a valid json, aborting...\n");
		else
		{
			cJSON *version = cJSON_GetObjectItemCaseSensitive(json, "version");
			cJSON *prod = cJSON_GetObjectItemCaseSensitive(json, "product");
			// check the version
			if (!cJSON_IsNumber(version))
			{
				uint major, minor, patch;
				major = atoi(strtok(version->valuestring, "."));
				minor = atoi(strtok(NULL, "."));
				patch = atoi(strtok(NULL, "."));

				newVersion = compareVersion(major, minor, patch);
			}
			else
				newVersion = false;
		}
		cJSON_Delete(json);
	}
	else
		ESP_LOGE(otaTAG, "unable to download version file, aborting...\n");
	return newVersion;
}

void otaUpdate()
{
	extern const char server_cert_pem_start[] asm("_binary_cert_pem_start");

	ESP_LOGI(otaTAG, "downloading and installing new firmware...\n");
	otaFlag = 1;
	char url[100];
	sprintf(url, "%s/update/blueravengprs", UPDATE_JSON_URL);
	esp_http_client_config_t ota_client_config = {
			.url = url,
			.cert_pem = server_cert_pem_start,
	};
	esp_err_t ret = esp_https_ota(&ota_client_config);
	if (ret == ESP_OK)
	{
		ESP_LOGW(otaTAG, "OTA OK, restarting...\n");
		otaFlag = 0;
		esp_restart();
	}
	else
	{
		ESP_LOGW(otaTAG, "OTA failed...\n");
		otaFlag = 0;
	}
}

void otaCheck()
{
	bool newVersion = otaCheckVersion();
	if (newVersion)
	{
		ESP_LOGI(otaTAG, "New Version, upgrading...");
		otaUpdate();
	}
	else
	{
		//ESP_LOGI(otaTAG, "My version is up-to-date!");
	}
}

esp_err_t _httpEventHandler(esp_http_client_event_t *evt)
{
	switch (evt->event_id)
	{
	case HTTP_EVENT_ERROR:
		break;
	case HTTP_EVENT_ON_CONNECTED:
		break;
	case HTTP_EVENT_HEADER_SENT:
		break;
	case HTTP_EVENT_ON_HEADER:
		break;
	case HTTP_EVENT_ON_DATA:
		if (!esp_http_client_is_chunked_response(evt->client))
		{
			strncpy(rcv_buffer, (char *)evt->data, evt->data_len);
		}
		break;
	case HTTP_EVENT_ON_FINISH:
		break;
	case HTTP_EVENT_DISCONNECTED:
		break;
	}
	return ESP_OK;
}

uint8_t getOtaFlag()
{
	return otaFlag;
}

bool compareVersion(uint major, uint minor, uint patch)
{
	if (CONFIG_FIRMWARE_MAJOR > major)
		return false;
	else if (CONFIG_FIRMWARE_MAJOR < major)
		return true;

	if (CONFIG_FIRMWARE_MINOR > minor)
		return false;
	else if (CONFIG_FIRMWARE_MINOR < minor)
		return true;

	if (CONFIG_FIRMWARE_PATCH > patch)
		return false;
	else if (CONFIG_FIRMWARE_PATCH < patch)
		return true;

	return false;
}
