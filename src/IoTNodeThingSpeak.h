/*
    Copyright (c) 2019 Sentient Things, Inc.  All right reserved.
    Version 0.0.2
    Updated process to return the last time (millis()) data were sent.
*/



// ensure this library description is only included once
#ifndef IoTNodeThingSpeak_h
#define IoTNodeThingSpeak_h

// include Particle library
#include "Particle.h"

#include "IoTNode.h"

    typedef struct
    {
      uint32_t messageTime;
      float field1;
      float field2;
      float field3;
      float field4;
      float field5;
      float field6;
      float field7;
      float field8;
      float latitude;
      float longitude;
      float elevation;
      char status[48];
      uint16_t nullMap; // bits define null values i.e. lsb=status, bit 10 = field1
    }fields8TS_t;

// library interface description
class IoTNodeThingSpeak
{
  // user-accessible "public" interface
  public:
    IoTNodeThingSpeak(IoTNode& node, String channelID, String writeKey, const uint16_t arraySize);

    void setup();

    void TSBulkWriteCSVHandler(const char *eventName, const char *data);

    void TSBulkWriteCSV(String csvData);

    void queueToSend(byte* buffer);

    uint32_t process();

    bool isQueueEmpty();

    // Note the new 622 character size limitation of Particle.publish() messages
    void TSBulkWriteCSV(String channel, String api_key, String time_format, String csvData);
    boolean TSCreateChan(char const* keys[], char const* values[], int& returnIndex);
    boolean updateTSChan(char const* channel, char const* values[], char const* labels[], int& arrayIndex);
    void TSWriteOneSetting(int channelNum, String value, String label);
  // library-accessible "private" interface
  private:
    IoTNode _node;

    framRing framTSData;
    
    fields8TS_t fields8TS;

    String _writeKey;

    String _channelID;

    const uint16_t _arraySize;

    int value;
    String urlencode(String str);

    uint32_t lastSendTime = 0;

    String minimiseNumericString(String ss, int n);
};

#endif
