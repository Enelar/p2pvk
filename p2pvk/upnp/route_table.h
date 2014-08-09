#pragma once

#include "route_rule.h"
#include <vector>

using namespace std;

struct route_table
{
  vector<route_rule> Fetch();
  string Gateway();
};