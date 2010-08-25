/*
Copyright (c) 2009,2010, Roger Light <roger@atchoo.org>
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice,
   this list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright
   notice, this list of conditions and the following disclaimer in the
   documentation and/or other materials provided with the distribution.
3. Neither the name of mosquitto nor the names of its
   contributors may be used to endorse or promote products derived from
   this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef MQTT3_H
#define MQTT3_H

#include <config.h>
#include <net_mosq.h>

#include <stdbool.h>
#include <stdint.h>
#include <syslog.h>
#include <time.h>

#ifndef __GNUC__
#define __attribute__(attrib)
#endif

/* For version 3 of the MQTT protocol */

#define PROTOCOL_NAME "MQIsdp"
#define PROTOCOL_VERSION 3

/* Database macros */
#define MQTT_DB_VERSION 2

/* Message types */
#define CONNECT 0x10
#define CONNACK 0x20
#define PUBLISH 0x30
#define PUBACK 0x40
#define PUBREC 0x50
#define PUBREL 0x60
#define PUBCOMP 0x70
#define SUBSCRIBE 0x80
#define SUBACK 0x90
#define UNSUBSCRIBE 0xA0
#define UNSUBACK 0xB0
#define PINGREQ 0xC0
#define PINGRESP 0xD0
#define DISCONNECT 0xE0

/* Log destinations */
#define MQTT3_LOG_NONE 0x00
#define MQTT3_LOG_SYSLOG 0x01
#define MQTT3_LOG_FILE 0x02
#define MQTT3_LOG_STDOUT 0x04
#define MQTT3_LOG_STDERR 0x08
#define MQTT3_LOG_TOPIC 0x10
#define MQTT3_LOG_ALL 0xFF

enum mqtt3_bridge_direction{
	bd_out = 0,
	bd_in = 1,
	bd_both = 2
};

struct _mqtt3_bridge_topic{
	char *topic;
	enum mqtt3_bridge_direction direction;
};

struct _mqtt3_bridge{
	char *name;
	char *address;
	uint16_t port;
	int keepalive;
	bool clean_session;
	struct _mqtt3_bridge_topic *topics;
	int topic_count;
	time_t restart_t;
};

typedef struct _mqtt3_context{
	int sock;
	time_t last_msg_in;
	time_t last_msg_out;
	uint16_t keepalive;
	bool clean_session;
	enum mosquitto_client_state state;
	bool duplicate;
	char *id;
	char *address;
	struct _mosquitto_packet in_packet;
	struct _mosquitto_packet *out_packet;
	struct _mqtt3_bridge *bridge;
} mqtt3_context;

enum mqtt3_msg_status {
	ms_invalid = 0,
	ms_publish = 1,
	ms_publish_puback = 2,
	ms_wait_puback = 3,
	ms_publish_pubrec = 4,
	ms_wait_pubrec = 5,
	ms_resend_pubrel = 6,
	ms_wait_pubrel = 7,
	ms_resend_pubcomp = 8,
	ms_wait_pubcomp = 9
};

struct mqtt3_iface {
	char *iface;
	int port;
};

typedef struct {
	int autosave_interval;
	bool daemon;
	char *ext_sqlite_regex;
	struct mqtt3_iface *iface;
	int iface_count;
	int log_dest;
	int log_type;
	int max_connections;
	bool persistence;
	char *persistence_location;
	char *persistence_file;
	int retry_interval;
	int store_clean_interval;
	int sys_interval;
	char *pid_file;
	char *user;
	struct _mqtt3_bridge *bridges;
	int bridge_count;
} mqtt3_config;

struct _mqtt3_listener {
	int fd;
	char *iface;
	uint16_t port;
	int max_connections;
	char *mount_point;
	char *client_prefix;
};

/* ============================================================
 * Utility functions
 * ============================================================ */
/* Return a string that corresponds to the MQTT command number (left shifted 4 bits). */
const char *mqtt3_command_to_string(uint8_t command);
void mqtt3_check_keepalive(mqtt3_context *context);

/* ============================================================
 * Config functions
 * ============================================================ */
/* Initialise config struct to default values. */
void mqtt3_config_init(mqtt3_config *config);
/* Parse command line options into config. */
int mqtt3_config_parse_args(mqtt3_config *config, int argc, char *argv[]);
/* Read configuration data from filename into config.
 * Returns 0 on success, 1 if there is a configuration error or if a file cannot be opened.
 */
int mqtt3_config_read(mqtt3_config *config, const char *filename);

/* ============================================================
 * Raw send functions - just construct the packet and send 
 * ============================================================ */
/* Generic function for sending a command to a client where there is no payload, just a mid.
 * Returns 0 on success, 1 on error.
 */
int mqtt3_send_command_with_mid(mqtt3_context *context, uint8_t command, uint16_t mid);
int mqtt3_raw_connack(mqtt3_context *context, uint8_t result);
int mqtt3_raw_connect(mqtt3_context *context, const char *client_id, bool will, uint8_t will_qos, bool will_retain, const char *will_topic, const char *will_msg, uint16_t keepalive, bool clean_session);
int mqtt3_raw_disconnect(mqtt3_context *context);
int mqtt3_raw_pingreq(mqtt3_context *context);
int mqtt3_raw_pingresp(mqtt3_context *context);
int mqtt3_raw_puback(mqtt3_context *context, uint16_t mid);
int mqtt3_raw_pubcomp(mqtt3_context *context, uint16_t mid);
int mqtt3_raw_publish(mqtt3_context *context, int dup, uint8_t qos, bool retain, uint16_t mid, const char *topic, uint32_t payloadlen, const uint8_t *payload);
int mqtt3_raw_pubrec(mqtt3_context *context, uint16_t mid);
int mqtt3_raw_pubrel(mqtt3_context *context, uint16_t mid);
int mqtt3_raw_suback(mqtt3_context *context, uint16_t mid, uint32_t payloadlen, const uint8_t *payload);
int mqtt3_raw_subscribe(mqtt3_context *context, bool dup, const char *topic, uint8_t topic_qos);
int mqtt3_raw_unsubscribe(mqtt3_context *context, bool dup, const char *topic);
int mqtt3_send_simple_command(mqtt3_context *context, uint8_t command);

/* ============================================================
 * Network functions
 * ============================================================ */
int mqtt3_socket_accept(mqtt3_context ***contexts, int *context_count, int listensock);
int mqtt3_socket_close(mqtt3_context *context);
int mqtt3_socket_listen(uint16_t port);
int mqtt3_socket_listen_if(const char *iface, uint16_t port);

int mqtt3_net_packet_queue(mqtt3_context *context, struct _mosquitto_packet *packet);
int mqtt3_net_read(mqtt3_context *context);
int mqtt3_net_write(mqtt3_context *context);

void mqtt3_net_set_max_connections(int max);
uint64_t mqtt3_net_bytes_total_received(void);
uint64_t mqtt3_net_bytes_total_sent(void);
unsigned long mqtt3_net_msgs_total_received(void);
unsigned long mqtt3_net_msgs_total_sent(void);

/* ============================================================
 * Read handling functions
 * ============================================================ */
int mqtt3_packet_handle(mqtt3_context *context);
int mqtt3_handle_connack(mqtt3_context *context);
int mqtt3_handle_connect(mqtt3_context *context);
int mqtt3_handle_disconnect(mqtt3_context *context);
int mqtt3_handle_pingreq(mqtt3_context *context);
int mqtt3_handle_pingresp(mqtt3_context *context);
int mqtt3_handle_puback(mqtt3_context *context);
int mqtt3_handle_pubcomp(mqtt3_context *context);
int mqtt3_handle_publish(mqtt3_context *context);
int mqtt3_handle_pubrec(mqtt3_context *context);
int mqtt3_handle_pubrel(mqtt3_context *context);
int mqtt3_handle_suback(mqtt3_context *context);
int mqtt3_handle_subscribe(mqtt3_context *context);
int mqtt3_handle_unsuback(mqtt3_context *context);
int mqtt3_handle_unsubscribe(mqtt3_context *context);

/* ============================================================
 * Database handling
 * ============================================================ */
int mqtt3_db_open(mqtt3_config *config);
int mqtt3_db_close(void);
int mqtt3_db_backup(bool cleanup);
int mqtt3_db_client_insert(mqtt3_context *context, int will, int will_retain, int will_qos, const char *will_topic, const char *will_message);
int mqtt3_db_client_update(mqtt3_context *context, int will, int will_retain, int will_qos, const char *will_topic, const char *will_message);
int mqtt3_db_client_count(int *count);
/* Remove the client detailed in context from the clients table only. */
int mqtt3_db_client_delete(mqtt3_context *context);
/* Return the socket number in sock of a client client_id. */
int mqtt3_db_client_find_socket(const char *client_id, int *sock);
/* Set the socket column = -1 for a disconnectd client. */
int mqtt3_db_client_invalidate_socket(const char *client_id, int sock);
/* Add the will of the client in context to the queue of clients subscribed to the appropriate topic. */
int mqtt3_db_client_will_queue(mqtt3_context *context);
void mqtt3_db_limits_set(int inflight, int queued);
/* Return the number of in-flight messages in count. */
int mqtt3_db_message_count(int *count);
int mqtt3_db_message_delete(const char *client_id, uint16_t mid, enum mosquitto_msg_direction dir);
int mqtt3_db_message_delete_by_oid(int64_t oid);
int mqtt3_db_message_insert(const char *client_id, uint16_t mid, enum mosquitto_msg_direction dir, enum mqtt3_msg_status status, int qos, int64_t store_id);
int mqtt3_db_message_release(const char *client_id, uint16_t mid, enum mosquitto_msg_direction dir);
int mqtt3_db_message_update(const char *client_id, uint16_t mid, enum mosquitto_msg_direction dir, enum mqtt3_msg_status status);
int mqtt3_db_message_write(mqtt3_context *context);
int mqtt3_db_messages_delete(const char *client_id);
int mqtt3_db_messages_easy_queue(const char *client_id, const char *topic, int qos, uint32_t payloadlen, const uint8_t *payload, int retain);
int mqtt3_db_messages_queue(const char *source_id, const char *topic, int qos, int retain, int64_t store_id);
int mqtt3_db_message_store(const char *source, const char *topic, int qos, uint32_t payloadlen, const uint8_t *payload, int retain, int64_t *store_id);
/* Check all messages waiting on a client reply and resend if timeout has been exceeded. */
int mqtt3_db_message_timeout_check(unsigned int timeout);
/* Generate an outgoing mid for client_id. */
uint16_t mqtt3_db_mid_generate(const char *client_id);
/* Add a retained message for a topic, overwriting an existing one if necessary. */
int mqtt3_db_retain_insert(const char *topic, int64_t store_id);
int mqtt3_db_retain_delete(const char *topic);
int mqtt3_db_retain_queue(mqtt3_context *context, const char *sub, int sub_qos);
int mqtt3_db_store_clean(void);
/* Insert a new subscription/qos for a client. */
int mqtt3_db_sub_insert(const char *client_id, const char *sub, int qos);
/* Remove a subscription for a client. */
int mqtt3_db_sub_delete(const char *client_id, const char *sub);
int mqtt3_db_sub_search_start(const char *source_id, const char *topic, int qos);
/* Remove all subscriptions for a client. */
int mqtt3_db_subs_clean_session(const char *client_id);
void mqtt3_db_sys_update(int interval, time_t start_time);
void mqtt3_db_vacuum(void);

/* ============================================================
 * Context functions
 * ============================================================ */
mqtt3_context *mqtt3_context_init(int sock);
void mqtt3_context_cleanup(mqtt3_context *context);
void mqtt3_context_close_duplicate(int sock);

/* ============================================================
 * Logging functions
 * ============================================================ */
int mqtt3_log_init(int level, int destinations);
int mqtt3_log_close(void);
int mqtt3_log_printf(int level, const char *fmt, ...) __attribute__((format(printf, 2, 3)));

/* ============================================================
 * Bridge functions
 * ============================================================ */
int mqtt3_bridge_new(mqtt3_context **contexts, int *context_count, struct _mqtt3_bridge *bridge);
int mqtt3_bridge_connect(mqtt3_context *context);
void mqtt3_bridge_packet_cleanup(mqtt3_context *context);

#endif
