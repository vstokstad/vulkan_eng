#ifndef PROFILER_H_
#define PROFILER_H_

#include <chrono>
#include <iostream>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

// A macro to disallow the copy constructor and operator= functions
// This should be used in the private: declarations for a class
#define DISALLOW_COPY_AND_ASSIGN(TypeName)                                     \
  TypeName(const TypeName &);                                                  \
  void operator=(const TypeName &)

// Keeps tracks of the taken profiles and saves the data to a JSON.
// In order to take profiles use the embedded class profile taker
// The duration is in microseconds
class timer {
public:
  // Computes the start time and sets the thread id
  // The duration of the profiles is in microseconds
  void start() {
    this->start_time_ = static_cast<double>(
        std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::high_resolution_clock::now().time_since_epoch())
            .count());
  }

  // Computes the end time
  void stop() {
    this->end_time_ = static_cast<double>(
        std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::high_resolution_clock::now().time_since_epoch())
            .count());
  }
  double get_time() { return end_time() - start_time(); }
  void print_time() {
    std::cout << "profile time: " << get_time() << std::endl;
  }

  // Gettors
  [[nodiscard]] double start_time() const { return start_time_; }
  [[nodiscard]] double end_time() const { return end_time_; }

private:
  // The time the profile started
  double start_time_;
  // The time the profile ended
  double end_time_;
};

#endif // PROFILER_H_