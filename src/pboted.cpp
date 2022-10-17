/**
 * Copyright (c) 2019-2022 polistern
 *
 * This file is part of pboted and licensed under BSD3
 *
 * See full license text in LICENSE file at top of project tree
 */

#include <cstdlib>

#include "BoteDaemon.h"

int
main (int argc, char *argv[])
{
  if (Daemon.init (argc, argv))
    {
      int res = Daemon.start ();
      if (res == 0)
        {
          Daemon.run ();
        }
      else if (res > 0)
        {
          return EXIT_SUCCESS;
        }
      else
        {
          return EXIT_FAILURE;
        }
      Daemon.stop ();
    }
  return EXIT_SUCCESS;
}

#ifdef _WIN32
#include <windows.h>

int
CALLBACK WinMain(
  _In_ HINSTANCE hInstance,
  _In_ HINSTANCE hPrevInstance,
  _In_ LPSTR     lpCmdLine,
  _In_ int       nShowCmd
)
{
  return main(__argc, __argv);
}
#endif