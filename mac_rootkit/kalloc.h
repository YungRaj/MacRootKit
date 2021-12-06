#ifndef __KALLOC_H_ 
#define __KALLOC_H_

#include <stdint.h>

#include "zalloc.h"

struct kheap_zones;

struct kalloc_zone_cfg
{
	bool kzc_caching;
	uint32_t kzc_size;
	const char *kzc_name;
};

typedef struct kalloc_heap
{
	struct kheap_zones *kh_zones;
	zone_stats_t        kh_stats;
	const char         *kh_name;
	struct kalloc_heap *kh_next;
	uint32_t           *kh_heap_id;
} *kalloc_heap_t;

struct kheap_zones
{
	struct kalloc_zone_cfg          *cfg;
	struct kalloc_heap              *views
	zone_kheap_id_t                  heap_id;
	uint16_t                         max_k_zone;
	uint8_t                          dlut[KALLOC_DLUT_SIZE];
	uint8_t                          k_zindex_start;
	zone_t                          *k_zone;
};

struct kalloc_type_view
{
	zone_t               zone;
	zone_stats_t         stats;
	char                *kt_signature;
	uint64_t             kt_flags;
	uint64_t             kt_size;
	char                *kt_site;
	uint64_t             unused;
};

#endif