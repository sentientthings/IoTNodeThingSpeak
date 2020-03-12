/*
    Copyright (c) 2019 Sentient Things, Inc.  All right reserved.
*/

//#DEFINE DEBUG

// include this library's description file
#include "IoTNodeThingSpeak.h"

enum TSState {
    START,
    ADD_NEXT,
    CREATE_CHANNEL,
    //UPDATE_CHANNEL
};

TSState TSstate = START;

// include description files for other libraries used (if any)

// Constructor /////////////////////////////////////////////////////////////////
// Function that handles the creation and setup of instances

IoTNodeThingSpeak::IoTNodeThingSpeak(IoTNode& node, String channelID, String writeKey, const uint16_t arraySize) :
                    _node(node),
                    framTSData(node.makeFramRing(arraySize, sizeof(fields8TS))),
                    _writeKey(writeKey),
                    _channelID(channelID),
                    _arraySize(arraySize)
{
    // initialize this instance's variables

    // do whatever is required to initialize the library

}

// Public Methods //////////////////////////////////////////////////////////////
// Functions available in Wiring sketches, this library, and other libraries

void IoTNodeThingSpeak::setup() {
    // Particle.subscribe("TSBulkWriteCSV", &IoTNodeThingSpeak::TSBulkWriteCSVHandler, this, MY_DEVICES);
    framTSData.initialize();

}

void IoTNodeThingSpeak::TSBulkWriteCSVHandler(const char *eventName, const char *data) {
    fields8TS_t lastMessage;
    framTSData.popLast((uint8_t*)&lastMessage);
    // Serial.println(eventName);
    // Serial.println(data);
}

void IoTNodeThingSpeak::queueToSend(byte* buffer)
{
    framTSData.push(buffer);
}

uint32_t IoTNodeThingSpeak::process()
{
    if ((lastSendTime == 0 || millis()-lastSendTime>16000) && !framTSData.isEmpty())
    {      
        fields8TS_t lastMessage;
        framTSData.peekLast((uint8_t*)&lastMessage);
        bool firstpass = true;
        String mess;
        String messdata;
        mess = "{\"c\":\"" + _channelID + "\",\"k\":\"" + _writeKey + "\",\"t\":\"" + "absolute" + "\",\"d\":\"" ;

        // FIELD1_VALUE,FIELD2_VALUE,FIELD3_VALUE,FIELD4_VALUE,FIELD5_VALUE,FIELD6_VALUE,FIELD7_VALUE,FIELD8_VALUE,LATITUDE,LONGITUDE,ELEVATION,STATUS
        // 12 bits in the nullMap (a bit map) are used to define a null value for the above fields (NULL = 1)
        // lastMessage.nullMap 0Bxxxx000000000000
        // string.concat(string2) - The second string is appended to the first, and the result is placed in the original string.
        while (mess.length()+messdata.length()<620 && !framTSData.isEmpty())
        {
            String catstr;            
            if (!firstpass)
            {
                framTSData.peekLast((uint8_t*)&lastMessage);
                messdata.concat("|");
            }
            catstr = String(lastMessage.messageTime)+",";
            messdata.concat(catstr);
            (0B100000000000 & lastMessage.nullMap) ? catstr = "," : catstr = minimiseNumericString(String(lastMessage.field1),3)+"," ;
            messdata.concat(catstr);        
            (0B010000000000 & lastMessage.nullMap) ? catstr = "," : catstr = minimiseNumericString(String(lastMessage.field2),3)+"," ;
            messdata.concat(catstr);
            (0B001000000000 & lastMessage.nullMap) ? catstr = "," : catstr = minimiseNumericString(String(lastMessage.field3),3)+"," ;
            messdata.concat(catstr);
            (0B000100000000 & lastMessage.nullMap) ? catstr = "," : catstr = minimiseNumericString(String(lastMessage.field4),3)+"," ;
            messdata.concat(catstr);
            (0B000010000000 & lastMessage.nullMap) ? catstr = "," : catstr = minimiseNumericString(String(lastMessage.field5),3)+"," ;
            messdata.concat(catstr);
            (0B000001000000 & lastMessage.nullMap) ? catstr = "," : catstr = minimiseNumericString(String(lastMessage.field6),3)+"," ;
            messdata.concat(catstr);
            (0B000000100000 & lastMessage.nullMap) ? catstr = "," : catstr = minimiseNumericString(String(lastMessage.field7),3)+"," ;
            messdata.concat(catstr);
            (0B000000010000 & lastMessage.nullMap) ? catstr = "," : catstr = minimiseNumericString(String(lastMessage.field8),3)+"," ;
            messdata.concat(catstr);
            (0B000000001000 & lastMessage.nullMap) ? catstr = "," : catstr = minimiseNumericString(String(lastMessage.latitude),3)+"," ;
            messdata.concat(catstr);
            (0B000000000100 & lastMessage.nullMap) ? catstr = "," : catstr = minimiseNumericString(String(lastMessage.longitude),3)+"," ;
            messdata.concat(catstr);
            (0B000000000010 & lastMessage.nullMap) ? catstr = "," : catstr = minimiseNumericString(String(lastMessage.elevation),3)+"," ;
            messdata.concat(catstr);
            String statusmessage = String(lastMessage.status);
            (0B000000000001 & lastMessage.nullMap) ? catstr = "" : catstr = statusmessage.trim();
            messdata.concat(catstr);
            if (mess.length()+messdata.length()<620)
            {
                mess.concat(messdata);
                framTSData.popLast((uint8_t*)&lastMessage);
            }
            else
            {
                break;
            }
            
            messdata="";
            firstpass = false;
        }
        mess.concat("\"}");

        //Webhook
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
       if (Particle.publish("TSBulkWriteCSV", mess, PRIVATE))
       {
           lastSendTime = millis();
       }
        
        // Serial.print("Sending: ");
        // Serial.println(mess);
    }
    return lastSendTime;
}

//Webhook
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
void IoTNodeThingSpeak::TSBulkWriteCSV(String csvData)
{
    String data = "{\"c\":\"" + _channelID + "\",\"k\":\"" + _writeKey + "\",\"t\":\"" + "absolute" + "\",\"d\":\"" + csvData + "\"}";
    Particle.publish("TSBulkWriteCSV", data, PRIVATE);
    Serial.print("writing: ");
    Serial.println(data);
}

bool IoTNodeThingSpeak::isQueueEmpty()
{
    return framTSData.isEmpty();
}


void IoTNodeThingSpeak::TSBulkWriteCSV(String channel, String api_key, String time_format, String csvData)
{

  String data = "{\"c\":\"" + channel + "\",\"k\":\"" + api_key + "\",\"t\":\"" + time_format + "\",\"d\":\"" + csvData + "\"}";

  Particle.publish("TSBulkWriteCSV", data, PRIVATE);

  }

  // Private Methods /////////////////////////////////////////////////////////////
  // Functions only available to other functions in this library
  String IoTNodeThingSpeak::urlencode(String str)
  {
      String encodedString="";
      char c;
      char code0;
      char code1;
      //char code2;
      for (unsigned int i =0; i < str.length(); i++){
        c=str.charAt(i);
        if (c == ' '){
          encodedString+= '+';
        } else if (isalnum(c)){
          encodedString+=c;
        } else{
          code1=(c & 0xf)+'0';
          if ((c & 0xf) >9){
              code1=(c & 0xf) - 10 + 'A';
          }
          c=(c>>4)&0xf;
          code0=c+'0';
          if (c > 9){
              code0=c - 10 + 'A';
          }
          //code2='\0';
          encodedString+='%';
          encodedString+=code0;
          encodedString+=code1;
          //encodedString+=code2;
        }
      }
      return encodedString;

}

// NOT fully tested - note that a callback is needed to retrieve the channel
// number and keys.
// This function takes equal length char arrays and creates a channel.
// "end" must be the last value in the array.
// The Particle.publish limits the data to 256 bytes so the array index is returned
// where the createTSChan function left off
// so that additional fields can be updated using updateTSChan.
// A returnIndex of -1 means that there are nothing more to update.
// Although a POST would be easier here this method of using webhooks keeps the
// ThingSpeak User API Key secure by not exposing it in the device.
// Returns true if it did not exit prematurely but the real test is if a valid
// webhook callback occurs and returns the channel id and keys.
// {
//     "event": "TSCreateChannel",
//     "responseTopic": "{{PARTICLE_DEVICE_ID}}/hook-response/TSCreateChannel",
//     "url": "https://api.thingspeak.com/channels.json",
//     "requestType": "POST",
//     "noDefaults": true,
//     "rejectUnauthorized": true,
//     "responseTemplate": "{\"i\":{{id}},\"w\":{{api_keys.0.api_key}}\", \"r\":\"{{api_keys.1.api_key}}\"}",
//     "headers": {
//         "Content-Type": "application/x-www-form-urlencoded"
//     },
//     "form": {
//         "api_key": "your_user_api_key",
//         "description": "{{d}}",
//         "elevation": "{{e}}",
//         "field1": "{{1}}",
//         "field2": "{{2}}",
//         "field3": "{{3}}",
//         "field4": "{{4}}",
//         "field5": "{{5}}",
//         "field6": "{{6}}",
//         "field7": "{{7}}",
//         "field8": "{{8}}",
//         "latitude": "{{a}}",
//         "longitude": "{{o}}",
//         "name": "{{n}}",
//         "public_flag": "{{f}}",
//         "tags": "{{t}}",
//         "url": "{{u}}",
//         "metadata": "{{m}}"
//     }
// }
boolean IoTNodeThingSpeak::TSCreateChan(char const* keys[], char const* values[], int& returnIndex)
{
    char pub[622]; // Size limited by Particle.publish()
    strcpy(pub, "{");
    boolean valid = false;
    uint8_t i=0;
    int len = 1;
    boolean arrayEnd = false;
    boolean done = false;
    unsigned long startTime = millis();
    unsigned long timeOut = 5000;
    while (!done && ((millis() - startTime) < timeOut)) // && (strcmp(labels[i], "end")!=0)
    {
        switch (TSstate) {
            case START:
                if (strcmp(values[i], "end")==0)
                {
                    // Got to the end with no first value
                    valid = false;
                    done = true;
                }
                else
                {
                    if (strlen(values[i])>0)
                    {
                        len = len + strlen(keys[i]) + strlen(values[i]) + 6;
                        if (len<=256)
                        {
                            // Seems lighter to just build the json string rather than
                            // using ArduinoJson library
                            strcat(pub, "\"");
                            strcat(pub, keys[i]);
                            strcat(pub, "\":\"");
                            strcat(pub, values[i]);
                            strcat(pub, "\"");
                            // pub = pub + "\"" + names[i] + "\":\"" + values[i] + "\"";
                            TSstate = ADD_NEXT;
                        }
                        else
                        {
                            // First record is longer than it can be
                            valid = false;
                            done = true;
                        }
                    }
                    i++;
                }
            break;

                TSstate = ADD_NEXT;
            break;

            case ADD_NEXT:
                if (strcmp(values[i], "end")==0)
                {
                    // Got to the end
                    arrayEnd = true;
                    TSstate = CREATE_CHANNEL;
                    break;
                }
                else
                {
                    if (strlen(values[i])>0)
                    {
                        len = len + strlen(keys[i]) + strlen(values[i]) + 6;
                        if (len<=622)
                        {
                            strcat(pub, ",\"");
                            strcat(pub, keys[i]);
                            strcat(pub, "\":\"");
                            strcat(pub, values[i]);
                            strcat(pub, "\"");
                            // pub = pub + ",\"" + names[i] + "\":\"" + values[i] + "\"";
                        }
                        else
                        {

                           // Buffer is full
                           TSstate = CREATE_CHANNEL;
                           break;
                        }
                    }
                }

                i++;
            break;

            case CREATE_CHANNEL:
                //Close json
                strcat(pub, "}");
                Particle.publish("TSCreateChannel",pub,PRIVATE);
                valid = true;
                //Serial.println(pub);
                if (arrayEnd)
                {
                    returnIndex = -1;
                }
                else
                {
                    returnIndex = (int)i;
                }
                done = true;
            break;

        }
    }

    return valid;
}

boolean IoTNodeThingSpeak::updateTSChan(char const* channel, char const* values[], char const* labels[], int& arrayIndex)
{
    char pub[622];
    unsigned long startTime = millis();
    unsigned long timeOut = 13000;
    boolean valid = false;
    boolean done = false;
    int count = 0;
// delay(1000);
    //unsigned long beginTime = millis();
    delay(1);
    // pointer to array values
    uint8_t i=(uint8_t)arrayIndex;
    int len;

    while (!done && ((millis() - startTime) < timeOut))
    {

        if (strcmp(values[i], "end")==0)
        {
            // Done
            done = true;
            break;
        }
        else
        {

            if (strlen(values[i])>0)
            {
                //Serial.println(millis()-beginTime);

                len = strlen(labels[i]) + strlen(values[i]) + strlen(channel) + 15;
                if (len<=622)
                {
                    strcpy(pub, "{\"n\":\"");
                    strcat(pub, labels[i]);
                    strcat(pub, "\",\"d\":\"");
                    strcat(pub, values[i]);
                    strcat(pub, "\",\"c\":\"");
                    strcat(pub, channel);
                    strcat(pub, "\"}");
                    // Need to rate limit to 4 every 4 seconds
                    // if (count >= 4)
                    // {
                    //     delay(4001);
                    //     // simple blocking delay
                    //     count = 0;
                    // }
                    delay(1001);
                    // {"c":"","n":"","d":"","c":""} = 29
                    Particle.publish("TSWriteOneSetting", pub, PRIVATE);
                    valid = true;
                    //Serial.println(pub);
                }

                count++;
            }
        }
        i++;
    }
    return valid;
}

void IoTNodeThingSpeak::TSWriteOneSetting(int channelNum, String value, String label)
{
  String pub = "{\"n\":\"" + label + "\",\"d\":\"" + value + "\",\"c\":\"" + String(channelNum) + "\"}";
  Particle.publish("TSWriteOneSetting", pub, PRIVATE);
}

//https://stackoverflow.com/questions/277772/avoid-trailing-zeroes-in-printf
String IoTNodeThingSpeak::minimiseNumericString(String ss, int n) {
    int str_len = ss.length() + 1;
    char s[str_len];
    ss.toCharArray(s, str_len);

//Serial.println(s);
    char *p;
    int count;

    p = strchr (s,'.');         // Find decimal point, if any.
    if (p != NULL) {
        count = n;              // Adjust for more or less decimals.
        while (count >= 0) {    // Maximum decimals allowed.
             count--;
             if (*p == '\0')    // If there's less than desired.
                 break;
             p++;               // Next character.
        }

        *p-- = '\0';            // Truncate string.
        while (*p == '0')       // Remove trailing zeros.
            *p-- = '\0';

        if (*p == '.') {        // If all decimals were zeros, remove ".".
            *p = '\0';
        }
    }
    return String(s);
}
