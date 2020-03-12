/*
 * Project IoTNodeThingSpeak
 * Description: Demo code to send data to ThingSpeak using webhooks and the Sentient Things IoTNode
 * Author: Robert Mawrey
 * Date: December 2019
 */

#include "IoTNode.h"
#include "IoTNodeThingSpeak.h"

IoTNode mynode;

// Get these parameters from your ThingSpeak Channel
#define THINGSPEAK_CHANNEL_ID "284943"
#define THINGSPEAK_WRITE_KEY "SI1YT1DO5O41P0YP"

// This Webhook must be created using your Particle account
// {
//     "event": "TSBulkWriteCSV",
//     "responseTopic": "{{PARTICLE_DEVICE_ID}}/hook-response/TSBulkWriteCSV",
//     "url": "https://api.thingspeak.com/channels/{{c}}/bulk_update.csv",
//     "requestType": "POST",
//     "noDefaults": true,
//     "rejectUnauthorized": true,
//     "responseTemplate": "{{success}}",
//     "headers": {
//         "Content-Type": "application/x-www-form-urlencoded"
//     },
//     "form": {
//         "write_api_key": "{{k}}",
//         "time_format": "{{t}}",
//         "updates": "{{d}}"
//     }
// }

// Create a IoTNodeThingSpeak object with queue size of 30 (for example)
IoTNodeThingSpeak thingspeak(mynode, THINGSPEAK_CHANNEL_ID, THINGSPEAK_WRITE_KEY, 30);

/**
 * @brief This is the necessary struct format for ThingSpeak data
 * It matches the ThingSpeal Bulk-Write CSV Data format: https://www.mathworks.com/help/thingspeak/bulkwritecsvdata.html
 * TIMESTAMP,FIELD1_VALUE,FIELD2_VALUE,FIELD3_VALUE,FIELD4_VALUE,FIELD5_VALUE,FIELD6_VALUE,FIELD7_VALUE,FIELD8_VALUE,LATITUDE,LONGITUDE,ELEVATION,STATUS
 * and is queued in the IoTNode Fram
 * nullMap is used as a bitmap to send nulls for bits that are equal to 1
 */
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
  float status;
  uint16_t nullMap; // bits define null values i.e. lsb=field1, bit 10 = elevation
}TSdata_t;

TSdata_t tsdata;

// setup() runs once, when the device is first turned on.
void setup() {
  Serial.begin(115200);

  // Run these in setup
  mynode.begin();
  thingspeak.setup();

  delay(1000);

  // null map - see above
  tsdata.nullMap = 0B011111111111;

  // Create sample data to send to ThingSpeak
  tsdata.messageTime = Time.now();
  for (int ii=0; ii<30; ii++)
  {    
    Serial.println(tsdata.messageTime);
    tsdata.field1 = (float)ii;
    Serial.println(tsdata.field1);
    thingspeak.queueToSend((uint8_t*)&tsdata);
    tsdata.messageTime++;
  } 

}

// loop() runs over and over again, as quickly as it can execute.
void loop() {
  
  Particle.process();
  // Process the queued ThingSpeak data by sending to ThingSpeak
  // using the Bulk-Write CSV Data API every 16 seconds 
  thingspeak.process();

}