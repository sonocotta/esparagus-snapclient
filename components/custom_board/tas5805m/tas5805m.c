/*
 * ESPRESSIF MIT License
 *
 * Copyright (c) 2020 <ESPRESSIF SYSTEMS (SHANGHAI) CO., LTD>
 *
 * Permission is hereby granted for use on all ESPRESSIF SYSTEMS products, in
 * which case, it is free of charge, to any person obtaining a copy of this
 * software and associated documentation files (the "Software"), to deal in the
 * Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 *
 */

#include "tas5805m.h"
#include "esp_log.h"
#include "driver/i2s.h"
#include "i2c_bus.h"

#if defined(CONFIG_DAC_TAS5805M_DSP_STEREO)
    #pragma message("tas5805m_2.0+basic config is used")
    #include "startup/tas5805m_2.0+basic.h"
    #elif defined(CONFIG_DAC_TAS5805M_DSP_STEREO_DRC_AGL)
    #pragma message("tas5805m_2.0+3-band_drc+agl_-12db config is used")
    #include "startup/tas5805m_2.0+3-band_drc+agl_-12db.h"
    #elif defined(CONFIG_DAC_TAS5805M_DSP_STEREO_DRC)
    #pragma message("tas5805m_2.0+3-band_drc config is used")
    #include "startup/tas5805m_2.0+3-band_drc.h"
    #elif defined(CONFIG_DAC_TAS5805M_DSP_STEREO_AGL)
    #pragma message("tas5805m_2.0+agl_-12db config is used")
    #include "startup/tas5805m_2.0+agl_-12db.h"
#elif defined(CONFIG_DAC_TAS5805M_DSP_MONO)
    #pragma message("tas5805m_1.0+basic config is used")
    #include "startup/tas5805m_1.0+basic.h"
    #elif defined(CONFIG_DAC_TAS5805M_DSP_MONO_DRC_AGL)
    #pragma message("tas5805m_1.0+3-band_drc+agl_-12db config is used")
    #include "startup/tas5805m_1.0+3-band_drc+agl_-12db.h"
    #elif defined(CONFIG_DAC_TAS5805M_DSP_MONO_DRC)
    #pragma message("tas5805m_1.0+3-band_drc config is used")
    #include "startup/tas5805m_1.0+3-band_drc.h"
    #elif defined(CONFIG_DAC_TAS5805M_DSP_MONO_AGL)
    #pragma message("tas5805m_1.0+agl_-12db config is used")
    #include "startup/tas5805m_1.0+agl_-12db.h"
#elif defined(CONFIG_DAC_TAS5805M_DSP_SUBWOOFER_100_AGL)
    #pragma message("tas5805m_0.1+eq_100Hz_cutoff+drc config is used")
    #include "startup/tas5805m_0.1+eq_100Hz_cutoff+drc.h"
    #elif defined(CONFIG_DAC_TAS5805M_DSP_SUBWOOFER_40)
    #pragma message("tas5805m_0.1+eq_40Hz_cutoff config is used")
    #include "startup/tas5805m_0.1+eq_40Hz_cutoff.h"
    #elif defined(CONFIG_DAC_TAS5805M_DSP_SUBWOOFER_60)
    #pragma message("tas5805m_0.1+eq_60Hz_cutoff config is used")
    #include "startup/tas5805m_0.1+eq_60Hz_cutoff.h"
    #elif defined(CONFIG_DAC_TAS5805M_DSP_SUBWOOFER_100)
    #pragma message("tas5805m_0.1+eq_100Hz_cutoff config is used")
    #include "startup/tas5805m_0.1+eq_100Hz_cutoff.h"
#elif defined(CONFIG_DAC_TAS5805M_DSP_BIAMP)
    #pragma message("tas5805m_1.1+eq_60Hz_cutoff+mono config is used")
    #include "startup/tas5805m_1.1+eq_60Hz_cutoff+mono.h"
    #elif defined(CONFIG_DAC_TAS5805M_DSP_BIAMP_60_LEFT)
    #pragma message("tas5805m_1.1+eq_60Hz_cutoff+left config is used")
    #include "startup/tas5805m_1.1+eq_60Hz_cutoff+left.h"
    #elif defined(CONFIG_DAC_TAS5805M_DSP_BIAMP_60_RIGHT)
    #pragma message("tas5805m_1.1+eq_60Hz_cutoff+right config is used") 
    #include "startup/tas5805m_1.1+eq_60Hz_cutoff+right.h"
#else
    #pragma message("tas5805m_2.0+minimal config is used")
    #include "startup/tas5805m_2.0+minimal.h"
#endif

static const char *TAG = "TAS5805";

#define TAS5805M_ASSERT(a, format, b, ...) \
  if ((a) != 0) {                          \
    ESP_LOGE(TAG, format, ##__VA_ARGS__);  \
    return b;                              \
  }

/* Default I2C config */

static i2c_config_t i2c_cfg = {
    .mode = I2C_MODE_MASTER,
    .sda_pullup_en = GPIO_PULLUP_ENABLE,
    .scl_pullup_en = GPIO_PULLUP_ENABLE,
    .master.clk_speed = I2C_MASTER_FREQ_HZ,
};

/*
 * Operate fuction of PA
 */
audio_hal_func_t AUDIO_CODEC_TAS5805M_DEFAULT_HANDLE = {
    .audio_codec_initialize = tas5805m_init,
    .audio_codec_deinitialize = tas5805m_deinit,
    .audio_codec_ctrl = tas5805m_ctrl,
    .audio_codec_config_iface = tas5805m_config_iface,
    .audio_codec_set_mute = tas5805m_set_mute,
    .audio_codec_set_volume = tas5805m_set_volume,
    .audio_codec_get_volume = tas5805m_get_volume,
    .audio_hal_lock = NULL,
    .handle = NULL,
};

/* Init the I2C Driver */

void i2c_master_init()
{
  int i2c_master_port = I2C_MASTER_NUM;

  ESP_ERROR_CHECK(get_i2c_pins(I2C_NUM_0, &i2c_cfg));

  ESP_ERROR_CHECK(i2c_param_config(i2c_master_port, &i2c_cfg));

  ESP_ERROR_CHECK(i2c_driver_install(i2c_master_port, i2c_cfg.mode,
                                     I2C_MASTER_RX_BUF_DISABLE, I2C_MASTER_TX_BUF_DISABLE,
                                     0));
}

/* Helper Functions */

// Reading of TAS5805M-Register

esp_err_t tas5805m_read_byte(uint8_t register_name, uint8_t *data)
{

  int ret;
  i2c_cmd_handle_t cmd = i2c_cmd_link_create();
  i2c_master_start(cmd);
  i2c_master_write_byte(cmd, TAS5805M_ADDRESS << 1 | WRITE_BIT, ACK_CHECK_EN);
  i2c_master_write_byte(cmd, register_name, ACK_CHECK_EN);
  i2c_master_stop(cmd);
  ret = i2c_master_cmd_begin(I2C_TAS5805M_MASTER_NUM, cmd, 1000 / portTICK_RATE_MS);
  i2c_cmd_link_delete(cmd);

  if (ret != ESP_OK)
  {
    ESP_LOGW(TAG, "I2C ERROR");
  }

  vTaskDelay(1 / portTICK_RATE_MS);
  cmd = i2c_cmd_link_create();
  i2c_master_start(cmd);
  i2c_master_write_byte(cmd, TAS5805M_ADDRESS << 1 | READ_BIT, ACK_CHECK_EN);
  i2c_master_read_byte(cmd, data, NACK_VAL);
  i2c_master_stop(cmd);
  ret = i2c_master_cmd_begin(I2C_TAS5805M_MASTER_NUM, cmd, 1000 / portTICK_RATE_MS);
  i2c_cmd_link_delete(cmd);

  return ret;
}

// Writing of TAS5805M-Register

esp_err_t tas5805m_write_byte(uint8_t register_name, uint8_t value)
{
  int ret = 0;
  i2c_cmd_handle_t cmd = i2c_cmd_link_create();
  i2c_master_start(cmd);
  i2c_master_write_byte(cmd, TAS5805M_ADDRESS << 1 | WRITE_BIT, ACK_CHECK_EN);
  i2c_master_write_byte(cmd, register_name, ACK_CHECK_EN);
  i2c_master_write_byte(cmd, value, ACK_CHECK_EN);
  i2c_master_stop(cmd);

  ret = i2c_master_cmd_begin(I2C_TAS5805M_MASTER_NUM, cmd, 1000 / portTICK_RATE_MS);

  // Check if ret is OK
  if (ret != ESP_OK)
  {
    ESP_LOGE(TAG, "Error during I2C transmission: %s", esp_err_to_name(ret));
  }

  i2c_cmd_link_delete(cmd);

  return ret;
}

esp_err_t tas5805m_write_bytes(uint8_t *reg,
                              int regLen, uint8_t *data, int datalen)
{
  int ret = 0;

  i2c_cmd_handle_t cmd = i2c_cmd_link_create();
  ret |= i2c_master_start(cmd);
  ret |= i2c_master_write_byte(cmd, TAS5805M_ADDRESS << 1 | WRITE_BIT, ACK_CHECK_EN);
  ret |= i2c_master_write(cmd, reg, regLen, ACK_CHECK_EN);
  ret |= i2c_master_write(cmd, data, datalen, ACK_CHECK_EN);
  ret |= i2c_master_stop(cmd);
  ret = i2c_master_cmd_begin(I2C_TAS5805M_MASTER_NUM, cmd, 1000 / portTICK_RATE_MS);

  // Check if ret is OK
  if (ret != ESP_OK)
  {
    ESP_LOGE(TAG, "Error during I2C transmission: %s", esp_err_to_name(ret));
  }

  i2c_cmd_link_delete(cmd);

  return ret;
}

static esp_err_t tas5805m_transmit_registers(const tas5805m_cfg_reg_t *conf_buf, int size) {
  int i = 0;
  esp_err_t ret = ESP_OK;
  while (i < size) {
    switch (conf_buf[i].offset) {
      case CFG_META_SWITCH:
        // Used in legacy applications.  Ignored here.
        break;
      case CFG_META_DELAY:
        vTaskDelay(conf_buf[i].value / portTICK_RATE_MS);
        break;
      case CFG_META_BURST:
        ret = tas5805m_write_bytes((unsigned char *)(&conf_buf[i + 1].offset), 1,
                                   (unsigned char *)(&conf_buf[i + 1].value), conf_buf[i].value);
        i += (conf_buf[i].value / 2) + 1;
        break;
      case CFG_END_1:
        if (CFG_END_2 == conf_buf[i + 1].offset &&
            CFG_END_3 == conf_buf[i + 2].offset) {
          ESP_LOGI(TAG, "End of tas5805m reg: %d\n", i);
        }
        break;
      default:
        ret = tas5805m_write_bytes((unsigned char *)(&conf_buf[i].offset), 1,
                                   (unsigned char *)(&conf_buf[i].value), 1);
        break;
    }
    i++;
  }
  if (ret != ESP_OK) {
    ESP_LOGE(TAG, "Fail to load configuration to tas5805m");
    return ESP_FAIL;
  }
  ESP_LOGI(TAG, "%s:  write %d reg done", __FUNCTION__, i);
  return ret;
}

// Inits the TAS5805M 
// change Settings in Menuconfig to enable Bridge-Mode
esp_err_t tas5805m_init()
{
  // Init the I2C-Driver
  i2c_master_init();

  /* Register the PDN pin as output and write 1 to enable the TAS chip */
  /* TAS5805M.INIT() */
  gpio_config_t io_conf;
  io_conf.intr_type = GPIO_INTR_DISABLE;
  io_conf.mode = GPIO_MODE_OUTPUT;
  io_conf.pin_bit_mask = TAS5805M_GPIO_PDN_MASK;
  io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
  io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
  ESP_LOGI(TAG, "Running PWR ON sequence on pin: %d", TAS5805M_GPIO_PDN);
  gpio_config(&io_conf);
  gpio_set_level(TAS5805M_GPIO_PDN, 0);
  vTaskDelay(20 / portTICK_RATE_MS);
  gpio_set_level(TAS5805M_GPIO_PDN, 1);
  vTaskDelay(100 / portTICK_RATE_MS);

  // return ESP_OK;

  //
  // I2S needs to be initialized before this happens
  //
  int ret = tas5805m_transmit_registers(
      tas5805m_registers,
      sizeof(tas5805m_registers) / sizeof(tas5805m_registers[0]));

  // tas5805m_is_initialized = true;
  
  TAS5805M_ASSERT(ret, "Fail to initialize tas5805m PA", ESP_FAIL);

  return ret;
  
}

// Setting the Volume

esp_err_t
tas5805m_set_volume(int vol)
{
  int vol_idx = 0; // Temp-Variable

  /* Checking if Volume is bigger or smaller than the max values */
  if (vol < TAS5805M_VOLUME_MIN)
  {
    vol = TAS5805M_VOLUME_MIN;
  }
  if (vol > TAS5805M_VOLUME_MAX)
  {
    vol = TAS5805M_VOLUME_MAX;
  }
  /* Mapping the Values from 0-100 to 254-0 */
  vol_idx = vol / 5;
  /* Updating the global volume Variable */
  currentVolume = vol_idx;
  /* Writing the Volume to the Register*/
  return tas5805m_write_byte(TAS5805M_DIG_VOL_CTRL_REGISTER, tas5805m_volume[vol_idx]);
}

esp_err_t
tas5805m_get_volume(int *vol)
{
  esp_err_t ret = ESP_OK;
  uint8_t rxbuf = 0;
  ret = tas5805m_read_byte(TAS5805M_DIG_VOL_CTRL_REGISTER, &rxbuf);
  int i;
  for (i = 0; i < sizeof(tas5805m_volume); i++)
  {
    if (rxbuf >= tas5805m_volume[i])
      break;
  }
  /* Updating the global volume Variable */
  currentVolume = i;
  ESP_LOGI(TAG, "Volume is %d", i * 5);
  *vol = 5 * i; // Converting it to percent
  return ret;
}

esp_err_t tas5805m_deinit(void)
{
  // TODO
  return ESP_OK;
}

esp_err_t
tas5805m_set_mute(bool enable)
{

  if (enable == true)
  {
    // Set the Volume to 255 to enable the MUTE
    return tas5805m_write_byte(TAS5805M_DIG_VOL_CTRL_REGISTER, TAS5805M_VOLUME_MUTE);
  }
  else
  {
    return tas5805m_write_byte(TAS5805M_DIG_VOL_CTRL_REGISTER, tas5805m_volume[currentVolume]); // Restore Volume to its old value
  }
  return ESP_OK;
}

esp_err_t
tas5805m_get_mute(bool *enabled)
{
  int currentVolume;
  if (tas5805m_get_volume(&currentVolume) != ESP_OK)
  {
    ESP_LOGW(TAG, "Cant get volume in get-Mute-Function");
  }
  if (currentVolume == TAS5805M_VOLUME_MUTE)
  {
    *enabled = true;
  }
  else
  {
    *enabled = false;
  }
  return ESP_OK;
}

// static bool tas5805m_is_initialized = false;


esp_err_t tas5805m_ctrl(audio_hal_codec_mode_t mode,
                        audio_hal_ctrl_t ctrl_state)
{
  // if (!tas5805m_is_initialized) {
  //   //
  //   // I2S needs to be initialized before this happens
  //   //
  //   int ret = tas5805m_transmit_registers(
  //       tas5805m_registers,
  //       sizeof(tas5805m_registers) / sizeof(tas5805m_registers[0]));

  //   tas5805m_is_initialized = true;
    
  //   TAS5805M_ASSERT(ret, "Fail to initialize tas5805m PA", ESP_FAIL);

  //   return ret;
  // } 

  return ESP_OK;
}

esp_err_t tas5805m_config_iface(audio_hal_codec_mode_t mode,
                                audio_hal_codec_i2s_iface_t *iface)
{
  // TODO
  return ESP_OK;
}
