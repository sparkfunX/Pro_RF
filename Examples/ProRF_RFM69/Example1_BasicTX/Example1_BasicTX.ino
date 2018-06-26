/*
  ProRF - Basic data transmission over the air
  By: Nathan Seidle
  SparkFun Electronics
  Date: June 12th, 2018
  License: This code is public domain but you buy me a beer if you use this and we meet someday (Beerware License).

  This example shows how to transmit a simple data message from the TX (transmitter) to the RX (receiver).
  Data can be sent in either direction but these example use TX and RX nomenclature to avoid confusion.

  The status LED turns on if a packet is successfully received. Helpful for range testing.
  
  This is the transmitter or 'client'. It transmits a message every second and the listens for a 
  response from the 'server'.

  This code is heavily based on RadioHead's rfm_client and rfm_server examples. 

  Feel like supporting open source hardware?
  Buy a board from SparkFun! 
  ProRF with RFM69: https://www.sparkfun.com/products/14757
  ProRF LoRa with RFM95: https://www.sparkfun.com/products/14785
  
  Software and Hardware Setup:
  You'll need to download and install the SparkFun Atmel Boards. See this repo:
  https://github.com/sparkfun/Arduino_Boards for more information.
  Select 'SparkFun Pro Micro, 3.3V' as your Tools->Board.
  Both the TX and RX ProRF boards will need a wire antenna. We recommend a 3" piece of wire.
*/

#include <SPI.h>

//Install this library by downloading the lastest zip from http://www.airspayce.com/mikem/arduino/RadioHead/
#include <RH_RF69.h> 

//On the SparkFun ProRF the RFM CS pin is connected to pin A5, the RFM interrupt is on pin 7
RH_RF69 rf69(A5, 7);

const byte LED = 13; //Status LED is on pin 13

int packetCounter = 0; //Counts the number of packets sent

long timeSinceLastPacket = 0; //Tracks the time stamp of last packet received

void setup()
{
  pinMode(LED, OUTPUT);
  digitalWrite(LED, LOW);

  Serial.begin(9600);
  //while (Serial == false) ; //This line requires USB to be hooked up. If you're going to do battery powered range testing, comment it out.

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

  //Europe is 863-870 with center at 868MHz
  //This works but it is unknown how well the radio configures to this frequency
  //rf69.setFrequency(864.1);

  //Set power to 14dBm. Can also go to higher 20dBm if needed.
  //rf69.setTxPower(14, true);
  rf69.setTxPower(20, true);

  //The encryption key has to be the same as the one in the receiver
  uint8_t key[] = {"AbraRabbit123456"}; //This works but the key must be 16 characters long
//  uint8_t key[] = { 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
//                    0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08
//                  }; //This also works
  rf69.setEncryptionKey(key);
}


void loop()
{
  Serial.println("Sending message");

  //Send a message to the other radio
  char toSend[50];
  sprintf(toSend, "Hi, my counter is: %d", packetCounter++);
  rf69.send(toSend, sizeof(toSend));
  rf69.waitPacketSent();

  // Now wait for a reply
  byte buf[RH_RF69_MAX_MESSAGE_LEN];
  byte len = sizeof(buf);

  if (rf69.waitAvailableTimeout(500))
  {
    // Should be a reply message for us now
    if (rf69.recv(buf, &len))
    {
      digitalWrite(LED, HIGH); //Turn on status LED

      Serial.print("Got reply: ");
      Serial.print((char*)buf);
      Serial.print(" RSSI: ");
      Serial.print(rf69.lastRssi(), DEC);
      Serial.println();
    }
    else
    {
      Serial.println("Receive failed");
    }
  }
  else
  {
    Serial.println("No reply, is the receiver running?");
    digitalWrite(LED, LOW); //Turn off status LED
  }
  delay(250);
}

