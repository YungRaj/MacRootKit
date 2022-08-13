#include "Zone.hpp"
#include "Kernel.hpp"
#include "Offset.hpp"

using namespace Heap;

namespace Heap
{
	namespace ZoneAllocator
	{
		Zone* findLargestZone()
		{

		}

		Zone* createZone()
		{

		}

		void destroyZone(Zone *zone)
		{

		}

		namespace ZoneInfo
		{
			zone_info_t* zoneInfo()
			{
				struct zone_info *info = new struct zone_info;

				kernel->read(zone_info, (void*) info, sizeof(struct zone_info));

				return reinterpret_cast<zone_info_t*>(info);
			}
		}

		namespace ZoneStats
		{
			zone_stats_t zoneStatsForZone(mach_vm_address_t zone)
			{
				struct zone_stats *stats = new struct zone_stats;

				kernel->read(zone + offsetof(struct zone, z_stats), (void*) stats, sizeof(struct zone_stats));

				return reinterpret_cast<zone_stats_t>(stats);
			}
		}

		namespace ZoneIndex
		{
			zone_id_t zoneIndexForZone(mach_vm_address_t zone)
			{
				return reinterpret_cast<zone_id_t>((uint16_t) ((zone - zone_array) / sizeof(zone_t) ));
			}

			zone_id_t zoneIndexFromAddress(mach_vm_address_t address)
			{
				return zoneIndexFromPointer(reinterpret_cast<void*>(address));
			}

			zone_id_t zoneIndexFromPointer(void *ptr)
			{
				zone_id_t z_id;

				struct zone_page_metadata *metadata;

				zone_pva_t pva;

				mach_vm_address_t meta;

				mach_vm_address_t address = reinterpret_cast<mach_vm_address_t>(ptr);

				pva = ZonePva::zonePvaFromAddress(address);

				meta = ZonePva::zonePvaToMeta(pva);

				metadata = PageMetadata::zoneMetadata(meta);

				z_id =  metadata->zm_index;

				delete metadata;

				return z_id;
			}
		}

		namespace ZonePva
		{
			zone_pva_t zonePvaFromAddress(mach_vm_address_t zone)
			{
				return (zone_pva_t) { (uint32_t) (address >> 8) };
			}

			zone_pva_t zonePvaFromMeta(mach_vm_address_t meta)
			{
				zone_pva_t pva;

				struct zone_page_metadata *metadata;

				mach_vm_address_t zi_meta_base;

				zone_info_t *info;

				info = ZoneInfo::zoneInfo();

				metadata = info->zi_meta_base;

				zi_meta_base = reinterpret_cast<mach_vm_address_t>(metadata);

				delete info;

				pva = (zone_pva_t) { ((uint32_t) (meta - zi_meta_base)) };

				return pva;
			}

			zone_pva_t zonePvaFromElement(zone_element_t element)
			{
				return zonePvaFromAddress(element.ze_value);
			}

			mach_vm_address_t zonePvaToAddress(zone_pva_t pva)
			{
				return reinterpret_cast<mach_vm_address_t>((uint64_t) (pva.packed_address << 8));
			}

			mach_vm_address_t zonePvaToMeta(zone_pva_t pva)
			{
				mach_vm_address_t meta;

				struct zone_page_metadata *metadata;

				zone_info_t *info;

				info = ZoneInfo::zoneInfo();

				metadata = &zoneInfo->zi_metabase[pva.packed_address];

				delete info;

				meta = reinterpret_cast<mach_vm_address_t>(metadata);

				return meta;
			}
		}

		namespace ZoneBits
		{
			struct zone_bits_allocator_header* zoneBitsAllocatorHeader(mach_vm_address_t zone_bits_header)
			{
				struct zone_bits_allocator_header *bits_allocator_header = new struct zone_bits_allocator_header;

				kernel->read(zone_bits_header, (void*) bits_allocator_header, sizeof(struct zone_bits_allocator_header));

				return bits_allocator_header;
			}

			mach_vm_address_t zoneBitsAllocatorRefPtr(mach_vm_address_t address)
			{
				Zone *zone;

				struct zone_page_metadata *metadata;

				mach_vm_address_t bits_allocator;

				mach_vm_address_t bit_allocator_base = ZoneBits::zoneBitsAllocatorBase();

				zone_element_t z_elem;

				uint16_t z_elem_size = zone->getElementSize();

				zone = Heap::zoneForAddress(address);

				mach_vm_address_t meta = PageMetadata::zoneMetaFromAddress(address);

				vm_offset_t page = TRUNC_PAGE(address);
				vm_offset_t eidx = (address - page) / z_elem_size;

				z_elem = (zone_element_t) { .ze_value = page | (eidx << 2) | ZPM_AUTO };

				metadata = PageMetadata::zoneMetadata(PageMetadata::zoneMetaFromAddress(address));

				bits_allocator = bit_allocator_base + (metadata->zm_bitmap >> 3)

				return bits_allocator;
			}

			mach_vm_address_t zoneBitsAllocatorBase()
			{
				zone_info_t *info;

				mach_vm_address_t bits_min_address;
				mach_vm_address_t bits_max_address;

				info = ZoneInfo::zoneInfo();

				bits_min_address = info->zi_bits_range.min_address;
				bits_max_address = info->zi_bits_range.max_address;

				delete info;

				return bits_min_address;
			}

			size_t zoneBitsAllocatorSize()
			{
				zone_info_t *info;

				mach_vm_address_t bits_min_address;
				mach_vm_address_t bits_max_address;

				info = ZoneInfo::zoneInfo();

				bits_min_address = info->zi_bits_range.min_address;
				bits_max_address = info->zi_bits_range.max_address;

				delete info;

				return (size_t) (bits_max_address - bits_min_address);
			}
		}

		namespace PageMetadata
		{
			namespace FreeList
			{
				uint64_t bitMap(mach_vm_address_t address, struct zone_page_metadata *metadata)
				{
					Zone *zone;

					uint64_t bitMap;

					uint16_t z_elem_size;

					zone = Heap::zoneForAddress(address);

					if(!zone)
					{
						return 0;
					}

					z_elem_size = zone->getElementSize();

					vm_offset_t page = TRUNC_PAGE(address);
					vm_offset_t eidx = (address - page) / z_elem_size;

					bitMap = kernel->read64(ZoneBits::zoneBitsAllocatorRefPtr(address));

					return bitMap;
				}

				uint32_t inlineBitMap(mach_vm_address_t address, struct zone_page_metadata *metadata)
				{
					return metadata->zm_bitmap;
				}
			}

			struct zone_page_metadata* zoneMetadata(mach_vm_address_t meta)
			{
				struct zone_page_metadata *metadata = new struct zone_page_metadata;

				kernel->read(meta, (void*) metadata, sizeof(struct zone_page_metadata));

				return metadata;
			}

			mach_vm_address_t zoneMetaFromAddress(mach_vm_address_t address)
			{
				mach_vm_address_t meta;

				zone_info_t *info;

				zone_pva_t pva = ZonePva::zonePvaFromAddress(address);

				info = ZoneInfo::zoneInfo();

				meta = reinterpret_cast<mach_vm_address_t>(info->zi_meta_base) + pva.packed_address;

				delete info;

				return meta;
			}

			mach_vm_address_t zoneMetaFromElement(zone_element_t element)
			{
				mach_vm_address_t meta;

				zone_pva_t pva;

				pva = ZonePva::zonePvaFromElement(element);

				meta = ZonePva::zonePvaToMeta(pva);

				return meta;
			}

			mach_vm_address_t zoneMetaToAddress(mach_vm_address_t meta)
			{
				struct zone_page_metadata *metadata;

				mach_vm_address_t metabase;

				zone_info_t *info;

				info = ZoneInfo::zoneInfo();

				metadata = info->zi_meta_base;

				delete info;

				zi_meta_base = reinterpret_cast<mach_vm_address_t>(metadata);

				return (mach_vm_address_t) (((uint32_t) (meta - zi_meta_base)) << 8);
			}
		}

		zone_t zone(mach_vm_address_t zone)
		{
			struct zone *z = new struct zone;

			kernel->read(zone, (void*) z, sizeof(struct zone));

			return reinterpret_cast<zone_t>(z);
		}

		mach_vm_address_t zoneFromName(char *zone_name)
		{
			mach_vm_address_t zone = 0;

			for(int i = 0; i < num_zones; i++)
			{
				mach_vm_address_t name;

				zone = kernel->read64(zone_array + sizeof(zone_t) * i);

				name = kernel->read64(zone + offsetof(struct zone, z_name));

				if(strcmp(kernel->readString(name), zone_name) == 0)
				{
					return zone;
				}
			}

			return 0;
		}

		mach_vm_address_t zoneFromAddress(mach_vm_address_t address)
		{
			mach_vm_address_t zone;

			zone_info_t *info;

			mach_vm_address_t min_address;
			mach_vm_address_t max_address;

			info = ZoneInfo::zoneInfo();

			min_address = info->zi_map_range[0].min_address;
			max_address = info->zi_map_range[0].max_address;

			if(address > min_address && address <= max_address)
			{
				zone_id_t zone_id = ZoneIndex::zoneIndexFromAddress(address);

				zone = Heap::zoneFromZoneId(zone_id);

				delete info;

				return zone;
			}

			delete info;

			min_address = info->zi_map_range[1].min_address;
			max_address = info->zi_map_range[1].max_address;

			if(address > min_address && address <= max_address)
			{
				zone_id_t zone_id = ZoneIndex::zoneIndexFromAddress(address);

				zone = Heap::zoneFromZoneId(zone_id);

				delete info;

				return zone;
			}

			delete info;

			return (mach_vm_address_t) 0;
		}
	}
}

namespace Heap
{
	using namespace ZoneAllocator;

	Zone::Zone(Kernel *kernel, mach_vm_address_t zone)
	{
		this->zone = zone;

		this->parseZone();
	}

	Zone::Zone(Kernel *kernel, zone_id_t zone_id)
	{
		this->zone = zoneFromZoneIndex(zone_id);

		this->parseZone();
	}

	Zone::Zone(Kernel *kernel, char *zone_name)
	{
		this->zone = zoneFromName(zone_name);

		this->parseZone();
	}

	Zone::~Zone()
	{
		if(this->z)
		{
			delete z;

			z = NULL;
		}

		if(this->z_name)
		{
			delete z_name;

			z_name = NULL;
		}

		if(this->typeview)
		{
			delete typeview;

			typeview = NULL;
		}

		if(this->kheap)
		{
			delete kheap;

			kheap = NULL;
		}
	}

	Zone* Zone::zoneForAddress(Kernel *kernel, mach_vm_address_t address)
	{

	}

	Zone* Zone::zoneForIndex(Kernel *kernel, zone_id_t zone_id)
	{
		
	}

	zone_info_t* Zone::zoneInfo(Kernel *kernel)
	{
		
	}

	zone_element_t Zone::getZoneElement(mach_vm_address_t address)
	{
		
	}

	zone_id_t Zone::zoneIndexForAddress(Kernel *kernel, mach_vm_address_t address)
	{
		
	}

	struct zone_page_metadata* Zone::zoneMetaForAddress(mach_vm_address_t address)
	{
		return PageMetadata::zoneMetadata(PageMetadata::zoneMetaFromAddress(address));
	}

	bool Zone::checkFree(mach_vm_address_t address)
	{
		struct zone_page_metadata *metadata = PageMetadata::zoneMetadata(PageMetadata::zoneMetaFromAddres(address));

		if(metadata)
		{
			uint64_t bitmap;

			uint64_t bit;

			if(metadata->zm_inline_bitmap)
			{
				vm_offset_t page = TRUNC_PAGE(address);
				vm_offset_t eidx = (address - page) / z_elem_size;

				bit = zba_map_bit(uint32_t, eidx);

				bitmap = PageMetadata::FreeList::inlineBitMap(address, metadata);

				delete metadata;

				return bitmap & bit;
			} else
			{
				vm_offset_t page = TRUNC_PAGE(address);
				vm_offset_t eidx = (address - page) / z_elem_size;

				bit = zba_map_bit(uint32_t, eidx);

				bit = zba_map_bit(uint32_t, eidx);

				bitmap = PageMetadata::FreeList::bitMap(address, metadata);

				delete metadata;

				return bitmap & bit;
			}
		}

		return false;
	}

	void Zone::parseZone()
	{
		this->z = ZoneAllocator::zone(this->zone);
		this->z_id = this->z->z_id;
		this->z_name = kernel->readString(reinterpret_cast<mach_vm_address_t>(this->z->z_name));
		this->z_stats = ZoneStats::zoneStatsForZone(this->zone);
		this->z_info = ZoneInfo::zoneInfo();
		this->z_elem_size = this->z_elem_size;
		this->typeview = NULL;
		this->kheap = NULL;
	}
}

namespace Heap
{
	KallocHeap::KallocHeap(struct kheap_zones *kheap_zones, struct kalloc_heap *kalloc_heap)
	{
		this->kheap_zones = kheap_zones;
		this->kalloc_heap = kalloc_heap;
		this->kh_id = kheap_zones->heap_id;
		this->kh_name = kernel->readString(reinterpret_cast<mach_vm_address_t>(kalloc_heap->kh_name));
	}

	KallocHeap::~KallocHeap()
	{
		if(this->kheap_zones)
		{
			delete kheap_zones;

			kheap_zones = NULL;
		}

		if(this->kalloc_heap)
		{
			delete kalloc_heap;

			kalloc_heap = NULL;
		}

		if(this->kh_name)
		{
			delete kh_name;

			kh_name = NULL;
		}
	}

	Zone* KallocHeap::getZoneByName(char *zone_name)
	{
		for(int i = 0; i < this->allZones.getSize(); i++)
		{
			Zone *zone = this->allZones.get(i);

			if(strcmp(zone->getName(), zone_name) == 0)
			{
				return zone;
			}
		}

		return NULL;
	}

	Zone* KallocHeap::getZoneByIndex(zone_id_t zone_id)
	{
		for(int i = 0; i < this->allZones.getSize(); i++)
		{
			Zone *zone = this->allZones.get(i);

			if(zone->getZoneId() == zone_id)
			{
				return zone;
			}
		}

		return NULL;
	}

	void KallocHeap::parseKallocHeap()
	{
		mach_vm_address_t z;

		mach_vm_address_t k_zones = reinterpret_cast<mach_vm_address_t>(this->kheap_zones->k_zones);

		mach_vm_address_t dlut = reinterpret_cast<mach_vm_address_t>(this->kheap_zones->dlut);

		for(int i = 0; i < KALLOC_DLUT_SIZE; i++)
		{
			Zone *zone;

			uint8_t idx = kernel->read8(dlut + sizeof(uint8_t) * i);

			z = kernel->read64(k_zones + idx * sizeof(zone_t));

			zone = Heap::zoneForAddress(z);

			zone->setKallocHeap(this);

			this->zones.add(zone);
		}
	}
}

namespace Heap
{
	KallocTypeView::KallocTypeView(struct kalloc_type_view *typeview)
	{
		this->kernel = Heap::kernel;
		this->typeview = typeview;
		this->segment = segment;
		this->section = section;

		this->parseTypeView();
	}

	KallocTypeView::~KallocTypeView()
	{
		if(this->typeview)
		{
			delete typeview;

			typeview = NULL;
		}

		if(this->kt_signature)
		{
			delete kt_signature;

			kt_signature = NULL;
		}

		if(this->kt_site)
		{
			delete kt_site;

			kt_site = NULL;
		}

		if(this->kt_stats)
		{
			delete kt_stats;

			kt_status = NULL;
		}
	}

	zone_stats_t KallocTypeView::getKallocTypeViewStats()
	{
		struct zone_stats *stats = new struct zone_stats;

		if(this->kt_stats)
		{
			delete stats;
		}

		kernel->read(reinterpret_cast<mach_vm_address_t>(this->typeview->kt_stats), (void*) stats, sizeof(struct zone_stats));

		this->kt_stats = reinterpret_cast<zone_stats_t>(stats);

		return stats;
	}

	void KallocTypeView::parseTypeView()
	{
		this->zone = Heap::zoneForAddress(reinterpret_cast<mach_vm_address_t>(this->typeview->kt_zone));
		this->kt_signature = kernel->readString(reinterpret_cast<mach_vm_address_t>(this->typeview->kt_signature));
		this->kt_site = kernel->readString(reinterpret_cast<mach_vm_address_t>(this->typeview->kt_site));
		this->kt_stats = this->getKallocTypeViewStats();
	}
}

namespace Heap
{
	Zone* zoneForAddress(mach_vm_address_t z)
	{
		Array<Zone*> *zones = &Heap::allZones;

		for(int i = 0; i < zones->getSize(); i++)
		{
			Zone *zone = zones->get(i);

			if(zone->getAddress() == 0)
			{
				return zone;
			}
		}

		return zones;
	}

	void sprayMemory()
	{

	}

	void enumerateAllZones(Kernel *kernel)
	{
		Array<Zone*> *all_zones = &Heap::allZones;

		MachO *macho = kernel->getMachO();

		Heap::kernel = kernel;

		zone_array = kernel->getSymbolAddressByName("_zone_array");
		zone_security_array = kernel->getSymbolAddressByName("_zone_security_array");

		zone_submaps = kernel->getSymbolAddressByName("_zone_submaps");
		zone_info = kernel->getSymbolAddressByName("_zone_info");

		kalloc_zones_default = kernel->getSymbolAddressByName("_kalloc_zones_default");
		kalloc_zones_kext = kernel->getSymbolAddressByName("_kalloc_zones_kext");

		num_zones = kernel->read32(kernel->getSymbolAddressByName("_num_zones"));

		for(int i = 0; i < num_zones; i++)
		{
			Zone *zone;

			zone_t z;

			zone_stats_t z_stats;

			mach_vm_address_t zone_address = kernel->read64(zone_array + sizeof(zone_t) * i);

			zone = new Zone(kernel, zone_address);

			z = zone->getZone();
			z_stats = zone->getZoneStats();

			MAC_RK_LOG("\tzone    = = 0x%llx\n", zone_address);
			MAC_RK_LOG("\tstats   = 0x%llx\n", kernel->read64(zone_address + offsetof(struct zone, z_stats)));
			MAC_RK_LOG("\tname    = %s\n", zone->getName());
			MAC_RK_LOG("\tsize    = 0x%llx\n", zone->getElementSize());
			MAC_RK_LOG("\tindex   = 0x%llx\n", zone->getZoneId());

			allZones.add(zone);
		}

		Heap::enumerateKallocHeaps(kernel, kalloc_zones_default, kalloc_zones_kext, kalloc_zones_data_buffers);

		Heap::enumerateKallocTypeViews(kernel, macho->getSegment("__DATA_CONST"), macho->getSection("__kalloc_type"));
	}

	void enumerateKallocHeaps(Kernel *kernel, mach_vm_address_t kalloc_zones_default, mach_vm_address_t kalloc_zones_kext, mach_vm_address_t kalloc_zones_data_buffers)
	{
		KallocHeap *kheap;

		struct kheap_zones *kheap_zones;
		struct kalloc_heap *kalloc_heap;

		if(kalloc_zones_default)
		{
			kheap_zones = new struct kheap_zones;
			kalloc_heap = new struct kalloc_heap;

			kernel->read(kalloc_zones_default, (void*) kheap_zones, sizeof(struct kheap_zones));

			kernel->read(reinterpret_cast<mach_vm_address_t>(kheap_zones->views), (void*) kalloc_heap, sizeof(struct kalloc_heap));

			kheap = new KallocHeap(kheap_zones, kalloc_heap);

			allHeaps.add(kheap);
		}

		if(kalloc_zones_kext)
		{
			kheap_zones = new struct kheap_zones;
			kalloc_heap = new struct kalloc_heap;

			kernel->read(kalloc_zones_kext, (void*) kheap_zones, sizeof(struct kheap_zones));
			
			kernel->read(reinterpret_cast<mach_vm_address_t>(kheap_zones->views), (void*) kalloc_heap, sizeof(struct kalloc_heap));

			kheap = new KallocHeap(kheap_zones, kalloc_heap);

			allHeaps.add(kheap);
		}

		if(kalloc_zones_data_buffers)
		{
			kheap_zones = new struct kheap_zones;
			kalloc_heap = new struct kalloc_heap;

			kernel->read(kalloc_zones_data_buffers, (void*) kheap_zones, sizeof(struct kheap_zones));
			
			kernel->read(reinterpret_cast<mach_vm_address_t>(kheap_zones->views), (void*) kalloc_heap, sizeof(struct kalloc_heap));

			kheap = new KallocHeap(kheap_zones, kalloc_heap);

			allHeaps.add(kheap);
		}
	}

	void enumerateKallocTypeViews(Kernel *kernel, Segment *segment, Section *section)
	{
		mach_vm_address_t kalloc_type;

		off_t section_offset;

		if(!segment || !section || strcmp(segment->getSegmentName(), "__DATA_CONST") || strcmp(section->getSectionName(), "__kalloc_type"))
		{
			MAC_RK_LOG("MacRK::Heap::enumerateKallocTypeViews() wrong segment and section\n");

			return;
		}

		kalloc_type = section->getAddress() + kernel->getKernelSlide();

		section_offset = 0;

		while(section_offset < section->getSize())
		{
			Zone *zone;

			KallocTypeView *typeview;

			struct kalloc_type_view *kalloc_type_view = new struct kalloc_type_view;

			kernel->read(kalloc_type + section_offset, (void*) kalloc_type_view, sizeof(struct kalloc_type_view));

			typeview = new KallocTypeView(kalloc_type_view, segment, section);

			zone = Heap::zoneForAddress(reinterpret_cast<mach_vm_address_t>(kalloc_type_view->kt_zone));

			zone->setKallocTypeView(typeview);

			section_offset += sizeof(struct kalloc_type_view);
		}
	}
}

namespace Heap
{
	namespace ZoneAllocatorTest
	{
		Zone* findLargestZone()
		{

		}

		Zone* createZone()
		{

		}

		void destroyZone(Zone* zone)
		{

		}

		void triggerGarbageCollection()
		{

		}

		void triggerGarbageCollectionForZone(Zone *zone)
		{

		}

		void expandZones()
		{

		}

		void expandZone(Zone *zone)
		{

		}

		void sprayMemory()
		{

		}
	}
}
