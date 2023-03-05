#include "Logger.h"

std::string Logger::GetLevelString(Level level)
{
  switch (level)
    {
    case Level::DEBUG: return "DEBUG";
    case Level::INFO: return "INFO";
    case Level::WARN: return "WARN";
    case Level::ERROR: return "ERROR";
    }
}

int Logger::GetLevelNumber(Level level)
{
  return level;
}


void Logger::Log(std::string& context, std::string& msg, Level level)
{
  auto level_str = GetLevelString(level);
  std::cout << "[" << context << "] (" << level_str << ") - " << msg << std::endl;
}

void Logger::Debug(std::string& context, std::string& msg)
{
  Logger::Log(context, msg, Logger::Level::DEBUG);
}

void Logger::Info(std::string context, std::string msg)
{
  Logger::Log(context, msg, Logger::Level::INFO);
}

void Logger::Warn(std::string& context, std::string& msg)
{
  Logger::Log(context, msg, Logger::Level::WARN);
}

void Logger::Error(std::string context, std::string msg)
{
  Logger::Log(context, msg, Logger::Level::ERROR);
}


void Logger::Log(std::string& msg, Level level)
{
  auto level_str = GetLevelString(level);
  std::cout << "[" << _context << "] (" << level_str << ") - " << msg << std::endl;
}

void Logger::Debug(std::string& msg)
{
  Logger::Log(msg, Logger::Level::INFO);
}

void Logger::Info(std::string msg)
{
  Logger::Log(msg, Logger::Level::INFO);
}

void Logger::Warn(std::string& msg)
{
  Logger::Log(msg, Logger::Level::WARN);
}

void Logger::Error(std::string msg)
{
  Logger::Log(msg, Logger::Level::ERROR);
}
