#pragma once

#include <atomic>
#include <boost\noncopyable.hpp>

class semaphore
{
  std::atomic_flag spinlock;
  bool is_strict;
public:
  semaphore(bool strict = false);
  ~semaphore();

  void TurnOn();
  void TurnOff();

  bool Move();
  bool Status();

  class lock_guard
  {
    semaphore &obj;
    mutable bool disabled = false;
    lock_guard(semaphore &o) : obj(o) {}
  public: // Cause suddenly move semantics wont work for Lock method
    lock_guard(const lock_guard &g) : obj(g.obj)
    {
      g.disabled = true;
    }
    friend class semaphore;
    ~lock_guard()
    {
      if (!disabled)
        obj.TurnOff();
    }
  };

  auto Lock()->lock_guard;
};