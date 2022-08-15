#ifndef __MACH_MSG_SPRAY_H_
#define __MACH_MSG_SPRAY_H_

#include <stdbool.h>
#include <stddef.h>

struct holding_port_array {
	mach_port_t *ports;
	size_t count;
};

struct ipc_kmsg_kalloc_spray {
	struct holding_port_array holding_ports;
	
	size_t spray_size;
	size_t kalloc_allocation_size_per_port;
};

struct ipc_kmsg_kalloc_fragmentation_spray {
	struct holding_port_array holding_ports;
	
	size_t spray_size;
	size_t kalloc_size_per_port;
};

struct ool_ports_spray {
	struct holding_port_array holding_ports;
	
	size_t sprayed_count;
};

size_t mach_message_size_for_ipc_kmsg_size(size_t ipc_kmsg_size);

size_t mach_message_size_for_kalloc_size(size_t kalloc_size);

size_t ipc_kmsg_size_for_mach_message_size(size_t message_size);

size_t kalloc_size_for_mach_message_size(size_t message_size);

bool ipc_kmsg_kalloc_send_one(mach_port_t holding_port, size_t kalloc_size);

bool ool_ports_send_one(mach_port_t holding_port,
		mach_port_t *ool_ports,
		size_t ool_port_count,
		mach_msg_type_name_t ool_ports_disposition,
		size_t ipc_kmsg_size);

size_t mach_message_spray(mach_port_t *holding_ports, size_t *holding_port_count,
		mach_msg_header_t *message, size_t message_size,
		size_t message_count, size_t messages_per_port);

size_t ipc_kmsg_kalloc_fragmentation_spray(mach_port_t *holding_ports, size_t *holding_port_count,
		size_t kalloc_size, size_t message_count, size_t messages_per_port);

size_t ipc_kmsg_kalloc_spray_contents_size(size_t kalloc_size,
		size_t *contents_start, size_t *contents_end);

size_t ipc_kmsg_kalloc_spray(mach_port_t *holding_ports, size_t *holding_port_count,
		void *data, size_t kalloc_size,
		size_t message_count, size_t messages_per_port);

size_t ool_ports_spray(mach_port_t *holding_ports, size_t *holding_port_count,
		mach_port_t *ool_ports, size_t ool_port_count,
		mach_msg_type_name_t ool_ports_disposition, size_t ool_port_descriptor_count,
		size_t ool_port_descriptors_per_message, size_t ipc_kmsg_size,
		size_t messages_per_port);

void ool_ports_spray_receive(mach_port_t *holding_ports, size_t holding_port_count,
		void (^ool_ports_handler)(mach_port_t *, size_t));

mach_port_t *mach_ports_create(size_t count);

void mach_ports_destroy(mach_port_t *ports, size_t count);

void mach_ports_deallocate(mach_port_t *ports, size_t count);

void mach_port_increase_queue_limit(mach_port_t port);

void mach_port_insert_send_right(mach_port_t port);

void mach_port_drain_messages(mach_port_t port, void (^message_handler)(mach_msg_header_t *));

void mach_port_discard_messages(mach_port_t port);

void ool_ports_receive(mach_port_t *holding_ports,
		size_t holding_port_count,
		void (^ool_ports_handler)(mach_port_t *, size_t));

struct holding_port_array holding_ports_create(size_t count);

void holding_ports_destroy(struct holding_port_array all_ports);

mach_port_t holding_port_grab(struct holding_port_array *holding_ports);

mach_port_t holding_port_pop(struct holding_port_array *holding_ports);

void ipc_kmsg_kalloc_fragmentation_spray_(struct ipc_kmsg_kalloc_fragmentation_spray *spray,
		size_t kalloc_size,
		size_t spray_size,
		size_t kalloc_size_per_port,
		struct holding_port_array *holding_ports);


void ipc_kmsg_kalloc_fragmentation_spray_fragment_memory_(
		struct ipc_kmsg_kalloc_fragmentation_spray *spray,
		size_t free_size,
		int from_end);

void ipc_kmsg_kalloc_spray_(struct ipc_kmsg_kalloc_spray *spray,
		void *data,
		size_t kalloc_size,
		size_t spray_size,
		size_t kalloc_allocation_limit_per_port,
		struct holding_port_array *holding_ports);

void ool_ports_spray_(struct ool_ports_spray *spray,
		mach_port_t *ool_ports,
		size_t ool_port_count,
		mach_msg_type_name_t ool_ports_disposition,
		size_t ool_port_descriptor_count,
		size_t ool_port_descriptors_per_message,
		size_t ipc_kmsg_size,
		size_t messages_per_port,
		struct holding_port_array *holding_ports);

#endif