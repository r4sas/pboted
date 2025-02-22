/**
 * Copyright (C) 2019-2022, polistern
 *
 * This file is part of pboted and licensed under BSD3
 *
 * See full license text in LICENSE file at top of project tree
 */

#ifndef PACKET_HANDLER_H__
#define PACKET_HANDLER_H__

#include <cstdint>
#include <functional>
#include <future>
#include <map>
#include <memory>
//#include <queue>
#include <string>
#include <thread>
#include <tuple>

#include "Logging.h"
#include "Packet.h"

namespace pbote
{
namespace packet
{

/// Timeout in msec
#define PACKET_RECEIVE_TIMEOUT 500

class IncomingRequest;
class RequestHandler;

typedef bool (IncomingRequest::*incomingPacketHandler) (
    const sp_comm_pkt &packet);
// typedef bool (OutgoingRequest::*outgoingPacketHandler)(const sp_comm_pkt
// &packet);

class IncomingRequest
{
public:
  IncomingRequest (RequestHandler& owner);

  bool handleNewPacket (const sp_queue_pkt &packet);

private:
  bool receiveRelayRequest (const sp_comm_pkt &packet);
  bool receiveRelayReturnRequest (const sp_comm_pkt &packet);
  bool receiveFetchRequest (const sp_comm_pkt &packet);
  bool receiveResponsePkt (const sp_comm_pkt &packet);
  bool receivePeerListRequest (const sp_comm_pkt &packet);
  ///
  bool receiveRetrieveRequest (const sp_comm_pkt &packet);
  bool receiveDeletionQueryRequest (const sp_comm_pkt &packet);
  bool receiveStoreRequest (const sp_comm_pkt &packet);
  bool receiveEmailPacketDeleteRequest (const sp_comm_pkt &packet);
  bool receiveIndexPacketDeleteRequest (const sp_comm_pkt &packet);
  bool receiveFindClosePeersRequest (const sp_comm_pkt &packet);

  incomingPacketHandler i_handlers[256];
  RequestHandler& m_owner;
};

class RequestHandler
{
public:
  RequestHandler ();
  ~RequestHandler ();

  void start ();
  void stop ();

  bool
  isRunning () const
  {
    return m_running;
  };

  std::shared_ptr<std::thread> get_request_thread ();

private:
  void run ();

  bool m_running;
  std::unique_ptr<std::thread> m_main_thread; //, m_request_thread;
  std::shared_ptr<std::thread> m_request_thread;
  queue_type m_recv_queue, m_send_queue;
};

extern RequestHandler packet_handler;

} // namespace packet
} // namespace pbote

#endif // PACKET_HANDLER_H__
