/*
 
  This example is heavily based on the 'ttn-abp' example by Thomas Telkamp and Matthijs Kooijman

  This example sends a valid LoRaWAN packet with payload "Hello,
  world!", using frequency and encryption settings matching those of
  the The Things Network.
 
  This uses ABP (Activation-by-personalisation), where a DevAddr and
  Session keys are preconfigured (unlike OTAA, where a DevEUI and
  application key is configured, while the DevAddr and session keys are
  assigned/generated in the over-the-air-activation procedure).
 
  Note: LoRaWAN per sub-band duty-cycle limitation is enforced (1% in
  g1, 0.1% in g2), but not the TTN fair usage policy (which is probably
  violated by this sketch when left running for longer)!
 
  To use this sketch, first register your application and device with
  the things network, to set or generate a DevAddr, NwkSKey and
  AppSKey. Each device should have their own unique values for these
  fields.
 
  Do not forget to define the radio type correctly in config.h.
 
 */

#include <lmic.h> //Click here to get the library: http://librarymanager/All#IBM_LMIC
#include <hal/hal.h>
#include <SPI.h>

// LoRaWAN Network Session Key
// Get this by registering a new device on TTN, using the ABP (Activation By Personalization) method (not Over The Air)
static const PROGMEM u1_t NWKSKEY[16] = { 0x2C, 0x02, 0x90, 0x00, 0xA7, 0x29, 0x48, 0x86, 0xB1, 0xAD, 0xDA, 0x0E, 0x05, 0xD6, 0x05, 0xF0 };

// LoRaWAN Application Session Key
// Get this by registering a new device from the TTN (The Things Network) console
static const u1_t PROGMEM APPSKEY[16] = { 0x17, 0x41, 0xA6, 0xC4, 0x95, 0xC5, 0xEB, 0xAA, 0x41, 0xAA, 0x69, 0x9E, 0x6E, 0x80, 0xA4, 0x91 };

// LoRaWAN Device Address
// Get this by registering a new device from TTN
static const u4_t DEVADDR = 0x2601131C;

// These callbacks are only used in over-the-air activation, so they are
// left empty here (we cannot leave them out completely unless
// DISABLE_JOIN is set in config.h, otherwise the linker will complain).
void os_getArtEui (u1_t* buf) { }
void os_getDevEui (u1_t* buf) { }
void os_getDevKey (u1_t* buf) { }

static uint8_t mydata[] = "Hello, world!";
static osjob_t sendjob;

// Schedule TX every this many seconds (might become longer due to duty cycle limitations).
const unsigned TX_INTERVAL = 60;

//Pin mapping
//On the SparkFun ProRF NSS (aka !CS) is A5, DIO is 7, 10, 11
//NOTE: You must close the 'LoRa WAN' jumpers on the back of the ProRF. This will connect D10 and D11 to the DIO pins on the RFM95 module
const lmic_pinmap lmic_pins = {
    .nss = A5,
    .rxtx = LMIC_UNUSED_PIN,
    .rst = 4,
    .dio = {7, 10, 11},
};

void onEvent (ev_t ev) {
    Serial.print(os_getTime());
    Serial.print(": ");
    switch(ev) {
        case EV_SCAN_TIMEOUT:
            Serial.println(F("EV_SCAN_TIMEOUT"));
            break;
        case EV_BEACON_FOUND:
            Serial.println(F("EV_BEACON_FOUND"));
            break;
        case EV_BEACON_MISSED:
            Serial.println(F("EV_BEACON_MISSED"));
            break;
        case EV_BEACON_TRACKED:
            Serial.println(F("EV_BEACON_TRACKED"));
            break;
        case EV_JOINING:
            Serial.println(F("EV_JOINING"));
            break;
        case EV_JOINED:
            Serial.println(F("EV_JOINED"));
            break;
        case EV_RFU1:
            Serial.println(F("EV_RFU1"));
            break;
        case EV_JOIN_FAILED:
            Serial.println(F("EV_JOIN_FAILED"));
            break;
        case EV_REJOIN_FAILED:
            Serial.println(F("EV_REJOIN_FAILED"));
            break;
        case EV_TXCOMPLETE:
            Serial.println(F("EV_TXCOMPLETE (includes waiting for RX windows)"));
            if (LMIC.txrxFlags & TXRX_ACK)
              Serial.println(F("Received ack"));
            if (LMIC.dataLen) {
              Serial.println(F("Received "));
              Serial.println(LMIC.dataLen);
              Serial.println(F(" bytes of payload"));
            }
            // Schedule next transmission
            os_setTimedCallback(&sendjob, os_getTime()+sec2osticks(TX_INTERVAL), do_send);
            break;
        case EV_LOST_TSYNC:
            Serial.println(F("EV_LOST_TSYNC"));
            break;
        case EV_RESET:
            Serial.println(F("EV_RESET"));
            break;
        case EV_RXCOMPLETE:
            // data received in ping slot
            Serial.println(F("EV_RXCOMPLETE"));
            break;
        case EV_LINK_DEAD:
            Serial.println(F("EV_LINK_DEAD"));
            break;
        case EV_LINK_ALIVE:
            Serial.println(F("EV_LINK_ALIVE"));
            break;
         default:
            Serial.println(F("Unknown event"));
            break;
    }
}

void do_send(osjob_t* j){
    // Check if there is not a current TX/RX job running
    if (LMIC.opmode & OP_TXRXPEND) {
        Serial.println(F("OP_TXRXPEND, not sending"));
    } else {
        // Prepare upstream data transmission at the next possible time.
        LMIC_setTxData2(1, mydata, sizeof(mydata)-1, 0);
        Serial.println(F("Packet queued"));
    }
    // Next TX is scheduled after TX_COMPLETE event.
}

void setup() {
    Serial.begin(115200);
    Serial.println(F("Starting"));
    
    os_init(); // LMIC init
    
    LMIC_reset(); // Reset the MAC state. Session and pending data transfers will be discarded.

    // Set static session parameters. Instead of dynamically establishing a session
    // by joining the network, precomputed session parameters are be provided.
    #ifdef PROGMEM
    // On AVR, these values are stored in flash and only copied to RAM
    // once. Copy them to a temporary buffer here, LMIC_setSession will
    // copy them into a buffer of its own again.
    uint8_t appskey[sizeof(APPSKEY)];
    uint8_t nwkskey[sizeof(NWKSKEY)];
    memcpy_P(appskey, APPSKEY, sizeof(APPSKEY));
    memcpy_P(nwkskey, NWKSKEY, sizeof(NWKSKEY));
    LMIC_setSession (0x1, DEVADDR, nwkskey, appskey);
    #else
    // If not running an AVR with PROGMEM, just use the arrays directly
    LMIC_setSession (0x1, DEVADDR, NWKSKEY, APPSKEY);
    #endif

    // NA-US channels 0-71 are configured automatically
    // but only one group of 8 (a subband) should be active
    // TTN recommends the second sub band, 1 in a zero based count.
    // https://github.com/TheThingsNetwork/gateway-conf/blob/master/US-global_conf.json
    LMIC_selectSubBand(1);

    // Disable link check validation
    LMIC_setLinkCheckMode(0);

    // TTN uses SF9 for its RX2 window.
    LMIC.dn2Dr = DR_SF9;

    // Set data rate and transmit power for uplink (note: txpow seems to be ignored by the library)
    LMIC_setDrTxpow(DR_SF7, 14);

    // Start job
    do_send(&sendjob);
}

void loop() {
    os_runloop_once();
}
