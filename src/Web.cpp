#include "Web.h"

WiFiManager wm;
extern Adafruit_SSD1306 display;
i2s_chan_handle_t rx_handle;

void handleWifiManager() {
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);
    display.println("starting...");
    display.println("waiting wifi....");
    display.println("waiting....");
    display.display();

    delay(50);
    bool res;
    res = wm.autoConnect("Pedro WiFI","2504300");

    if(!res) {
        Serial.println("Failed to connect");
    } 
    else {
        pushLog("WIFI CONNECTED");
        Serial.println("connected...yeey :)");
    }
}

void setupI2s() {
    i2s_chan_config_t chan_cfg = I2S_CHANNEL_DEFAULT_CONFIG(I2S_NUM_0, I2S_ROLE_MASTER);
    i2s_new_channel(&chan_cfg, NULL, &rx_handle);

    i2s_std_config_t std_cfg = {
        .clk_cfg = I2S_STD_CLK_DEFAULT_CONFIG(16000), 
        // PHILIPS_SLOT garante o timing exato que o INMP441 espera
        .slot_cfg = I2S_STD_PHILIPS_SLOT_DEFAULT_CONFIG(I2S_DATA_BIT_WIDTH_32BIT, I2S_SLOT_MODE_MONO),
        .gpio_cfg = {
            .mclk = I2S_GPIO_UNUSED,
            .bclk = (gpio_num_t)I2S_BCK_PIN,
            .ws   = (gpio_num_t)I2S_WS_PIN,
            .dout = I2S_GPIO_UNUSED,
            .din  = (gpio_num_t)I2S_DATA_PIN,
        },
    };

    // Força o ESP32 a ler o slot da esquerda (pino L/R no GND)
    std_cfg.slot_cfg.slot_mask = I2S_STD_SLOT_LEFT;

    i2s_channel_init_std_mode(rx_handle, &std_cfg); // Use RX especificamente
    i2s_channel_enable(rx_handle);
}

int32_t readMIC() {
    int32_t raw_sample = 0;
    size_t bytes_read = 0;
    
    // Lê uma única amostra de 32 bits
    esp_err_t result = i2s_channel_read(rx_handle, &raw_sample, sizeof(int32_t), &bytes_read, 100);
    
    if (result == ESP_OK && bytes_read > 0) {
        return raw_sample;
    }
    return 0;
}