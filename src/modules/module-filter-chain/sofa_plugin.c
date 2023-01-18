#include "config.h"

#include <spa/utils/json.h>
#include <pipewire/log.h>
#include "plugin.h"
#include "convolver.h"
#include "dsp-ops.h"
#include "pffft.h"

#ifdef HAVE_LIBMYSOFA
#include <mysofa.h>
#include <pthread.h>

// > If your program is using several threads, you must use
// > appropriate synchronisation mechanisms so only
// > a single thread can access the mysofa_open_cached
// > and mysofa_close_cached functions at a given time.
static pthread_mutex_t libmysofa_mutex = PTHREAD_MUTEX_INITIALIZER;
#endif

static struct dsp_ops *dsp_ops;

struct spatializer_impl {
  unsigned long rate;
	float *port[64];
	float old_coords[3];
	float coords[3];
	int n_samples, blocksize, tailsize;

#ifdef HAVE_LIBMYSOFA
	struct MYSOFA_EASY *sofa;
#endif

	struct convolver *l_conv;
	struct convolver *r_conv;
};

static void * spatializer_instantiate(const struct fc_descriptor * Descriptor,
		unsigned long SampleRate, int index, const char *config)
{
#ifdef HAVE_LIBMYSOFA
	struct spatializer_impl *impl;
	struct spa_json it[2];
	const char *val;
	char key[256];
	char filename[PATH_MAX] = "";

	errno = EINVAL;
	if (config == NULL)
		return NULL;

	spa_json_init(&it[0], config, strlen(config));
	if (spa_json_enter_object(&it[0], &it[1]) <= 0)
		return NULL;

	impl = calloc(1, sizeof(*impl));
	if (impl == NULL) {
		errno = ENOMEM;
		goto error;
	}

	while (spa_json_get_string(&it[1], key, sizeof(key)) > 0) {
		if (spa_streq(key, "blocksize")) {
			if (spa_json_get_int(&it[1], &impl->blocksize) <= 0) {
				pw_log_error("spatializer:blocksize requires a number");
				errno = EINVAL;
				goto error;
			}
		}
		else if (spa_streq(key, "tailsize")) {
			if (spa_json_get_int(&it[1], &impl->tailsize) <= 0) {
				pw_log_error("spatializer:tailsize requires a number");
				errno = EINVAL;
				goto error;
			}
		}
		else if (spa_streq(key, "filename")) {
			if (spa_json_get_string(&it[1], filename, sizeof(filename)) <= 0) {
				pw_log_error("spatializer:filename requires a string");
				errno = EINVAL;
				goto error;
			}
		}
		else if (spa_json_next(&it[1], &val) < 0)
			break;
	}
	if (!filename[0]) {
		pw_log_error("spatializer:filename was not given");
		errno = EINVAL;
		goto error;
	}

	int ret = MYSOFA_OK;

	pthread_mutex_lock(&libmysofa_mutex);
	impl->sofa = mysofa_open_cached(filename, SampleRate, &impl->n_samples, &ret);
	pthread_mutex_unlock(&libmysofa_mutex);

	if (ret != MYSOFA_OK) {
		pw_log_error("Unable to load HRTF from %s: %d %m", filename, ret);
		errno = ENOENT;
		goto error;
	}

	for (uint8_t i = 0; i < 3; i++) {
		impl->old_coords[i] = impl->coords[i] = NAN;
	}

	if (impl->blocksize <= 0)
		impl->blocksize = SPA_CLAMP(impl->n_samples, 64, 256);
	if (impl->tailsize <= 0)
		impl->tailsize = SPA_CLAMP(4096, impl->blocksize, 32768);

	pw_log_info("using n_samples:%u %d:%d blocksize sofa:%s", impl->n_samples,
		impl->blocksize, impl->tailsize, filename);

	impl->rate = SampleRate;
	return impl;
error:
	if (impl->sofa) {
		pthread_mutex_lock(&libmysofa_mutex);
		mysofa_close_cached(impl->sofa);
		pthread_mutex_unlock(&libmysofa_mutex);
	}
	free(impl);
	return NULL;
#else
	pw_log_error("libmysofa is required for spatializer, but disabled at compile time");
	errno = EINVAL;
	return NULL;
#endif
}

static void spatializer_run(void * Instance, unsigned long SampleCount)
{
#ifdef HAVE_LIBMYSOFA
	struct spatializer_impl *impl = Instance;

	bool reload = false;
	for (uint8_t i = 0; i < 3; i++) {
		if ((impl->port[3 + i] && impl->old_coords[i] != impl->port[3 + i][0])
			|| isnan(impl->old_coords[i])) {
			reload = true;
		}
		impl->old_coords[i] = impl->coords[i] =
				impl->port[3 + i][0];
	}

	if (reload) {
		float *left_ir = calloc(impl->n_samples, sizeof(float));
		float *right_ir = calloc(impl->n_samples, sizeof(float));
		float left_delay;
		float right_delay;

		mysofa_s2c(impl->coords);
		mysofa_getfilter_float(
			impl->sofa,
			impl->coords[0],
			impl->coords[1],
			impl->coords[2],
			left_ir,
			right_ir,
			&left_delay,
			&right_delay
		);

		// TODO: make use of delay
		if ((left_delay || right_delay) && (!isnan(left_delay) || !isnan(right_delay))) {
			pw_log_warn("delay dropped l: %f, r: %f", left_delay, right_delay);
		}

		if (impl->l_conv)
			convolver_free(impl->l_conv);
		impl->l_conv = convolver_new(dsp_ops, impl->blocksize, impl->tailsize, left_ir, impl->n_samples);
		free(left_ir);
		if (impl->l_conv == NULL) {
			pw_log_error("reloading left convolver failed");
			return;
		}

		if (impl->r_conv)
			convolver_free(impl->r_conv);
		impl->r_conv = convolver_new(dsp_ops, impl->blocksize, impl->tailsize, right_ir, impl->n_samples);
		free(right_ir);
		if (impl->r_conv == NULL) {
			pw_log_error("reloading right convolver failed");
			return;
		}
	}

	convolver_run(impl->l_conv, impl->port[2], impl->port[0], SampleCount);
	convolver_run(impl->r_conv, impl->port[2], impl->port[1], SampleCount);
#endif
}

static void spatializer_connect_port(void * Instance, unsigned long Port,
                        float * DataLocation)
{
	struct spatializer_impl *impl = Instance;
	impl->port[Port] = DataLocation;
}

static void spatializer_cleanup(void * Instance)
{
	struct spatializer_impl *impl = Instance;
	if (impl->l_conv)
		convolver_free(impl->l_conv);
	if (impl->r_conv)
		convolver_free(impl->r_conv);

#ifdef HAVE_LIBMYSOFA
	if (impl->sofa) {
		pthread_mutex_lock(&libmysofa_mutex);
		mysofa_close_cached(impl->sofa);
		pthread_mutex_unlock(&libmysofa_mutex);
	}
#endif

	free(impl);
}

static void spatializer_deactivate(void * Instance)
{
	struct spatializer_impl *impl = Instance;
	if (impl->l_conv)
		convolver_reset(impl->l_conv);
	if (impl->r_conv)
		convolver_reset(impl->r_conv);
}

static struct fc_port spatializer_ports[] = {
	{ .index = 0,
	  .name = "Out L",
	  .flags = FC_PORT_OUTPUT | FC_PORT_AUDIO,
	},
	{ .index = 1,
	  .name = "Out R",
	  .flags = FC_PORT_OUTPUT | FC_PORT_AUDIO,
	},
	{ .index = 2,
	  .name = "In",
	  .flags = FC_PORT_INPUT | FC_PORT_AUDIO,
	},

	{ .index = 3,
	  .name = "Azimuth",
	  .flags = FC_PORT_INPUT | FC_PORT_CONTROL,
	  .def = 0.0f, .min = 0.0f, .max = 360.0f
	},
	{ .index = 4,
	  .name = "Elevation",
	  .flags = FC_PORT_INPUT | FC_PORT_CONTROL,
	  .def = 0.0f, .min = -90.0f, .max = 90.0f
	},
	{ .index = 5,
	  .name = "Radius",
	  .flags = FC_PORT_INPUT | FC_PORT_CONTROL,
	  .def = 1.0f, .min = 0.0f, .max = 100.0f
	},
};

static const struct fc_descriptor spatializer_desc = {
	.name = "spatializer",

	.n_ports = 6,
	.ports = spatializer_ports,

	.instantiate = spatializer_instantiate,
	.connect_port = spatializer_connect_port,
	.deactivate = spatializer_deactivate,
	.run = spatializer_run,
	.cleanup = spatializer_cleanup,
};

static const struct fc_descriptor * sofa_descriptor(unsigned long Index)
{
	switch(Index) {
	case 0:
		return &spatializer_desc;
	}
	return NULL;
}


static const struct fc_descriptor *sofa_make_desc(struct fc_plugin *plugin, const char *name)
{
	unsigned long i;
	for (i = 0; ;i++) {
		const struct fc_descriptor *d = sofa_descriptor(i);
		if (d == NULL)
			break;
		if (spa_streq(d->name, name))
			return d;
	}
	return NULL;
}

static struct fc_plugin builtin_plugin = {
	.make_desc = sofa_make_desc
};

struct fc_plugin *load_sofa_plugin(const struct spa_support *support, uint32_t n_support,
		struct dsp_ops *dsp, const char *plugin, const char *config)
{
	dsp_ops = dsp;
	pffft_select_cpu(dsp->cpu_flags);
	return &builtin_plugin;
}

