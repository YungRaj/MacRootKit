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
	class Zone;

	class KallocHeap;
	class KallocTypeView;
};

namespace Heap
{
	static Kernel *kernel = NULL;

	static Array<Zone*> zones;

	static Array<KallocHeap*> allHeaps;

	static Array<KallocTypeView*> allTypeViews;

	static mach_vm_address_t zone_array;
	static mach_vm_address_t zone_security_array;
	static mach_vm_address_t zone_submaps;

	static mach_vm_address_t kalloc_zones_default;
	static mach_vm_address_t kalloc_zones_kext;
	static mach_vm_address_t kalloc_zones_data_buffers;

	static size_t num_zones;

	Zone* zoneForAddress(mach_vm_address_t address);

	void enumerateAllZones(Kernel *kernel);

	void enumerateKallocHeaps(Kernel *kernel, mach_vm_address_t kalloc_zones_default, mach_vm_address_t kalloc_zones_kext, mach_vm_address_t kalloc_zones_data_buffers);

	void enumerateKallocTypeViews(Kernel *kernel, Segment *segment, Section *section);

	namespace ZoneAllocator
	{
		Zone* findLargestZone();

		Zone* createZone();

		void destroyZone(Zone *zone);

		namespace ZoneInfo
		{
			zone_info_t* zoneInfo();
		}

		namespace ZoneStats
		{
			zone_stats_t zoneStatsForZone(mach_vm_address_t zone);
		}

		namespace ZoneIndex
		{
			zone_id_t zoneIndexForZone(mach_vm_address_t zone);

			zone_id_t zoneIndexFromAddress(mach_vm_address_t address);

			zone_id_t zoneIndexFromPointer(void *ptr);
		}

		namespace ZonePva
		{
			zone_pva_t zonePvaFromAddress(mach_vm_address_t zone);

			zone_pva_t zonePvaFromMeta(mach_vm_address_t meta);

			zone_pva_t zonePvaFromElement(zone_element_t element);

			mach_vm_address_t zonePvaToAddress(zone_pva_t pva);

			mach_vm_address_t zonePvaToMeta(zone_pva_t pva);
		}

		namespace ZoneBits
		{
			struct zone_bits_allocator_header* zoneBitsAllocatorHeader(mach_vm_address_t zone_bits_header);

			mach_vm_address_t zoneBitsAllocatorRefPtr(mach_vm_address_t address);

			mach_vm_address_t zoneBitsAllocatorBase();

			size_t zoneBitsAllocatorSize();
		}

		namespace PageMetadata
		{
			namespace FreeList
			{
				uint64_t bitMap(mach_vm_address_t address, struct zone_page_metadata *metadata);

				uint32_t inlineBitMap(mach_vm_address_t address, struct zone_page_metadata *metadata);
			}

			struct zone_page_metadata* zoneMetadata(mach_vm_address_t meta);

			mach_vm_address_t zoneMetaFromAddress(mach_vm_address_t address);

			mach_vm_address_t zoneMetaFromElement(zone_element_t element);

			mach_vm_address_t zoneMetaToAddress(mach_vm_address_t meta);
		}

		zone_t zone(mach_vm_address_t zone);

		mach_vm_address_t zoneFromName(char *zone_name);

		mach_vm_address_t zoneFromAddress(mach_vm_address_t address);
	};

	class Zone
	{
		public:
			Zone(Kernel *kernel, mach_vm_address_t zone);
			Zone(Kernel *kernel, zone_id_t zone_id);
			Zone(Kernel *kernel, char *zone_name);

			~Zone();

			static Zone* zoneForAddress(Kernel *kernel, mach_vm_address_t address);
			static Zone* zoneForIndex(Kernel *kernel, zone_id_t zone_id);

			static zone_info_t zoneInfo(Kernel *kernel);

			void setKallocHeap(KallocHeap *kheap) { this->kheap = kheap; }

			void setKallocTypeView(KallocTypeView *typeview) { this->typeview = typeview; }

			mach_vm_address_t getZoneAddress() { return zone; }

			zone_t getZone() { return z }

			char* getZoneName() { return z_name; }

			char* getZoneSiteName() { return z_site_name; }

			char* getKHeapName() { return kheap_name; }

			uint16_t getZoneElementSize() { return z_elem_size; }

			zone_id_t getZoneId() { return z_id; }

			zone_info_t* getZoneInfo() { return z_info; }

			zone_stats_t getZoneStats() { return z_stats; }

			zone_element_t getZoneElement(mach_vm_address_t address);

			struct zone_page_metadata* zoneMetaForAddress(mach_vm_address_t address);

			zone_id_t zoneIndexForAddress(Kernel *kernel, mach_vm_address_t address);

			bool checkFree(mach_vm_address_t address);

			void parseZone();

		private:
			Kernel *kernel;

			mach_vm_address_t zone;

			zone_t z;

			zone_id_t z_id;

			zone_info_t z_info;

			zone_stats z_stats;

			char* z_name;
			char *z_site_name;

			uint16_t z_elem_size;

			KallocHeap *kheap;

			KallocTypeView *typeview;

	};

	class KallocHeap
	{
		public:
			KallocHeap(struct kheap_zones *kheap_zones, struct kalloc_heap *kalloc_heap);

			~KallocHeap();

			Array<Zone*> getZones() { return &zones; }

			char* getKHeapName() { return kh_name; }

			zone_kheap_id_t getKHeapId() { return kheap_id; }

			Zone* getZoneByName(char *zone_name);
			Zone* getZoneByIndex(zone_id_t zone_id);

			void parseKallocHeap();

		private:
			Kernel *kernel;

			Array<Zone*> zones;

			enum zone_kheap_id_t kheap_id;

			struct kalloc_heap *kalloc_heap;
			struct kheap_zones *kheap_zones;

			char *kh_name;

			zone_kheap_id_t kh_id;
	};

	class KallocTypeView
	{
		public:
			KallocTypeView(struct kalloc_type_view *typeview);

			~KallocTypeView();

			struct kalloc_type_view* getKallocTypeView() { return typeview; }

			Zone* getZone() { return zone; }

			KallocHeap* getKallocHeap() { return kheap; }

			Segment* getSegment() { return segment; }

			Section* getSection() { return section; }

			zone_stats_t getKallocTypeViewStats();

			off_t getSegmentOffset() { return offset; }

			char* getSignature() { return kt_signature; }

			char * getSiteName() { return kt_site; }

			void parseTypeView();

		private:
			Kernel *kernel;

			Zone* zone;

			KallocHeap *kheap;

			Segment *segment;
			Section *section;

			off_t offset;

			struct kalloc_type_view *typeview;

			char *kt_signature;

			char *kt_site;
	}

	namespace ZoneAllocatorTest
	{
		Zone* findLargestZone();

		Zone* createZone();

		void destroyZone(Zone* zone);

		void triggerGarbageCollection();
		void triggerGarbageCollectionForZone(Zone *zone);

		void expandZones();
		void expandZone(Zone *zone);

		void sprayMemory();
	}
};

#endif