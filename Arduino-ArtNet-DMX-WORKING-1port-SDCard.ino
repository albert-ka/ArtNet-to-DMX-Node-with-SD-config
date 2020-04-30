#include <SPI.h>
#include <Ethernet.h>
#include <ArtNode.h>
#include <DmxSimple.h>
#include <SD.h>

#define VERSION_HI 0
#define VERSION_LO 1

File myFile;
String configSD;
int ipLong; // ????? Delete

String mac1;
String mac2;
String mac3;
String mac4;

String ip1;
String ip2;
String ip3;
String ip4;

//MAC
int ind1;
int ind2;
int ind3;
int ind4;

//IP
int ind5;
int ind6;
int ind7;
int ind8;

byte ipAdress[4];
byte macAdress[4];


////////////////////////////////////////////////////////////
ArtConfig config = {
  .mac =  {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED}, // MAC
  .ip =   {192, 168, 1, 69},                         // IP
  .mask = {255, 255, 255, 0},                       // Subnet mask
  .udpPort = 0x1936,
  .dhcp = false,
  .net = 0, // Net (0-127)
  .subnet = 0,  // Subnet (0-15)
  "ArtNode", // Short name
  "ArtNode", // Long name
  .numPorts = 1,

  .portTypes = {
    PortTypeDmx | PortTypeOutput
  },
  .portAddrIn = {0}, // Port input universes (0-15)
  .portAddrOut = {0}, // Port output universes (0-15)
  .verHi = VERSION_HI,
  .verLo = VERSION_LO
};
////////////////////////////////////////////////////////////
IPAddress gateway(config.ip[0], 0, 0, 1);
EthernetUDP udp;
byte buffer[600];
ArtNode node = ArtNode(config, sizeof(buffer), buffer);

////////////////////////////////////////////////////////////
void setup() {
  // Open serial communications and wait for port to open:
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }

  pinMode(10, OUTPUT); // change this to 53 on a mega  // don't follow this!!
  digitalWrite(10, HIGH); // Add this line

  //Serial.print("Initializing SD card...");

  if (!SD.begin(4)) {
    Ethernet.begin(config.mac, config.ip,  gateway, gateway, config.mask);
    Serial.println("initialization failed!");
    //while (1);
    goto noSDconfig;
  }
  //Serial.println("initialization done.");


  // re-open the file for reading:
  myFile = SD.open("config.txt");
  if (myFile) {
    // read from the file until there's nothing else in it:
    while (myFile.available()) {
      
      configSD += (char)myFile.read();
      
      ind1 = configSD.indexOf('.');  //finds location of first .
      mac1 = configSD.substring(0, ind1);   //captures first data String
      ind2 = configSD.indexOf('.', ind1 + 1 ); //finds location of second .
      mac2 = configSD.substring(ind1 + 1, ind2 + 1); //captures second data String
      ind3 = configSD.indexOf('.', ind2 + 1 );
      mac3 = configSD.substring(ind2 + 1, ind3 + 1);
      ind4 = configSD.indexOf('-', ind3 + 1 );
      mac4 = configSD.substring(ind3 + 1, ind4 + 1); //captures remain part of data after last .

      ind5 = configSD.indexOf(".", ind4 + 1 );
      ip1 = configSD.substring(ind4 + 1, ind5 + 1);
      ind6 = configSD.indexOf('.', ind5 + 1 );
      ip2 = configSD.substring(ind5 + 1, ind6 + 1);
      ind7 = configSD.indexOf('.', ind6 + 1 );
      ip3 = configSD.substring(ind6 + 1, ind7 + 1);
      ind8 = configSD.indexOf('.', ind7 + 1 );
      ip4 = configSD.substring(ind7 + 1);

    }
    /*Serial.println(mac1.toInt());
    Serial.println(mac2.toInt());
    Serial.println(mac3.toInt());
    Serial.println(mac4.toInt());
    Serial.println(ip1.toInt());
    Serial.println(ip2.toInt());
    Serial.println(ip3.toInt());
    Serial.println(ip4.toInt());*/

    macAdress[0] = mac1.toInt();
    macAdress[1] = mac2.toInt();
    macAdress[2] = mac3.toInt();
    macAdress[3] = mac4.toInt();
    
    ipAdress[0] = ip1.toInt();
    ipAdress[1] = ip2.toInt();
    ipAdress[2] = ip3.toInt();
    ipAdress[3] = ip4.toInt();

    // close the file:
    myFile.close();
    Ethernet.begin(macAdress, ipAdress,  gateway, gateway, config.mask);
    //Serial.println("ip from sd card");

  } else {
    // if the file didn't open, print an error:
    //Serial.println("error opening test.txt");
  }

  //Ethernet.begin(config.mac, config.ip,  gateway, gateway, config.mask);
  //Ethernet.begin(macAdress, ipAdress,  gateway, gateway, config.mask);
  //udp.begin(config.udpPort);
  
  noSDconfig:
  udp.begin(config.udpPort);
  DmxSimple.usePin(3);
  DmxSimple.maxChannel(512);
}

////////////////////////////////////////////////////////////
void loop() {

  while (udp.parsePacket()) {

    int n = udp.read(buffer, min(udp.available(), sizeof(buffer)));
    if (n >= sizeof(ArtHeader) && node.isPacketValid()) {

      // Package Op-Code determines type of packet
      switch (node.getOpCode()) {

        // Poll packet. Send poll reply.
        case OpPoll: {
            ArtPoll* poll = (ArtPoll*)buffer;
            node.createPollReply();
            udp.beginPacket(node.broadcastIP(), config.udpPort);
            udp.write(buffer, sizeof(ArtPollReply));
            udp.endPacket();
          } break;

        // DMX packet
        case OpDmx: {
            ArtDmx* dmx = (ArtDmx*)buffer;
            int port = node.getPort(dmx->Net, dmx->SubUni);
            int len = dmx->getLength();
            byte *data = dmx->Data;
            if (port >= 0 && port < config.numPorts) {
              for (int i = 1; i <= len;  i++) { // MODIFICAT!!! 4 ABRIL 2020
                DmxSimple.write(i, data[i - 1]);
                //Serial.println(data[511]);
              }
            }
          } break;

        default:
          break;
      }
    }
  }
}
