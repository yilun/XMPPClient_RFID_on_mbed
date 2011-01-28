#include "mbed.h"
#include "EthernetNetIf.h"
#include "XMPPClient.h"
#include "ID12RFID.h"

ID12RFID rfid(p14);
ID12RFID rfid2(p27);

EthernetNetIf ethernet;  

DigitalOut tag_present(LED1);
DigitalOut ledStage1 (LED2);
DigitalOut ledStage2 (LED3);
DigitalOut ledStage3 (LED4);

volatile char stage = 0;

Ticker stage_blinker;

void stageblinker() {
    switch (stage) {
        case 0:
            ledStage1 = !ledStage1;
            ledStage2 = false;
            ledStage3 = false;
            break;
        case 1:
            ledStage1 = false;
            ledStage2 = !ledStage2;
            ledStage3 = false;
            break;
        case 2:
            ledStage1 = !ledStage1;
            ledStage2 = !ledStage2;
            ledStage3 = false;
            break;
        case 3:
            ledStage1 = false;
            ledStage2 = false;
            ledStage3 = !ledStage3;
            break;
        default:
            ledStage1 = true;
            ledStage2 = true;
            ledStage3 = true;
            stage_blinker.detach();
            break;
    }
}

char userName[] = "mbed";
char password[] = "mirror";
char serverDomain[] = "ceit.org";    
IpAddr serverIpAddr(10,1,1,5);

#define IDS_COUNT 6
const int ids_list[IDS_COUNT] = {9656082, 4478536, 4486983, 4486994, 4469045, 89481811};
const char* names_list[IDS_COUNT] = {"yilun", "Simon", "Dan", "Mark", "Peter", "Rob"};

void callback_msg(char* msg);

XMPPClient xmpp(serverIpAddr, serverDomain, callback_msg);

//Echoing message stanzas from server
void callback_msg(char* msg){
    printf("Message: %s\r\n\r\n", msg);
    xmpp.send_message("mirror@ceit.org", msg);
}

int main() {
    printf("\r\n############### XMPP TCP client #######\r\n\r\n");
    
    stage = 0;
    stage_blinker.attach_us(&stageblinker, 1000*500);
    
    EthernetErr ethErr = ethernet.setup();
    if(ethErr){
        printf("Ethernet Error %d\r\n", ethErr);  
    } else {
        printf("mbed is online...\r\n");
    }
    stage = 1;
    if(xmpp.connect(userName, password)){
        printf("\r\nConnect to server sucessed ..\r\n\r\n");
    } else {
        printf("\r\nConnect to server failed ..\r\n");
        return -1;
    }
    
    printf("\r\Please swape your card ..\r\n\r\n");
    xmpp.send_message("mirror@ceit.org", "Please swape your card ..");
    
    stage = 3;
    while(true) {
        //Read from first reader
        if(rfid.readable()) {
            int id = rfid.read();
            tag_present = 1;
            char temp[1024];
            for(int i = 0; i < IDS_COUNT; i++) {
                if (ids_list[i] == id){
                    printf("Tag: %d    Name:%s\r\n", id, names_list[i]);
                    sprintf(temp, "%s is at Room 412 now!", names_list[i]);
                    xmpp.send_message("mirror@ceit.org", temp);
                }
            }
            tag_present = 0;        
        }
        
        //Read from second reader
        if(rfid2.readable()) {
            int id2 = rfid2.read();
            tag_present = 1;
            char temp2[1024];
            for(int i = 0; i < IDS_COUNT; i++) {
                if (ids_list[i] == id2){
                    printf("Tag: %d    Name:%s\r\n", id2, names_list[i]);
                    sprintf(temp2, "%s is at Room 423 now!", names_list[i]);
                    xmpp.send_message("mirror@ceit.org", temp2);
                }
            }
            tag_present = 0;        
        }
        Net::poll();
    }
    
    xmpp.close();
    
    printf("#### End of the test.. ####\r\n\r\n");
}