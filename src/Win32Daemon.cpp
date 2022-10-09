/**
 * Copyright (C) 2022, polistern
 *
 * This file is part of pboted and licensed under BSD3
 *
 * See full license text in LICENSE file at top of project tree
 */

#ifdef WIN32 // Windows-only

#include <fcntl.h>
#include <csignal>
#include <cstdlib>
#include <sys/stat.h>
#include <thread>
#include <unistd.h>

#include "BoteContext.h"
#include "ConfigParser.h"
#include "BoteDaemon.h"
#include "DHTworker.h"
#include "FileSystem.h"
#include "Logging.h"
#include "RelayWorker.h"


namespace pbote
{
namespace util
{

int DaemonWin32::start()
{
  // Pidfile
  // this code is c-styled and a bit ugly, but we need fd for locking pidfile
  pbote::config::GetOption("pidfile", pidfile);
  if (pidfile.empty())
    {
      pidfile = pbote::fs::DataDirPath("pbote.pid");
    }
  if (!pidfile.empty())
    {
      pidFH = open(pidfile.c_str(), O_RDWR | O_CREAT, 0600);
      if (pidFH < 0)
        {
          LogPrint(eLogError, "Daemon: Could not create pidfile ", pidfile,
                   ": ", strerror(errno));
          return EXIT_FAILURE;
        }

      char pid[10];
      sprintf(pid, "%d\n", getpid());
      ftruncate(pidFH, 0);
      if (write(pidFH, pid, strlen(pid)) < 0)
        {
          LogPrint(eLogError, "Daemon: Can't write pidfile: ", strerror(errno));
          return EXIT_FAILURE;
        }
    }

  gracefulShutdownInterval = 0; // not specified

  return Daemon_Singleton::start();
}

bool DaemonWin32::stop()
{
  if (running)
    running = false;

  pbote::fs::Remove(pidfile);

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