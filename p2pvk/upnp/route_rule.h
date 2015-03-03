#pragma once

#include <string>

struct route_rule
{
  std::string destination;
  std::string netmask;
  std::string gateway;
  unsigned long index = 0;
  unsigned long type = 0;
  unsigned long metric = 0;
  bool is_default_gateway = false;
  bool is_primary_gateway = false;
};