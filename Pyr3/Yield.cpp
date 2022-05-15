#include "Headers.h"
#include "Yield.h"

template<typename T>
inline YieldObject<T>::YieldObject()
{
	is_null_value = true;
	is_pause = false;
	is_stop = false;
}

template<typename T>
void YieldObject<T>::assign_value(T value)
{
	assert(is_pause == false && is_stop == false);
	this.value = value;
	is_null_value = value == NULL;
}

template<typename T>
bool YieldObject<T>::is_null()
{
	return is_null_value;
}

template<typename T>
bool YieldObject<T>::is_paused()
{
	return is_pause || is_stop;
}

template<typename T>
void YieldObject<T>::pause()
{
	is_pause = true;
}

template<typename T>
void YieldObject<T>::stop()
{
	is_stop = true;
}