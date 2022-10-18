/**
 * Copyright (C) 2022, R4SAS, PurpleBote team
 *
 * This file is part of pboted and licensed under BSD3
 *
 * See full license text in LICENSE file at top of project tree
 */

#ifdef _WIN32 // Windows-only

#include <signal.h>
#include <thread>

#include "BoteDaemon.h"
#include "DHTworker.h"
#include "Logging.h"

#include "win32/Service.h"


class Service : public WindowsService {
  using WindowsService::WindowsService;

protected:
  virtual DWORD WINAPI worker(LPVOID)
  {
    Daemon.run();
    return ERROR_SUCCESS;
  }
  virtual void on_startup()
  {
    Daemon.start();
  }
  virtual void on_stop()
  {
    Daemon.stop();
  }
};

void SignalHandler(int sig)
{
  switch (sig)
    {
      case SIGINT:
      case SIGABRT:
      case SIGTERM:
        LogPrint(eLogWarning, "Daemon: signal received");
        Daemon.running = false;
        break;
      default:
        LogPrint(eLogWarning, "Daemon: Unknown signal received: ", sig);
        break;
    }
}

namespace pbote
{
namespace util
{

bool DaemonWin32::init(int argc, char* argv[])
{
  Daemon_Singleton::init(argc, argv);

  if (isDaemon)
    {
      Service pboted("pboted service", false);
      pboted.run();
      return false; // Application terminated, no need to continue it more
    }
  else
    {
      pbote::log::SetThrowFunction ([](const std::string& s)
        {
          MessageBox(0, TEXT(s.c_str ()), TEXT("pboted"), MB_ICONERROR | MB_TASKMODAL | MB_OK );
        }
      );
    }
  return false;
}

int DaemonWin32::start()
{
  signal(SIGINT, SignalHandler);
  signal(SIGABRT, SignalHandler);
  signal(SIGTERM, SignalHandler);

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
      std::this_thread::sleep_for(std::chrono::seconds(1));

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
