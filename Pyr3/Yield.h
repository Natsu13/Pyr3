#pragma once
#include <thread>

template<typename T>
class YieldObject
{
public:
	YieldObject();
	void assign_value(T value);
	bool is_null();
	bool is_paused();
	void pause();
	void stop();
private:
	T value;
	bool is_null_value;
	bool is_pause;
	bool is_stop;
};

#define yield(x) _yield.assign_value(x); _yield.pause(); while(_yield.is_paused()){ std::this_thread::sleep_for(1ms); };
#define yield_return() _yield.stop(); return false;
#define yield_return(x) _yield.assign_value(x); _yield.stop(); return false;