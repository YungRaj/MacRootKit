#ifndef __ZONE_HPP_
#define __ZONE_HPP_

#include <stdio.h>
#include <stddef.h>
#include <unistd.h>
#include <string.h>

#include <assert.h>

#include <mach/mach.h>

#include "Kernel.hpp"

#include "MachO.hpp"
#include "Segment.hpp"

class Kernel;

namespace Heap
{
	static Array<Zone*> zones;

	static mach_vm_address_t zone_array;
	static mach_vm_address_t zone_security_array;
	static mach_vm_address_t zone_submaps;

	static mach_vm_address_t kalloc_zones_default;
	static mach_vm_address_t kalloc_zones_kext;
	static mach_vm_address_t kalloc_zones_data_buffers;

	static size_t num_zones;

	void enumerateAllZones(Kernel *kernel);

	namespace ZoneAllocator
	{

	};

	class Zone
	{
		public:
			Zone() { }

			Zone(Kernel *kernel, zone_id_t zone_id);
			Zone(Kernel *kernel, char *zone_name);

			~Zone();

			static Zone* zoneForAddress(Kernel *kernel, mach_vm_address_t address);
			static Zone* zoneForIndex(Kernel *kernel, zone_id_t zone_id);

			static zone_info_t zoneInfo(Kernel *kernel);

			zone_t getZone() { return zone; }

			char* getZoneName() { return zone_name; }

			char* getZoneSiteName() { return zone_site_name; }

			char* getKHeapName() { return kheap_name; }

			zone_id_t getZoneId() { return zone_id; }

			zone_stats_t getZoneStats();

			zone_element_t getZoneElement(mach_vm_address_t address);

			struct zone_page_metadata* zoneMetaForAddress(mach_vm_address_t address);

			zone_id_t zoneIndexForAddress(Kernel *kernel, mach_vm_address_t address);

			bool checkFree(mach_vm_address_t address);

		private:
			Kernel *kernel;

			zone_t zone;

			char* zone_name;
			char *zone_site_name;

			zone_id_t zone_id;

			zone_info_t zone_info;

			zone_stats zone_stats;

			KallocHeap *kheap;

			KallocTypeView *typeview;

	};

	class KallocHeap
	{
		public:
			KallocHeap();

			~KallocHeap();

			Array<Zone*> getZones() { return &zones; }

			zone_kheap_id_t getKHeapId() { return kheap_id; }

			Zone* getZoneByName(char *zone_name);
			Zone* getZoneByIndex(zone_id_t zone_id);

			char* getKHeapName();
		private:
			Kernel *kernel;

			Array<Zone*> zones;

			enum zone_kheap_id_t kheap_id;

			struct kalloc_heap *kalloc_heap;
			struct kheap_zones *kheap_zones;
	};

	class KallocTypeView
	{
		public:
			KallocTypeView();

			~KallocTypeView();

			struct kalloc_type_view* getKallocTypeView() { return typeview; }

			Zone* getZone() { return zone; }

			KallocHeap* getKallocHeap() { return kheap; }

			Segment* getSegment() { return segment; }

			off_t getSegmentOffset() { return offset; }

			char* getSignature() { return signature; }

			char * getSiteName() { return site; }

		private:
			Kernel *kernel;

			Zone* zone;

			KallocHeap *kheap;

			Segment *segment;

			off_t offset;

			struct kalloc_type_viewe *typeview;

			char *signature;

			char *site;
	}