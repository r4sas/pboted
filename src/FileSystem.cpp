/**
 * Copyright (C) 2013-2016, The PurpleI2P Project
 * Copyright (C) 2019-2022, polistern
 * Copyright (C) 2022, The PurpleBote Team
 *
 * This file is part of pboted project and licensed under BSD3
 *
 * See full license text in LICENSE file at top of project tree
 */

#include <algorithm>
#include <filesystem>
#include <system_error>

#ifdef _WIN32
#include <shlobj.h>
#endif

#include "FileSystem.h"
#include "Logging.h"

namespace pbote
{
namespace fs
{

std::string appName = "pboted";
std::string dataDir = "";
#ifdef _WIN32
std::string dirSep = "\\";
#else
std::string dirSep = "/";
#endif
const std::vector<std::string> dir_list = {"DHTindex", "DHTemail", "DHTdirectory", "inbox", "incomplete", "outbox", "sent"};

const std::string &GetAppName () { return appName; }

void SetAppName (const std::string &name) { appName = name; }

const std::string &GetDataDir () { return dataDir; }

void
DetectDataDir (const std::string &cmdline_param, bool isService)
{
  if (!cmdline_param.empty())
    {
      dataDir = cmdline_param;
      return;
    }

#ifndef _WIN32
  char *home = getenv("HOME");
#endif

  if (isService)
    {
#ifdef _WIN32
      wchar_t commonAppData[MAX_PATH];
      if(SHGetFolderPathW(NULL, CSIDL_COMMON_APPDATA, NULL, 0, commonAppData) != S_OK)
        {
           fprintf(stderr, "Error: Unable to get common AppData path!");
           exit(1);
        }
      else
        {
           dataDir = std::filesystem::path(commonAppData).string() + dirSep + appName;
        }
#else
      dataDir = "/var/lib/" + appName;
#endif
    }
#ifdef _WIN32
  else
    {
      wchar_t localAppData[MAX_PATH];

      // check executable directory first
      if(!GetModuleFileNameW(NULL, localAppData, MAX_PATH))
        {
          fprintf(stderr, "Error: Unable to get application path!");
          exit(1);
        }
      else
        {
          auto execPath = std::filesystem::path(localAppData).parent_path();

          if(std::filesystem::exists(execPath/"pboted.conf"))
            {
              dataDir = execPath.string ();
            }
          else // otherwise %appdata%
            {
              if(SHGetFolderPathW(NULL, CSIDL_APPDATA, NULL, 0, localAppData) != S_OK)
                {
                  fprintf(stderr, "Error: Unable to get AppData path!");
                  exit(1);
                }
              else
                {
                  dataDir = std::filesystem::path(localAppData).string() + dirSep + appName;
                }
            }
        }
    }
#else
  else if (home != nullptr && strlen(home) > 0)
    {
      dataDir = std::string(home) + "/." + appName;
    }
  else
    {
      dataDir = "/tmp/" + appName;
    }
#endif
}

bool
Init ()
{
  if (!std::filesystem::exists(dataDir))
    std::filesystem::create_directory(dataDir);

  for (const auto& dir_name : dir_list)
    {
      std::string dir_path = DataDirPath(dir_name);
      if (!std::filesystem::exists(dir_path))
        std::filesystem::create_directory(dir_path);
    }

  return true;
}

bool
ReadDir (const std::string &path, std::vector<std::string> &files)
{
  if (!std::filesystem::exists(path))
    return false;
  std::filesystem::directory_iterator it(path);
  std::filesystem::directory_iterator end;

  for (; it != end; it++)
    {
      if (!std::filesystem::is_regular_file(it->status()))
        continue;
      files.push_back(it->path().string());
    }

  return true;
}

bool Exists (const std::string &path) { return std::filesystem::exists(path); }

uint32_t
GetLastUpdateTime (const std::string &path)
{
  if (!std::filesystem::exists(path))
    return 0;

  std::error_code ec;
  (void)std::filesystem::last_write_time(path, ec);
  return ec.value () ? 0 : 1;
}

bool
Remove (const std::string &path)
{
  if (!std::filesystem::exists(path))
    return false;
  return std::filesystem::remove(path);
}

bool
CreateDirectory (const std::string &path)
{
  if (std::filesystem::exists(path) &&
      std::filesystem::is_directory(std::filesystem::status(path)))
    return true;
  return std::filesystem::create_directory(path);
}

void
HashedStorage::SetPlace (const std::string &path)
{
  root = path + pbote::fs::dirSep + name;
}

bool
HashedStorage::Init (const char *chars, size_t count)
{
  if (!std::filesystem::exists(root))
    std::filesystem::create_directories(root);

  for (size_t i = 0; i < count; i++)
    {
      auto p = root + pbote::fs::dirSep + prefix1 + chars[i];
      if (std::filesystem::exists(p))
        continue;
      if (std::filesystem::create_directory(p))
        continue; /* ^ throws exception on failure */
      return false;
    }

  return true;
}

std::string
HashedStorage::Path (const std::string &ident) const
{
  std::string safe_ident = ident;
  std::replace(safe_ident.begin(), safe_ident.end(), '/', '-');
  std::replace(safe_ident.begin(), safe_ident.end(), '\\', '-');

  std::stringstream t("");
  t << this->root << pbote::fs::dirSep;
  t << prefix1 << safe_ident[0] << pbote::fs::dirSep;
  t << prefix2 << safe_ident << "." << suffix;

  return t.str();
}

void
HashedStorage::Remove (const std::string &ident)
{
  std::string path = Path(ident);
  if (!std::filesystem::exists(path))
    return;
  std::filesystem::remove(path);
}

void
HashedStorage::Traverse(std::vector<std::string> &files)
{
  Iterate([&files](const std::string &fname) { files.push_back(fname); });
}

void
HashedStorage::Iterate(FilenameVisitor v)
{
  std::filesystem::path p(root);
  std::filesystem::recursive_directory_iterator it(p);
  std::filesystem::recursive_directory_iterator end;

  for (; it != end; it++)
    {
      if (!std::filesystem::exists(it->path()))
        continue;
      if (!std::filesystem::is_regular_file(it->status()))
        continue;
      const std::string &t = it->path().string();
      v(t);
    }
}

} // namespace fs
} // namespace pbote
