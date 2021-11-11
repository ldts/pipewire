/* PipeWire
 *
 * Copyright © 2021 Wim Taymans
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <limits.h>
#include <math.h>
#include <arpa/inet.h>

#include <openssl/err.h>
#include <openssl/rand.h>
#include <openssl/rsa.h>
#include <openssl/engine.h>
#include <openssl/aes.h>

#include "config.h"

#include <spa/utils/result.h>
#include <spa/utils/string.h>
#include <spa/utils/json.h>
#include <spa/utils/ringbuffer.h>
#include <spa/debug/pod.h>
#include <spa/pod/builder.h>
#include <spa/param/audio/format-utils.h>
#include <spa/param/audio/raw.h>

#include <pipewire/impl.h>
#include <pipewire/i18n.h>

#include "module-raop/rtsp-client.h"

/** \page page_module_raop_sink PipeWire Module: AirPlay Sink
 */

#define NAME "raop-sink"

PW_LOG_TOPIC_STATIC(mod_topic, "mod." NAME);
#define PW_LOG_TOPIC_DEFAULT mod_topic

#define FRAMES_PER_TCP_PACKET 4096
#define FRAMES_PER_UDP_PACKET 352

#define DEFAULT_TCP_AUDIO_PORT   6000
#define DEFAULT_UDP_AUDIO_PORT   6000
#define DEFAULT_UDP_CONTROL_PORT 6001
#define DEFAULT_UDP_TIMING_PORT  6002

#define AES_CHUNK_SIZE 16

#define MAX_PORT_RETRY	16

#define DEFAULT_FORMAT "S16"
#define DEFAULT_RATE 48000
#define DEFAULT_CHANNELS "2"
#define DEFAULT_POSITION "[ FL FR ]"

#define MODULE_USAGE	"[ node.latency=<latency as fraction> ] "				\
			"[ node.name=<name of the nodes> ] "					\
			"[ node.description=<description of the nodes> ] "			\
			"[ audio.format=<format, default:"DEFAULT_FORMAT"> ] "			\
			"[ audio.rate=<sample rate, default: 48000> ] "				\
			"[ audio.channels=<number of channels, default:"DEFAULT_CHANNELS"> ] "	\
			"[ audio.position=<channel map, default:"DEFAULT_POSITION"> ] "		\
			"[ stream.props=<properties> ] "


static const struct spa_dict_item module_props[] = {
	{ PW_KEY_MODULE_AUTHOR, "Wim Taymans <wim.taymans@gmail.com>" },
	{ PW_KEY_MODULE_DESCRIPTION, "An RAOP audio sink" },
	{ PW_KEY_MODULE_USAGE, MODULE_USAGE },
	{ PW_KEY_MODULE_VERSION, PACKAGE_VERSION },
};

struct impl {
	struct pw_context *context;

	struct pw_properties *props;

	struct pw_impl_module *module;
	struct pw_loop *loop;
	struct pw_work_queue *work;

	struct spa_hook module_listener;

	int protocol;

	struct pw_core *core;
	struct spa_hook core_proxy_listener;
	struct spa_hook core_listener;

	struct pw_properties *stream_props;
	struct pw_stream *stream;
	struct spa_hook stream_listener;
	struct spa_audio_info_raw info;
	uint32_t frame_size;

	struct pw_rtsp_client *rtsp;
	struct spa_hook rtsp_listener;
	struct pw_properties *headers;

	char *session_id;

	unsigned int do_disconnect:1;
	unsigned int unloading:1;

	uint8_t key[AES_CHUNK_SIZE]; /* Key for aes-cbc */
	uint8_t iv[AES_CHUNK_SIZE];  /* Initialization vector for cbc */
	AES_KEY aes;                 /* AES encryption */

	uint16_t control_port;
	int control_fd;
	uint16_t timing_port;
	int timing_fd;
	uint16_t server_port;
	int server_fd;

	uint16_t seq;
	uint32_t rtptime;
	uint32_t ssrc;
};

static void do_unload_module(void *obj, void *data, int res, uint32_t id)
{
	struct impl *impl = data;
	pw_impl_module_destroy(impl->module);
}

static void unload_module(struct impl *impl)
{
	if (!impl->unloading) {
		impl->unloading = true;
		pw_work_queue_add(impl->work, impl, 0, do_unload_module, impl);
	}
}

static void stream_destroy(void *d)
{
	struct impl *impl = d;
	spa_hook_remove(&impl->stream_listener);
	impl->stream = NULL;
}

static void stream_state_changed(void *d, enum pw_stream_state old,
		enum pw_stream_state state, const char *error)
{
	struct impl *impl = d;
	switch (state) {
	case PW_STREAM_STATE_ERROR:
	case PW_STREAM_STATE_UNCONNECTED:
		unload_module(impl);
		break;
	case PW_STREAM_STATE_PAUSED:
	case PW_STREAM_STATE_STREAMING:
		break;
	default:
		break;
	}
}

static inline void bit_writer(uint8_t **p, int *pos, uint8_t data, int len)
{
	int lb, rb;

        lb = 7 - *pos;
        rb = lb - len + 1;

	if (rb >= 0) {
		**p = (*pos ? **p : 0) | (data << rb);
                *pos += len;
	} else {
		**p |= (data >> -rb);
		(*p)++;
		**p = data << (8+rb);
		*pos = -rb;
	}
}

static int aes_encrypt(struct impl *impl, uint8_t *data, int len)
{
    uint8_t nv[AES_CHUNK_SIZE];
    uint8_t *buffer;
    int i, j;

    memcpy(nv, impl->iv, AES_CHUNK_SIZE);
    for (i = 0; i + AES_CHUNK_SIZE <= len; i += AES_CHUNK_SIZE) {
        buffer = data + i;
        for (j = 0; j < AES_CHUNK_SIZE; j++)
            buffer[j] ^= nv[j];

        AES_encrypt(buffer, buffer, &impl->aes);

        memcpy(nv, buffer, AES_CHUNK_SIZE);
    }
    return i;
}


static int add_to_packet(struct impl *impl, uint8_t *data, size_t size)
{
	const size_t max = 12 + 8 + (FRAMES_PER_UDP_PACKET * 4);
	uint8_t *pkt, *bp;
	int bpos = 0, res;

	pkt = alloca(max);
	pkt[0] = 0x80; /* RTP v2: 0x80 */
	pkt[1] = 0x60; /* Payload type: 0x60 */
	pkt[2] = impl->seq >> 8; /* Sequence number: 0x0000 */
	pkt[3] = impl->seq;
	pkt[4] = impl->rtptime >> 24; /* Timestamp */
	pkt[5] = impl->rtptime >> 16;
	pkt[6] = impl->rtptime >> 8;
	pkt[7] = impl->rtptime;
	pkt[8] = impl->ssrc >> 24;
	pkt[9] = impl->ssrc >> 16;
	pkt[10] = impl->ssrc >> 8;
	pkt[11] = impl->ssrc;

	bp = &pkt[12];

	size = SPA_MIN(size, FRAMES_PER_UDP_PACKET * 4u);

	bit_writer(&bp, &bpos, 1, 3); /* channel=1, stereo */
	bit_writer(&bp, &bpos, 0, 4); /* Unknown */
	bit_writer(&bp, &bpos, 0, 8); /* Unknown */
	bit_writer(&bp, &bpos, 0, 4); /* Unknown */
	bit_writer(&bp, &bpos, 1, 1); /* Hassize */
	bit_writer(&bp, &bpos, 0, 2); /* Unused */
	bit_writer(&bp, &bpos, 1, 1); /* Is-not-compressed */
	/* Size of data, integer, big endian. */
	bit_writer(&bp, &bpos, (size >> 24) & 0xff, 8);
	bit_writer(&bp, &bpos, (size >> 16) & 0xff, 8);
	bit_writer(&bp, &bpos, (size >> 8)  & 0xff, 8);
	bit_writer(&bp, &bpos, (size)       & 0xff, 8);

	impl->rtptime += size / 4;

	while (size > 4) {
		/* Byte swap stereo data. */
		bit_writer(&bp, &bpos, *(data + 1), 8);
		bit_writer(&bp, &bpos, *(data + 0), 8);
		bit_writer(&bp, &bpos, *(data + 3), 8);
		bit_writer(&bp, &bpos, *(data + 2), 8);
		data += 4;
		size -= 4;
	}

	impl->seq = (impl->seq + 1) & 0xffff;

	aes_encrypt(impl, pkt + 12, max - 12);

	pw_log_info("send %zu", max);
	res = write(impl->server_fd, pkt, max);

	return 0;
}

static void playback_stream_process(void *d)
{
	struct impl *impl = d;
	struct pw_buffer *buf;
	struct spa_data *bd;
	void *data;
	uint32_t size;


	if ((buf = pw_stream_dequeue_buffer(impl->stream)) == NULL) {
		pw_log_debug("out of buffers: %m");
		return;
	}

	bd = &buf->buffer->datas[0];
	data = SPA_PTROFF(bd->data, bd->chunk->offset, void);
	size = bd->chunk->size;

	if (impl->server_fd > 0)
		add_to_packet(impl, data, size);

	pw_stream_queue_buffer(impl->stream, buf);
}

static const struct pw_stream_events playback_stream_events = {
	PW_VERSION_STREAM_EVENTS,
	.destroy = stream_destroy,
	.state_changed = stream_state_changed,
	.process = playback_stream_process
};

static int create_udp_socket(struct impl *impl, uint16_t *port)
{
	int res, ip_version, fd, val, i, af;
	struct sockaddr_in sa4;
	struct sockaddr_in6 sa6;

	if ((res = pw_rtsp_client_get_local_ip(impl->rtsp,
				&ip_version, NULL, 0)) < 0)
		return res;

	if (ip_version == 4) {
		sa4.sin_family = af = AF_INET;
		sa4.sin_addr.s_addr = INADDR_ANY;
	} else {
		sa6.sin6_family = af = AF_INET6;
	        sa6.sin6_addr = in6addr_any;
	}

	if ((fd = socket(af, SOCK_DGRAM | SOCK_CLOEXEC | SOCK_NONBLOCK, 0)) < 0) {
		pw_log_error("socket failed: %m");
		return -errno;
	}

#ifdef SO_TIMESTAMP
	val = 1;
	if (setsockopt(fd, SOL_SOCKET, SO_TIMESTAMP, &val, sizeof(val)) < 0) {
		res = -errno;
		pw_log_error("setsockopt failed: %m");
		goto error;
	}
#endif
	val = 1;
	if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val)) < 0) {
		res = -errno;
		pw_log_error("setsockopt failed: %m");
		goto error;
	}

	for (i = 0; i < MAX_PORT_RETRY; i++) {
		int ret;

		if (ip_version == 4) {
			sa4.sin_port = htons(*port);
			ret = bind(fd, &sa4, sizeof(sa4));
		} else {
			sa6.sin6_port = htons(*port);
			ret = bind(fd, &sa6, sizeof(sa6));
		}
		if (ret == 0)
			break;
		if (ret < 0 && errno != EADDRINUSE) {
			res = -errno;
			pw_log_error("bind failed: %m");
			goto error;
		}
		(*port)++;
	}
	return fd;
error:
	close(fd);
	return res;
}

static int connect_udp_socket(struct impl *impl, int fd, uint16_t port)
{
	const char *host;
	struct sockaddr_in sa4;
	struct sockaddr_in6 sa6;
	struct sockaddr *sa;
	size_t salen;
	int res, af;

	host = pw_properties_get(impl->props, "raop.hostname");
	if (host == NULL)
		return -EINVAL;

	if (inet_pton(AF_INET, host, &sa4.sin_addr) > 0) {
		sa4.sin_family = af = AF_INET;
		sa4.sin_port = htons(port);
		sa = (struct sockaddr *) &sa4;
		salen = sizeof(sa4);
	} else if (inet_pton(AF_INET6, host, &sa6.sin6_addr) > 0) {
		sa6.sin6_family = af = AF_INET6;
		sa6.sin6_port = htons(port);
		sa = (struct sockaddr *) &sa6;
		salen = sizeof(sa6);
	} else {
		pw_log_error("Invalid host '%s'", host);
		return -EINVAL;
	}

	if (fd < 0 &&
	    (fd = socket(af, SOCK_DGRAM | SOCK_CLOEXEC | SOCK_NONBLOCK, 0)) < 0) {
		pw_log_error("socket failed: %m");
		return -errno;
	}

	if (connect(fd, sa, salen) < 0) {
		res = -errno;
		pw_log_error("connect failed: %m");
		goto error;
	}
	pw_log_info("Connected to host:%s port:%d", host, port);
	return fd;

error:
	if (fd >= 0)
		close(fd);
	return res;
}

static void rtsp_record_reply(void *data, int status, const struct spa_dict *headers)
{
	pw_log_info("reply %d", status);
}

static void rtsp_do_record(struct impl *impl)
{
	pw_getrandom(&impl->seq, sizeof(impl->seq), 0);
	pw_getrandom(&impl->rtptime, sizeof(impl->rtptime), 0);

	pw_properties_set(impl->headers, "Range", "npt=0-");
	pw_properties_setf(impl->headers, "RTP-Info",
			"seq=%u;rtptime=%u", impl->seq, impl->rtptime);

	pw_rtsp_client_send(impl->rtsp, "RECORD", &impl->headers->dict,
			NULL, NULL, rtsp_record_reply, impl);

	pw_properties_set(impl->headers, "Range", NULL);
	pw_properties_set(impl->headers, "RTP-Info", NULL);
}

static void rtsp_setup_reply(void *data, int status, const struct spa_dict *headers)
{
	struct impl *impl = data;
	const char *str, *state = NULL, *s;
	size_t len;
	uint16_t control_port, timing_port;

	pw_log_info("reply %d", status);

	if ((str = spa_dict_lookup(headers, "Session")) == NULL) {
		pw_log_error("missing Session header");
		return;
	}
	pw_properties_set(impl->headers, "Session", str);

	if ((str = spa_dict_lookup(headers, "Transport")) == NULL) {
		pw_log_error("missing Transport header");
		return;
	}

	impl->server_port = control_port = timing_port = 0;
	while ((s = pw_split_walk(str, ";", &len, &state)) != NULL) {
		if (spa_strstartswith(s, "server_port=")) {
			impl->server_port = atoi(s + 12);
		}
		else if (spa_strstartswith(s, "control_port=")) {
			control_port = atoi(s + 13);
		}
		else if (spa_strstartswith(s, "timing_port=")) {
			timing_port = atoi(s + 12);
		}
	}
	if (impl->server_port == 0 || control_port == 0 || timing_port == 0) {
		pw_log_error("missing ports in Transport");
		return;
	}
	pw_log_info("server port:%u control:%u timing:%u",
			impl->server_port, control_port, timing_port);

	if ((impl->server_fd = connect_udp_socket(impl, -1, impl->server_port)) <= 0)
		return;
	if ((impl->control_fd = connect_udp_socket(impl, impl->control_fd, control_port)) <= 0)
		return;
	if ((impl->timing_fd = connect_udp_socket(impl, impl->timing_fd, timing_port)) <= 0)
		return;

	rtsp_do_record(impl);
}

static void rtsp_do_setup(struct impl *impl)
{
	if (impl->protocol == 1) {
		pw_properties_set(impl->headers, "Transport",
				"RTP/AVP/TCP;unicast;interleaved=0-1;mode=record");
	} else {
		impl->control_port = DEFAULT_UDP_CONTROL_PORT;
		impl->timing_port = DEFAULT_UDP_TIMING_PORT;

		impl->control_fd = create_udp_socket(impl, &impl->control_port);
		impl->timing_fd = create_udp_socket(impl, &impl->timing_port);
		if (impl->control_fd < 0 || impl->timing_fd < 0)
			goto error;

		pw_properties_setf(impl->headers, "Transport",
				"RTP/AVP/UDP;unicast;interleaved=0-1;mode=record;"
				"control_port=%u;timing_port=%u",
				impl->control_port, impl->timing_port);
	}

	pw_rtsp_client_send(impl->rtsp, "SETUP", &impl->headers->dict,
			NULL, NULL, rtsp_setup_reply, impl);

	pw_properties_set(impl->headers, "Transport", NULL);

	return;

error:
	if (impl->control_fd > 0)
		close(impl->control_fd);
	impl->control_fd = -1;
	if (impl->timing_fd > 0)
		close(impl->timing_fd);
	impl->timing_fd = -1;
}

static void rtsp_announce_reply(void *data, int status, const struct spa_dict *headers)
{
	struct impl *impl = data;

	pw_log_info("reply %d", status);

	pw_properties_set(impl->headers, "Apple-Challenge", NULL);

	rtsp_do_setup(impl);
}

static void base64_encode(const uint8_t *data, size_t len, char *enc, char pad)
{
	static const char tab[] =
	    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
	size_t i;
	for (i = 0; i < len; i += 3) {
		uint32_t v;
		v  =              data[i+0]      << 16;
		v |= (i+1 < len ? data[i+1] : 0) << 8;
		v |= (i+2 < len ? data[i+2] : 0);
		*enc++ =             tab[(v >> (3*6)) & 0x3f];
		*enc++ =             tab[(v >> (2*6)) & 0x3f];
		*enc++ = i+1 < len ? tab[(v >> (1*6)) & 0x3f] : pad;
		*enc++ = i+2 < len ? tab[(v >> (0*6)) & 0x3f] : pad;
	}
	*enc = '\0';
}

static size_t base64_decode(const char *data, size_t len, uint8_t *dec)
{
	uint8_t tab[] = {
		62, -1, -1, -1, 63, 52, 53, 54, 55, 56,
		57, 58, 59, 60, 61, -1, -1, -1, -1, -1,
		-1, -1,  0,  1,  2,  3,  4,  5,  6,  7,
		 8,  9, 10, 11, 12, 13, 14, 15, 16, 17,
		18, 19, 20, 21, 22, 23, 24, 25, -1, -1,
		-1, -1, -1, -1, 26, 27, 28, 29, 30, 31,
		32, 33, 34, 35, 36, 37, 38, 39, 40, 41,
		42, 43, 44, 45, 46, 47, 48, 49, 50, 51 };
	size_t i, j;
	for (i = 0, j = 0; i < len; i += 4) {
		uint32_t v;
		v =                          tab[data[i+0]-43]  << (3*6);
		v |=                         tab[data[i+1]-43]  << (2*6);
		v |= (data[i+2] == '=' ? 0 : tab[data[i+2]-43]) << (1*6);
		v |= (data[i+3] == '=' ? 0 : tab[data[i+3]-43]);
		                      dec[j++] = (v >> 16) & 0xff;
		if (data[i+2] != '=') dec[j++] = (v >> 8)  & 0xff;
		if (data[i+3] != '=') dec[j++] =  v        & 0xff;
	}
	return j;
}

static int rsa_encrypt(uint8_t *data, int len, uint8_t *res)
{
	RSA *rsa;
	uint8_t modulus[256];
	uint8_t exponent[8];
	size_t size;
	BIGNUM *n_bn = NULL;
	BIGNUM *e_bn = NULL;
	char n[] =
		"59dE8qLieItsH1WgjrcFRKj6eUWqi+bGLOX1HL3U3GhC/j0Qg90u3sG/1CUtwC"
		"5vOYvfDmFI6oSFXi5ELabWJmT2dKHzBJKa3k9ok+8t9ucRqMd6DZHJ2YCCLlDR"
		"KSKv6kDqnw4UwPdpOMXziC/AMj3Z/lUVX1G7WSHCAWKf1zNS1eLvqr+boEjXuB"
		"OitnZ/bDzPHrTOZz0Dew0uowxf/+sG+NCK3eQJVxqcaJ/vEHKIVd2M+5qL71yJ"
		"Q+87X6oV3eaYvt3zWZYD6z5vYTcrtij2VZ9Zmni/UAaHqn9JdsBWLUEpVviYnh"
		"imNVvYFZeCXg/IdTQ+x4IRdiXNv5hEew==";
	char e[] = "AQAB";

	rsa = RSA_new();

	size = base64_decode(n, strlen(n), modulus);
	n_bn = BN_bin2bn(modulus, size, NULL);

	size = base64_decode(e, strlen(e), exponent);
	e_bn = BN_bin2bn(exponent, size, NULL);

	RSA_set0_key(rsa, n_bn, e_bn, NULL);

	size = RSA_public_encrypt(len, data, res, rsa, RSA_PKCS1_OAEP_PADDING);
	RSA_free(rsa);
	return size;
}

static void rtsp_do_announce(struct impl *impl)
{
	const char *host;
	uint8_t rsakey[512];
	char key[512*2];
	char iv[16*2];
	int frames, i, ip_version;
	char *sdp;
        char local_ip[256];

	host = pw_properties_get(impl->props, "raop.hostname");

	if (impl->protocol == 0)
		frames = FRAMES_PER_TCP_PACKET;
	else
		frames = FRAMES_PER_UDP_PACKET;

	pw_getrandom(impl->key, sizeof(impl->key), 0);
	AES_set_encrypt_key(impl->key, 128, &impl->aes);
	pw_getrandom(impl->iv, sizeof(impl->iv), 0);

	pw_log_info("aes %p", &impl->aes);

	i = rsa_encrypt(impl->key, 16, rsakey);
        base64_encode(rsakey, i, key, '=');
        base64_encode(impl->iv, 16, iv, '=');

	pw_rtsp_client_get_local_ip(impl->rtsp, &ip_version,
			local_ip, sizeof(local_ip));

#if 0
	asprintf(&sdp, "v=0\r\n"
			"o=iTunes %s 0 IN IP%d %s\r\n"
			"s=iTunes\r\n"
			"c=IN IP%d %s\r\n"
			"t=0 0\r\n"
			"m=audio 0 RTP/AVP 96\r\n"
			"a=rtpmap:96 AppleLossless\r\n"
			"a=fmtp:96 %d 0 16 40 10 14 2 255 0 0 44100\r\n",
			impl->session_id, ip_version, local_ip,
			ip_version, host, frames);
#else
	asprintf(&sdp, "v=0\r\n"
			"o=iTunes %s 0 IN IP%d %s\r\n"
			"s=iTunes\r\n"
			"c=IN IP%d %s\r\n"
			"t=0 0\r\n"
			"m=audio 0 RTP/AVP 96\r\n"
			"a=rtpmap:96 AppleLossless\r\n"
			"a=fmtp:96 %d 0 16 40 10 14 2 255 0 0 44100\r\n"
			"a=rsaaeskey:%s\r\n"
			"a=aesiv:%s\r\n",
			impl->session_id, ip_version, local_ip,
			ip_version, host, frames, key, iv);
#endif

	pw_rtsp_client_send(impl->rtsp, "ANNOUNCE", &impl->headers->dict,
			"application/sdp", sdp, rtsp_announce_reply, impl);
}


static void rtsp_options_reply(void *data, int status, const struct spa_dict *headers)
{
	struct impl *impl = data;
	pw_log_info("options");

	rtsp_do_announce(impl);
}

static void rtsp_connected(void *data)
{
	struct impl *impl = data;
	uint32_t sci[2];
	uint8_t rac[16];
	char sac[16*4];

	pw_log_info("connected");

	pw_getrandom(sci, sizeof(sci), 0);
	pw_properties_setf(impl->headers, "Client-Instance",
			"%08x%08x", sci[0], sci[1]);

	pw_getrandom(rac, sizeof(rac), 0);
	base64_encode(rac, sizeof(rac), sac, '\0');
	pw_properties_set(impl->headers, "Apple-Challenge", sac);

	pw_rtsp_client_send(impl->rtsp, "OPTIONS", &impl->headers->dict,
			NULL, NULL, rtsp_options_reply, impl);
}

static void rtsp_disconnected(void *data)
{
	pw_log_info("disconnected");
}

static void rtsp_error(void *data, int res)
{
	pw_log_info("error %d", res);
}

static void rtsp_message(void *data, int status, int state,
			const struct spa_dict *headers)
{
	const struct spa_dict_item *it;
	pw_log_info("message %d %d", status, state);
	spa_dict_for_each(it, headers)
		pw_log_info(" %s: %s", it->key, it->value);

}

static const struct pw_rtsp_client_events rtsp_events = {
	PW_VERSION_RTSP_CLIENT_EVENTS,
	.connected = rtsp_connected,
	.error = rtsp_error,
	.disconnected = rtsp_disconnected,
	.message = rtsp_message,
};

static int create_stream(struct impl *impl)
{
	int res;
	uint32_t n_params;
	const struct spa_pod *params[1];
	uint8_t buffer[1024];
	struct spa_pod_builder b;
	const char *hostname, *port;
	uint32_t session_id;

	impl->stream = pw_stream_new(impl->core, "example sink", impl->stream_props);
	impl->stream_props = NULL;

	if (impl->stream == NULL)
		return -errno;

	pw_stream_add_listener(impl->stream,
			&impl->stream_listener,
			&playback_stream_events, impl);

	n_params = 0;
	spa_pod_builder_init(&b, buffer, sizeof(buffer));
	params[n_params++] = spa_format_audio_raw_build(&b,
			SPA_PARAM_EnumFormat, &impl->info);

	if ((res = pw_stream_connect(impl->stream,
			PW_DIRECTION_INPUT,
			PW_ID_ANY,
			PW_STREAM_FLAG_AUTOCONNECT |
			PW_STREAM_FLAG_MAP_BUFFERS |
			PW_STREAM_FLAG_RT_PROCESS,
			params, n_params)) < 0)
		return res;

	impl->headers = pw_properties_new(NULL, NULL);

	impl->rtsp = pw_rtsp_client_new(impl->loop, NULL, 0);
	if (impl->rtsp == NULL)
		return -errno;

	pw_rtsp_client_add_listener(impl->rtsp, &impl->rtsp_listener,
			&rtsp_events, impl);

	hostname = pw_properties_get(impl->props, "raop.hostname");
	port = pw_properties_get(impl->props, "raop.port");
	if (hostname == NULL || port == NULL)
		return -EINVAL;

	pw_getrandom(&session_id, sizeof(session_id), 0);
	asprintf(&impl->session_id, "%u", session_id);

	pw_rtsp_client_connect(impl->rtsp, hostname, atoi(port), impl->session_id);

	return 0;
}

static void core_error(void *data, uint32_t id, int seq, int res, const char *message)
{
	struct impl *impl = data;

	pw_log_error("error id:%u seq:%d res:%d (%s): %s",
			id, seq, res, spa_strerror(res), message);

	if (id == PW_ID_CORE && res == -EPIPE)
		unload_module(impl);
}

static const struct pw_core_events core_events = {
	PW_VERSION_CORE_EVENTS,
	.error = core_error,
};

static void core_destroy(void *d)
{
	struct impl *impl = d;
	spa_hook_remove(&impl->core_listener);
	impl->core = NULL;
	unload_module(impl);
}

static const struct pw_proxy_events core_proxy_events = {
	.destroy = core_destroy,
};

static void impl_destroy(struct impl *impl)
{
	if (impl->stream)
		pw_stream_destroy(impl->stream);
	if (impl->core && impl->do_disconnect)
		pw_core_disconnect(impl->core);

	pw_properties_free(impl->stream_props);
	pw_properties_free(impl->props);

	if (impl->work)
		pw_work_queue_cancel(impl->work, impl, SPA_ID_INVALID);
	free(impl);
}

static void module_destroy(void *data)
{
	struct impl *impl = data;
	impl->unloading = true;
	spa_hook_remove(&impl->module_listener);
	impl_destroy(impl);
}

static const struct pw_impl_module_events module_events = {
	PW_VERSION_IMPL_MODULE_EVENTS,
	.destroy = module_destroy,
};

static inline uint32_t format_from_name(const char *name, size_t len)
{
	int i;
	for (i = 0; spa_type_audio_format[i].name; i++) {
		if (strncmp(name, spa_debug_type_short_name(spa_type_audio_format[i].name), len) == 0)
			return spa_type_audio_format[i].type;
	}
	return SPA_AUDIO_FORMAT_UNKNOWN;
}

static uint32_t channel_from_name(const char *name)
{
	int i;
	for (i = 0; spa_type_audio_channel[i].name; i++) {
		if (spa_streq(name, spa_debug_type_short_name(spa_type_audio_channel[i].name)))
			return spa_type_audio_channel[i].type;
	}
	return SPA_AUDIO_CHANNEL_UNKNOWN;
}

static void parse_position(struct spa_audio_info_raw *info, const char *val, size_t len)
{
	struct spa_json it[2];
	char v[256];

	spa_json_init(&it[0], val, len);
        if (spa_json_enter_array(&it[0], &it[1]) <= 0)
                spa_json_init(&it[1], val, len);

	info->channels = 0;
	while (spa_json_get_string(&it[1], v, sizeof(v)) > 0 &&
	    info->channels < SPA_AUDIO_MAX_CHANNELS) {
		info->position[info->channels++] = channel_from_name(v);
	}
}

static int parse_audio_info(struct impl *impl)
{
	struct pw_properties *props = impl->stream_props;
	struct spa_audio_info_raw *info = &impl->info;
	const char *str;

	spa_zero(*info);

	if ((str = pw_properties_get(props, PW_KEY_AUDIO_FORMAT)) == NULL)
		str = DEFAULT_FORMAT;
	info->format = format_from_name(str, strlen(str));
	switch (info->format) {
	case SPA_AUDIO_FORMAT_S8:
	case SPA_AUDIO_FORMAT_U8:
		impl->frame_size = 1;
		break;
	case SPA_AUDIO_FORMAT_S16:
		impl->frame_size = 2;
		break;
	case SPA_AUDIO_FORMAT_S24:
		impl->frame_size = 3;
		break;
	case SPA_AUDIO_FORMAT_S24_32:
	case SPA_AUDIO_FORMAT_S32:
	case SPA_AUDIO_FORMAT_F32:
		impl->frame_size = 4;
		break;
	case SPA_AUDIO_FORMAT_F64:
		impl->frame_size = 8;
		break;
	default:
		pw_log_error("unsupported format '%s'", str);
		return -EINVAL;
	}
	info->rate = pw_properties_get_uint32(props, PW_KEY_AUDIO_RATE, DEFAULT_RATE);
	if (info->rate == 0) {
		pw_log_error("invalid rate '%s'", str);
		return -EINVAL;
	}
	if ((str = pw_properties_get(props, PW_KEY_AUDIO_CHANNELS)) == NULL)
		str = DEFAULT_CHANNELS;
	info->channels = atoi(str);
	if ((str = pw_properties_get(props, SPA_KEY_AUDIO_POSITION)) == NULL)
		str = DEFAULT_POSITION;
	parse_position(info, str, strlen(str));
	if (info->channels == 0) {
		pw_log_error("invalid channels '%s'", str);
		return -EINVAL;
	}
	impl->frame_size *= info->channels;

	return 0;
}

static void copy_props(struct impl *impl, struct pw_properties *props, const char *key)
{
	const char *str;
	if ((str = pw_properties_get(props, key)) != NULL) {
		if (pw_properties_get(impl->stream_props, key) == NULL)
			pw_properties_set(impl->stream_props, key, str);
	}
}

SPA_EXPORT
int pipewire__module_init(struct pw_impl_module *module, const char *args)
{
	struct pw_context *context = pw_impl_module_get_context(module);
	struct pw_properties *props = NULL;
	uint32_t id = pw_global_get_id(pw_impl_module_get_global(module));
	struct impl *impl;
	const char *str;
	int res;

	PW_LOG_TOPIC_INIT(mod_topic);

	impl = calloc(1, sizeof(struct impl));
	if (impl == NULL)
		return -errno;

	pw_log_debug("module %p: new %s", impl, args);

	if (args == NULL)
		args = "";

	props = pw_properties_new_string(args);
	if (props == NULL) {
		res = -errno;
		pw_log_error( "can't create properties: %m");
		goto error;
	}
	impl->props = props;

	impl->stream_props = pw_properties_new(NULL, NULL);
	if (impl->stream_props == NULL) {
		res = -errno;
		pw_log_error( "can't create properties: %m");
		goto error;
	}

	impl->module = module;
	impl->context = context;
	impl->loop = pw_context_get_main_loop(context);
	impl->work = pw_context_get_work_queue(context);
	if (impl->work == NULL) {
		res = -errno;
		pw_log_error( "can't get work queue: %m");
		goto error;
	}

	if (pw_properties_get(props, PW_KEY_NODE_GROUP) == NULL)
		pw_properties_set(props, PW_KEY_NODE_GROUP, "pipewire.dummy");
	if (pw_properties_get(props, PW_KEY_NODE_VIRTUAL) == NULL)
		pw_properties_set(props, PW_KEY_NODE_VIRTUAL, "true");

	if (pw_properties_get(props, PW_KEY_MEDIA_CLASS) == NULL)
		pw_properties_set(props, PW_KEY_MEDIA_CLASS, "Audio/Sink");

	if (pw_properties_get(props, PW_KEY_NODE_NAME) == NULL)
		pw_properties_setf(props, PW_KEY_NODE_NAME, "raop-sink-%u", id);
	if (pw_properties_get(props, PW_KEY_NODE_DESCRIPTION) == NULL)
		pw_properties_set(props, PW_KEY_NODE_DESCRIPTION,
				pw_properties_get(props, PW_KEY_NODE_NAME));

	if ((str = pw_properties_get(props, "stream.props")) != NULL)
		pw_properties_update_string(impl->stream_props, str, strlen(str));

	copy_props(impl, props, PW_KEY_AUDIO_FORMAT);
	copy_props(impl, props, PW_KEY_AUDIO_RATE);
	copy_props(impl, props, PW_KEY_AUDIO_CHANNELS);
	copy_props(impl, props, SPA_KEY_AUDIO_POSITION);
	copy_props(impl, props, PW_KEY_NODE_NAME);
	copy_props(impl, props, PW_KEY_NODE_DESCRIPTION);
	copy_props(impl, props, PW_KEY_NODE_GROUP);
	copy_props(impl, props, PW_KEY_NODE_LATENCY);
	copy_props(impl, props, PW_KEY_NODE_VIRTUAL);
	copy_props(impl, props, PW_KEY_MEDIA_CLASS);

	if ((res = parse_audio_info(impl)) < 0) {
		pw_log_error( "can't parse audio format");
		goto error;
	}

	impl->core = pw_context_get_object(impl->context, PW_TYPE_INTERFACE_Core);
	if (impl->core == NULL) {
		str = pw_properties_get(props, PW_KEY_REMOTE_NAME);
		impl->core = pw_context_connect(impl->context,
				pw_properties_new(
					PW_KEY_REMOTE_NAME, str,
					NULL),
				0);
		impl->do_disconnect = true;
	}
	if (impl->core == NULL) {
		res = -errno;
		pw_log_error("can't connect: %m");
		goto error;
	}

	pw_proxy_add_listener((struct pw_proxy*)impl->core,
			&impl->core_proxy_listener,
			&core_proxy_events, impl);
	pw_core_add_listener(impl->core,
			&impl->core_listener,
			&core_events, impl);

	if ((res = create_stream(impl)) < 0)
		goto error;

	pw_impl_module_add_listener(module, &impl->module_listener, &module_events, impl);

	pw_impl_module_update_properties(module, &SPA_DICT_INIT_ARRAY(module_props));

	return 0;

error:
	impl_destroy(impl);
	return res;
}