/*
 *  SPDX-FileCopyrightText: 2009 Boudewijn Rempt <boud@valdyas.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef PSD_RESOURCE_BLOCK_H
#define PSD_RESOURCE_BLOCK_H

class QIODevice;

#include <klocalizedstring.h>
#include <kis_debug.h>
#include <QString>
#include <QBuffer>

#include <kis_annotation.h>

#include "psd.h"
#include "psd_utils.h"
#include "psd_resource_section.h"

/**
 * @brief The PSDResourceInterpreter struct interprets the data in a psd resource block
 */
class PSDInterpretedResource
{
public:

    virtual ~PSDInterpretedResource() {}

    virtual bool interpretBlock(QByteArray /*data*/) { return true; }
    virtual bool createBlock(QByteArray & /*data*/) { return true; }
    virtual bool valid() { return true; }

    virtual QString displayText() { return QString(); }

    QString error;

protected:
    void startBlock(QBuffer &buf, PSDImageResourceSection::PSDResourceID id, quint32 size) {
        if (!buf.isOpen()) {
            buf.open(QBuffer::WriteOnly);
        }
        buf.write("8BIM", 4);
        psdwrite(&buf, (quint16)id);
        psdwrite(&buf, (quint16)0); // We simply never save out the name, for now
        psdwrite(&buf, (quint32)size);
    }
};

/**
 * Contains the unparsed contents of the image resource blocks
 */
class PSDResourceBlock : public KisAnnotation
{

public:
    PSDResourceBlock();

    ~PSDResourceBlock() override
    {
        delete resource;
    }

    KisAnnotation* clone() const Q_DECL_OVERRIDE {
        // HACK ALERT: we are evil! use normal copying instead!

        PSDResourceBlock *copied = new PSDResourceBlock();

        QBuffer buffer;
        write(&buffer);
        copied->read(&buffer);

        return copied;
    }

    QString displayText() const override {
        if (resource) {
            return resource->displayText();
        }
        return i18n("Unparsed Resource Block");
    }

    bool read(QIODevice* io);
    bool write(QIODevice* io) const;
    bool valid();

    quint16     identifier;
    QString     name;
    quint32     dataSize;
    QByteArray  data;

    PSDInterpretedResource *resource;

    mutable QString error;
};


/* 0x03e9 - Optional - Mac print manager print info record */
struct MAC_PRINT_INFO_1001 : public PSDInterpretedResource
{
    bool interpretBlock(QByteArray /*data*/) override
    {
        dbgFile << "Reading MAC_PRINT_INFO_1001";
        return true;
    }

};

/* 0x03ed - ResolutionInfo structure */
struct RESN_INFO_1005 : public PSDInterpretedResource
{
    // XXX: Krita only uses INCH internally
    enum PSDUnit {
        PSD_UNIT_INCH         = 1,            /* inches */
        PSD_UNIT_CM           = 2,            /* cm */
        PSD_UNIT_POINT        = 3,            /* points  (72 points =   1 inch) */
        PSD_UNIT_PICA         = 4,            /* pica    ( 6 pica   =   1 inch) */
        PSD_UNIT_COLUMN       = 5            /* columns ( column defined in ps prefs, default = 2.5 inches) */
    };

    RESN_INFO_1005()
        : hRes(300)
        , hResUnit(PSD_UNIT_INCH)
        , widthUnit(PSD_UNIT_INCH)
        , vRes(300)
        , vResUnit(PSD_UNIT_INCH)
        , heightUnit(PSD_UNIT_INCH)
    {}

    bool interpretBlock(QByteArray data) override;
    bool createBlock(QByteArray & data) override;

    Fixed   hRes;
    quint16 hResUnit;
    quint16 widthUnit;
    Fixed   vRes;
    quint16 vResUnit;
    quint16 heightUnit;

};


/* 0x03ee - Alpha channel names */
struct ALPHA_NAMES_1006 : public PSDInterpretedResource
{
    bool interpretBlock(QByteArray /*data*/) override
    {
        dbgFile << "Reading ALPHA_NAMES_1006";

        return true;
    }
};


/* 0x03ef - DisplayInfo structure */
struct DISPLAY_INFO_1007 : public PSDInterpretedResource
{
    bool interpretBlock(QByteArray /*data*/) override
    {
        dbgFile << "Reading DISPLAY_INFO_1007";
        return true;
    }
};


/* 0x03f0 - Optional - Caption string */
struct CAPTION_1008 : public PSDInterpretedResource
{
    bool interpretBlock(QByteArray /*data*/) override
    {
        dbgFile << "Reading CAPTION_1008";
        return true;
    }
};


/* 0x03f1 - Border info */
struct BORDER_INFO_1009 : public PSDInterpretedResource
{
    bool interpretBlock(QByteArray /*data*/) override
    {
        dbgFile << "Reading BORDER_INFO_1009";
        return true;
    }

};


/* 0x03f2 - Background colour */
struct BACKGROUND_COL_1010 : public PSDInterpretedResource
{
    bool interpretBlock(QByteArray /*data*/) override
    {
        dbgFile << "Reading BACKGROUND_COL_1010";
        return true;
    }
};


/* 0x03f3 - Print flags */
struct PRINT_FLAGS_1011 : public PSDInterpretedResource
{
    bool interpretBlock(QByteArray /*data*/) override
    {
        dbgFile << "Reading PRINT_FLAGS_1011";
        return true;
    }
};


/* 0x03f4 - Greyscale and multichannel halftoning info */
struct GREY_HALFTONE_1012 : public PSDInterpretedResource
{
    bool interpretBlock(QByteArray /*data*/) override
    {
        dbgFile << "Reading GREY_HALFTONE_1012";
        return true;
    }
};


/* 0x03f5 - Colour halftoning info */
struct COLOR_HALFTONE_1013 : public PSDInterpretedResource
{
    bool interpretBlock(QByteArray /*data*/) override
    {
        dbgFile << "Reading COLOR_HALFTONE_1013";
        return true;
    }
};


/* 0x03f6 - Duotone halftoning info */
struct DUOTONE_HALFTONE_1014 : public PSDInterpretedResource
{
    bool interpretBlock(QByteArray /*data*/) override
    {
        dbgFile << "Reading DUOTONE_HALFTONE_1014";
        return true;
    }
};


/* 0x03f7 - Greyscale and multichannel transfer functions */
struct GREY_XFER_1015 : public PSDInterpretedResource
{
    bool interpretBlock(QByteArray /*data*/) override
    {
        dbgFile << "Reading GREY_XFER_1015";
        return true;
    }
};


/* 0x03f8 - Colour transfer functions */
struct COLOR_XFER_1016 : public PSDInterpretedResource
{
    bool interpretBlock(QByteArray /*data*/) override
    {
        dbgFile << "Reading COLOR_XFER_1016";
        return true;
    }
};


/* 0x03f9 - Duotone transfer functions */
struct DUOTONE_XFER_1017 : public PSDInterpretedResource
{
    bool interpretBlock(QByteArray /*data*/) override
    {
        dbgFile << "Reading DUOTONE_XFER_1017";
        return true;
    }
};


/* 0x03fa - Duotone image information */
struct DUOTONE_INFO_1018 : public PSDInterpretedResource
{
    bool interpretBlock(QByteArray /*data*/) override
    {
        dbgFile << "Reading DUOTONE_INFO_1018";
        return true;
    }
};


/* 0x03fb - Effective black & white values for dot range */
struct EFFECTIVE_BW_1019 : public PSDInterpretedResource
{
    bool interpretBlock(QByteArray /*data*/) override
    {
        dbgFile << "Reading EFFECTIVE_BW_1019";
        return true;
    }
};


/* 0x03fd - EPS options */
struct EPS_OPT_1021 : public PSDInterpretedResource
{
    bool interpretBlock(QByteArray /*data*/) override
    {
        dbgFile << "Reading EPS_OPT_1021";
        return true;
    }
};


/* 0x03fe - Quick mask info */
struct QUICK_MASK_1022 : public PSDInterpretedResource
{    bool interpretBlock(QByteArray /*data*/) override
    {
        dbgFile << "Reading QUICK_MASK_1022";
        return true;
    }
};


/* 0x0400 - Layer state info */
struct LAYER_STATE_1024 : public PSDInterpretedResource
{
    bool interpretBlock(QByteArray /*data*/) override
    {
        dbgFile << "Reading LAYER_STATE_1024";
        return true;
    }
};


/* 0x0401 - Working path (not saved) */
struct WORKING_PATH_1025 : public PSDInterpretedResource
{
    bool interpretBlock(QByteArray /*data*/) override
{
    dbgFile << "Reading WORKING_PATH_1025";
    return true;
}
};


/* 0x0402 - Layers group info */
struct LAYER_GROUP_1026 : public PSDInterpretedResource
{
    bool interpretBlock(QByteArray /*data*/) override
    {
        dbgFile << "Reading LAYER_GROUP_1026";
        return true;
    }
};


/* 0x0404 - IPTC-NAA record (IMV4.pdf) */
struct IPTC_NAA_DATA_1028 : public PSDInterpretedResource
{
    bool interpretBlock(QByteArray /*data*/) override
    {
        dbgFile << "Reading IPTC_NAA_DATA_1028";
        return true;
    }
};


/* 0x0405 - Image mode for raw format files */
struct IMAGE_MODE_RAW_1029 : public PSDInterpretedResource
{
    bool interpretBlock(QByteArray /*data*/) override
    {
        dbgFile << "Reading IMAGE_MODE_RAW_1029";
        return true;
    }
};


/* 0x0406 - JPEG quality */
struct JPEG_QUAL_1030 : public PSDInterpretedResource
{
    bool interpretBlock(QByteArray /*data*/) override
    {
        dbgFile << "Reading JPEG_QUAL_1030";
        return true;
    }
};


/* 0x0408 - Grid & guide info */
struct GRID_GUIDE_1032 : public PSDInterpretedResource
{
    bool interpretBlock(QByteArray /*data*/) override
    {
        dbgFile << "Reading GRID_GUIDE_1032";
        return true;
    }
};


/* 0x0409 - Thumbnail resource */
struct THUMB_RES_1033 : public PSDInterpretedResource
{
    bool interpretBlock(QByteArray /*data*/) override
    {
        dbgFile << "Reading THUMB_RES_1033";
        return true;
    }
};


/* 0x040a - Copyright flag */
struct COPYRIGHT_FLG_1034 : public PSDInterpretedResource
{
    bool interpretBlock(QByteArray /*data*/) override
    {
        dbgFile << "Reading COPYRIGHT_FLG_1034";
        return true;
    }
};


/* 0x040b - URL string */
struct URL_1035 : public PSDInterpretedResource
{
    bool interpretBlock(QByteArray /*data*/) override
    {
        dbgFile << "Reading URL_1035";
        return true;
    }
};


/* 0x040c - Thumbnail resource */
struct THUMB_RES2_1036 : public PSDInterpretedResource
{
    bool interpretBlock(QByteArray /*data*/) override
    {
        dbgFile << "Reading THUMB_RES2_1036";
        return true;
    }
};


/* 0x040d - Global angle */
struct GLOBAL_ANGLE_1037 : public PSDInterpretedResource
{

    GLOBAL_ANGLE_1037()
        : angle(30)
    {}

    bool interpretBlock(QByteArray data) override
    {
        dbgFile << "Reading GLOBAL_ANGLE_1037";

        QDataStream ds(data);
        ds.setByteOrder(QDataStream::BigEndian);

        ds >> angle;

        return true;
    }

    bool createBlock(QByteArray & data) override
    {
        QBuffer buf(&data);
        startBlock(buf, PSDImageResourceSection::GLOBAL_ANGLE, 4);
        psdwrite(&buf, (quint32)angle);
        return true;
    }

    bool valid() override { return true; }

    QString displayText() override { return QString("Global Angle: %1").arg(angle); }

    qint32 angle;
};


/* 0x040e - Colour samplers resource */
struct COLOR_SAMPLER_1038 : public PSDInterpretedResource
{
    bool interpretBlock(QByteArray /*data*/) override
    {
        dbgFile << "Reading COLOR_SAMPLER_1038";
        return true;
    }
};


/* 0x040f - ICC Profile */
struct ICC_PROFILE_1039 : public PSDInterpretedResource
{
    bool interpretBlock(QByteArray data) override;
    bool createBlock(QByteArray & data) override;

    QByteArray icc;
};


/* 0x0410 - Watermark */
struct WATERMARK_1040 : public PSDInterpretedResource
{
    bool interpretBlock(QByteArray /*data*/) override
    {
        dbgFile << "Reading WATERMARK_1040";
        return true;
    }
};


/* 0x0411 - Do not use ICC profile flag */
struct ICC_UNTAGGED_1041 : public PSDInterpretedResource
{
    bool interpretBlock(QByteArray /*data*/) override
    {
        dbgFile << "Reading ICC_UNTAGGED_1041";
        return true;
    }
};


/* 0x0412 - Show / hide all effects layers */
struct EFFECTS_VISIBLE_1042 : public PSDInterpretedResource
{
    bool interpretBlock(QByteArray /*data*/) override
    {
        dbgFile << "Reading EFFECTS_VISIBLE_1042";
        return true;
    }
};


/* 0x0413 - Spot halftone */
struct SPOT_HALFTONE_1043 : public PSDInterpretedResource
{
    bool interpretBlock(QByteArray /*data*/) override
    {
        dbgFile << "Reading SPOT_HALFTONE_1043";
        return true;
    }
};


/* 0x0414 - Document specific IDs */
struct DOC_IDS_1044 : public PSDInterpretedResource
{
    bool interpretBlock(QByteArray /*data*/) override
    {
        dbgFile << "Reading DOC_IDS_1044";
        return true;
    }
};


/* 0x0415 - Unicode alpha names */
struct ALPHA_NAMES_UNI_1045 : public PSDInterpretedResource
{
    bool interpretBlock(QByteArray /*data*/) override
    {
        dbgFile << "Reading ALPHA_NAMES_UNI_1045";
        return true;
    }
};


/* 0x0416 - Indexed colour table count */
struct IDX_COL_TAB_CNT_1046 : public PSDInterpretedResource
{
    bool interpretBlock(QByteArray /*data*/) override
    {
        dbgFile << "Reading IDX_COL_TAB_CNT_1046";
        return true;
    }
};


/* 0x0417 - Index of transparent colour (if any) */
struct IDX_TRANSPARENT_1047 : public PSDInterpretedResource
{
    bool interpretBlock(QByteArray /*data*/) override
    {
        dbgFile << "Reading IDX_TRANSPARENT_1047";
        return true;
    }
};


/* 0x0419 - Global altitude */
struct GLOBAL_ALT_1049 : public PSDInterpretedResource
{
    GLOBAL_ALT_1049()
        : altitude(30)
    {}
    bool interpretBlock(QByteArray data) override
    {
        dbgFile << "Reading GLOBAL_ALT_1049";
        QDataStream ds(data);
        ds.setByteOrder(QDataStream::BigEndian);
        ds >> altitude;
        return true;
    }

    bool createBlock(QByteArray & data) override
    {
        QBuffer buf(&data);
        startBlock(buf, PSDImageResourceSection::GLOBAL_ALT, 4);
        psdwrite(&buf, (quint32)altitude);
        return true;
    }

    bool valid() override { return true; }

    QString displayText() override { return QString("Global Altitude: %1").arg(altitude); }

    qint32 altitude;
};


/* 0x041a - Slices */
struct SLICES_1050 : public PSDInterpretedResource
{
    bool interpretBlock(QByteArray /*data*/) override
    {
        dbgFile << "Reading SLICES_1050";
        return true;
    }
};


/* 0x041b - Workflow URL - Unicode string */
struct WORKFLOW_URL_UNI_1051 : public PSDInterpretedResource
{
    bool interpretBlock(QByteArray /*data*/) override
    {
        dbgFile << "Reading WORKFLOW_URL_UNI_1051";
        return true;
    }
};


/* 0x041c - Jump to XPEP (?) */
struct JUMP_TO_XPEP_1052 : public PSDInterpretedResource
{
    bool interpretBlock(QByteArray /*data*/) override
    {
        dbgFile << "JUMP_TO_XPEP_1052";
        return true;
    }
};


/* 0x041d - Alpha IDs */
struct ALPHA_ID_1053 : public PSDInterpretedResource
{
    bool interpretBlock(QByteArray /*data*/) override
    {
        dbgFile << "ALPHA_ID_1053";
        return true;
    }
};

/* 0x041e - URL list - unicode */
struct URL_LIST_UNI_1054 : public PSDInterpretedResource
{
    bool interpretBlock(QByteArray /*data*/) override
    {
        dbgFile << "URL_LIST_UNI_1054";
        return true;
    }
};

/* 0x0421 - Version info */
struct VERSION_INFO_1057 : public PSDInterpretedResource
{
    bool interpretBlock(QByteArray /*data*/) override
    {
        dbgFile << "VERSION_INFO_1057";
        return true;
    }
};


/* 0x0422 - Exif data block */
struct EXIF_DATA_1058 : public PSDInterpretedResource
{
    bool interpretBlock(QByteArray /*data*/) override
    {
        dbgFile << "Reading EXIF_DATA_1058";
        return true;
    }
};


/* 0x0424 - XMP data block */
struct XMP_DATA_1060 : public PSDInterpretedResource
{
    bool interpretBlock(QByteArray /*data*/) override
    {
        dbgFile << "Reading XMP_DATA_1060";
        return true;
    }
};


/* 0x07d0 - First path info block */
struct PATH_INFO_FIRST_2000 : public PSDInterpretedResource
{
    bool interpretBlock(QByteArray /*data*/) override
    {
        dbgFile << "PATH_INFO_FIRST_2000";
        return true;
    }
};


/* 0x0bb6 - Last path info block */
struct PATH_INFO_LAST_2998 : public PSDInterpretedResource
{
    bool interpretBlock(QByteArray /*data*/) override
    {
        dbgFile << "PATH_INFO_LAST_2998";
        return true;
    }
};


/* 0x0bb7 - Name of clipping path */
struct CLIPPING_PATH_2999 : public PSDInterpretedResource
{
    bool interpretBlock(QByteArray /*data*/) override
    {
        dbgFile << "Reading CLIPPING_PATH_2999";
        return true;
    }
};


/* 0x2710 - Print flags */
struct PRINT_FLAGS_2_10000 : public PSDInterpretedResource
{
    bool interpretBlock(QByteArray /*data*/) override
    {
        dbgFile << "Reading PRINT_FLAGS_2_10000";
        return true;
    }
};


#endif // PSD_RESOURCE_BLOCK_H
