/* Spa
 *
 * Copyright © 2019 Wim Taymans
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

#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/eventfd.h>
#include <sys/signalfd.h>

#include <evl/clock.h>
#include <evl/poll.h>
#include <evl/syscall.h>
#include <evl/timer.h>

#include <spa/support/log.h>
#include <spa/support/system.h>
#include <spa/support/plugin.h>
#include <spa/utils/type.h>

#define NAME "evl-system"

#define MAX_POLL	512

struct poll_entry {
	int pfd;
	int fd;
	uint32_t events;
	void *data;
};

struct impl {
	struct spa_handle handle;
	struct spa_system system;

        struct spa_log *log;

	struct poll_entry entries[MAX_POLL];
	uint32_t n_entries;
};

static ssize_t impl_read(void *object, int fd, void *buf, size_t count)
{
	return oob_read(fd, buf, count);
}

static ssize_t impl_write(void *object, int fd, const void *buf, size_t count)
{
	return oob_write(fd, buf, count);
}

static int impl_ioctl(void *object, int fd, unsigned long request, ...)
{
	int res;
	va_list ap;
	long arg;

	va_start(ap, request);
	arg = va_arg(ap, long);
	res = oob_ioctl(fd, request, arg);
	va_end(ap);

	return res;
}

static int impl_close(void *object, int fd)
{
	return close(fd);
}

static inline int clock_id_to_evl(int clockid)
{
	switch(clockid) {
	case CLOCK_MONOTONIC:
		return EVL_CLOCK_MONOTONIC;
	case CLOCK_REALTIME:
		return EVL_CLOCK_REALTIME;
	default:
		return -clockid;
	}
}

/* clock */
static int impl_clock_gettime(void *object,
			int clockid, struct timespec *value)
{
	return evl_read_clock(clock_id_to_evl(clockid), value);
}

static int impl_clock_getres(void *object,
			int clockid, struct timespec *res)
{
	return evl_get_clock_resolution(clock_id_to_evl(clockid), res);
}

/* poll */
static inline uint32_t spa_io_to_poll(uint32_t mask)
{
	uint32_t events = 0;

	if (mask & SPA_IO_IN)
		events |= POLLIN;
	if (mask & SPA_IO_OUT)
		events |= POLLOUT;
	if (mask & SPA_IO_ERR)
		events |= POLLERR;
	if (mask & SPA_IO_HUP)
		events |= POLLHUP;

	return events;
}

static inline uint32_t spa_poll_to_io(uint32_t events)
{
	uint32_t mask = 0;

	if (events & POLLIN)
		mask |= SPA_IO_IN;
	if (events & POLLOUT)
		mask |= SPA_IO_OUT;
	if (events & POLLHUP)
		mask |= SPA_IO_HUP;
	if (events & POLLERR)
		mask |= SPA_IO_ERR;

	return mask;
}

static int impl_pollfd_create(void *object, int flags)
{
	int retval;
	retval = evl_new_poll();
	return retval;
}

static inline struct poll_entry *find_entry(struct impl *impl, int pfd, int fd)
{
	uint32_t i;
	for (i = 0; i < impl->n_entries; i++) {
		struct poll_entry *e = &impl->entries[i];
		if (e->pfd == pfd && e->fd == fd)
			return e;
	}
	return NULL;
}

static int impl_pollfd_add(void *object, int pfd, int fd, uint32_t events, void *data)
{
	struct impl *impl = object;
	struct poll_entry *e;

	if (impl->n_entries == MAX_POLL) {
		errno = ENOSPC;
		return -1;
	}
	e = &impl->entries[impl->n_entries++];
	e->pfd = pfd;
	e->fd = fd;
	e->events = spa_io_to_poll(events);
	e->data = data;
	return evl_add_pollfd(pfd, fd, e->events)
}

static int impl_pollfd_mod(void *object, int pfd, int fd, uint32_t events, void *data)
{
	struct impl *impl = object;
	struct poll_entry *e;

	e = find_entry(impl, pfd, fd);
	if (e == NULL) {
		errno = ENOENT;
		return -1;
	}
	e->events = spa_io_to_poll(events);
	e->data = data;
	return evl_mod_pollfd(pfd, fd, e->events)
}

static int impl_pollfd_del(void *object, int pfd, int fd)
{
	struct impl *impl = object;
	struct poll_entry *e;

	e = find_entry(impl, pfd, fd);
	if (e == NULL) {
		errno = ENOENT;
		return -1;
	}
	e->pfd = -1;
	e->fd = -1;
	return evl_del_pollfd(pfd, fd)
}

static int impl_pollfd_wait(void *object, int pfd,
		struct spa_poll_event *ev, int n_ev, int timeout)
{
	struct impl *impl = object;
	uint32_t i, j, n_pollset;
	struct evl_poll_event pollset[n_ev];
	void *polldata[n_ev];
	struct timespec tv;
	int nfds;

        for (i = 0, j = 0; i < n_entries && j < n_ev; i++) {
		struct poll_entry *e = &impl->entries[i];
		if (e->pfd != pfd)
			continue;
		pollset[j].fd = e->fd;
		pollset[j].events = e->events;
		pollset[j].revents = 0;
		polldata[j] = e->data;
		j++;
	}
	n_pollset = j;

	tv.tv_sec = timeout / SPA_MSEC_PER_SEC;
	tv.tv_nsec = (timeout % SPA_MSEC_PER_SEC) * SPA_NSEC_PER_MSEC;
	nfds = evl_timedpoll(pfd, pollset, n_pollset, &tv);
	if (SPA_UNLIKELY(nfds < 0))
		return nfds;

        for (i = 0, j = 0; i < n_pollset; i++) {
		if (pollset[i].revents == 0)
			continue;

		ev[j].events = pollset[i].revents;
		ev[j].data = polldata[i];
		j++;
	}
	return j;
}

/* timers */
static int impl_timerfd_create(void *object, int clockid, int flags)
{
	return evl_new_timer(clockid);
}

static int impl_timerfd_settime(void *object,
			int fd, int flags,
			const struct itimerspec *new_value,
			struct itimerspec *old_value)
{
	return evl_set_timer(fd, new_value, old_value);
}

static int impl_timerfd_gettime(void *object,
			int fd, struct itimerspec *curr_value)
{
	return evl_get_timer(fd, curr_value);

}
static int impl_timerfd_read(void *object, int fd, uint64_t *expirations)
{
	uint32_t ticks;
	if (oob_read(fd, &ticks, sizeof(ticks)) != sizeof(ticks))
		return -errno;
	*expirations = ticks;
	return 0;
}

/* events */
static int impl_eventfd_create(void *object, int flags)
{
	int fl = 0;
	if (flags & SPA_FD_CLOEXEC)
		fl |= EFD_CLOEXEC;
	if (flags & SPA_FD_NONBLOCK)
		fl |= EFD_NONBLOCK;
	if (flags & SPA_FD_EVENT_SEMAPHORE)
		fl |= EFD_SEMAPHORE;
	return eventfd(0, fl);
}

static int impl_eventfd_write(void *object, int fd, uint64_t count)
{
	if (write(fd, &count, sizeof(uint64_t)) != sizeof(uint64_t))
		return -errno;
	return 0;
}

static int impl_eventfd_read(void *object, int fd, uint64_t *count)
{
	if (read(fd, count, sizeof(uint64_t)) != sizeof(uint64_t))
		return -errno;
	return 0;
}

/* signals */
static int impl_signalfd_create(void *object, int signal, int flags)
{
	sigset_t mask;
	int res, fl = 0;

	if (flags & SPA_FD_CLOEXEC)
		fl |= SFD_CLOEXEC;
	if (flags & SPA_FD_NONBLOCK)
		fl |= SFD_NONBLOCK;

	sigemptyset(&mask);
	sigaddset(&mask, signal);
	res = signalfd(-1, &mask, fl);
	sigprocmask(SIG_BLOCK, &mask, NULL);

	return res;
}

static int impl_signalfd_read(void *object, int fd, int *signal)
{
	struct signalfd_siginfo signal_info;
	int len;

	len = read(fd, &signal_info, sizeof signal_info);
	if (!(len == -1 && errno == EAGAIN) && len != sizeof signal_info)
		return -errno;

	*signal = signal_info.ssi_signo;

	return 0;
}

static const struct spa_system_methods impl_system = {
	SPA_VERSION_SYSTEM_METHODS,
	.read = impl_read,
	.write = impl_write,
	.ioctl = impl_ioctl,
	.close = impl_close,
	.clock_gettime = impl_clock_gettime,
	.clock_getres = impl_clock_getres,
	.pollfd_create = impl_pollfd_create,
	.pollfd_add = impl_pollfd_add,
	.pollfd_mod = impl_pollfd_mod,
	.pollfd_del = impl_pollfd_del,
	.pollfd_wait = impl_pollfd_wait,
	.timerfd_create = impl_timerfd_create,
	.timerfd_settime = impl_timerfd_settime,
	.timerfd_gettime = impl_timerfd_gettime,
	.timerfd_read = impl_timerfd_read,
	.eventfd_create = impl_eventfd_create,
	.eventfd_write = impl_eventfd_write,
	.eventfd_read = impl_eventfd_read,
	.signalfd_create = impl_signalfd_create,
	.signalfd_read = impl_signalfd_read,
};

static int impl_get_interface(struct spa_handle *handle, uint32_t type, void **interface)
{
	struct impl *impl;

	spa_return_val_if_fail(handle != NULL, -EINVAL);
	spa_return_val_if_fail(interface != NULL, -EINVAL);

	impl = (struct impl *) handle;

	switch (type) {
	case SPA_TYPE_INTERFACE_System:
		*interface = &impl->system;
		break;
	default:
		return -ENOENT;
	}
	return 0;
}

static int impl_clear(struct spa_handle *handle)
{
	spa_return_val_if_fail(handle != NULL, -EINVAL);
	return 0;
}

static size_t
impl_get_size(const struct spa_handle_factory *factory,
	      const struct spa_dict *params)
{
	return sizeof(struct impl);
}

static int
impl_init(const struct spa_handle_factory *factory,
	  struct spa_handle *handle,
	  const struct spa_dict *info,
	  const struct spa_support *support,
	  uint32_t n_support)
{
	struct impl *impl;
	uint32_t i;

	spa_return_val_if_fail(factory != NULL, -EINVAL);
	spa_return_val_if_fail(handle != NULL, -EINVAL);

	handle->get_interface = impl_get_interface;
	handle->clear = impl_clear;

	impl = (struct impl *) handle;
	impl->system.iface = SPA_INTERFACE_INIT(
			SPA_TYPE_INTERFACE_System,
			SPA_VERSION_SYSTEM,
			&impl_system, impl);

	for (i = 0; i < n_support; i++) {
		switch (support[i].type) {
		case SPA_TYPE_INTERFACE_Log:
			impl->log = support[i].data;
			break;
		}
	}

	spa_log_debug(impl->log, NAME " %p: initialized", impl);

	return 0;
}

static const struct spa_interface_info impl_interfaces[] = {
	{SPA_TYPE_INTERFACE_System,},
};

static int
impl_enum_interface_info(const struct spa_handle_factory *factory,
			 const struct spa_interface_info **info,
			 uint32_t *index)
{
	spa_return_val_if_fail(factory != NULL, -EINVAL);
	spa_return_val_if_fail(info != NULL, -EINVAL);
	spa_return_val_if_fail(index != NULL, -EINVAL);

	if (*index >= SPA_N_ELEMENTS(impl_interfaces))
		return 0;

	*info = &impl_interfaces[(*index)++];
	return 1;
}

const struct spa_handle_factory spa_support_evl_system_factory = {
	SPA_VERSION_HANDLE_FACTORY,
	"evl.system",
	NULL,
	impl_get_size,
	impl_init,
	impl_enum_interface_info
};
