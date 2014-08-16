#include "split.h"
using namespace std;
#include <assert.h>

namespace
{
  string AddBefore(const char ch, const string &source)
  {
    string ret = source;
    ret.insert(0, " ");
    ret[0] = ch;
    return ret;
  }
}
#include <deque>
std::string parser::Trim(const std::string &str, const std::string &whitespace)
{ // http://stackoverflow.com/questions/1798112/removing-leading-and-trailing-spaces-from-a-string
  const auto strBegin = str.find_first_not_of(whitespace);
  if (strBegin == std::string::npos)
    return ""; // no content
  const auto strEnd = str.find_last_not_of(whitespace);
  const auto strRange = strEnd - strBegin + 1;
  return str.substr(strBegin, strRange);
}
vector<string> parser::Split(const std::string &source, const char delimeter, const bool forget_empty, const bool remain_delimeter)
{
  string t;
  t += delimeter;
  return Split(source, t, forget_empty, remain_delimeter);
}
std::vector<std::string> parser::Split(const std::string &str, const std::string &delimeter, const bool forget_empty, const bool remain_delimeter)
{
  assert(delimeter.length() > 0);
  char del = delimeter[0];
  const int delimeter_length = delimeter.length();
  deque<string> ret;
  auto Hook = [&](const string &str)
  {
    auto EmptyHook = [forget_empty, &ret](const string &str)
    {
      if (forget_empty && !str.length())
        return;
      ret.push_back(str);
    };
    if (remain_delimeter)
    {
      EmptyHook(str);
      return;
    }
    if (str.substr(0, delimeter_length) == delimeter)
      EmptyHook(str.substr(delimeter_length));
    else
      EmptyHook(str);
  };
  unsigned int i = 0, pos;
  while (i < str.length())
  {
    pos = str.find(delimeter, i + 1);
    if (pos == str.npos)
      break;
    Hook(str.substr(i, pos - i));
    i = pos;
  }
  Hook(str.substr(i));
  return{ ret.begin(), ret.end() };
}