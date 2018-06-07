#include "nel/net/buf_server_websocket_func.h"
#include "nel/misc/sha1.h"
#include "nel/misc/base64.h"
#include "nel/net/buf_sock.h"
#include "nel/net/callback_server_websocket.h"
#include "nel/net/buf_server_websocket.h"

#include "event2/bufferevent_ssl.h"
#include "openssl/ssl.h"

using namespace std;
using namespace NLMISC;
using namespace NLNET;


CSString NLNET::generate_key( const NLMISC::CSString &key )
{
    CSString tmp = key + "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";
    CHashKey hash_key = getSHA1( (const uint8 *)tmp.c_str(), tmp.size() );
    return base64_encode( hash_key.HashKeyString );
}

CSString NLNET::generate_websocket_response( CSString& sec_websocket_key )
{
    CSString resp;

    resp += "HTTP/1.1 101 WebSocket Protocol HandShake\r\n";
    resp += "Connection: Upgrade\r\n";
    resp += "Upgrade: WebSocket\r\n";
    //resp += "Server: WebChat Demo Server\r\n";
    resp += "Sec-WebSocket-Accept: " + generate_key(sec_websocket_key) + "\r\n";
    resp += "\r\n";

    return resp;
}

sint32 NLNET::parse_frame_header( const uint8 *buf, WebSocketFrame& frame )
{
    if (!buf)
    {
        return -1;
    }

    unsigned char c1 = *buf;
    unsigned char c2 = *(buf + 1);
    frame.fin = (c1 >> 7) & 0xff;
    frame.opcode = c1 & 0x0f;
    frame.mask = (c2 >> 7) & 0xff;
    frame.payload_len = c2 & 0x7f;

    return 0;
}

void NLNET::ws_socket_read_cb( bufferevent *bev, void *args )
{  
    char read_buffer[4096];  
    size_t  len = bufferevent_read(bev, read_buffer, sizeof(read_buffer) );
    CServerBufSock* pBufSock = (CServerBufSock*)args;

    if( !pBufSock->m_Handshake )
    {
        if( len < sizeof(read_buffer) )
        {
            read_buffer[len] = '\0';
            CSString  http_head(read_buffer);

            CVectorSString  split_req;
            http_head.splitLines( split_req );

            for ( uint i=0; i<split_req.size(); ++i )
            {
                CVectorSString  split_key;
                split_req[i].splitBySeparator(':', split_key);

                if ( split_key.size() == 2 && split_key[0] == "Sec-WebSocket-Key"  )
                {
                    CSString res_key = generate_websocket_response( split_key[1].strip() );
                    bufferevent_write(bev, res_key.c_str(), res_key.size() );
                    pBufSock->m_Handshake = true;
                }
            }
        }
    }
    else
    {
        uint32 buff_len = pBufSock->appendToBuffer( (const uint8*)read_buffer, len );

        while( buff_len >= 2 )
        {
            WebSocketFrame frame;

            uint8* buff = pBufSock->getBuffer();
            parse_frame_header( buff, frame );

            uint32 offset = 2;
            if (frame.payload_len == 126)
            {
                frame.payload_len = ntohs(*(uint16*)(buff+offset));
                offset = 4;
            }
            else if (frame.payload_len == 127)
            {
                frame.payload_len = myntohll(*(uint64*)(buff+offset));
                offset = 10;
            }
            else if( frame.payload_len < 126 )
            {
                if( frame.fin == 1 && frame.opcode == WEBSOCK_FRAME_DIS_CONNECT )
                {
                    if( frame.mask == 1 )  {  offset += 4;  }

                    //0x8 denotes a connection close
                    buff_len = pBufSock->leftShiftBuffer( offset + frame.payload_len );

                    nlinfo( "WebSocket Connection Close 0x8." );
                    continue;
                }
                else if( frame.fin == 1 && frame.opcode == WEBSOCK_FRAME_PING )
                {
                    //0x9 denotes a ping
                    if( frame.mask == 1 )  {  offset += 4;  }
                    buff_len = pBufSock->leftShiftBuffer( offset + frame.payload_len );
                    continue;
                }
                else if( frame.opcode == WEBSOCK_FRAME_HAVE_NEXT )
                {
                    nlwarning("frame.opcode == 0x0");
                    if( frame.mask == 1 )  {  offset += 4;  }
                    buff_len = pBufSock->leftShiftBuffer( offset + frame.payload_len );
                }
                else if( frame.opcode == WEBSOCK_FRAME_TEXT || frame.opcode == WEBSOCK_FRAME_BIN )
                {
                    
                }
                else
                {
                    nlwarning( "frame.opcode = %d", frame.opcode );
                    if( frame.mask == 1 )  {  offset += 4;  }
                    buff_len = pBufSock->leftShiftBuffer( offset + frame.payload_len );
                    continue;
                }
            }

            uint32 data_len = frame.payload_len + offset + 4;

            if( buff_len >= data_len )
            {
                ///  ����������Ѿ��չ���һ����������Ϣ

                if( frame.mask == 1 )
                {
                    // load mask
                    memcpy( frame.masking_key, buff+offset, 4 );
                    offset += 4;

                    // unmask
                    unmask_payload_data( frame, buff+offset );
                }

                if( frame.fin==0 || frame.fin==1 || frame.fin==2 )
                {
                    CSString msg_buff;
                    msg_buff.assign( buff+offset, buff+offset+frame.payload_len );


                    if( msg_buff.size() >= 4 )
                    {
                        uint32 msg_type_len = ntohl(*(uint32*)msg_buff.data());

                        if( msg_type_len>9 && msg_type_len<65535 )
                        {
                            NLMISC::CMemStream& msg = pBufSock->CompleteMsg;

                            if (msg.isReading())
                            {
                                msg.invert();
                            }

                            msg.fill( (const uint8*)msg_buff.data()+sizeof(uint32), msg_type_len );

                            uint8 event_type    = CBufNetBase::User;
                            uint64 sockid       = (uint64)pBufSock;
                            msg.serial( sockid );
                            msg.serial( event_type );
                            msg.invert();

                            pBufSock->m_BufNetHandle->pushMessageIntoReceiveQueue(msg.buffer(), msg.size() );
                        }
                    }
                }

                buff_len = pBufSock->leftShiftBuffer(data_len);
            }
            else
            {
                break;
            }
        }
    }
}  

void NLNET::ws_socket_event_cb( bufferevent *bev, short events, void *args )
{
    if (events & BEV_EVENT_CONNECTED)  
    {
        return;
    }

    if (events & BEV_EVENT_EOF)  
        LNETL1_DEBUG("connection closed\n");  
    else if (events & BEV_EVENT_ERROR)  
        LNETL1_DEBUG("some other error\n");

    LNETL1_DEBUG( "socket_event_cb:%d", events );

    CServerBufSock* pBufSock = (CServerBufSock*)args;
    pBufSock->advertiseDisconnection( pBufSock->m_BufNetHandle, pBufSock );
}

void NLNET::ws_listener_cb( evconnlistener *listener, evutil_socket_t fd, struct sockaddr *sock, int socklen, void *args )
{  
    WSListenArgs*   pListenArgs = (WSListenArgs*)args;
    NLNET::SOCKET   newSock = (NLNET::SOCKET)fd;

    if ( newSock == INVALID_SOCKET )
    {
        throw ESocket( "Accept returned an invalid socket");
    }

    // Construct and save a CTcpSock object
    CInetAddress addr;
    addr.setSockAddr( (struct sockaddr_in*)sock );
    //LNETL0_DEBUG( "LNETL0: Socket %d accepted an incoming connection from %s, opening socket %d", _Sock, addr.asString().c_str(), newsock );
    CTcpSock *pTcpSock = new CTcpSock( newSock, addr );
    CServerBufSock *pBufSock = new CServerBufSock( pTcpSock );
    LNETL1_DEBUG( "LNETL1: New connection : %s", pBufSock->asString().c_str() );

    // Notify the new connection
    pBufSock->advertiseConnection( pListenArgs->pServer );

    //Ϊ����ͻ��˷���һ��bufferevent  
    bufferevent *bev =  NULL;
    
    if ( pListenArgs->pSslCtx!=NULL )
    {
        SSL* pSsl   = SSL_new((SSL_CTX *)pListenArgs->pSslCtx);
        bev         = bufferevent_openssl_socket_new(pListenArgs->pEventBase, fd, pSsl, BUFFEREVENT_SSL_ACCEPTING, BEV_OPT_CLOSE_ON_FREE|BEV_OPT_THREADSAFE|BEV_OPT_DEFER_CALLBACKS);
        pBufSock->m_Ssl = pSsl;
    }
    else
    {
        bev = bufferevent_socket_new(pListenArgs->pEventBase, fd, BEV_OPT_CLOSE_ON_FREE | BEV_OPT_THREADSAFE | BEV_OPT_DEFER_CALLBACKS);
    }

    pBufSock->m_BufNetHandle    = pListenArgs->pServer;
    pBufSock->m_BEVHandle       = bev;

    bufferevent_setcb(bev, ws_socket_read_cb , NULL, ws_socket_event_cb, (void*)pBufSock);
    bufferevent_enable(bev, EV_READ | EV_PERSIST); 
}  

void NLNET::fill_frame_buffer( const uint8* payload_data, uint32 payload_len, NLMISC::CObjectVector<uint8>& out_frame, uint8 opcode, uint8 fin/* =1 */ )
{
    out_frame.clear();

    if (fin > 1 || opcode > 0xf) {
        nlwarning("fill_frame_buffer  fin>1  opcode>0xf");
        return;
    }

    uint32  buff_len = payload_len + 4;
    uint8   mask = 0;               //  must not mask at server endpoint
    uint8   masking_key[4] = {0};   //  no need at server endpoint
    uint8   c1 = 0x00;
    uint8   c2 = 0x00;

    c1 = c1 | (fin << 7);           //  set fin
    c1 = c1 | opcode;               //  set opcode
    c2 = c2 | (mask << 7);          //  set mask

    if ( buff_len == 0 )
    {
        if (mask == 0)
        {
            out_frame.resize(2);
            out_frame[0] = c1;
            out_frame[1] = c2;
        }
        else
        {
            out_frame.resize(2+4);
            out_frame[0] = c1;
            out_frame[1] = c2;

            memcpy( &out_frame[2], masking_key, sizeof(masking_key) );
        }
    }
    else if ( buff_len <= 125 )
    {
        if (mask == 0)
        {
            out_frame.resize(2+buff_len);

            out_frame[0] = c1;
            out_frame[1] = c2 + buff_len;

            uint32 netlen = myhtonl( (uint32)payload_len );
            *(uint32*)&(out_frame[2])=netlen;

            memcpy( out_frame.getPtr()+2+sizeof(uint32), payload_data, payload_len );
        }
        else
        {
            out_frame.resize(2+4+buff_len);      // frame len + mask len + payload len + data

            out_frame[0] = c1;
            out_frame[1] = c2 + buff_len;

            memcpy( out_frame.getPtr()+2, masking_key, sizeof(masking_key) );

            uint32 netlen = myhtonl( (uint32)payload_len );
            *(uint32*)&(out_frame[6])=netlen;

            memcpy( out_frame.getPtr()+10, payload_data, payload_len );
        }
    }
    else if ( buff_len >= 126 && buff_len <= 65535 )
    {
        if (mask == 0)
        {
            out_frame.resize(4+buff_len);      //  frame len + payload len + data

            out_frame[0] = c1;
            out_frame[1] = c2 + 126;

            uint16 tmplen = myhtons((uint16)buff_len);
            memcpy( out_frame.getPtr()+2, &tmplen, 2 );

            uint32 netlen = myhtonl( (uint32)payload_len );
            *(uint32*)&(out_frame[4])=netlen;

            memcpy( out_frame.getPtr()+8, payload_data, payload_len );
        }
        else
        {
            out_frame.resize(4+4+buff_len);

            out_frame[0] = c1;
            out_frame[1] = c2 + 126;

            uint16 tmplen = myhtons((uint16)buff_len);
            memcpy( out_frame.getPtr()+2, &tmplen, 2 );
            memcpy( out_frame.getPtr()+4, masking_key, sizeof(masking_key) );

            uint32 netlen = myhtonl( (uint32)payload_len );
            *(uint32*)&(out_frame[8])=netlen;

            memcpy( out_frame.getPtr()+12, payload_data, payload_len );
        }
    }
    else if ( buff_len >= 65536 )
    {
        if (mask == 0)
        {
            out_frame.resize(2+8+buff_len);

            out_frame[0] = c1;
            out_frame[1] = c2 + 127;

            uint64 tmplen = myhtonll(buff_len);
            memcpy( out_frame.getPtr()+2, &tmplen, 8 );

            uint32 netlen = myhtonl( (uint32)payload_len );
            *(uint32*)&(out_frame[10])=netlen;

            memcpy( out_frame.getPtr()+14, payload_data, payload_len );
        }
        else
        {
            out_frame.resize(2+8+4+buff_len);

            out_frame[0] = c1;
            out_frame[1] = c2 + 127;

            uint64 tmplen = myhtonll(buff_len);
            memcpy( out_frame.getPtr()+2, &tmplen, 8 );
            memcpy( out_frame.getPtr()+10, masking_key, sizeof(masking_key) );

            uint32 netlen = myhtonl( (uint32)payload_len );
            *(uint32*)&(out_frame[14])=netlen;

            memcpy( out_frame.getPtr()+18, payload_data, payload_len );
        }
    }
}


