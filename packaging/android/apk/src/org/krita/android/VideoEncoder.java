// SPDX-License-Identifier: GPL-3.0-or-later
package org.krita.android;

import android.content.ContentResolver;
import android.content.Context;
import android.media.AudioFormat;
import android.media.Image;
import android.media.MediaCodec;
import android.media.MediaCodecInfo;
import android.media.MediaCodecList;
import android.media.MediaFormat;
import android.media.MediaMuxer;
import android.net.Uri;
import android.os.Build;
import android.os.ParcelFileDescriptor;
import android.util.Log;

import java.nio.ByteBuffer;
import java.util.ArrayList;
import java.util.Collections;
import java.util.List;

public class VideoEncoder {

    public static class Support {
        public final String name;
        public final boolean hardware;

        public Support(String name, boolean hardware) {
            this.name = name;
            this.hardware = hardware;
        }
    }

    private static class BufferedSample {
        public final byte[] data;
        public final MediaCodec.BufferInfo bufferInfo;
        public final boolean audio;

        private BufferedSample(ByteBuffer buffer, MediaCodec.BufferInfo bufferInfo, boolean audio) {
            this.data = new byte[bufferInfo.size];
            this.bufferInfo = bufferInfo;
            this.audio = audio;
            buffer.get(data);
        }
    }

    private static final String TAG = "krita.VideoEncoder";

    // Keep these formats in sync with KisAndroidMediaEncoderRunnable!
    private static final int FORMAT_MP4_H264 = 0;
    private static final int FORMAT_WEBM_VP8 = 1;
    private static final int FORMAT_MP4_AV1 = 2;
    // These statuses too!
    private static final int STATUS_OK = 0;
    private static final int STATUS_TIMEOUT = 1;
    private static final int STATUS_END_OF_STREAM = 2;
    private static final int STATUS_NEEDS_TEMP_TO_OUTPUT_COPY = 3;
    private static final int STATUS_ERROR_START_ALREADY_STARTED = 101;
    private static final int STATUS_ERROR_START_UNKNOWN_VIDEO_FORMAT = 102;
    private static final int STATUS_ERROR_START_VIDEO_FORMAT = 103;
    private static final int STATUS_ERROR_START_VIDEO_ENCODER = 104;
    private static final int STATUS_ERROR_START_MUXER = 105;
    private static final int STATUS_ERROR_START_UNKNOWN_AUDIO_FORMAT = 106;
    private static final int STATUS_ERROR_START_AUDIO_FORMAT = 107;
    private static final int STATUS_ERROR_START_AUDIO_ENCODER = 108;
    private static final int STATUS_ERROR_PREPARE_DEQUEUE = 201;
    private static final int STATUS_ERROR_PREPARE_GET_IMAGE = 202;
    private static final int STATUS_ERROR_PREPARE_NULL_IMAGE = 203;
    private static final int STATUS_ERROR_PREPARE_GET_AUDIO_BUFFER = 204;
    private static final int STATUS_ERROR_PREPARE_NULL_AUDIO_BUFFER = 205;
    private static final int STATUS_ERROR_COMMIT_QUEUE = 301;
    private static final int STATUS_ERROR_DRAIN_VIDEO_DEQUEUE = 401;
    private static final int STATUS_ERROR_DRAIN_VIDEO_TRACK_ALREADY_INITIALIZED = 402;
    private static final int STATUS_ERROR_DRAIN_VIDEO_ENCODER_GET_OUTPUT_FORMAT = 403;
    private static final int STATUS_ERROR_DRAIN_VIDEO_MUXER_ADD_TRACK = 404;
    private static final int STATUS_ERROR_DRAIN_VIDEO_MUXER_START = 405;
    private static final int STATUS_ERROR_DRAIN_VIDEO_ENCODER_GET_OUTPUT_BUFFER = 406;
    private static final int STATUS_ERROR_DRAIN_VIDEO_ENCODER_NULL_OUTPUT_BUFFER = 407;
    private static final int STATUS_ERROR_DRAIN_VIDEO_TRACK_NOT_INITIALIZED = 408;
    private static final int STATUS_ERROR_DRAIN_VIDEO_ENCODER_RELEASE_OUTPUT_BUFFER = 409;
    private static final int STATUS_ERROR_DRAIN_VIDEO_OUTPUT_BUFFER_POSITION = 410;
    private static final int STATUS_ERROR_DRAIN_VIDEO_OUTPUT_BUFFER_LIMIT = 411;
    private static final int STATUS_ERROR_DRAIN_VIDEO_MUXER_WRITE = 412;
    private static final int STATUS_ERROR_DRAIN_VIDEO_MUXER_BUFFER = 413;
    private static final int STATUS_ERROR_FINISH_QUEUE = 501;
    private static final int STATUS_ERROR_CLOSE_TEAR_DOWN = 601;
    private static final int STATUS_ERROR_DRAIN_AUDIO_DEQUEUE = 701;
    private static final int STATUS_ERROR_DRAIN_AUDIO_TRACK_ALREADY_INITIALIZED = 702;
    private static final int STATUS_ERROR_DRAIN_AUDIO_ENCODER_GET_OUTPUT_FORMAT = 703;
    private static final int STATUS_ERROR_DRAIN_AUDIO_MUXER_ADD_TRACK = 704;
    private static final int STATUS_ERROR_DRAIN_AUDIO_MUXER_START = 705;
    private static final int STATUS_ERROR_DRAIN_AUDIO_ENCODER_GET_OUTPUT_BUFFER = 706;
    private static final int STATUS_ERROR_DRAIN_AUDIO_ENCODER_NULL_OUTPUT_BUFFER = 707;
    private static final int STATUS_ERROR_DRAIN_AUDIO_TRACK_NOT_INITIALIZED = 708;
    private static final int STATUS_ERROR_DRAIN_AUDIO_ENCODER_RELEASE_OUTPUT_BUFFER = 709;
    private static final int STATUS_ERROR_DRAIN_AUDIO_OUTPUT_BUFFER_POSITION = 710;
    private static final int STATUS_ERROR_DRAIN_AUDIO_OUTPUT_BUFFER_LIMIT = 711;
    private static final int STATUS_ERROR_DRAIN_AUDIO_MUXER_WRITE = 712;
    private static final int STATUS_ERROR_DRAIN_AUDIO_MUXER_BUFFER = 713;

    private final int mFormat;
    private final int mWidth;
    private final int mHeight;
    private final float mFramerate;
    private final double mFrameDurationUs;
    private final int mAudioSampleRate;
    private final int mAudioChannelCount;
    private final String mOutputPath;
    private final String mTempPath;
    private final String mVideoEncoderName;
    private final String mAudioEncoderName;
    private final String mAudioFormatName;
    private final int mVideoBitrate;
    private final int mAudioBitrate;
    private MediaCodec.BufferInfo mVideoBufferInfo = null;
    private MediaCodec.BufferInfo mAudioBufferInfo = null;
    private MediaCodec mVideoEncoder = null;
    private MediaCodec mAudioEncoder = null;
    private MediaMuxer mMuxer = null;
    private boolean mUsesTempPath = false;
    private boolean mMuxerStarted = false;
    private int mVideoFrameIndex = 0;
    private int mAudioSamplesWritten = 0;
    private int mVideoTrack = -1;
    private int mAudioTrack = -1;
    private int mVideoInputBufferIndex = -1;
    private int mAudioInputBufferIndex = -1;
    private Image mVideoInputImage = null;
    private ByteBuffer mAudioInputBuffer = null;
    private boolean mVideoFinished = false;
    private boolean mAudioFinished = false;
    private boolean mVideoStreamEnded = false;
    private boolean mAudioStreamEnded = false;
    private final List<BufferedSample> mBufferedSamples = new ArrayList<>();

    public VideoEncoder(int format, int width, int height, float framerate, String outputPath,
                        String tempPath, String videoEncoderName, int videoBitrate,
                        String audioEncoderName, String audioFormatName, int audioSampleRate,
                        int audioChannelCount, int audioBitrate) {
        mFormat = format;
        mWidth = width;
        mHeight = height;
        mFramerate = framerate;
        mFrameDurationUs = 1000000.0 / ((double) framerate);
        mOutputPath = outputPath;
        mTempPath = tempPath;
        mVideoEncoderName = videoEncoderName;
        mVideoBitrate = videoBitrate;
        mAudioEncoderName = audioEncoderName;
        mAudioFormatName = audioFormatName;
        mAudioSampleRate = audioSampleRate;
        mAudioChannelCount = audioChannelCount;
        mAudioBitrate = audioBitrate;
    }

    public int start(Context context) {
        if (mVideoEncoder != null || mMuxer != null) {
            Log.e(TAG, "Attempting to start an already started video encoder");
            return STATUS_ERROR_START_ALREADY_STARTED;
        }

        String videoMimeType = getFormatVideoMimeType(mFormat);
        if (videoMimeType == null) {
            return STATUS_ERROR_START_UNKNOWN_VIDEO_FORMAT;
        }

        MediaFormat videoFormat;
        try {
            videoFormat = initializeVideoFormat(videoMimeType);
        } catch (Exception e) {
            Log.e(TAG, "Error initializing video format", e);
            return STATUS_ERROR_START_VIDEO_FORMAT;
        }

        String audioMimeType = null;
        if (mAudioSampleRate > 0 && mAudioChannelCount > 0 && mAudioBitrate > 0) {
            audioMimeType = getFormatAudioMimeType(mFormat);
            if (audioMimeType == null) {
                return STATUS_ERROR_START_UNKNOWN_AUDIO_FORMAT;
            }
        }

        MediaFormat audioFormat = null;
        if (audioMimeType != null) {
            try {
                audioFormat = initializeAudioFormat(audioMimeType);
            } catch (Exception e) {
                Log.e(TAG, "Error initializing audio format", e);
                return STATUS_ERROR_START_AUDIO_FORMAT;
            }
        }

        try {
            mVideoEncoder = initializeEncoder(mVideoEncoderName, videoMimeType, videoFormat);
        } catch (Exception e) {
            Log.e(TAG, "Error initializing video encoder", e);
            return STATUS_ERROR_START_VIDEO_ENCODER;
        }

        if (audioFormat != null) {
            try {
                mAudioEncoder = initializeEncoder(mAudioEncoderName, audioMimeType, audioFormat);
            } catch (Exception e) {
                Log.e(TAG, "Error initializing audio encoder", e);
                return STATUS_ERROR_START_AUDIO_ENCODER;
            }
        }

        try {
            mMuxer = initializeMuxer(context);
        } catch (Exception e) {
            Log.e(TAG, "Error initializing muxer", e);
            return STATUS_ERROR_START_MUXER;
        }

        return STATUS_OK;
    }

    public int prepareVideo(long timeoutUs) {
        if (mVideoInputBufferIndex == -1) {
            int videoInputBufferIndex;
            try {
                videoInputBufferIndex = mVideoEncoder.dequeueInputBuffer(timeoutUs);
            } catch (Exception e) {
                Log.e(TAG, "Error dequeuing video input buffer", e);
                return STATUS_ERROR_PREPARE_DEQUEUE;
            }

            if (videoInputBufferIndex == MediaCodec.INFO_TRY_AGAIN_LATER) {
                return STATUS_TIMEOUT;
            }

            Image inputImage;
            try {
                inputImage = mVideoEncoder.getInputImage(videoInputBufferIndex);
            } catch (Exception e) {
                Log.e(TAG, "Error getting input image " + videoInputBufferIndex, e);
                return STATUS_ERROR_PREPARE_GET_IMAGE;
            }
            if (inputImage == null) {
                Log.e(TAG, "No input image " + videoInputBufferIndex);
                return STATUS_ERROR_PREPARE_NULL_IMAGE;
            }

            mVideoInputBufferIndex = videoInputBufferIndex;
            mVideoInputImage = inputImage;
        }
        return STATUS_OK;
    }

    public int prepareAudio(long timeoutUs) {
        if (mAudioInputBufferIndex == -1) {
            int audioInputBufferIndex;
            try {
                audioInputBufferIndex = mAudioEncoder.dequeueInputBuffer(timeoutUs);
            } catch (Exception e) {
                Log.e(TAG, "Error dequeuing audio input buffer", e);
                return STATUS_ERROR_PREPARE_DEQUEUE;
            }

            if (audioInputBufferIndex == MediaCodec.INFO_TRY_AGAIN_LATER) {
                return STATUS_TIMEOUT;
            }

            ByteBuffer audioInputBuffer;
            try {
                audioInputBuffer = mAudioEncoder.getInputBuffer(audioInputBufferIndex);
            } catch (Exception e) {
                Log.e(TAG, "Error getting audio input buffer " + audioInputBufferIndex, e);
                return STATUS_ERROR_PREPARE_GET_AUDIO_BUFFER;
            }
            if (audioInputBuffer == null) {
                Log.e(TAG, "No audio input buffer " + audioInputBufferIndex);
                return STATUS_ERROR_PREPARE_NULL_AUDIO_BUFFER;
            }

            mAudioInputBufferIndex = audioInputBufferIndex;
            mAudioInputBuffer = audioInputBuffer;
            mAudioInputBuffer.clear();
        }
        return STATUS_OK;
    }

    public ByteBuffer getInputImagePlaneBuffer(int index) {
        try {
            Image.Plane[] planes = mVideoInputImage.getPlanes();
            Image.Plane plane = planes[index];
            return plane.getBuffer();
        } catch (Exception e) {
            Log.e(TAG, "Error getting image plane buffer " + index, e);
            return null;
        }
    }

    public int getInputImagePlaneRowStride(int index) {
        try {
            Image.Plane[] planes = mVideoInputImage.getPlanes();
            Image.Plane plane = planes[index];
            return plane.getRowStride();
        } catch (Exception e) {
            Log.e(TAG, "Error getting image plane row stride " + index, e);
            return -1;
        }
    }

    public int getInputImagePlanePixelStride(int index) {
        try {
            Image.Plane[] planes = mVideoInputImage.getPlanes();
            Image.Plane plane = planes[index];
            return plane.getPixelStride();
        } catch (Exception e) {
            Log.e(TAG, "Error getting image plane pixel stride " + index, e);
            return -1;
        }
    }

    public ByteBuffer getInputAudioBuffer() {
        return mAudioInputBuffer;
    }

    public int commitVideo() {
        try {
            ByteBuffer inputBuffer = mVideoEncoder.getInputBuffer(mVideoInputBufferIndex);
            mVideoEncoder.queueInputBuffer(mVideoInputBufferIndex, 0, inputBuffer == null ? 0 :
                    inputBuffer.capacity(), getVideoPresentationTimeUs(), 0);
        } catch (Exception e) {
            Log.e(TAG, "Error committing video frame " + mVideoFrameIndex, e);
            return STATUS_ERROR_COMMIT_QUEUE;
        }
        mVideoInputBufferIndex = -1;
        mVideoInputImage = null;
        ++mVideoFrameIndex;
        return STATUS_OK;
    }

    public int commitAudio(int chunkSamples, int chunkSize) {
        try {
            mAudioEncoder.queueInputBuffer(mAudioInputBufferIndex, 0, chunkSize,
                    getAudioPresentationTimeUs(), 0);
        } catch (Exception e) {
            Log.e(TAG, "Error committing samples at " + mAudioSamplesWritten, e);
            return STATUS_ERROR_COMMIT_QUEUE;
        }
        mAudioInputBufferIndex = -1;
        mAudioInputBuffer = null;
        mAudioSamplesWritten += chunkSamples;
        return STATUS_OK;
    }

    public int drainVideo(long timeoutUs) {
        if (mVideoStreamEnded) {
            return STATUS_END_OF_STREAM;
        }

        if (mVideoBufferInfo == null) {
            mVideoBufferInfo = new MediaCodec.BufferInfo();
        }

        int outputBufferIndex;
        int flags;
        try {
            outputBufferIndex = mVideoEncoder.dequeueOutputBuffer(mVideoBufferInfo, timeoutUs);
            flags = mVideoBufferInfo.flags;
        } catch (Exception e) {
            Log.e(TAG, "Error dequeuing video output buffer", e);
            return STATUS_ERROR_DRAIN_VIDEO_DEQUEUE;
        }

        if (outputBufferIndex == MediaCodec.INFO_TRY_AGAIN_LATER) {
            return STATUS_TIMEOUT;

        } else if (outputBufferIndex == MediaCodec.INFO_OUTPUT_FORMAT_CHANGED) {
            if (mVideoTrack != -1) {
                Log.e(TAG, "Video format changed, but track already initialized");
                return STATUS_ERROR_DRAIN_VIDEO_TRACK_ALREADY_INITIALIZED;
            }

            MediaFormat outputFormat;
            try {
                outputFormat = mVideoEncoder.getOutputFormat();
            } catch (Exception e) {
                Log.e(TAG, "Error getting video encoder output format", e);
                return STATUS_ERROR_DRAIN_VIDEO_ENCODER_GET_OUTPUT_FORMAT;
            }
            try {
                mVideoTrack = mMuxer.addTrack(outputFormat);
            } catch (Exception e) {
                Log.e(TAG, "Error adding video muxer track", e);
                return STATUS_ERROR_DRAIN_VIDEO_MUXER_ADD_TRACK;
            }

            if (!tryStartMuxer()) {
                return STATUS_ERROR_DRAIN_VIDEO_MUXER_START;
            }

        } else if (outputBufferIndex >= 0) {
            ByteBuffer outputBuffer;
            try {
                outputBuffer = mVideoEncoder.getOutputBuffer(outputBufferIndex);
            } catch (Exception e) {
                Log.e(TAG, "Error getting video encoder output buffer " + outputBufferIndex, e);
                return STATUS_ERROR_DRAIN_VIDEO_ENCODER_GET_OUTPUT_BUFFER;
            }
            if (outputBuffer == null) {
                Log.e(TAG, "Null video encoder output buffer " + outputBufferIndex);
                return STATUS_ERROR_DRAIN_VIDEO_ENCODER_NULL_OUTPUT_BUFFER;
            }

            if ((flags & MediaCodec.BUFFER_FLAG_CODEC_CONFIG) == 0 && mVideoBufferInfo.size != 0) {
                if (mVideoTrack == -1) {
                    Log.e(TAG,
                            "Got video output buffer " + outputBufferIndex + " with size " + mVideoBufferInfo.size + " while track is not initialized");
                    return STATUS_ERROR_DRAIN_VIDEO_TRACK_NOT_INITIALIZED;
                }

                int position = mVideoBufferInfo.offset;
                try {
                    outputBuffer.position(position);
                } catch (Exception e) {
                    Log.e(TAG,
                            "Error setting video output buffer " + outputBufferIndex + " position " + position, e);
                    return STATUS_ERROR_DRAIN_VIDEO_OUTPUT_BUFFER_POSITION;
                }

                int limit = position + mVideoBufferInfo.size;
                try {
                    outputBuffer.limit(limit);
                } catch (Exception e) {
                    Log.e(TAG,
                            "Error setting video output buffer " + outputBufferIndex + " limit " + limit, e);
                    return STATUS_ERROR_DRAIN_VIDEO_OUTPUT_BUFFER_LIMIT;
                }

                if (mMuxerStarted) {
                    try {
                        mMuxer.writeSampleData(mVideoTrack, outputBuffer, mVideoBufferInfo);
                    } catch (Exception e) {
                        Log.e(TAG,
                                "Error writing video sample data from output buffer " + outputBufferIndex + " to track " + mVideoTrack, e);
                        return STATUS_ERROR_DRAIN_VIDEO_MUXER_WRITE;
                    }
                } else {
                    try {
                        mBufferedSamples.add(new BufferedSample(outputBuffer, mVideoBufferInfo, false));
                    } catch (Exception e) {
                        Log.e(TAG, "Error buffering video sample data from output buffer " + outputBufferIndex, e);
                        return STATUS_ERROR_DRAIN_VIDEO_MUXER_BUFFER;
                    }
                    mVideoBufferInfo = null;
                }
            }

            try {
                mVideoEncoder.releaseOutputBuffer(outputBufferIndex, false);
            } catch (Exception e) {
                Log.e(TAG, "Error releasing video encoder output buffer " + outputBufferIndex, e);
                return STATUS_ERROR_DRAIN_VIDEO_ENCODER_RELEASE_OUTPUT_BUFFER;
            }

            if ((flags & MediaCodec.BUFFER_FLAG_END_OF_STREAM) != 0) {
                mVideoStreamEnded = true;
                return STATUS_END_OF_STREAM;
            }

        } else {
            Log.d(TAG, "Unhandled video output buffer index value " + outputBufferIndex);
        }

        return STATUS_OK;
    }

    public int drainAudio(long timeoutUs) {
        if (mAudioStreamEnded) {
            return STATUS_END_OF_STREAM;
        }

        if (mAudioBufferInfo == null) {
            mAudioBufferInfo = new MediaCodec.BufferInfo();
        }

        int outputBufferIndex;
        int flags;
        try {
            outputBufferIndex = mAudioEncoder.dequeueOutputBuffer(mAudioBufferInfo, timeoutUs);
            flags = mAudioBufferInfo.flags;
        } catch (Exception e) {
            Log.e(TAG, "Error dequeuing audio output buffer", e);
            return STATUS_ERROR_DRAIN_AUDIO_DEQUEUE;
        }

        if (outputBufferIndex == MediaCodec.INFO_TRY_AGAIN_LATER) {
            return STATUS_TIMEOUT;

        } else if (outputBufferIndex == MediaCodec.INFO_OUTPUT_FORMAT_CHANGED) {
            if (mAudioTrack != -1) {
                Log.e(TAG, "Audio format changed, but track already initialized");
                return STATUS_ERROR_DRAIN_AUDIO_TRACK_ALREADY_INITIALIZED;
            }

            MediaFormat outputFormat;
            try {
                outputFormat = mAudioEncoder.getOutputFormat();
            } catch (Exception e) {
                Log.e(TAG, "Error getting audio encoder output format", e);
                return STATUS_ERROR_DRAIN_AUDIO_ENCODER_GET_OUTPUT_FORMAT;
            }
            try {
                mAudioTrack = mMuxer.addTrack(outputFormat);
            } catch (Exception e) {
                Log.e(TAG, "Error adding audio muxer track", e);
                return STATUS_ERROR_DRAIN_AUDIO_MUXER_ADD_TRACK;
            }

            if (!tryStartMuxer()) {
                return STATUS_ERROR_DRAIN_AUDIO_MUXER_START;
            }

        } else if (outputBufferIndex >= 0) {
            ByteBuffer outputBuffer;
            try {
                outputBuffer = mAudioEncoder.getOutputBuffer(outputBufferIndex);
            } catch (Exception e) {
                Log.e(TAG, "Error getting audio encoder output buffer " + outputBufferIndex, e);
                return STATUS_ERROR_DRAIN_AUDIO_ENCODER_GET_OUTPUT_BUFFER;
            }
            if (outputBuffer == null) {
                Log.e(TAG, "Null audio encoder output buffer " + outputBufferIndex);
                return STATUS_ERROR_DRAIN_AUDIO_ENCODER_NULL_OUTPUT_BUFFER;
            }

            if ((flags & MediaCodec.BUFFER_FLAG_CODEC_CONFIG) == 0 && mAudioBufferInfo.size != 0) {
                if (mAudioTrack == -1) {
                    Log.e(TAG,
                            "Got audio output buffer " + outputBufferIndex + " with size " + mAudioBufferInfo.size + " while track is not initialized");
                    return STATUS_ERROR_DRAIN_AUDIO_TRACK_NOT_INITIALIZED;
                }

                int position = mAudioBufferInfo.offset;
                try {
                    outputBuffer.position(position);
                } catch (Exception e) {
                    Log.e(TAG,
                            "Error setting audio output buffer " + outputBufferIndex + " position " + position, e);
                    return STATUS_ERROR_DRAIN_AUDIO_OUTPUT_BUFFER_POSITION;
                }

                int limit = position + mAudioBufferInfo.size;
                try {
                    outputBuffer.limit(limit);
                } catch (Exception e) {
                    Log.e(TAG,
                            "Error setting audio output buffer " + outputBufferIndex + " limit " + limit, e);
                    return STATUS_ERROR_DRAIN_AUDIO_OUTPUT_BUFFER_LIMIT;
                }

                if (mMuxerStarted) {
                    try {
                        mMuxer.writeSampleData(mAudioTrack, outputBuffer, mAudioBufferInfo);
                    } catch (Exception e) {
                        Log.e(TAG,
                                "Error writing audio sample data from output buffer " + outputBufferIndex + " to track " + mAudioTrack, e);
                        return STATUS_ERROR_DRAIN_AUDIO_MUXER_WRITE;
                    }
                } else {
                    try {
                        mBufferedSamples.add(new BufferedSample(outputBuffer, mAudioBufferInfo, true));
                    } catch (Exception e) {
                        Log.e(TAG, "Error buffering audio sample data from output buffer " + outputBufferIndex, e);
                        return STATUS_ERROR_DRAIN_AUDIO_MUXER_BUFFER;
                    }
                    mAudioBufferInfo = null;
                }
            }

            try {
                mAudioEncoder.releaseOutputBuffer(outputBufferIndex, false);
            } catch (Exception e) {
                Log.e(TAG, "Error releasing audio encoder output buffer " + outputBufferIndex, e);
                return STATUS_ERROR_DRAIN_AUDIO_ENCODER_RELEASE_OUTPUT_BUFFER;
            }

            if ((flags & MediaCodec.BUFFER_FLAG_END_OF_STREAM) != 0) {
                mAudioStreamEnded = true;
                return STATUS_END_OF_STREAM;
            }

        } else {
            Log.d(TAG, "Unhandled audio output buffer index value " + outputBufferIndex);
        }

        return STATUS_OK;
    }

    private boolean tryStartMuxer() {
        if (mVideoTrack != -1 && (mAudioEncoder == null || mAudioTrack != -1)) {
            try {
                mMuxer.start();
            } catch (Exception e) {
                Log.e(TAG, "Error starting muxer", e);
                return false;
            }
            mMuxerStarted = true;

            for (BufferedSample bs : mBufferedSamples) {
                String title = bs.audio ? "audio" : "video";
                int track = bs.audio ? mAudioTrack : mVideoTrack;

                ByteBuffer buf;
                try {
                    buf = ByteBuffer.wrap(bs.data);
                } catch (Exception e) {
                    Log.e(TAG, "Error wrapping buffered " + title + " sample", e);
                    return false;
                }

                try {
                    mMuxer.writeSampleData(track, buf, bs.bufferInfo);
                } catch (Exception e) {
                    Log.e(TAG, "Error writing buffered " + title + " sample data to track " + track, e);
                    return false;
                }
            }
            mBufferedSamples.clear();
        }
        return true;
    }

    public int finishVideo() {
        if (!mVideoFinished) {
            try {
                mVideoEncoder.queueInputBuffer(mVideoInputBufferIndex, 0, 0,
                        getVideoPresentationTimeUs(), MediaCodec.BUFFER_FLAG_END_OF_STREAM);
            } catch (Exception e) {
                Log.e(TAG, "Error committing video end of stream frame " + mVideoFrameIndex, e);
                return STATUS_ERROR_FINISH_QUEUE;
            }
            mVideoFinished = true;
        }
        return STATUS_OK;
    }

    public int finishAudio() {
        if (mAudioEncoder != null && !mAudioFinished) {
            try {
                mAudioEncoder.queueInputBuffer(mAudioInputBufferIndex, 0, 0,
                        getAudioPresentationTimeUs(), MediaCodec.BUFFER_FLAG_END_OF_STREAM);
            } catch (Exception e) {
                Log.e(TAG, "Error committing audio end of stream at " + mAudioSamplesWritten, e);
                return STATUS_ERROR_FINISH_QUEUE;
            }
            mAudioFinished = true;
        }
        return STATUS_OK;
    }

    public int close() {
        if (!tearDown()) {
            return STATUS_ERROR_CLOSE_TEAR_DOWN;
        }

        if (mUsesTempPath) {
            return STATUS_NEEDS_TEMP_TO_OUTPUT_COPY;
        } else {
            return STATUS_OK;
        }
    }

    public void cancel() {
        tearDown();
    }

    private boolean tearDown() {
        boolean ok = true;

        if (mAudioEncoder != null) {
            try {
                mAudioEncoder.stop();
            } catch (Exception e) {
                Log.e(TAG, "Error stopping audio encoder", e);
                ok = false;
            }
            try {
                mAudioEncoder.release();
            } catch (Exception e) {
                Log.e(TAG, "Error releasing audio encoder", e);
                ok = false;
            }
            mAudioEncoder = null;
        }

        if (mVideoEncoder != null) {
            try {
                mVideoEncoder.stop();
            } catch (Exception e) {
                Log.e(TAG, "Error stopping video encoder", e);
                ok = false;
            }
            try {
                mVideoEncoder.release();
            } catch (Exception e) {
                Log.e(TAG, "Error releasing video encoder", e);
                ok = false;
            }
            mVideoEncoder = null;
        }

        if (mMuxer != null) {
            if (mMuxerStarted) {
                mMuxerStarted = false;
                try {
                    mMuxer.stop();
                } catch (Exception e) {
                    Log.e(TAG, "Error stopping muxer", e);
                    ok = false;
                }
            }
            try {
                mMuxer.release();
            } catch (Exception e) {
                Log.e(TAG, "Error releasing muxer", e);
                ok = false;
            }
            mMuxer = null;
        }

        return ok;
    }

    private MediaFormat initializeVideoFormat(String videoMimeType) throws Exception {
        MediaFormat videoFormat = MediaFormat.createVideoFormat(videoMimeType, mWidth, mHeight);
        videoFormat.setInteger(MediaFormat.KEY_COLOR_FORMAT,
                MediaCodecInfo.CodecCapabilities.COLOR_FormatYUV420Flexible);
        videoFormat.setInteger(MediaFormat.KEY_I_FRAME_INTERVAL, 2);
        videoFormat.setFloat(MediaFormat.KEY_FRAME_RATE, mFramerate);
        videoFormat.setInteger(MediaFormat.KEY_BIT_RATE, mVideoBitrate);
        switch (mFormat) {
            case FORMAT_MP4_H264:
            case FORMAT_WEBM_VP8:
                break;
            case FORMAT_MP4_AV1:
                videoFormat.setInteger(MediaFormat.KEY_BITRATE_MODE,
                        MediaCodecInfo.EncoderCapabilities.BITRATE_MODE_VBR);
                if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.Q) {
                    // Should be the default, but let's set it if supported.
                    videoFormat.setInteger(MediaFormat.KEY_PROFILE,
                            MediaCodecInfo.CodecProfileLevel.AV1ProfileMain8);
                }
                break;
            default:
                throw new RuntimeException("Unhandled format " + mFormat);
        }
        return videoFormat;
    }

    private MediaFormat initializeAudioFormat(String audioMimeType) throws Exception {
        int pcmEncoding = getPcmEncodingForAudioFormatName(mAudioFormatName);
        // Note that this doesn't necessarily actually result in a track with
        // the given parameters! See the notes in the C++ code about only using
        // known good parameters to actually get something useful here.
        MediaFormat audioFormat =
                MediaFormat.createAudioFormat(audioMimeType, mAudioSampleRate, mAudioChannelCount);
        audioFormat.setInteger(MediaFormat.KEY_PCM_ENCODING, pcmEncoding);
        audioFormat.setInteger(MediaFormat.KEY_BIT_RATE, mAudioBitrate);
        if (audioMimeType.equals(MediaFormat.MIMETYPE_AUDIO_AAC)) {
            audioFormat.setInteger(
                    MediaFormat.KEY_AAC_PROFILE, MediaCodecInfo.CodecProfileLevel.AACObjectLC);
        }
        return audioFormat;
    }

    private static int getPcmEncodingForAudioFormatName(String name) {
        int pcmEncoding;
        if ("s16".equals(name)) {
            pcmEncoding = AudioFormat.ENCODING_PCM_16BIT;
        } else if ("float".equals(name) || "f32le".equals(name)) {
            pcmEncoding = AudioFormat.ENCODING_PCM_FLOAT;
        } else if ("u8".equals(name)) {
            pcmEncoding = AudioFormat.ENCODING_PCM_8BIT;
        } else if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.S && ("s32".equals(name) || "s32le".equals(name))) {
            pcmEncoding = AudioFormat.ENCODING_PCM_32BIT;
        } else {
            throw new IllegalArgumentException("Unhandled audio format " + name);
        }
        return pcmEncoding;
    }

    private MediaCodec initializeEncoder(String encoderName, String mimeType, MediaFormat format) throws Exception {
        MediaCodec encoder = createEncoder(encoderName, mimeType);
        encoder.configure(format, null, null, MediaCodec.CONFIGURE_FLAG_ENCODE);
        encoder.start();
        return encoder;
    }

    private MediaCodec createEncoder(String encoderName, String videoMimeType) throws Exception {
        if (encoderName != null && !encoderName.isEmpty()) {
            try {
                return MediaCodec.createByCodecName(encoderName);
            } catch (Exception e) {
                Log.w(TAG, "Failed to create encoder " + encoderName, e);
            }
        }
        return MediaCodec.createEncoderByType(videoMimeType);
    }

    private MediaMuxer initializeMuxer(Context context) throws Exception {
        int outputFormat;
        switch (mFormat) {
            case FORMAT_MP4_H264:
            case FORMAT_MP4_AV1:
                outputFormat = MediaMuxer.OutputFormat.MUXER_OUTPUT_MPEG_4;
                break;
            case FORMAT_WEBM_VP8:
                outputFormat = MediaMuxer.OutputFormat.MUXER_OUTPUT_WEBM;
                break;
            default:
                throw new RuntimeException("Muxer: unhandled format " + mFormat);
        }

        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O && mOutputPath.contains("://")) {
            try {
                Uri uri = Uri.parse(mOutputPath);
                ContentResolver contentResolver = context.getContentResolver();
                try (ParcelFileDescriptor pfd = contentResolver.openFileDescriptor(uri, "rwt")) {
                    if (pfd == null) {
                        Log.w(TAG, "Could not open parcel file descriptor");
                    } else {
                        return new MediaMuxer(pfd.getFileDescriptor(), outputFormat);
                    }
                }
            } catch (Exception e) {
                Log.w(TAG, "Failed to parse output path", e);
            }
        }

        mUsesTempPath = true;
        return new MediaMuxer(mTempPath, outputFormat);
    }

    private long getVideoPresentationTimeUs() {
        return Math.round(((double) mVideoFrameIndex) * mFrameDurationUs);
    }

    private long getAudioPresentationTimeUs() {
        return (mAudioSamplesWritten * 1000000L) / mAudioSampleRate;
    }

    public static List<Support> getSupportsForVideoFormat(int format) {
        String mimeType = getFormatVideoMimeType(format);
        if (mimeType == null) {
            return Collections.emptyList();
        } else {
            return getSupportsForMimeType(mimeType);
        }
    }

    public static List<Support> getSupportsForAudioFormat(int format) {
        String mimeType = getFormatAudioMimeType(format);
        if (mimeType == null) {
            return Collections.emptyList();
        } else {
            return getSupportsForMimeType(mimeType);
        }
    }

    private static List<Support> getSupportsForMimeType(String mimeType) {
        List<Support> supports = new ArrayList<>();

        MediaCodecInfo[] codecInfos;
        try {
            MediaCodecList codecList = new MediaCodecList(MediaCodecList.REGULAR_CODECS);
            codecInfos = codecList.getCodecInfos();
        } catch (Exception e) {
            Log.e(TAG, "Error getting available codecs for " + mimeType, e);
            return supports;
        }

        if (codecInfos != null) {
            for (MediaCodecInfo codecInfo : codecInfos) {
                try {
                    if (codecInfo.isEncoder()) {
                        for (String type : codecInfo.getSupportedTypes()) {
                            if (mimeType.equalsIgnoreCase(type)) {
                                supports.add(new Support(codecInfo.getName(),
                                        !looksLikeSoftwareEncoder(codecInfo)));
                            }
                        }
                    }
                } catch (Exception e) {
                    Log.e(TAG, "Error checking codec for " + mimeType, e);
                }
            }
        }

        return supports;
    }

    private static boolean looksLikeSoftwareEncoder(MediaCodecInfo codecInfo) {
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.Q) {
            try {
                if (codecInfo.isSoftwareOnly()) {
                    return true;
                }
            } catch (Exception e) {
                Log.w(TAG, "Failed to check software-only for codec " + codecInfo.getName());
            }
        }
        // These are the built-in Android software encoders.
        String lowerName = codecInfo.getName().toLowerCase();
        return lowerName.startsWith("omx.google.") || lowerName.startsWith("c2.android.");
    }

    private static String getFormatVideoMimeType(int format) {
        switch (format) {
            case FORMAT_MP4_H264:
                return MediaFormat.MIMETYPE_VIDEO_AVC;
            case FORMAT_WEBM_VP8:
                return MediaFormat.MIMETYPE_VIDEO_VP8;
            case FORMAT_MP4_AV1:
                if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.Q) {
                    return MediaFormat.MIMETYPE_VIDEO_AV1;
                } else {
                    return null;
                }
            default:
                return null;
        }
    }

    private static String getFormatAudioMimeType(int format) {
        switch (format) {
            case FORMAT_MP4_H264:
            case FORMAT_MP4_AV1:
                return MediaFormat.MIMETYPE_AUDIO_AAC;
            case FORMAT_WEBM_VP8:
                return MediaFormat.MIMETYPE_AUDIO_OPUS;
            default:
                return null;
        }
    }
}
