#ifndef ENCODER_H
#define ENCODER_H

#include <stdio.h>
#include <string.h>
#include <fstream>
#include <unistd.h>
#include <gst/gst.h>
#include <gst/app/gstappsrc.h>

class Encoder
{
    GstPipeline *m_pipeline;
	GstAppSrc  *m_src;
	GstElement *m_filter1;
	GstElement *m_encoder;
	GstElement *m_videoconvert;
    GstElement *m_queue;
	GstElement *m_webmmux;
	GstElement *m_sink;
    int m_width;
    int m_height;
    GstClockTime m_timestamp;

public:
    Encoder()
    :m_pipeline(nullptr),
    m_src(nullptr),
    m_filter1(nullptr),
    m_encoder(nullptr),
    m_videoconvert(nullptr),
    m_webmmux(nullptr),
    m_sink(nullptr)
    {}

    void init(const char *filename, int width, int height);

    void pushFrame( gpointer data, gsize size);

    void finish();
};

#endif