#pragma once

#include <string>
#include <vector>

namespace parser
{
  std::string Trim(const std::string &str, const std::string &whitespace);
  std::vector<std::string> Split(const std::string &source, const char delimeter, const bool forget_empty, const bool remain_delimeter);
  std::vector<std::string> Split(const std::string &str, const std::string &delimeter, const bool forget_empty, const bool remain_delimeter);
}