#ifndef __SWIFT_HPP_
#define __SWIFT_HPP_

#include "ObjC.hpp"

#include "Array.hpp"
#include "Dictionary.hpp"

class MachO;

class Segment;
class Section;

namespace Swift
{
	class Class
	{
		public:

		private:
	};

	class Protocol
	{
		public:

		private:
	}

	class Extension
	{
		public:

		private:
	};

	class SwiftMetadata
	{
		public:

		private:
			MachO *macho;

			Segment *segment;

			std::Array<Section>* sections;
	};
};

#endif