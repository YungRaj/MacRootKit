#ifndef __PAIR_HPP_
#define __PAIR_HPP_

#include <stddef.h>
#include <stdint.h>

template<typename T>
static void emptyDeleter(T t)
{
	/* do nothing here because we don't wanna destroy the T object */
	/* we just wanna destroy the Pair */
}

template<typename T, typename Y, void (*deleterT)(T)=emptyDeleter<T>, void (*deleterY)(Y)=emptyDeleter<Y> >
class Pair
{
	public:
		T first;
		Y second;

		Pair(T first, Y second) { this->first = first; this->second = second; }

		~Pair() { deleter(this); }

		static Pair<T, Y>* create(T first, Y second)
		{
			return new Pair(first, second);
		}

		static void deleter(Pair<T, Y> *pair)
		{
			deleterT(pair->first);
			deleterY(pair->second);

			delete pair;
		}
};

#endif