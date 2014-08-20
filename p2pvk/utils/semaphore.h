#pragma once

#include <mutex>

class semaphore
{
  std::mutex m;
  bool is_strict;
public:
  semaphore(bool strict = false);
  ~semaphore();

  void TurnOn();
  void TurnOff();

  bool Move();
  bool Status();

  auto Lock()->std::unique_lock<std::mutex>;
};