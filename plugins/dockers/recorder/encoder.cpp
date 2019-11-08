#include "encoder.h"
#include "ivfenc.h"
#include <vpx/vp8cx.h>
#include <QDebug>

void Encoder::init(const char* filename, int width, int height)
{
    m_width = width;
    m_height = height;

    qDebug() << "width " << m_width << " height " << m_height;

    if (!vpx_img_alloc(&m_raw, VPX_IMG_FMT_I420, m_width, m_height, 1)) {
        qDebug() << "Failed to allocate image.";
        return;
    }

    vpx_codec_enc_config_default(vpx_codec_vp9_cx(), &m_cfg, 0);

    m_cfg.g_w = m_width;
    m_cfg.g_h = m_height;
    m_cfg.g_timebase.num = 1;
    m_cfg.g_timebase.den = 4;
    // m_cfg.rc_target_bitrate = 500;

    if (m_res = vpx_codec_enc_init(&m_codec, vpx_codec_vp9_cx(), &m_cfg, 0)) {
        qDebug() << "Failed to initialize encoder." << m_res;
    }

    if (vpx_codec_control_(&m_codec, VP9E_SET_LOSSLESS, 1)) {
        qDebug() << "Failed to set lossless." << m_res;
    }
    m_frameCount = 0;

    m_file = fopen(filename, "wb");

    ivf_write_file_header(m_file, m_width, m_height, 1, 4, 0x30395056, 0);
}

static int vpx_img_plane_width(const vpx_image_t* img, int plane)
{
    if (plane > 0 && img->x_chroma_shift > 0)
        return (img->d_w + 1) >> img->x_chroma_shift;
    else
        return img->d_w;
}

static int vpx_img_plane_height(const vpx_image_t* img, int plane)
{
    if (plane > 0 && img->y_chroma_shift > 0)
        return (img->d_h + 1) >> img->y_chroma_shift;
    else
        return img->d_h;
}

static int encode_frame(vpx_codec_ctx_t* codec, vpx_image_t* img, int frame_index, int flags, FILE* file)
{
    int got_pkts = 0;
    vpx_codec_iter_t iter = NULL;
    const vpx_codec_cx_pkt_t* pkt = NULL;
    const vpx_codec_err_t res = vpx_codec_encode(codec, img, frame_index, 1, flags, VPX_DL_GOOD_QUALITY);
    if (res != VPX_CODEC_OK) {
        qDebug() << "Failed to encode frame";
    }
    while ((pkt = vpx_codec_get_cx_data(codec, &iter)) != NULL) {
        got_pkts = 1;
        if (pkt->kind == VPX_CODEC_CX_FRAME_PKT) {
            ivf_write_frame_header(file, pkt->data.frame.pts, pkt->data.frame.sz);
            fwrite(pkt->data.frame.buf, pkt->data.frame.sz, 1, file);
        }
    }
    return got_pkts;
}

void Encoder::pushFrame(uint8_t* data[3], uint32_t size)
{
    qDebug() << "push frame";

    int flags = 0;
    if (m_keyframeInterval > 0 && m_frameCount % m_keyframeInterval == 0) {
        flags |= VPX_EFLAG_FORCE_KF;
    }

    int plane;

    for (plane = 0; plane < 3; ++plane) {
        unsigned char* buf = m_raw.planes[plane];
        const int stride = m_raw.stride[plane];
        const int w = vpx_img_plane_width(&m_raw, plane);
        const int h = vpx_img_plane_height(&m_raw, plane);
        qDebug() << "stride:" << stride << "w:" << w << "h:" << h;
        for (int y = 0; y < h; ++y) {
            std::copy(data[plane] + w * y, data[plane] + w * y + w, buf);
            buf += stride;
        }
    }
    encode_frame(&m_codec, &m_raw, m_frameCount, flags, m_file);

    m_frameCount++;
}

void Encoder::finish()
{
    qDebug() << "finishe called";
    while (encode_frame(&m_codec, NULL, -1, 0, m_file)) {
    }
    printf("\n");
    vpx_img_free(&m_raw);
    if (vpx_codec_destroy(&m_codec)) {
        qDebug() << "Failed to destroy codec.";
    }
    fseek(m_file, 0, SEEK_SET);
    ivf_write_file_header(m_file, m_width, m_height, 1, 4, 0x30395056, m_frameCount);
    m_frameCount = 0;
    fclose(m_file);
    m_file = nullptr;

    qDebug() << "finished";
}
