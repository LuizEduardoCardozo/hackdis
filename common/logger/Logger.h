#ifndef HACKDIS_LOGGER_H
#define HACKDIS_LOGGER_H

#include <iostream>
#include <string>

class Logger
{
public:
  enum Level
  {
    DEBUG,
    INFO,
    WARN,
    ERROR
  };

  explicit Logger(std::string context) : _context(std::move(context)) {}

  static void Log(std::string& context, std::string& msg, Level level);
  static void Debug(std::string& context, std::string& msg);
  static void Info(std::string context, std::string msg);
  static void Warn(std::string& context, std::string& msg);
  static void Error(std::string context, std::string msg);

  void Log(std::string& msg, Level level);
  void Debug(std::string& msg);
  void Info(std::string msg);
  void Warn(std::string& msg);
  void Error(std::string msg);

  static std::string GetLevelString(Level level);
  static int GetLevelNumber(Level level);

private:
  std::string _context;
};

#endif // HACKDIS_LOGGER_H
