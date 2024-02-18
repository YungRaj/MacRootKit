/*
 * Copyright (c) YungRaj
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU General Public License,
 * version 2, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <assert.h>

#include <mach/mach.h>

#include "Kernel.hpp"

#include "MachO.hpp"
#include "Segment.hpp"

class Kernel;

namespace Heap {
    class Zone;

    class KallocHeap;
    class KallocTypeView;
}; // namespace Heap

namespace Heap {
    static Kernel* kernel = NULL;

    static std::vector<Zone*> zones;

    static std::vector<KallocHeap*> allHeaps;

    static std::vector<KallocTypeView*> allTypeViews;

    static xnu::Mach::VmAddress zone_std::vector;
    static xnu::Mach::VmAddress zone_security_std::vector;
    static xnu::Mach::VmAddress zone_submaps;

    static xnu::Mach::VmAddress kalloc_zones_default;
    static xnu::Mach::VmAddress kalloc_zones_kext;
    static xnu::Mach::VmAddress kalloc_zones_data_buffers;

    static Size num_zones;

    Zone* zoneForAddress(xnu::Mach::VmAddress address);

    void enumerateAllZones(Kernel* kernel);

    void enumerateKallocHeaps(Kernel* kernel, xnu::Mach::VmAddress kalloc_zones_default,
                              xnu::Mach::VmAddress kalloc_zones_kext,
                              xnu::Mach::VmAddress kalloc_zones_data_buffers);

    void enumerateKallocTypeViews(Kernel* kernel, Segment* segment, Section* section);

    namespace ZoneAllocator {
        Zone* findLargestZone();

        Zone* createZone();

        void destroyZone(Zone* zone);

        namespace ZoneInfo {
            zone_info_t* zoneInfo();
        }

        namespace ZoneStats {
            zone_stats_t zoneStatsForZone(xnu::Mach::VmAddress zone);
        }

        namespace ZoneIndex {
            zone_id_t zoneIndexForZone(xnu::Mach::VmAddress zone);

            zone_id_t zoneIndexFromAddress(xnu::Mach::VmAddress address);

            zone_id_t zoneIndexFromPointer(void* ptr);
        } // namespace ZoneIndex

        namespace ZonePva {
            zone_pva_t zonePvaFromAddress(xnu::Mach::VmAddress zone);

            zone_pva_t zonePvaFromMeta(xnu::Mach::VmAddress meta);

            zone_pva_t zonePvaFromElement(zone_element_t element);

            xnu::Mach::VmAddress zonePvaToAddress(zone_pva_t pva);

            xnu::Mach::VmAddress zonePvaToMeta(zone_pva_t pva);
        } // namespace ZonePva

        namespace ZoneBits {
            struct zone_bits_allocator_header* zoneBitsAllocatorHeader(
                xnu::Mach::VmAddress zone_bits_header);

            xnu::Mach::VmAddress zoneBitsAllocatorRefPtr(xnu::Mach::VmAddress address);

            xnu::Mach::VmAddress zoneBitsAllocatorBase();

            Size zoneBitsAllocatorSize();
        } // namespace ZoneBits

        namespace PageMetadata {
            namespace FreeList {
                UInt64 bitMap(xnu::Mach::VmAddress address, struct zone_page_metadata* metadata);

                UInt32 inlineBitMap(xnu::Mach::VmAddress address,
                                    struct zone_page_metadata* metadata);
            } // namespace FreeList

            struct zone_page_metadata* zoneMetadata(xnu::Mach::VmAddress meta);

            xnu::Mach::VmAddress zoneMetaFromAddress(xnu::Mach::VmAddress address);

            xnu::Mach::VmAddress zoneMetaFromElement(zone_element_t element);

            xnu::Mach::VmAddress zoneMetaToAddress(xnu::Mach::VmAddress meta);
        } // namespace PageMetadata

        zone_t zone(xnu::Mach::VmAddress zone);

        xnu::Mach::VmAddress zoneFromName(char* zone_name);

        xnu::Mach::VmAddress zoneFromAddress(xnu::Mach::VmAddress address);
    }; // namespace ZoneAllocator

    class Zone {
    public:
        Zone(Kernel* kernel, xnu::Mach::VmAddress zone);
        Zone(Kernel* kernel, zone_id_t zone_id);
        Zone(Kernel* kernel, char* zone_name);

        ~Zone();

        static Zone* zoneForAddress(Kernel* kernel, xnu::Mach::VmAddress address);
        static Zone* zoneForIndex(Kernel* kernel, zone_id_t zone_id);

        static zone_info_t zoneInfo(Kernel* kernel);

        void setKallocHeap(KallocHeap* kheap) {
            this->kheap = kheap;
        }

        void setKallocTypeView(KallocTypeView* typeview) {
            this->typeview = typeview;
        }

        xnu::Mach::VmAddress getZoneAddress() {
            return zone;
        }

        zone_t getZone() {
            return z
        }

        char* getZoneName() {
            return z_name;
        }

        char* getZoneSiteName() {
            return z_site_name;
        }

        char* getKHeapName() {
            return kheap_name;
        }

        UInt16 getZoneElementSize() {
            return z_elem_size;
        }

        zone_id_t getZoneId() {
            return z_id;
        }

        zone_info_t* getZoneInfo() {
            return z_info;
        }

        zone_stats_t getZoneStats() {
            return z_stats;
        }

        zone_element_t getZoneElement(xnu::Mach::VmAddress address);

        struct zone_page_metadata* zoneMetaForAddress(xnu::Mach::VmAddress address);

        zone_id_t zoneIndexForAddress(Kernel* kernel, xnu::Mach::VmAddress address);

        bool checkFree(xnu::Mach::VmAddress address);

        void parseZone();

    private:
        Kernel* kernel;

        xnu::Mach::VmAddress zone;

        zone_t z;

        zone_id_t z_id;

        zone_info_t z_info;

        zone_stats z_stats;

        char* z_name;
        char* z_site_name;

        UInt16 z_elem_size;

        KallocHeap* kheap;

        KallocTypeView* typeview;
    };

    class KallocHeap {
    public:
        KallocHeap(struct kheap_zones* kheap_zones, struct kalloc_heap* kalloc_heap);

        ~KallocHeap();

        std::vector<Zone*> getZones() {
            return &zones;
        }

        char* getKHeapName() {
            return kh_name;
        }

        zone_kheap_id_t getKHeapId() {
            return kheap_id;
        }

        Zone* getZoneByName(char* zone_name);
        Zone* getZoneByIndex(zone_id_t zone_id);

        void parseKallocHeap();

    private:
        Kernel* kernel;

        std::vector<Zone*> zones;

        enum zone_kheap_id_t kheap_id;

        struct kalloc_heap* kalloc_heap;
        struct kheap_zones* kheap_zones;

        char* kh_name;

        zone_kheap_id_t kh_id;
    };

    class KallocTypeView {
    public:
        KallocTypeView(struct kalloc_type_view* typeview);

        ~KallocTypeView();

        struct kalloc_type_view* getKallocTypeView() {
            return typeview;
        }

        Zone* getZone() {
            return zone;
        }

        KallocHeap* getKallocHeap() {
            return kheap;
        }

        Segment* getSegment() {
            return segment;
        }

        Section* getSection() {
            return section;
        }

        zone_stats_t getKallocTypeViewStats();

        Offset getSegmentOffset() {
            return offset;
        }

        char* getSignature() {
            return kt_signature;
        }

        char* getSiteName() {
            return kt_site;
        }

        void parseTypeView();

    private:
        Kernel* kernel;

        Zone* zone;

        KallocHeap* kheap;

        Segment* segment;
        Section* section;

        Offset offset;

        struct kalloc_type_view* typeview;

        char* kt_signature;

        char* kt_site;
    }

    namespace ZoneAllocatorTest {
        Zone* findLargestZone();

        Zone* createZone();

        void destroyZone(Zone * zone);

        void triggerGarbageCollection();
        void triggerGarbageCollectionForZone(Zone * zone);

        void expandZones();
        void expandZone(Zone * zone);

        void sprayMemory();
    }
}; // namespace Heap

#endif