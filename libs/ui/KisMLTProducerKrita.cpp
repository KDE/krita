/*
 * KisMLTProducerKrita
 * Produces variable-speed audio within a restricted range of frames. Used internally by Krita to drive audio-synced animation playback.
 * Copyright (C) 2022 Eoin O'Neill <eoinoneill1991@gmail.com>
 * Copyright (C) 2022 Emmet O'Neill <emmetoneill.pdx@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include "KisMLTProducerKrita.h"

#include <framework/mlt.h>
#include <limits.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "kis_assert.h"

#include <framework/mlt_factory.h>
#include <framework/mlt_frame.h>
#include <framework/mlt_producer.h>
#include <framework/mlt_property.h>
#include <framework/mlt_service.h>

typedef struct
{
    mlt_producer producer_internal;
} private_data;

/** Restricts frame index to within range by modulus wrapping (not clamping).
*/
static int restrict_range(int index, int min, int max)
{
    const int span = max - min;
    return (MAX(index - min, 0) % (span + 1)) + min;
}

static int is_valid_range(const int frame_start, const int frame_end)
{
    const bool NON_NEGATIVE = frame_start >= 0 && frame_end >= 0;
    const bool NON_INVERTED = frame_end > frame_start;

    return NON_NEGATIVE && NON_INVERTED;
}

static int producer_get_audio(mlt_frame frame,
                              void **buffer,
                              mlt_audio_format *format,
                              int *frequency,
                              int *channels,
                              int *samples)
{
    mlt_producer producer = static_cast<mlt_producer>(mlt_frame_pop_audio(frame));

    struct mlt_audio_s audio;

    mlt_audio_set_values(&audio, *buffer, *frequency, *format, *samples, *channels);

    int error = mlt_frame_get_audio(frame,
                                    &audio.data,
                                    &audio.format,
                                    &audio.frequency,
                                    &audio.channels,
                                    &audio.samples);

    mlt_properties props = MLT_PRODUCER_PROPERTIES(producer);

    // Scale the frequency to account for the dynamic speed (normalized).
    double SPEED = mlt_properties_get_double(props, "speed");

    KIS_SAFE_ASSERT_RECOVER(!qFuzzyIsNull(SPEED)) {
        SPEED = 1.0;
    }

    audio.frequency = (double) audio.frequency * fabs(SPEED);
    if (SPEED < 0.0) {
        mlt_audio_reverse(&audio);
    }

    mlt_audio_get_values(&audio, buffer, frequency, format, samples, channels);

    return error;
}

static int producer_get_frame(mlt_producer producer, mlt_frame_ptr frame, int index)
{
    mlt_properties props = MLT_PRODUCER_PROPERTIES(producer);
    const int FRAME_START = mlt_properties_get_int(props, "start_frame");
    const int FRAME_END = mlt_properties_get_int(props, "end_frame");
    const bool IS_RANGE_LIMITED = mlt_properties_get_int(props, "limit_enabled");

    private_data *pdata = (private_data *) producer->child;
    const int POSITION = mlt_producer_position(pdata->producer_internal);

    if (IS_RANGE_LIMITED && is_valid_range(FRAME_START, FRAME_END)) {
        mlt_properties_set_position(MLT_PRODUCER_PROPERTIES(pdata->producer_internal),
                                    "_position",
                                    restrict_range(POSITION, FRAME_START, FRAME_END));
    }

    int retval = mlt_service_get_frame((mlt_service) pdata->producer_internal, frame, index);

    if (!mlt_frame_is_test_audio(*frame)) {
        mlt_frame_push_audio(*frame, producer);
        mlt_frame_push_audio(*frame, (void*)producer_get_audio);
    }

    return retval;
}

static void updateInternalProducesEofStatus(mlt_producer producer_internal, int effectiveLimitEnabled)
{
    const char *effectiveEofMode = effectiveLimitEnabled ? "continue" : "pause";

    mlt_properties internalProducerProps = MLT_PRODUCER_PROPERTIES(producer_internal);

    const char *currentEofMode = mlt_properties_get(internalProducerProps, "eof");

    if (!currentEofMode || strcmp(currentEofMode, effectiveEofMode) != 0) {
        mlt_properties_set_string(internalProducerProps, "eof", effectiveEofMode);
    }
}

static void producer_property_changed( mlt_service owner, mlt_producer self, mlt_event_data event_data)
{
    const char *name = mlt_event_data_to_string(event_data);
    if (!name) return;

    if (strcmp(name, "start_frame") == 0 ||
        strcmp(name, "end_frame" ) == 0 ||
        strcmp(name, "limit_enabled") == 0) {

        mlt_properties props = MLT_PRODUCER_PROPERTIES(self);

        const int effectiveLimitEnabled =
            is_valid_range(mlt_properties_get_int(props, "start_frame"),
                           mlt_properties_get_int(props, "end_frame")) &&
            mlt_properties_get_int(props, "limit_enabled");

        private_data* pdata = (private_data*)self->child;
        updateInternalProducesEofStatus(pdata->producer_internal, effectiveLimitEnabled);
    }
}

static int producer_seek(mlt_producer producer, mlt_position position)
{
    private_data *pdata = (private_data *) producer->child;

    int retval = mlt_producer_seek(pdata->producer_internal, position);

    return retval;
}

static void producer_close(mlt_producer producer)
{
    private_data *pdata = (private_data *) producer->child;

    if (pdata) {
        mlt_producer_close(pdata->producer_internal);
        free(pdata);
    }

    producer->close = NULL;
    mlt_producer_close(producer);
    free(producer);
}

/** Constructor for the producer.
*/
extern "C" void* producer_krita_init(mlt_profile profile,
                                 mlt_service_type type,
                                 const char *id,
                                 const void *arg)
{
    // Create a new producer object
    mlt_producer producer = mlt_producer_new(profile);
    private_data *pdata = (private_data *) calloc(1, sizeof(private_data));

    if (arg && producer && pdata) {
        mlt_properties producer_properties = MLT_PRODUCER_PROPERTIES(producer);

        // Initialize the producer
        mlt_properties_set(producer_properties, "resource", (char*)arg);
        producer->child = pdata;
        producer->get_frame = producer_get_frame;
        producer->seek = producer_seek;
        producer->close = (mlt_destructor) producer_close;

        // Get the resource to be passed to the clip producer
        char *resource = (char*)arg;

        // Create internal producer
        pdata->producer_internal = mlt_factory_producer(profile, "abnormal", resource);

        if (pdata->producer_internal) {
            mlt_producer_set_speed(pdata->producer_internal, 1.0);
        }

        mlt_events_listen( producer_properties, producer, "property-changed", ( mlt_listener )producer_property_changed );
        updateInternalProducesEofStatus(pdata->producer_internal, 0);
    }

    const bool INVALID_CONTEXT = !producer || !pdata || !pdata->producer_internal;
    if (INVALID_CONTEXT) { // Clean up early...
        if (pdata) {
            mlt_producer_close(pdata->producer_internal);
            free(pdata);
        }

        if (producer) {
            producer->child = NULL;
            producer->close = NULL;
            mlt_producer_close(producer);
            free(producer);
            producer = NULL;
        }
    }

    return producer;
}

void registerKritaMLTProducer(Mlt::Repository *repository)
{
    repository->register_service(mlt_service_producer_type, "krita_play_chunk", producer_krita_init);
}
