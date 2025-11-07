#pragma once

#include <chrono>
#include <iostream>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

// A macro to disallow the copy constructor and operator= functions
// This should be used in the private: declarations for a class
//#define DISALLOW_COPY_AND_ASSIGN(TypeName)                                     \
//TypeName(const TypeName &); \
//void operator=(const TypeName &)

#define TIMED_SCOPE(Name)\
	scoped_timer(#Name);


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
	double get_time() {
		stop();
		return (end_time() - start_time()) / 1000.0;
	}
	void print_time(std::string msg = "") {
		stop();
		std::cout << msg << " took: " << get_time() << std::endl;
	}
	// Getters
	[[nodiscard]] double start_time() const { return start_time_; }
	[[nodiscard]] double end_time() const { return end_time_; }


private:
	// The time the profile started
	double start_time_ = 0.0;
	// The time the profile ended
	double end_time_ = 0.0;
	
};

class scoped_timer : public timer {

public:
	scoped_timer(std::string scopeName)
	{ this->scope_name_ = scopeName; start(); };
	//destructor wich prints time to be used in scopes.
	~scoped_timer() {
		print_time(this->scope_name_);
	}
protected:
	std::string scope_name_ = "";
}

