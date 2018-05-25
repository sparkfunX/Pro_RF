// rf69_client.pde
// -*- mode: C++ -*-
// Example sketch showing how to create a simple messageing client
// with the RH_RF69 class. RH_RF69 class does not provide for addressing or
// reliability, so you should only use RH_RF69  if you do not need the higher
// level messaging abilities.
// It is designed to work with the other example rf69_server.
// Demonstrates the use of AES encryption, setting the frequency and modem 
// configuration
// Tested on Moteino with RFM69 http://lowpowerlab.com/moteino/
// Tested on miniWireless with RFM69 www.anarduino.com/miniwireless
// Tested on Teensy 3.1 with RF69 on PJRC breakout board

#include <SPI.h>
#include <RH_RF69.h>

//On the SparkFun ProRF the RFM CS pin is connected to pin 10, the RFM interrupt is on pin 3
RH_RF69 rf69(10, 3);

void setup() 
{
  Serial.begin(9600);
  //while (!Serial) ;

  if (rf69.init() == false)
    Serial.println("init failed");

  //The SparkFun ProRF uses the high power RFM69HCW-915S2
  //This is a highpower, 915MHz device. The 915MHz ISM band is centered
  //around 915MHz but ranges from 902 to 928MHz so if needed you could use
  //908.2 for example if the band is very busy.
  if (rf69.setFrequency(921.2) == false)
    Serial.println("setFrequency failed");

  //Set power to 14dBm. Can also go to higher 20dBm if needed.
  rf69.setTxPower(14, true);

  // The encryption key has to be the same as the one in the server
  uint8_t key[] = { 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
                    0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08};
  rf69.setEncryptionKey(key);
}


void loop()
{
  Serial.println("Sending to rf69_server");

  //Send a message to rf69_server
  uint8_t data[] = "Hello World!";
  rf69.send(data, sizeof(data));
  rf69.waitPacketSent();

  // Now wait for a reply
  uint8_t buf[RH_RF69_MAX_MESSAGE_LEN];
  uint8_t len = sizeof(buf);

  if (rf69.waitAvailableTimeout(500))
  { 
    // Should be a reply message for us now   
    if (rf69.recv(buf, &len))
    {
      Serial.print("got reply: ");
      Serial.println((char*)buf);
    }
    else
    {
      Serial.println("recv failed");
    }
  }
  else
  {
    Serial.println("No reply, is rf69_server running?");
  }
  delay(400);
}

