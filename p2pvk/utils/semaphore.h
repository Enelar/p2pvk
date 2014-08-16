#pragma once

#include <mutex>

class semaphore
{
  std::mutex m;
public:
  semaphore();

  void TurnOn();
  void TurnOff();

  bool Move();
  bool Status();

  auto Lock()->std::unique_lock<std::mutex>;
};