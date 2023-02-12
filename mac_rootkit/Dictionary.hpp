#ifndef __DICTIONARY_HPP_
#define __DICTIONARY_HPP_

#include "Array.hpp"

namespace std
{

	class Dictionary
	{
		public:
			Dictionary() { }

			~Dictionary() { }

			void* get(char *key)
			{
				for(int i = 0; i < keys.getSize(); i++)
				{
					char *k = keys.get(i);

					if(strcmp(k, key) == 0)
					{
						return values.get(i);
					}
				}

				return NULL;
			}

			int find(char *key)
			{
				for(int i = 0; i < keys.getSize(); i++)
				{
					char *k = keys.get(i);

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

				keys.add(key);
				values.add(value);
			}

		private:
			std::Array<char*> keys;
			std::Array<void*> values;
	};
};

#endif