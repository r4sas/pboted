/**
 * Copyright (C) 2022, R4SAS, PurpleBote team
 *
 * This file is part of pboted and licensed under BSD3
 *
 * See full license text in LICENSE file at top of project tree
 */

#ifdef _WIN32 // Windows-only

#include <thread>

#include "BoteDaemon.h"
#include "DHTworker.h"
#include "Logging.h"


namespace pbote
{
namespace util
{

int DaemonWin32::start()
{
  gracefulShutdownInterval = 0; // not specified
  return Daemon_Singleton::start();
}

bool DaemonWin32::stop()
{
  if (running)
    running = false;

  return Daemon_Singleton::stop();
}

void DaemonWin32::run()
{
  while (running)
    {
      // ToDo: check status of network, DHT, relay, etc. and try restart on error
      std::this_thread::sleep_for(std::chrono::seconds(10));

      if (pbote::network::network_worker.is_sick ())
        {
          LogPrint(eLogError, "Daemon: SAM session is sick, try to re-connect");
          pbote::network::network_worker.init ();
          pbote::network::network_worker.start ();
        }
    }
}

} // namespace util
} // namespace pbote

#endif // WIN32