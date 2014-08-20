#include "rpc_instance.h"

using namespace std;

rpc_instance::rpc_instance(rpc_instance &&a)
  : socket(a.io), io(a.io), is_local(a.is_local)
{
}