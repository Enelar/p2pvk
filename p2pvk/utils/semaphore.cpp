#include "semaphore.h"
#include <thread>
using namespace std;

semaphore::semaphore(bool strict) : is_strict(strict)
{
  TurnOn();
}

semaphore::~semaphore()
{
  if (is_strict)
    return;
  Move();
  TurnOff();
}

void semaphore::TurnOn()
{
  spinlock.test_and_set();
}

void semaphore::TurnOff()
{
  spinlock.clear();
}

bool semaphore::Move()
{
  return !spinlock.test_and_set();
}

bool semaphore::Status()
{
  auto ret = Move();
  if (ret)
    TurnOff();
  return !ret;
}

auto semaphore::Lock()->lock_guard
{
  while (spinlock.test_and_set())
    std::this_thread::sleep_for(10us);

  return{ *this };
}