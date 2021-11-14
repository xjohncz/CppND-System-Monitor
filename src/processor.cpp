#include "processor.h"

#include "linux_parser.h"

int Processor::previdle = 0;
int Processor::previowait = 0;
int Processor::prevuser = 0;
int Processor::prevnice = 0;
int Processor::prevsystem = 0;
int Processor::previrq = 0;
int Processor::prevsoftirq = 0;
int Processor::prevsteal = 0;

// Return the aggregate CPU utilization
float Processor::Utilization() {
  /*
   *    source:
   * http://stackoverflow.com/questions/23367857/accurate-calculation-of-cpu-usage-given-in-percentage-in-linux
   *
   *    PrevIdle = previdle + previowait
   *    Idle = idle + iowait
   *
   *    PrevNonIdle = prevuser + prevnice + prevsystem + previrq + prevsoftirq +
   * prevsteal NonIdle = user + nice + system + irq + softirq + steal
   *
   *    PrevTotal = PrevIdle + PrevNonIdle
   *    Total = Idle + NonIdle
   *
   *    # differentiate: actual value minus the previous one
   *    totald = Total - PrevTotal
   *    idled = Idle - PrevIdle
   *
   *    CPU_Percentage = (totald - idled)/totald
   */

  std::vector<std::string> cpu_strings = LinuxParser::CpuUtilization();
  std::vector<int> cpu_int;

  for (auto& str : cpu_strings) {
    cpu_int.push_back(stoi(str));
  }

  int user = cpu_int[0];
  int nice = cpu_int[1];
  int system = cpu_int[2];
  int idle = cpu_int[3];
  int iowait = cpu_int[4];
  int irq = cpu_int[5];
  int softirq = cpu_int[6];
  int steal = cpu_int[7];

  int PrevIdle = previdle + previowait;
  int Idle = idle + iowait;

  int PrevNonIdle =
      prevuser + prevnice + prevsystem + previrq + prevsoftirq + prevsteal;

  int NonIdle = user + nice + system + irq + softirq + steal;

  int PrevTotal = PrevIdle + PrevNonIdle;
  int Total = Idle + NonIdle;

  int totald = Total - PrevTotal;
  int idled = Idle - PrevIdle;

  float CPU_Percentage = (float)(totald - idled) / totald;

  // Store previous values
  previdle = idle;
  previowait = iowait;
  prevuser = user;
  prevnice = nice;
  prevsystem = system;
  previrq = irq;
  prevsoftirq = softirq;
  prevsteal = steal;

  return CPU_Percentage;
}