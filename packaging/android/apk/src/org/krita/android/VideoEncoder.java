// SPDX-License-Identifier: GPL-3.0-or-later
package org.krita.android;

import android.content.ContentResolver;
import android.content.Context;
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
    private static final int STATUS_ERROR_START_UNKNOWN_FORMAT = 102;
    private static final int STATUS_ERROR_START_FORMAT = 103;
    private static final int STATUS_ERROR_START_ENCODER = 104;
    private static final int STATUS_ERROR_START_MUXER = 105;
    private static final int STATUS_ERROR_PREPARE_DEQUEUE = 201;
    private static final int STATUS_ERROR_PREPARE_GET_IMAGE = 202;
    private static final int STATUS_ERROR_PREPARE_NULL_IMAGE = 203;
    private static final int STATUS_ERROR_COMMIT_QUEUE = 301;
    private static final int STATUS_ERROR_DRAIN_DEQUEUE = 401;
    private static final int STATUS_ERROR_DRAIN_MUXER_ALREADY_STARTED = 402;
    private static final int STATUS_ERROR_DRAIN_ENCODER_GET_OUTPUT_FORMAT = 403;
    private static final int STATUS_ERROR_DRAIN_MUXER_ADD_TRACK = 404;
    private static final int STATUS_ERROR_DRAIN_MUXER_START = 405;
    private static final int STATUS_ERROR_DRAIN_ENCODER_GET_OUTPUT_BUFFER = 406;
    private static final int STATUS_ERROR_DRAIN_ENCODER_NULL_OUTPUT_BUFFER = 407;
    private static final int STATUS_ERROR_DRAIN_MUXER_NOT_STARTED = 408;
    private static final int STATUS_ERROR_DRAIN_ENCODER_RELEASE_OUTPUT_BUFFER = 409;
    private static final int STATUS_ERROR_DRAIN_OUTPUT_BUFFER_POSITION = 410;
    private static final int STATUS_ERROR_DRAIN_OUTPUT_BUFFER_LIMIT = 411;
    private static final int STATUS_ERROR_DRAIN_MUXER_WRITE = 412;
    private static final int STATUS_ERROR_DRAIN_MUXER_NEVER_STARTED = 413;
    private static final int STATUS_ERROR_FINISH_QUEUE = 501;
    private static final int STATUS_ERROR_CLOSE_TEAR_DOWN = 601;

    private final int mFormat;
    private final int mWidth;
    private final int mHeight;
    private final float mFramerate;
    private final double mFrameDurationUs;
    private final String mOutputPath;
    private final String mTempPath;
    private final String mEncoderName;
    private final int mBitrate;
    private final MediaCodec.BufferInfo mBufferInfo = new MediaCodec.BufferInfo();
    private MediaCodec mEncoder = null;
    private MediaMuxer mMuxer = null;
    private boolean mUsesTempPath = false;
    private boolean mMuxerStarted = false;
    private int mFrameIndex = 0;
    private int mVideoTrack = -1;
    private int mInputBufferIndex = -1;
    private Image mInputImage = null;

    public VideoEncoder(int format, int width, int height, float framerate, String outputPath,
                        String tempPath, String encoderName, int bitrate) {
        mFormat = format;
        mWidth = width;
        mHeight = height;
        mFramerate = framerate;
        mFrameDurationUs = 1000000.0 / ((double) framerate);
        mOutputPath = outputPath;
        mTempPath = tempPath;
        mEncoderName = encoderName;
        mBitrate = bitrate;
    }

    public int start(Context context) {
        if (mEncoder != null || mMuxer != null) {
            Log.e(TAG, "Attempting to start an already started video encoder");
            return STATUS_ERROR_START_ALREADY_STARTED;
        }

        String videoMimeType = getFormatMimeType(mFormat);
        if (videoMimeType == null) {
            return STATUS_ERROR_START_UNKNOWN_FORMAT;
        }

        MediaFormat videoFormat;
        try {
            videoFormat = initializeVideoFormat(videoMimeType);
        } catch (Exception e) {
            Log.e(TAG, "Error initializing video format", e);
            return STATUS_ERROR_START_FORMAT;
        }

        try {
            mEncoder = initializeEncoder(videoMimeType, videoFormat);
        } catch (Exception e) {
            Log.e(TAG, "Error initializing encoder", e);
            return STATUS_ERROR_START_ENCODER;
        }

        try {
            mMuxer = initializeMuxer(context);
        } catch (Exception e) {
            Log.e(TAG, "Error initializing muxer", e);
            return STATUS_ERROR_START_MUXER;
        }

        return STATUS_OK;
    }

    public int prepare(long timeoutUs) {
        int inputBufferIndex;
        try {
            inputBufferIndex = mEncoder.dequeueInputBuffer(timeoutUs);
        } catch (Exception e) {
            Log.e(TAG, "Error dequeuing input buffer", e);
            return STATUS_ERROR_PREPARE_DEQUEUE;
        }

        if (inputBufferIndex == MediaCodec.INFO_TRY_AGAIN_LATER) {
            return STATUS_TIMEOUT;
        }

        Image inputImage;
        try {
            inputImage = mEncoder.getInputImage(inputBufferIndex);
        } catch (Exception e) {
            Log.e(TAG, "Error getting input image " + inputBufferIndex, e);
            return STATUS_ERROR_PREPARE_GET_IMAGE;
        }
        if (inputImage == null) {
            Log.e(TAG, "No input image " + inputBufferIndex);
            return STATUS_ERROR_PREPARE_NULL_IMAGE;
        }

        mInputBufferIndex = inputBufferIndex;
        mInputImage = inputImage;
        return STATUS_OK;
    }

    public ByteBuffer getInputImagePlaneBuffer(int index) {
        try {
            Image.Plane[] planes = mInputImage.getPlanes();
            Image.Plane plane = planes[index];
            return plane.getBuffer();
        } catch (Exception e) {
            Log.e(TAG, "Error getting image plane buffer " + index, e);
            return null;
        }
    }

    public int getInputImagePlaneRowStride(int index) {
        try {
            Image.Plane[] planes = mInputImage.getPlanes();
            Image.Plane plane = planes[index];
            return plane.getRowStride();
        } catch (Exception e) {
            Log.e(TAG, "Error getting image plane row stride " + index, e);
            return -1;
        }
    }

    public int getInputImagePlanePixelStride(int index) {
        try {
            Image.Plane[] planes = mInputImage.getPlanes();
            Image.Plane plane = planes[index];
            return plane.getPixelStride();
        } catch (Exception e) {
            Log.e(TAG, "Error getting image plane pixel stride " + index, e);
            return -1;
        }
    }

    public int commit() {
        try {
            ByteBuffer inputBuffer = mEncoder.getInputBuffer(mInputBufferIndex);
            mEncoder.queueInputBuffer(mInputBufferIndex, 0, inputBuffer == null ? 0 :
                    inputBuffer.capacity(), getPresentationTimeUs(), 0);
        } catch (Exception e) {
            Log.e(TAG, "Error committing frame " + mFrameIndex, e);
            return STATUS_ERROR_COMMIT_QUEUE;
        }
        mInputBufferIndex = -1;
        mInputImage = null;
        ++mFrameIndex;
        return STATUS_OK;
    }

    public int drain(long timeoutUs) {
        int outputBufferIndex;
        try {
            outputBufferIndex = mEncoder.dequeueOutputBuffer(mBufferInfo, timeoutUs);
        } catch (Exception e) {
            Log.e(TAG, "Error dequeuing output buffer", e);
            return STATUS_ERROR_DRAIN_DEQUEUE;
        }

        if (outputBufferIndex == MediaCodec.INFO_TRY_AGAIN_LATER) {
            return STATUS_TIMEOUT;

        } else if (outputBufferIndex == MediaCodec.INFO_OUTPUT_FORMAT_CHANGED) {
            if (mMuxerStarted) {
                Log.e(TAG, "Format changed, but muxer already started");
                return STATUS_ERROR_DRAIN_MUXER_ALREADY_STARTED;
            }

            MediaFormat outputFormat;
            try {
                outputFormat = mEncoder.getOutputFormat();
            } catch (Exception e) {
                Log.e(TAG, "Error getting encoder output format", e);
                return STATUS_ERROR_DRAIN_ENCODER_GET_OUTPUT_FORMAT;
            }
            try {
                mVideoTrack = mMuxer.addTrack(outputFormat);
            } catch (Exception e) {
                Log.e(TAG, "Error adding muxer track", e);
                return STATUS_ERROR_DRAIN_MUXER_ADD_TRACK;
            }

            try {
                mMuxer.start();
            } catch (Exception e) {
                Log.e(TAG, "Error starting muxer", e);
                return STATUS_ERROR_DRAIN_MUXER_START;
            }
            mMuxerStarted = true;

        } else if (outputBufferIndex >= 0) {
            ByteBuffer outputBuffer;
            try {
                outputBuffer = mEncoder.getOutputBuffer(outputBufferIndex);
            } catch (Exception e) {
                Log.e(TAG, "Error getting encoder output buffer " + outputBufferIndex, e);
                return STATUS_ERROR_DRAIN_ENCODER_GET_OUTPUT_BUFFER;
            }
            if (outputBuffer == null) {
                Log.e(TAG, "Null encoder output buffer " + outputBufferIndex);
                return STATUS_ERROR_DRAIN_ENCODER_NULL_OUTPUT_BUFFER;
            }

            if ((mBufferInfo.flags & MediaCodec.BUFFER_FLAG_CODEC_CONFIG) == 0 && mBufferInfo.size != 0) {
                if (!mMuxerStarted) {
                    Log.e(TAG,
                            "Got output buffer " + outputBufferIndex + " with size " + mBufferInfo.size + " while muxer is not started");
                    return STATUS_ERROR_DRAIN_MUXER_NOT_STARTED;
                }

                int position = mBufferInfo.offset;
                try {
                    outputBuffer.position(position);
                } catch (Exception e) {
                    Log.e(TAG,
                            "Error setting output buffer " + outputBufferIndex + " position " + position, e);
                    return STATUS_ERROR_DRAIN_OUTPUT_BUFFER_POSITION;
                }

                int limit = position + mBufferInfo.size;
                try {
                    outputBuffer.limit(limit);
                } catch (Exception e) {
                    Log.e(TAG,
                            "Error setting output buffer " + outputBufferIndex + " limit " + limit, e);
                    return STATUS_ERROR_DRAIN_OUTPUT_BUFFER_LIMIT;
                }

                try {
                    mMuxer.writeSampleData(mVideoTrack, outputBuffer, mBufferInfo);
                } catch (Exception e) {
                    Log.e(TAG,
                            "Error writing sample data from output buffer " + outputBufferIndex + " to track " + mVideoTrack, e);
                    return STATUS_ERROR_DRAIN_MUXER_WRITE;
                }
            }

            try {
                mEncoder.releaseOutputBuffer(outputBufferIndex, false);
            } catch (Exception e) {
                Log.e(TAG, "Error releasing encoder output buffer " + outputBufferIndex, e);
                return STATUS_ERROR_DRAIN_ENCODER_RELEASE_OUTPUT_BUFFER;
            }

            if ((mBufferInfo.flags & MediaCodec.BUFFER_FLAG_END_OF_STREAM) != 0) {
                if (mMuxerStarted) {
                    return STATUS_END_OF_STREAM;
                } else {
                    Log.e(TAG, "Got end of stream without the muxer ever starting");
                    return STATUS_ERROR_DRAIN_MUXER_NEVER_STARTED;
                }
            }

        } else {
            Log.d(TAG, "Unhandled output buffer index value " + outputBufferIndex);
        }

        return STATUS_OK;
    }

    public int finish() {
        try {
            mEncoder.queueInputBuffer(mInputBufferIndex, 0, 0, getPresentationTimeUs(),
                    MediaCodec.BUFFER_FLAG_END_OF_STREAM);
        } catch (Exception e) {
            Log.e(TAG, "Error committing end of stream frame " + mFrameIndex, e);
            return STATUS_ERROR_FINISH_QUEUE;
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

        if (mEncoder != null) {
            try {
                mEncoder.stop();
            } catch (Exception e) {
                Log.e(TAG, "Error stopping encoder", e);
                ok = false;
            }
            try {
                mEncoder.release();
            } catch (Exception e) {
                Log.e(TAG, "Error releasing encoder", e);
                ok = false;
            }
            mEncoder = null;
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
        videoFormat.setInteger(MediaFormat.KEY_BIT_RATE, mBitrate);
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

    private MediaCodec initializeEncoder(String videoMimeType, MediaFormat videoFormat) throws Exception {
        MediaCodec encoder = createEncoder(videoMimeType);
        encoder.configure(videoFormat, null, null, MediaCodec.CONFIGURE_FLAG_ENCODE);
        encoder.start();
        return encoder;
    }

    private MediaCodec createEncoder(String videoMimeType) throws Exception {
        if (mEncoderName != null && !mEncoderName.isEmpty()) {
            try {
                return MediaCodec.createByCodecName(mEncoderName);
            } catch (Exception e) {
                Log.w(TAG, "Failed to create encoder " + mEncoderName, e);
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

    private long getPresentationTimeUs() {
        return Math.round(((double) mFrameIndex) * mFrameDurationUs);
    }

    public static List<Support> getSupportsForFormat(int format) {
        String mimeType = getFormatMimeType(format);
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

    private static String getFormatMimeType(int format) {
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
}
