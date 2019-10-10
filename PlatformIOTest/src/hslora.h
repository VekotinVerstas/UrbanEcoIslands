// Funktio määrittelyt ennen ifdef:ä, jotta stubeja voi kutsua vaikka ei käytössä
void hslora_setup();

#ifdef SEND_DATA_LORA_2_ENABLED
#include <lmic.h>
#include <hal/hal.h>

void hslora_setup();

bool clear_to_sleep = true; // LoRa send can prevent sleep

// These settings seems to work with TTGO LORA OLED
const lmic_pinmap lmic_pins = {
    .nss = 18,
    .rxtx = LMIC_UNUSED_PIN,

    //For board revision V1.5 use GPIO12 for LoRa RST
    //.rst = 12,
    //For board revision(s) newer than V1.5 use GPIO19 for LoRa RST
    .rst = 19,
    .dio = {26, 33, 32}};

//MDummy message to emulate 26 byte (AQBURK) payload ( bat counter probably has much shorter
static PROGMEM u1_t STATICMSG[26] = {0x24, 0x2c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

// These callbacks are only used in over-the-air activation, so they are
// left empty here (we cannot leave them out completely unless
// DISABLE_JOIN is set in config.h, otherwise the linker will complain).
void os_getArtEui(u1_t *buf) {}
void os_getDevEui(u1_t *buf) {}
void os_getDevKey(u1_t *buf) {}

static osjob_t sendjob;

#endif //SEND_DATA_LORA_2_ENABLED
