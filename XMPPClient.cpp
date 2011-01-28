#include "XMPPClient.h"

XMPPClient::XMPPClient(IpAddr server, char* serverDomain,void (*callback)(char*))
{
    
    this->serverDomain = serverDomain;
    this->callback_msg = callback;
    
    m_state = INIT;
    xmpp_statu = READ_SASL;
    serverIp = server;
    
    m_closed = true;
    m_connected = false;
    
}

XMPPClient::~XMPPClient()
{


}

int XMPPClient::connect(char* user, char* pass)
{
    this->userName = user;
    this->password = pass;
    
    /*Initial TCP socket*/
    pTCPSocket = new TCPSocket;
    pTCPSocket->setOnEvent(this, &XMPPClient::onTCPSocketEvent);

    //Net::poll();
    
    host.setPort(5222);
    host.setIp(serverIp);
    host.setName(serverDomain);
    
    /*Trying to connect to host*/
    printf("Trying to connect to host..\r\n\r\n");
    TCPSocketErr err = pTCPSocket->connect(host);
    
    if(err){
        printf("Error connecting to host [%d]\r\n", (int) err);
        return -1;
    } else {
        printf("Connect to host sucessed..\r\n\r\n");
    }        
    
    m_state = CONNECTING;
    
    /*Wait TCP connection with server to be established*/
    while(!m_connected){
         Net::poll();
         wait(1);
         //printf("Wait for connections..\r\n\r\n");
    }
    
    /*Do SASL, resouce binding and open session*/
    while(m_state != SESSION_STARTED){
        Net::poll();
        wait(1);
        //printf("State: %d..\r\n\r\n", m_state);
    }
    
    return 1;
}

void XMPPClient::send_message(char* to, char* msg){
    
    //char temp[strlen(to) + strlen(msg) + strlen(msgStanzTemp) + 10];
    char temp[1024];
    sprintf(temp, msgStanzTemp, to, msg);
    send_data(temp);
}

void XMPPClient::close()
{   
    /*First check the connection has not been closed before*/
    if(m_closed)
    {
        return;
    }
    
    /*Inform server the session is over*/
    send_data(closeStream);
    
    m_state = XMPP_CLOSED;
    m_closed = true; //Prevent recursive calling or calling on an object being destructed by someone else
    //m_watchdog.stop(); //Stop timeout
    //m_watchdog.reset();
    
    /*Reset TCP event to NULL and close socket*/
    pTCPSocket->resetOnEvent();
    pTCPSocket->close();
    delete pTCPSocket;
    pTCPSocket = NULL;
}

void XMPPClient::send_data(const char* msg){
    
    int transLen;
    
    transLen = pTCPSocket->send(msg,strlen(msg));
    
    /*Check send length matches the message length*/
    if(transLen != strlen(msg)){
        printf("Error in send %d\r\n", m_state);
    } else {
        if(m_state != NEXT){
            //printf("State %d message send correctlly..\r\n", m_state);
        }
    }
}


/**
  *Authorization in PLAIN
  */
void XMPPClient::send_auth_plain()
{
    char send[512], encoded[512], user[1024], pass[1024];
    
    strcpy(user, userName);
    strcpy(pass, password);
    
    /*Wrap to "\0userName\0passWord" */
    char key[2 + sizeof(user) + sizeof(pass)];
    memset(key,0,sizeof(key));
    memcpy(key+1,user, strlen(user));
    memcpy(key+2+strlen(user), pass, strlen(pass));
    
    /*Encode user name and password in Base64*/
    base64_encode(encoded, key, strlen(user) + strlen(pass) + 2); 
    sprintf(send, authPlainTemp, encoded);
    //printf("The encoded is: %s\r\n", encoded);
    //printf("The Send is: %s\r\n", send);
    send_data(send);
}

/**
  *Send data trough TCP connection
  */
void XMPPClient::request_session()
{
    char temp[512];
    sprintf(temp, sessionRequestTemp, serverDomain); 
    send_data(temp);
}


//Send open stream
void XMPPClient::open_stream()
{
    char temp[512];
    sprintf(temp, openStreamTemp, serverDomain); 
    send_data(temp);
}

char* XMPPClient::parse_msg(char *msg, char *pattern)
{
    char content[1024], last[512], out[1024], temp[512];
    char *textPart, *ptrContent;
    int i, targetLen, flag;

    strcpy(content, msg);
    strcpy(temp, pattern);
    
    targetLen = strlen(pattern);

    textPart = strtok((char*)content, "<>");

    while(textPart != NULL) {
        ptrContent = textPart;
        if(strlen(last) != 0 ) {
            flag = 0;
            for(i = 0; i < targetLen; i++){
                if(last[i] != temp[i]){
                    flag = 1;
                    break;
                }
            }
            if(!flag){
                strcpy(out,textPart);
                return out;
            }
        }
        if(strlen(textPart) >= targetLen){
            for(i = 0; i < targetLen; i++){
                last[i] = *ptrContent;
                ptrContent++;
            }
        }
        textPart = strtok(NULL, "<>");
    } 
    return "";
}
   
void XMPPClient::read_data()
{
    char buf[1024];
    int len = 0, readLen;

    while((readLen = pTCPSocket->recv(buf, 1024)) != 0){
        len += readLen;
    }
    
    buf[len] = '\0';
    
    //printf("Read length: %d\r\n%s\r\n",len, buf);
    
    char bufffer[1024];
    strcpy(bufffer, parse_msg(buf,"body"));
    
    if(xmpp_statu == READ_MSG)
    {
        //printf("Read length: %d\r\n%s\r\n",len, buf);
        //printf("Received Message: %s\r\n\r\n", bufffer);
        callback_msg(bufffer);
        //send_message("mirror@ceit.org", parse_msg(buf, "body"));
    }
}

void XMPPClient::onTCPSocketEvent(TCPSocketEvent e)
{
    switch(e)
    {
        case TCPSOCKET_ACCEPT:
            printf("New TCPSocketEvent: TCPSOCKET_ACCEPT\r\n");
            break;
        case TCPSOCKET_CONNECTED: 
            printf("New TCPSocketEvent: TCPSOCKET_CONNECTED\r\n");
            m_connected = true;
            open_stream();
            m_state = AUTH_PLAIN;
            break;
        case TCPSOCKET_WRITEABLE: 
            //printf("New TCPSocketEvent: TCPSOCKET_WRITEABLE\r\n");  
            switch(m_state){
                case AUTH_PLAIN:  //Select Plain Authration method
                    send_auth_plain();
                    m_state = AUTH_NEW_STREAM;
                    break;
                case AUTH_NEW_STREAM:
                     open_stream();
                     m_state = CLIENT_BIND;
                     break;
                case CLIENT_BIND:
                     send_data(clientBind);
                     m_state = NEW_SESSION;
                     break;
                case NEW_SESSION:
                     request_session();
                     m_state = PRESENCE;
                     break;
                case PRESENCE:
                     send_data(presenceStanz);
                     m_state = MSG;
                     break;
                case MSG:
                     this->send_message("mirror@ceit.org", "Hello, here is mbed!!!");
                     m_state = SESSION_STARTED;
                     xmpp_statu = READ_MSG;
                     break;
                default:
                    //printf("Nothing to send..\r\n");
            }
            break;
        case TCPSOCKET_READABLE:
            //printf("New TCPSocketEvent: TCPSOCKET_READABLE\r\n");
            read_data();
            break;
        case TCPSOCKET_CONTIMEOUT:
            printf("New TCPSocketEvent: TCPSOCKET_CONTIMEOUT\r\n");
            break;
        case TCPSOCKET_CONRST:
            printf("New TCPSocketEvent: TCPSOCKET_CONRST\r\n");
            break;
        case TCPSOCKET_CONABRT:
            printf("New TCPSocketEvent: TCPSOCKET_CONABRT\r\n");
            break;
        case TCPSOCKET_ERROR:
            printf("New TCPSocketEvent: TCPSOCKET_ERROR\r\n");
            break;
        case TCPSOCKET_DISCONNECTED:
            printf("New TCPSocketEvent: TCPSOCKET_DISCONNECTED\r\n");
            pTCPSocket->close();
            m_connected = false;
            m_state = DISCONNECTED;
            break;
    }
}
