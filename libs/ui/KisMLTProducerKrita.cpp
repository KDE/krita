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
    int audio_sample_rate = 0;
    bool force_reset_audio_frequency_for_frames = false; // used for 'count' producer
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

void scale_audio_frequency(mlt_producer producer, mlt_audio audio)
{
    mlt_properties props = MLT_PRODUCER_PROPERTIES(producer);

    // Scale the frequency to account for the dynamic speed (normalized).
    double SPEED = mlt_properties_get_double(props, "speed");

    KIS_SAFE_ASSERT_RECOVER(!qFuzzyIsNull(SPEED)) {
        SPEED = 1.0;
    }

    audio->frequency = (double) audio->frequency * fabs(SPEED);

    KIS_SAFE_ASSERT_RECOVER(audio->frequency > 0) {
        audio->frequency = 1;
    }

    if (SPEED < 0.0) {
        mlt_audio_reverse(audio);
    }
}

static int producer_get_audio(mlt_frame frame,
                              void **buffer,
                              mlt_audio_format *format,
                              int *frequency,
                              int *channels,
                              int *samples)
{
    mlt_producer producer = static_cast<mlt_producer>(mlt_frame_pop_audio(frame));
    private_data *pdata = (private_data *) producer->child;

    struct mlt_audio_s audio;

    /**
     * MLT doesn't reset the requested frequency on every call, that is, if the
     * underlying producer just passes through the frequency, it will eventually
     * drop to zero and crash. AVformat resets the frequency every time to the
     * value of the underlying media. Count producer doesn't reset the frequency
     * by default, so we should reset it manually by passing negative values.
     */
    if (pdata->force_reset_audio_frequency_for_frames) {
        *frequency = -1;
        *samples = -1;
    }

    mlt_audio_set_values(&audio, *buffer, *frequency, *format, *samples, *channels);

    int error = mlt_frame_get_audio(frame,
                                    &audio.data,
                                    &audio.format,
                                    &audio.frequency,
                                    &audio.channels,
                                    &audio.samples);

    scale_audio_frequency(producer, &audio);
    mlt_audio_get_values(&audio, buffer, frequency, format, samples, channels);

    return error;
}

static int producer_generate_silent_audio(mlt_frame frame,
                                          void **buffer,
                                          mlt_audio_format *format,
                                          int *frequency,
                                          int *channels,
                                          int *samples)
{
    mlt_producer producer = static_cast<mlt_producer>(mlt_frame_pop_audio(frame));
    private_data *pdata = (private_data *) producer->child;

    {
        // Get the producer fps
        double fps = mlt_producer_get_fps(producer);

        if (mlt_properties_get(MLT_FRAME_PROPERTIES(frame), "producer_consumer_fps"))
            fps = mlt_properties_get_double(MLT_FRAME_PROPERTIES(frame), "producer_consumer_fps");

        mlt_position position = mlt_properties_get_position(MLT_FRAME_PROPERTIES(frame), "_position");

        int size = 0;
        *channels = *channels <= 0 ? 2 : *channels;
        *frequency = pdata->audio_sample_rate > 0 ? pdata->audio_sample_rate : 44100;
        *samples = mlt_audio_calculate_frame_samples(fps, *frequency, position); //NOTE: Audio BUFFER_SIZE
        *format = *format == mlt_audio_none ? mlt_audio_s16 : *format;

        size = mlt_audio_format_size(*format, *samples, *channels);
        if (size)
            *buffer = mlt_pool_alloc(size);
        else
            *buffer = NULL;

        /**
         * Save the generated audio into the frame itself,
         * since this overloaded function will not be called
         * on the further calls to mlt_frame_get_audio (the
         * pointer to the function was placed on the stack,
         * which has already been taken)
         */
        mlt_frame_set_audio(frame, *buffer, *format, size, mlt_pool_release);
    }

    struct mlt_audio_s audio;
    mlt_audio_set_values(&audio, *buffer, *frequency, *format, *samples, *channels);
    mlt_audio_silence(&audio, *samples, 0);

    scale_audio_frequency(producer, &audio);

    mlt_audio_get_values(&audio, buffer, frequency, format, samples, channels);

    return 0;
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
    } else {
        /**
         * Generate a slowed-down silence frame and reset
         * the `test_audio` flag
         *
         * When the data stream ends, the AVformat library
         * returns and empty frame, flagged with "test_audio"
         * tag. Later on, when the consumer reads this frame,
         * mlt_frame_get_audio() generates a frame of silence
         * with 48kHz resolution. We cannot use this variant
         * of silence, since it is not scaled. We need to
         * generate our own version of silence, which is
         * scaled according to our format.
         */
        mlt_frame_push_audio(*frame, producer);
        mlt_frame_push_audio(*frame, (void*)producer_generate_silent_audio);

        mlt_properties properties = MLT_FRAME_PROPERTIES(*frame);
        mlt_properties_set_int(properties, "test_audio", 0);
    }

    return retval;
}

static void producer_property_changed( mlt_service owner, mlt_producer self, mlt_event_data event_data)
{
    const char *name = mlt_event_data_to_string(event_data);
    if (!name) return;

    /**
     * We don't use MLT's "speed" value for anything, but some
     * MLT's functions may adjust the speed of the producer, e.g.
     * when seeking. So we should keep the two values in sync.
     */
    if (strcmp(name, "_speed") == 0) {
        const double speed = mlt_producer_get_speed(self);
        private_data* pdata = (private_data*)self->child;
        mlt_producer_set_speed(pdata->producer_internal, speed);
    }
}

static int producer_seek(mlt_producer producer, mlt_position position)
{
    private_data *pdata = (private_data *) producer->child;

    int retval = mlt_producer_seek(pdata->producer_internal, position);

    /**
     * Update the position values of the parent producer
     */
    mlt_properties_set_position(MLT_PRODUCER_PROPERTIES(producer),
                                "_position",
                                position);
    mlt_properties_set_position(MLT_PRODUCER_PROPERTIES(producer),
                                "_frame",
                                position);

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
            mlt_properties internalProducerProps = MLT_PRODUCER_PROPERTIES(pdata->producer_internal);

            /**
             * We permanently set the EOF mode to "continue", because
             * other modes have weird effects, like clipping the data when
             * seeking or resetting the producer speed to null.
             */
            mlt_properties_set_string(internalProducerProps, "eof", "continue");

            const char *serviceName = mlt_properties_get(internalProducerProps, "mlt_service");

            if (!strcmp(serviceName, "avformat")) {
                /**
                 * Disable caching of frames in avformat producer
                 *
                 * Caching in MLT library is broken. When a frame is taken from the
                 * cache its "audio" property is not restored. It breaks the work
                 * of "read-ahead" consumer thread, which also temporarily stores
                 * this frame.
                 */
                mlt_properties_set_int(internalProducerProps, "noimagecache", 1);

                /**
                 * Fetch media sample rate to be able to generate correct
                 * silence stream
                 */

                char key[200];
                const int numberOfStreams = mlt_properties_get_int(internalProducerProps, "meta.media.nb_streams");

                for (int i = 0; i < numberOfStreams; i++) {
                    snprintf(key, sizeof(key), "meta.media.%u.stream.type", i);

                    const char* type = mlt_properties_get(internalProducerProps, key);
                    if (type && !strcmp(type, "audio")) {
                        snprintf(key, sizeof(key), "meta.media.%u.codec.sample_rate", i);
                        pdata->audio_sample_rate = mlt_properties_get_int(internalProducerProps, key);
                    }
                }
            } else if (!strcmp(serviceName, "count")) {
                pdata->audio_sample_rate = 48000;
                pdata->force_reset_audio_frequency_for_frames = true;
            } else {
                KIS_SAFE_ASSERT_RECOVER_NOOP(0 && "mlt_service used for media is unknown!");
            }
        }

        mlt_properties_set_string(producer_properties, "eof", "continue");

        mlt_events_listen( producer_properties, producer, "property-changed", ( mlt_listener )producer_property_changed );
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
