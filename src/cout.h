#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <list>
#include <queue>
#include <deque>
#include <stack>
#include <map>
#include <set>
using namespace std;

string pluralize_if_plural(const string &singular_form, int number)
{
  return singular_form + (number >= 2 ? "s" : "");
}
string number_with_unit(int number, const string &unit_str)
{
  stringstream ss;
  // ss << number << " " << unit_str << (number >= 2 ? "s" : "");
  ss << number << " " << pluralize_if_plural(unit_str,number);
  return ss.str();
}

ostream& operator<<(ostream &s, vector<string> v)
{
  int cnt = v.size();
  s << "[ ";
  for (int i=0; i<cnt; i++) {
	if (i > 0) s << ", ";
	s << '"' << v[i] << '"';
  }
  return s << " ]  // " << number_with_unit(cnt,"item");
}

template <typename T> ostream& operator<<(ostream &s, vector<T> v)
{
  int cnt = v.size();
  s << "[ ";
  for (int i=0; i<cnt; i++) {
	if (i > 0) s << ", ";
	s << v[i];
  }
  return s << " ]  // " << number_with_unit(cnt,"item");
}

template <typename T> ostream& operator<<(ostream &s, list<T> ls)
{
  int cnt = 0;
  s << "( ";
  for (typeof(ls.begin()) it=ls.begin(); it!=ls.end(); it++) {
	if (it != ls.begin()) s << ", ";
	s << *it;
	cnt++;
  }
  return s << " )  // " << number_with_unit(cnt,"item");
}

template <typename T> ostream& operator<<(ostream &s, deque<T> st)
{
  int cnt = st.size();
  s << "[ ";
  for (typeof(st.begin()) it=st.begin(); it!=st.end(); it++) {
	if (it != st.begin()) s << ", ";
	s << *it;
  }
  return s << " ]  // " << number_with_unit(cnt,"item");
}

template <typename T1, typename T2> ostream& operator<<(ostream &s, map<T1,T2> m)
{
  int cnt = m.size();
  s << "{ ";
  for (typeof(m.begin()) it=m.begin(); it!=m.end(); it++) {
	if (it != m.begin()) s << ", ";
	s << it->first << " => " << it->second;
  }
  return s << " }  // " << number_with_unit(cnt,"item");
}

template <typename T> ostream& operator<<(ostream &s, set<T> st)
{
  int cnt = st.size();
  s << "[ ";
  for (typeof(st.begin()) it=st.begin(); it!=st.end(); it++) {
	if (it != st.begin()) s << ", ";
	s << *it;
  }
  return s << " ]  // " << number_with_unit(cnt,"item");
}

template <typename T1, typename T2> ostream& operator<<(ostream &s, pair<T1,T2> p)
{
  return s << "(" << p.first << "," << p.second << ")";
}

/////
/*
clock_t start;
void timer_clear()
{
  start = clock();
}
char *timer()
{
  clock_t end = clock();
  double interval = (double)(end - start)/CLOCKS_PER_SEC;
  
  char *ret = NULL;
  asprintf(&ret, " (%g msec)", interval*1000);
  return ret;
}
*/
