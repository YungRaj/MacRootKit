#ifndef __ARRAY_HPP_
#define __ARRAY_HPP_

#include "Pair.hpp"

#include <stddef.h>
#include <stdint.h>

namespace std
{
	template<typename T>
	struct Node
	{
		Node<T> *next;
		Node<T> *previous;

		T t;
	};

	template<typename T>
	class Array
	{
		public:
			Array() { head = NULL; tail = NULL; size = 0; }

			~Array()
			{
				struct Node<T> *current = head;

				while(current)
				{
					struct Node<T> *next = current->next;

					delete current;

					current = next;
				}

				head = NULL;
				tail = NULL;
			}

			size_t getSize() { return size; }

			void add(T t)
			{
				Node<T> *node = new Node<T>;

				node->t = t;

				if(!head && !tail)
				{
					head = tail = node;

					size++;
				} else if(head)
				{
					Node<T> *current = head;

					while(current->next) current = current->next;

					current->next = node;
					node->previous = current;

					tail = node;

					size++;
				} else
				{
					delete node;
				}
			}

			void set(T t, int index)
			{
				Node<T> *node;
				Node<T> *current;

				node = new Node<T>;
				node->t = node;

				current = head;

				if(index < 0)
					return;

				if(!head && !tail)
				{
					head = tail = node;

					size++;
				} else if(head)
				{
					while(current && index)
					{
						current = current->next;

						index--;
					}

					if(!index)
					{
						if(!current)
						{
							tail->next = node;
							node->previous = tail;
						} else
						{
							Node<T> *previous = current->previous;

							node->previous = previous;
							previous->next = node;

							node->next = current;
							current->previous = node;
						}

						size++;
					}
				}
			}

			T get(int index)
			{
				Node<T> *current = head;

				if(index < 0 && index >= size)
					return (T) NULL;

				while(current && index)
				{
					current = current->next;

					index--;
				}

				if(!current)
					return (T) NULL;

				return current->t;
			}

			int find(T t)
			{
				int index = 0;

				Node<T> *current = head;

				while(current)
				{
					if(index >= size)
						break;
					if(current->t == t)
						return index;

					current = current->next;

					index++;
				}

				return -1;
			}

			void remove(int index)
			{
				Node<T> *current = head;

				if(index < 0 && index >= size)
					return;

				while(current && index)
				{
					current = current->next;

					index--;
				}

				if(!index)
				{
					if(head == tail)
					{
						head = NULL;
						tail = NULL;
					} else if(current == head)
					{
						Node<T> *next = current->next;

						if(next)
						{
							next->previous = NULL;

							head = next;
						}
					} else if(current == tail)
					{
						Node<T> *previous = current->previous;

						if(previous)
						{
							previous->next = NULL;

							tail = previous;
						}
					} else
					{
						Node<T> *next;
						Node<T> *previous;

						next = current->next;
						previous = current->previous;

						next->previous = previous;
						previous->next = next;
					}

					size--;

					delete current;
				}
			}

			void remove(T t)
			{
				int index = find(t);

				if(index >= 0)
					remove(index);
			}

			void print() 
			{
				
			}

		private:
			Node<T> *head;
			Node<T> *tail;

			size_t size;
	};
};

#endif