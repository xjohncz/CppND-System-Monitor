#include "process.h"

#include <unistd.h>

#include <cctype>
#include <sstream>
#include <string>
#include <vector>

#include "linux_parser.h"

using std::string;
using std::to_string;
using std::vector;

Process::Process(int pid) : pid_(pid), cpu_usage_(CalculateCpuUsage()){};

// Return this process's ID
int Process::Pid() { return pid_; }

// Return this process's CPU utilization
float Process::CpuUtilization() const { return cpu_usage_; }

float Process::CalculateCpuUsage() {
  long total_time = LinuxParser::ActiveJiffies(this->Pid());
  total_time /= sysconf(_SC_CLK_TCK);
  long seconds = LinuxParser::UpTime() - LinuxParser::UpTime(this->Pid());
  float CPU_usage = (float)total_time / seconds;
  return CPU_usage;
}

// Return the command that generated this process
string Process::Command() { return LinuxParser::Command(Pid()); }

// Return this process's memory utilization
string Process::Ram() { return LinuxParser::Ram(Pid()); }

// Return the user (name) that generated this process
string Process::User() { return LinuxParser::User(Pid()); }

// Return the age of this process (in seconds)
long int Process::UpTime() { return LinuxParser::UpTime(Pid()); }

// Overload the "less than" comparison operator for Process objects
bool Process::operator<(Process const& a) const {
  return a.CpuUtilization() < CpuUtilization();
}