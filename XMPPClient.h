/**
  *A simple XMPP  client on mbed. 
  *
  *By Yilun FAN, @CEIT, @JAN 2011
  *
  */
#ifndef XMPP_CLIENT_H
#define XMPP_CLIENT_H

#include "mbed.h"
#include "TCPSocket.h"
#include "base64.h"

const char md5[] = "<auth xmlns='urn:ietf:params:xml:ns:xmpp-sasl'"
                   " mechanism='DIGEST-MD5'/>\r\n";

                   
const char presenceStanz[] = "<presence>"
                            "<status>On mbed</status>"
                            "</presence>\r\n";  
                            

const char clientBind[] = "<iq type='set' id='bind_1'>"
                          "<bind xmlns='urn:ietf:params:xml:ns:xmpp-bind'/>"
                          "</iq>\r\n";
                          
                          
const char closeStream[] = "<presence type='unavailable'/>"
                          "</stream:stream>";              

/*XML stream and XMPP stanza templates*/
//1.to  2.message
//sprintf(temp, msgStanzTemp, to, msg);                             
const char msgStanzTemp[] = "<message to='%s'" 
                            " xmlns='jabber:client'"
                            " type='chat'"
                            " xml:lang='en'>"
                            "<body>%s</body>"
                            "</message>\r\n";          

const char preStatuStanzTemp[] = "<presence>"
                            "<status>%s</status>"
                            "</presence>\r\n";  
                            
//1.serverDomain
//sprintf(temp, openStreamTemp, serverDomain); 
const char openStreamTemp[] = "<stream:stream "
                          "xmlns='jabber:client' "
                          "xmlns:stream='http://etherx.jabber.org/streams' "
                          "to='%s' "
                          "version='1.0'>\r\n";
                          
//1.key
//sprintf(temp, authPlainTemp, key);                                                 
//\0mbed\0mirror
const char authPlainTemp[] = "<auth xmlns='urn:ietf:params:xml:ns:xmpp-sasl'"
                             " mechanism='PLAIN'>%s</auth>\r\n"; 

//1.serverDomain
//sprintf(temp, sessionRequestTemp, serverDomain); 
const char sessionRequestTemp[] = "<iq to='%s'"
                              " type='set'"
                              " id='sess_1'>"
                              "<session xmlns='urn:ietf:params:xml:ns:xmpp-session'/>"
                              "</iq>\r\n";


enum XMPPState {
    INIT = 0,
    CONNECTING = 1,
    AUTH_PLAIN = 2,
    AUTH_MD5 = 8,
    AUTH_NEW_STREAM = 3,
    CLIENT_BIND = 4,
    NEW_SESSION = 5,
    PRESENCE = 6,
    MSG = 7,
    NEXT = 111,
    SESSION_STARTED = 8,
    DISCONNECTED  = 5222,
    XMPP_CLOSED,
    STOP = 5280,
};
    
enum READState {
    READ_SASL,
    READ_BIND,
    READ_SESSION,
    READ_NEW_STREAM,
    READ_MSG,
};

class XMPPClient
{
public:
    /*Instantiates the XMPP client*/
    XMPPClient(IpAddr server, char* serverDomain,void (*callback)(char*));
    ~XMPPClient();
    
    /**
      *Connect to jabber server.
      *@param user: user name of a JID want to log on.
      *@param pass: passworld of the JID.
      *
      *@return 1: if a session is sucessful opened otherwise return <0.
      */
    int connect(char* user, char* pass);
    
    /**
      *Send a message sentza to server.
      *@param to: The destination of the message.
      *@param msg: The message to be sended.
      */
    void send_message(char* to, char* msg);
    
    /**
      *Tell server to close current session.
      */
    void close();

private:
    void send_data(const char* msg);
    void request_session();
    void send_auth_plain();
    char* parse_msg(char *msg, char *pattern);
    void read_data();
    void open_stream();
    
    void (*callback_msg)(char*);
    void onTCPSocketEvent(TCPSocketEvent e);
    
    TCPSocket* pTCPSocket;
    Host host;
    
    char* serverDomain;
    char* userName;
    char* password;
    
    IpAddr serverIp;
    
    bool m_connected;
    bool m_closed;
    
    XMPPState m_state;
    READState xmpp_statu;
    
};

#endif