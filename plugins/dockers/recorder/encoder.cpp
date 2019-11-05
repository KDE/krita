#include "encoder.h"
#include <QDebug>

void Encoder::init(const char* filename, int width, int height)
{
	GstStateChangeReturn state_ret;
	gst_init(NULL, NULL);

    m_width = width;
    m_height = height;
	
	m_pipeline = (GstPipeline*)gst_pipeline_new("mypipeline");
	m_src    =   (GstAppSrc*)gst_element_factory_make("appsrc", "mysrc");
	m_filter1 =  gst_element_factory_make ("capsfilter", "myfilter1");
    m_videoconvert = gst_element_factory_make ("videoconvert", "vc");
	m_encoder =  gst_element_factory_make ("vp9enc", "my9enc");
    m_queue = gst_element_factory_make("queue", "qu");
	m_webmmux =    gst_element_factory_make("webmmux", "mymux");
	m_sink =     gst_element_factory_make ("filesink"  , NULL);
	m_timestamp = 0;
	if(	!m_pipeline || 
		!m_src      || !m_filter1 || 
		!m_encoder  || /*!m_videoconvert   ||*/ !m_webmmux    ||  /*!m_queue ||*/
		!m_sink    )  {
		printf("Error creating pipeline elements!\n");
		exit(2);
	}

	gst_bin_add_many(
		GST_BIN(m_pipeline), 
		(GstElement*)m_src,
		m_filter1,	
		m_videoconvert,	
		m_encoder,
        m_queue,
		m_webmmux,
		m_sink,
		NULL);

        qDebug() << "width " << m_width << " height " << m_height;

    g_object_set (m_src, "format", GST_FORMAT_TIME, NULL);
    g_object_set( m_src, "is-live", true, NULL);
	GstCaps *filtercaps1 = gst_caps_new_simple ("video/x-raw",
		"format", G_TYPE_STRING, "RGBA",
		"width", G_TYPE_INT, m_width,
		"height", G_TYPE_INT, m_height,
		"framerate", GST_TYPE_FRACTION, 4, 1,
		NULL);
	g_object_set (G_OBJECT (m_filter1), "caps", filtercaps1, NULL);
	g_object_set (G_OBJECT (m_sink), "location", filename, NULL);

	g_assert( gst_element_link_many(
		(GstElement*)m_src, 
		m_filter1,
        m_videoconvert,
		m_encoder,
        m_queue,
		m_webmmux,
		m_sink,
		NULL ) );

	state_ret = gst_element_set_state((GstElement*)m_pipeline, GST_STATE_PLAYING);
	g_assert(state_ret == GST_STATE_CHANGE_ASYNC);
}

void Encoder::pushFrame(gpointer data, gsize size)
{
    GstBuffer *buffer = gst_buffer_new_wrapped(data, size); //Actual databuffer
	GstFlowReturn ret; //Return value
	//Set frame timestamp
    GST_BUFFER_PTS      (buffer) = m_timestamp;
    GST_BUFFER_DTS      (buffer) = m_timestamp;
    GST_BUFFER_DURATION (buffer) = gst_util_uint64_scale_int ( GST_SECOND, 1,4);
    m_timestamp += GST_BUFFER_DURATION (buffer);
	ret = gst_app_src_push_buffer( m_src, buffer); //Push data into pipeline

	g_assert(ret ==  GST_FLOW_OK);
    qDebug() << "push frame";
}

void Encoder::finish()
{
    qDebug() << "finishe called";
    //Declare end of stream
	gst_app_src_end_of_stream (GST_APP_SRC (m_src));
    // Wait for EOS message
    GstBus *bus = gst_pipeline_get_bus(GST_PIPELINE(m_pipeline));
    gst_bus_poll(bus, GST_MESSAGE_EOS, GST_CLOCK_TIME_NONE);

    qDebug() << "finished";
}