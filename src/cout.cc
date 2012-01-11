#include "cout.h"

std::string pluralize_if_plural(const std::string &singular_form, int number)
{
  return singular_form + (number >= 2 ? "s" : "");
}

std::string number_with_unit(int number, const std::string &unit_str)
{
  std::stringstream ss;
  // ss << number << " " << unit_str << (number >= 2 ? "s" : "");
  ss << number << " " << pluralize_if_plural(unit_str,number);
  return ss.str();
}

std::ostream& operator<<(std::ostream &s, std::vector<std::string> v)
{
  int cnt = v.size();
  s << "[ ";
  for (int i=0; i<cnt; i++) {
	if (i > 0) s << ", ";
	s << '"' << v[i] << '"';
  }
  return s << " ]  // " << number_with_unit(cnt,"item");
}
