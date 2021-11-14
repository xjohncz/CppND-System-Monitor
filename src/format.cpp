#include "format.h"

#include <string>

using std::string;

// INPUT: Long int measuring seconds
// OUTPUT: HH:MM:SS
string Format::ElapsedTime(long seconds) {
  int hours = seconds / 3600;
  int min = ((seconds % 3600) / 60);
  int sec = seconds %= 60;

  return adjustValue(hours) + ":" + adjustValue(min) + ":" + adjustValue(sec);
}

string Format::adjustValue(int value) {
  return (value < 10) ? '0' + std::to_string(value) : std::to_string(value);
}