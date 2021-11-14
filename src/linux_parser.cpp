#include "linux_parser.h"

#include <dirent.h>
#include <unistd.h>

#include <cmath>  // ROUND
#include <string>
#include <vector>

using std::stof;
using std::string;
using std::to_string;
using std::vector;

#include <iostream>  // DEBUG odstranit

// DONE: An example of how to read data from the filesystem
string LinuxParser::OperatingSystem() {
  string line;
  string key;
  string value;
  std::ifstream filestream(kOSPath);
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::replace(line.begin(), line.end(), ' ', '_');
      std::replace(line.begin(), line.end(), '=', ' ');
      std::replace(line.begin(), line.end(), '"', ' ');
      std::istringstream linestream(line);
      while (linestream >> key >> value) {
        if (key == "PRETTY_NAME") {
          std::replace(value.begin(), value.end(), '_', ' ');
          return value;
        }
      }
    }
  }
  return value;
}

// DONE: An example of how to read data from the filesystem
string LinuxParser::Kernel() {
  string os, version, kernel;
  string line;
  std::ifstream stream(kProcDirectory + kVersionFilename);
  if (stream.is_open()) {
    std::getline(stream, line);
    std::istringstream linestream(line);
    linestream >> os >> version >> kernel;
  }
  return kernel;
}

// BONUS: Update this to use std::filesystem
vector<int> LinuxParser::Pids() {
  vector<int> pids;
  DIR* directory = opendir(kProcDirectory.c_str());
  struct dirent* file;
  while ((file = readdir(directory)) != nullptr) {
    // Is this a directory?
    if (file->d_type == DT_DIR) {
      // Is every character of the name a digit?
      string filename(file->d_name);
      if (std::all_of(filename.begin(), filename.end(), isdigit)) {
        int pid = stoi(filename);
        pids.push_back(pid);
      }
    }
  }
  closedir(directory);
  return pids;
}

// Read and return the system memory utilization
float LinuxParser::MemoryUtilization() {
  long totalMem{0}, freeMem{0};

  string line;
  string key;
  string value;
  std::ifstream filestream(kProcDirectory + kMeminfoFilename);
  if (filestream.is_open()) {
    for (int i = 0; i < 2; i++) {
      std::getline(filestream, line);
      std::istringstream linestream(line);
      linestream >> key >> value;
      if (key == "MemTotal:") {
        totalMem = std::stol(value);
      }
      if (key == "MemFree:") {
        freeMem = std::stol(value);
      }
    }
  }

  return ((float)(totalMem - freeMem) / totalMem);
}

// Read and return the system uptime
long LinuxParser::UpTime() {
  long upTime;
  string line;
  std::ifstream stream(kProcDirectory + kUptimeFilename);
  if (stream.is_open()) {
    std::getline(stream, line);
    std::istringstream linestream(line);
    linestream >> upTime;
  }

  return upTime;
}

// Read and return the number of jiffies for the system
long LinuxParser::Jiffies() { return ActiveJiffies() + IdleJiffies(); }

// Read and return the number of active jiffies for a PID
long LinuxParser::ActiveJiffies(int pid) {
  string stat_path = kProcDirectory + std::to_string(pid) + kStatFilename;

  vector<string> vec;
  string line;
  string value;
  std::ifstream filestream(stat_path);

  if (filestream.is_open()) {
    std::getline(filestream, line);
    std::istringstream linestream(line);

    while (linestream >> value) {
      vec.push_back(value);
    }
  }

  if (vec.size() > 16) {
    return stol(vec[13]) + stol(vec[14]) + stol(vec[15]) + stol(vec[16]);
  }

  return 0;
}

// Read and return the number of active jiffies for the system
long LinuxParser::ActiveJiffies() {
  vector<string> jiffies = CpuUtilization();
  long activeJiffies = stol(jiffies[kUser_]) + stol(jiffies[kNice_]) +
                       stol(jiffies[kSystem_]) + stol(jiffies[kIRQ_]) +
                       stol(jiffies[kSoftIRQ_]);

  return activeJiffies;
}

// Read and return the number of idle jiffies for the system
long LinuxParser::IdleJiffies() {
  vector<string> jiffies = CpuUtilization();
  long idle = stol(jiffies[kIdle_]) + stol(jiffies[kIOwait_]);
  return idle;
}

// Read and return CPU utilization
vector<string> LinuxParser::CpuUtilization() {
  vector<string> vec;
  string line;
  string value;
  std::ifstream filestream(kProcDirectory + kStatFilename);
  if (filestream.is_open()) {
    std::getline(filestream, line);
    std::istringstream linestream(line);

    while (linestream >> value) {
      if (value != "cpu") {
        vec.push_back(value);
      }
    }
  }

  return vec;
}

// Read and return the total number of processes
int LinuxParser::TotalProcesses() {
  int totalProcesses{0};
  string line;
  string key;
  string value;
  std::ifstream filestream(kProcDirectory + kStatFilename);
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::istringstream linestream(line);
      while (linestream >> key >> value) {
        if (key == "processes") {
          totalProcesses = std::stoi(value);
        }
      }
    }
  }
  return totalProcesses;
}

// Read and return the number of running processes
int LinuxParser::RunningProcesses() {
  string line;
  string key;
  string value;
  std::ifstream filestream(kProcDirectory + kStatFilename);
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::istringstream linestream(line);
      while (linestream >> key >> value) {
        if (key == "procs_running") {
          return std::stoi(value);
        }
      }
    }
  }
  return 0;
}

// Read and return the command associated with a process
string LinuxParser::Command(int pid) {
  //
  string path = kProcDirectory + std::to_string(pid) + kCmdlineFilename;
  string command{};

  std::ifstream filestream(path);
  if (filestream.is_open()) {
    std::getline(filestream, command);
    return command;
  }

  return command;
}

// Read and return the memory used by a process
string LinuxParser::Ram(int pid) {
  string line;
  string key;
  long value;

  string path = kProcDirectory + std::to_string(pid) + kStatusFilename;
  std::ifstream filestream(path);

  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::istringstream linestream(line);
      while (linestream >> key >> value) {
        if (key == "VmSize:") {
          value /= 1024;
          return to_string(value);
        }
      }
    }
  }

  return string();
}

// Read and return the user ID associated with a process
string LinuxParser::Uid(int pid) {
  string line;
  string key;
  string value;

  string path = kProcDirectory + std::to_string(pid) + kStatusFilename;
  std::ifstream filestream(path);

  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::istringstream linestream(line);
      while (linestream >> key >> value) {
        if (key == "Uid:") {
          return value;
        }
      }
    }
  }

  return string();
}

// Read and return the user associated with a process
string LinuxParser::User(int pid) {
  string uid = Uid(pid);
  string line;

  string username;
  string encrypted;
  string userID;

  std::ifstream filestream(kPasswordPath);
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::replace(line.begin(), line.end(), ':', ' ');
      std::istringstream linestream(line);
      while (linestream >> username >> encrypted >> userID) {
        if (userID == uid) {
          return username;
        }
      }
    }
  }
  return string();
}

// Read and return the uptime of a process
long LinuxParser::UpTime(int pid) {
  string path = kProcDirectory + std::to_string(pid) + kStatFilename;
  std::ifstream filestream(path);

  string line;
  string value;
  long uptime;

  if (filestream.is_open()) {
    getline(filestream, line);

    std::istringstream linestream(line);

    for (int pos = 1; pos <= 22; pos++) {
      linestream >> value;
    }
    uptime = std::stol(value) / sysconf(_SC_CLK_TCK);
  }

  return uptime;
}

float LinuxParser::CpuUtilization(int pid) {
  string path = kProcDirectory + std::to_string(pid) + kStatFilename;

  long utime, stime, cutime, cstime, starttime;
  string line, value;

  std::ifstream filestream(path);

  if (filestream.is_open()) {
    getline(filestream, line);
    std::istringstream linestream(line);

    for (int pos = 1; pos < 22; pos++) {
      linestream >> value;

      if (pos == kUtime_) {
        utime = std::stol(value);
      }
      if (pos == kStime_) {
        stime = std::stol(value);
      }
      if (pos == kCutime_) {
        cutime = std::stol(value);
      }
      if (pos == kCstime_) {
        cstime = std::stol(value);
      }
      if (pos == kStarttime_) {
        starttime = std::stol(value);
      }
    }
    long uptime = UpTime();
    long total_time = utime + stime + cutime + cstime;
    long Hertz = sysconf(_SC_CLK_TCK);
    float seconds = uptime - (starttime / Hertz);
    float cpu_usage = ((total_time / Hertz) / seconds);
    return cpu_usage;
  }
  return 0.0;
}