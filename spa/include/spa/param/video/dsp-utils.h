/* Simple Plugin API
 *
 * Copyright © 2018 Wim Taymans
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

#ifndef SPA_VIDEO_DSP_UTILS_H
#define SPA_VIDEO_DSP_UTILS_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \addtogroup spa_param
 * \{
 */

#include <spa/param/video/format-utils.h>
#include <spa/param/video/dsp.h>

static inline int
spa_format_video_dsp_parse(const struct spa_pod *format,
			   struct spa_video_info_dsp *info)
{
	info->flags = SPA_VIDEO_FLAG_NONE;
	if (spa_pod_find_prop (format, NULL, SPA_FORMAT_VIDEO_modifier)) {
		info->flags |= SPA_VIDEO_FLAG_MODIFIER;
	}

	return spa_pod_parse_object(format,
		SPA_TYPE_OBJECT_Format, NULL,
		SPA_FORMAT_VIDEO_format,		SPA_POD_OPT_Id(&info->format),
		SPA_FORMAT_VIDEO_modifier,		SPA_POD_OPT_Long(&info->modifier));
}

static inline struct spa_pod *
spa_format_video_dsp_build(struct spa_pod_builder *builder, uint32_t id,
			   struct spa_video_info_dsp *info)
{
	struct spa_pod_frame f;
	spa_pod_builder_push_object(builder, &f, SPA_TYPE_OBJECT_Format, id);
	spa_pod_builder_add(builder,
			SPA_FORMAT_mediaType,		SPA_POD_Id(SPA_MEDIA_TYPE_video),
			SPA_FORMAT_mediaSubtype,	SPA_POD_Id(SPA_MEDIA_SUBTYPE_dsp),
			0);
	if (info->format != SPA_VIDEO_FORMAT_UNKNOWN)
		spa_pod_builder_add(builder,
			SPA_FORMAT_VIDEO_format,	SPA_POD_Id(info->format), 0);
	if (info->modifier != 0 || info->flags & SPA_VIDEO_FLAG_MODIFIER)
		spa_pod_builder_add(builder,
			SPA_FORMAT_VIDEO_modifier,	SPA_POD_Long(info->modifier), 0);
	return (struct spa_pod*)spa_pod_builder_pop(builder, &f);
}

/**
 * \}
 */

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* SPA_VIDEO_DSP_H */
