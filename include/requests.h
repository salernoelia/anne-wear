#ifndef REQUESTS_H
#define REQUESTS_H
#include <ArduinoWebsockets.h>

namespace ws = websockets;
extern ws::WebsocketsClient client;

#include "config.h"   
#include <HTTPClient.h> 
#include <ArduinoJson.h>
#include <M5Unified.h> 

struct Task {
    int64_t ID;         
    String UserID;      
    String Title;       
    String Description; 
    String DueDate;      
    bool Completed;     
    String CreatedAt;  
};

extern std::vector<Task> tasks;



bool checkIfServerRespondsOK();
void connectWebSocketIfNeeded();
void sendPingWebSocket();

void sendAudioPacketOverWebSocket(
    int16_t* rec_data,
    size_t record_size
);

bool getUserTasksThatAreStillDue();
void sendWebsocketMessageIsOver();


#endif // REQUESTS_H
