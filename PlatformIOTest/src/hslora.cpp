#include <arduino.h>
#include "tasksdefine.h"
#include "hslora.h"

extern uint8_t NWKSKEY[16];
extern PROGMEM uint8_t APPSKEY[16];
extern uint32_t DEVADDR;

// These settings seems to work with TTGO LORA OLED
const lmic_pinmap lmic_pins = {
    .nss = 18,
    .rxtx = LMIC_UNUSED_PIN,

    //For board revision V1.5 use GPIO12 for LoRa RST
    //.rst = 12,
    //For board revision(s) newer than V1.5 use GPIO19 for LoRa RST
    .rst = 19,
    .dio = {26, 33, 32}};

// These callbacks are only used in over-the-air activation, so they are
// left empty here (we cannot leave them out completely unless
// DISABLE_JOIN is set in config.h, otherwise the linker will complain).
void os_getArtEui(u1_t *buf) {}
void os_getDevEui(u1_t *buf) {}
void os_getDevKey(u1_t *buf) {}

void hslora_setup()
{
    #ifdef SEND_DATA_LORA_2_ENABLED
    if (bootCount > -1)
    {
        // LMIC init
        os_init();
        // Reset the MAC state. Session and pending data transfers will be discarded.
        LMIC_reset();

        LMIC_setSession(0x1, DEVADDR, NWKSKEY, APPSKEY);

        // Set up the channels used by the Things Network, which corresponds
        // to the defaults of most gateways. Without this, only three base
        // channels from the LoRaWAN specification are used, which certainly
        // works, so it is good for debugging, but can overload those
        // frequencies, so be sure to configure the full frequency range of
        // your network here (unless your network autoconfigures them).
        // Setting up channels should happen after LMIC_setSession, as that
        // configures the minimal channel set.
        LMIC_setupChannel(0, 868100000, DR_RANGE_MAP(DR_SF12, DR_SF7), BAND_CENTI);  // g-band
        LMIC_setupChannel(1, 868300000, DR_RANGE_MAP(DR_SF12, DR_SF7B), BAND_CENTI); // g-band
        LMIC_setupChannel(2, 868500000, DR_RANGE_MAP(DR_SF12, DR_SF7), BAND_CENTI);  // g-band
        LMIC_setupChannel(3, 867100000, DR_RANGE_MAP(DR_SF12, DR_SF7), BAND_CENTI);  // g-band
        LMIC_setupChannel(4, 867300000, DR_RANGE_MAP(DR_SF12, DR_SF7), BAND_CENTI);  // g-band
        LMIC_setupChannel(5, 867500000, DR_RANGE_MAP(DR_SF12, DR_SF7), BAND_CENTI);  // g-band
        LMIC_setupChannel(6, 867700000, DR_RANGE_MAP(DR_SF12, DR_SF7), BAND_CENTI);  // g-band
        LMIC_setupChannel(7, 867900000, DR_RANGE_MAP(DR_SF12, DR_SF7), BAND_CENTI);  // g-band
        LMIC_setupChannel(8, 868800000, DR_RANGE_MAP(DR_FSK, DR_FSK), BAND_MILLI);   // g2-band

        // Disable link check validation
        LMIC_setLinkCheckMode(0);

        // Set data rate and transmit power (note: txpow seems to be ignored by the library)
        //LMIC_setDrTxpow(DR_SF12, 14);
        LMIC_setDrTxpow(DR_SF12, 14);

        // Start job
        //do_send(&sendjob);
    }
    #endif //SEND_DATA_LORA_2_ENABLED
}


