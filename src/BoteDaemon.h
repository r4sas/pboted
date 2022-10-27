/**
 * Copyright (C) 2019-2022, polistern
 * Copyright (C) 2022, The PurpleBote Team
 *
 * This file is part of pboted and licensed under BSD3
 *
 * See full license text in LICENSE file at top of project tree
 */

#ifndef PBOTED_SRC_BOTEDAEMON_H
#define PBOTED_SRC_BOTEDAEMON_H

#include <condition_variable>
#include <memory>
#include <mutex>
#include <ostream>
#include <string>

namespace pbote
{
namespace util
{

class Daemon_Singleton_Private;

class Daemon_Singleton
{
public:
  virtual bool init (int argc, char *argv[],
                     std::shared_ptr<std::ostream> logstream);
  virtual bool init (int argc, char *argv[]);
  virtual int start ();
  virtual bool stop ();
  virtual void run () {};

  bool isDaemon;
  bool running;

protected:
  Daemon_Singleton();
  virtual ~Daemon_Singleton();

  bool IsService() const;

  // d-pointer for Control, SMTP, POP3, etc.
  class Daemon_Singleton_Private;
  Daemon_Singleton_Private &d;
};

#ifdef _WIN32
#define Daemon pbote::util::DaemonWin32::Instance()
class DaemonWin32 : public Daemon_Singleton
{
public:
  static DaemonWin32 &Instance()
    {
      static DaemonWin32 instance;
      return instance;
    }

  bool init(int argc, char *argv[]) override;
  int start() override;
  bool stop() override;
  void run() override;

private:
  mutable std::mutex m_cv_mutex;
  std::condition_variable m_check_cv;
};

#else // WIN32
#define Daemon pbote::util::DaemonLinux::Instance ()
class DaemonLinux : public Daemon_Singleton
{
public:
  static DaemonLinux &Instance()
    {
      static DaemonLinux instance;
      return instance;
    }
  //DaemonLinux ();
  //~DaemonLinux ();

  int start () override;
  bool stop () override;
  void run () override;

private:
  std::string pidfile;
  int pidFH;
  mutable std::mutex m_cv_mutex;
  std::condition_variable m_check_cv;
};

#endif // WIN32

} // namespace util
} // namespace pbote

#endif // PBOTED_SRC_BOTEDAEMON_H
