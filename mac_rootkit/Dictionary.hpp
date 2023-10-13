#ifndef __DICTIONARY_HPP_
#define __DICTIONARY_HPP_

#include "vector.hpp"

namespace std
{
	template<typename T>
	class Dictionary
	{
		public:
			Dictionary() { }

			~Dictionary() { }

			T operator[](char *key) { return this->get(key); }

			T get(char *key)
			{
				for(int i = 0; i < keys.size(); i++)
				{
					char *k = keys.at(i);

					if(strcmp(k, key) == 0)
					{
						return values.at(i);
					}
				}

				return NULL;
			}

			int find(char *key)
			{
				for(int i = 0; i < keys.size(); i++)
				{
					char *k = keys.at(i);

					if(strcmp(k, key) == 0)
					{
						return i;
					}
				}

				return -1;
			}

			void set(char *key, void *value)
			{
				int index;

				if((index = find(key)) != -1)
				{
					values.set(value, index);

					return;
				}

				keys.push_back(key);
				values.push_back(value);
			}

		private:
			std::vector<char*> keys;
			std::vector<T> values;
	};
};

#endif