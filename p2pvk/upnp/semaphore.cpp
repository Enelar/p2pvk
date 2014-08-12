#include "semaphore.h"

semaphore::semaphore()
{
  TurnOn();
}

void semaphore::TurnOn()
{
  m.lock();
}

void semaphore::TurnOff()
{
  m.unlock();
}

bool semaphore::Move()
{
  return m.try_lock();
}

bool semaphore::Status()
{
  auto ret = Move();
  if (ret)
    TurnOff();
  return !ret;
}