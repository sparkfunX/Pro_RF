/*
  This is the transmitter or 'client'. It transmits a message every second
  and the listens for a response from the 'server'.
  COM24 on left
*/

#include <SPI.h>
#include <RH_RF69.h>

//On the SparkFun ProRF the RFM CS pin is connected to pin A5, the RFM interrupt is on pin 7
RH_RF69 rf69(A5, 7);

void setup()
{
  Serial.begin(9600);
  //while (!Serial) ; //If you're going to do range testing this line requires USB to be hooked up

  if (rf69.init() == false)
  {
    Serial.println("Radio Init Failed - Freezing");
    while (1);
  }

  //The SparkFun ProRF uses the RFM69HCW-915S2
  //This is a highpower, 915MHz device. The 915MHz ISM band is centered
  //around 915MHz but ranges from 902 to 928MHz so if needed you could use
  //908.2 for example if the band is very busy.
  rf69.setFrequency(921.2);

  //Set power to 14dBm. Can also go to higher 20dBm if needed.
  rf69.setTxPower(14, true);

  //The encryption key has to be the same as the one in the server
  uint8_t key[] = { 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
                    0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08
                  };
  rf69.setEncryptionKey(key);
}


void loop()
{
  Serial.println("Sending message");

  //Send a message to the other radio
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
      Serial.print("Got reply: ");
      Serial.println((char*)buf);
    }
    else
    {
      Serial.println("Receive failed");
    }
  }
  else
  {
    Serial.println("No reply, is the receiver running?");
  }
  delay(400);
}
