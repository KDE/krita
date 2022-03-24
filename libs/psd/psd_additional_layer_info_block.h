/*
 *  SPDX-FileCopyrightText: 2014 Boudewijn Rempt <boud@valdyas.org>
 *  SPDX-FileCopyrightText: 2021 L. E. Segovia <amy@amyspark.me>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef PSD_ADDITIONAL_LAYER_INFO_BLOCK_H
#define PSD_ADDITIONAL_LAYER_INFO_BLOCK_H

#include "kritapsd_export.h"

#include <QBitArray>
#include <QByteArray>
#include <QDomDocument>
#include <QIODevice>
#include <QString>
#include <QVector>
#include <functional>

#include <kis_types.h>
#include <kis_global.h>

#include <kis_node.h>
#include <kis_paint_device.h>
#include <kis_generator_registry.h>
#include <KisGlobalResourcesInterface.h>
#include <KoStopGradient.h>
#include <KoSegmentGradient.h>
#include <psd.h>

#include <asl/kis_asl_xml_writer.h>

#include "psd_header.h"

// additional layer information

// LEVELS
// Level record
struct KRITAPSD_EXPORT psd_layer_level_record {
    quint16 input_floor {0}; // (0...253)
    quint16 input_ceiling {2}; // (2...255)
    quint16 output_floor {0}; // 255). Matched to input floor.
    quint16 output_ceiling {0}; // (0...255)
    float gamma {0.0}; // Short integer from 10...999 representing 0.1...9.99. Applied to all image data.
};

// Levels settings files are loaded and saved in the Levels dialog.
struct KRITAPSD_EXPORT psd_layer_levels {
    psd_layer_level_record record[29]; // 29 sets of level records, each level containing 5 qint8 integers
    // Photoshop CS (8.0) Additional information
    // At the end of the Version 2 file is the following information:
    quint16 extra_level_count {0}; // Count of total level record structures. Subtract the legacy number of level record structures, 29, to determine how many are
                               // remaining in the file for reading.
    psd_layer_level_record *extra_record {nullptr}; // Additianol level records according to count
    quint8 lookup_table[3][256];
};

// CURVES
// The following is the data for each curve specified by count above
struct KRITAPSD_EXPORT psd_layer_curves_data {
    quint16 channel_index {0}; // Before each curve is a channel index.
    quint16 point_count {0}; // Count of points in the curve (qint8 integer from 2...19)
    quint16 output_value[19]; // All coordinates have range 0 to 255
    quint16 input_value[19];
};

// Curves file format
struct KRITAPSD_EXPORT psd_layer_curves {
    quint16 curve_count {0}; // Count of curves in the file.
    psd_layer_curves_data *curve {nullptr};
    quint8 lookup_table[3][256];
};

// BRIGHTNESS AND CONTRAST
struct KRITAPSD_EXPORT psd_layer_brightness_contrast {
    qint8 brightness {0};
    qint8 contrast {0};
    qint8 mean_value {0}; // for brightness and contrast
    qint8 Lab_color {0};
    quint8 lookup_table[256];
};

// COLOR BALANCE
struct KRITAPSD_EXPORT psd_layer_color_balance {
    qint8 cyan_red[3]; // (-100...100). shadows, midtones, highlights
    qint8 magenta_green[3];
    qint8 yellow_blue[3];
    bool preserve_luminosity {false};
    quint8 lookup_table[3][256];
};

// HUE/SATURATION
// Hue/Saturation settings files are loaded and saved in Photoshop¡¯s Hue/Saturation dialog
struct KRITAPSD_EXPORT psd_layer_hue_saturation {
    quint8 hue_or_colorization {0}; // 0 = Use settings for hue-adjustment; 1 = Use settings for colorization.
    qint8 colorization_hue {0}; // Photoshop 5.0: The actual values are stored for the new version. Hue is - 180...180, Saturation is 0...100, and Lightness is
                            // -100...100.
    qint8 colorization_saturation {0}; // Photoshop 4.0: Three qint8 integers Hue, Saturation, and Lightness from ¨C100...100.
    qint8 colorization_lightness {0}; // The user interface represents hue as ¨C180...180, saturation as 0...100, and Lightness as -100...1000, as the traditional
                                  // HSB color wheel, with red = 0.
    qint8 master_hue {0}; // Master hue, saturation and lightness values.
    qint8 master_saturation {0};
    qint8 master_lightness {0};
    qint8 range_values[6][4]; // For RGB and CMYK, those values apply to each of the six hextants in the HSB color wheel: those image pixels nearest to red,
                              // yellow, green, cyan, blue, or magenta. These numbers appear in the user interface from ¨C60...60, however the slider will
                              // reflect each of the possible 201 values from ¨C100...100.
    qint8 setting_values[6][3]; // For Lab, the first four of the six values are applied to image pixels in the four Lab color quadrants, yellow, green, blue,
                                // and magenta. The other two values are ignored ( = 0). The values appear in the user interface from ¨C90 to 90.
    quint8 lookup_table[6][360];
};

// SELECTIVE COLOR
// Selective Color settings files are loaded and saved in Photoshop¡¯s Selective Color dialog.
struct KRITAPSD_EXPORT psd_layer_selective_color {
    quint16 correction_method {0}; // 0 = Apply color correction in relative mode; 1 = Apply color correction in absolute mode.
    qint8 cyan_correction[10]; // Amount of cyan correction. Short integer from ¨C100...100.
    qint8 magenta_correction[10]; // Amount of magenta correction. Short integer from ¨C100...100.
    qint8 yellow_correction[10]; // Amount of yellow correction. Short integer from ¨C100...100.
    qint8 black_correction[10]; // Amount of black correction. Short integer from ¨C100...100.
};

// THRESHOLD
struct KRITAPSD_EXPORT psd_layer_threshold {
    quint16 level {1}; // (1...255)
};

// INVERT
// no parameter

// POSTERIZE
struct KRITAPSD_EXPORT psd_layer_posterize {
    quint16 levels {2}; // (2...255)
    quint8 lookup_table[256];
};

// CHANNEL MIXER
struct KRITAPSD_EXPORT psd_layer_channel_mixer {
    bool monochrome {false};
    qint8 red_cyan[4]; // RGB or CMYK color plus constant for the mixer settings. 4 * 2 bytes of color with 2 bytes of constant.
    qint8 green_magenta[4]; // (-200...200)
    qint8 blue_yellow[4];
    qint8 black[4];
    qint8 constant[4];
};

// PHOTO FILTER
struct KRITAPSD_EXPORT psd_layer_photo_filter {
    qint32 x_color {0}; // 4 bytes each for XYZ color
    qint32 y_color {0};
    qint32 z_color {0};
    qint32 density {0}; // (1...100)
    bool preserve_luminosity {true};
};

#include <kis_psd_layer_style.h>

struct KRITAPSD_EXPORT psd_layer_solid_color {
    KoColor fill_color;
    const KoColorSpace *cs {nullptr};

    // Used by ASL callback;
    void setColor(const KoColor &color) {
        fill_color = color;
        if (fill_color.colorSpace()->colorModelId() == cs->colorModelId()) {
            fill_color.setProfile(cs->profile());
        }
    }
    QDomDocument getFillLayerConfig() {
        KisFilterConfigurationSP cfg;
        cfg = KisGeneratorRegistry::instance()->value("color")->defaultConfiguration(KisGlobalResourcesInterface::instance());
        QVariant v;
        v.setValue(fill_color);
        cfg->setProperty("color", v);
        QDomDocument doc;
        doc.setContent(cfg->toXML());
        return doc;
    }
    
    bool loadFromConfig(KisFilterConfigurationSP cfg) {
        if (cfg->name() != "color") {
            return false;
        }
        fill_color = cfg->getColor("color");
        return true;
    }
    
    QDomDocument getASLXML() {
        KisAslXmlWriter w;
        w.enterDescriptor("", "", "null");
        if (cs) {
            w.writeColor("Clr ", fill_color.convertedTo(cs));
        } else {
             w.writeColor("Clr ", fill_color);
        }

        w.leaveDescriptor();

        return w.document();
    }
};

struct KRITAPSD_EXPORT psd_layer_gradient_fill {
    double angle {0.0};
    QString style {QString("linear")};
    QString repeat {QString("none")};
    double scale {100.0};
    bool reverse {false}; // Is gradient reverse
    bool dithered {false}; // Is gradient dithered
    bool align_with_layer {false};
    QPointF offset;
    QDomDocument gradient;
    int imageWidth {1}; // Set when loading.
    int imageHeight {1};

    // Used by ASL callback;
    void setGradient(const KoAbstractGradientSP &newGradient) {
        QDomDocument document;
        QDomElement gradientElement = document.createElement("gradient");
        gradientElement.setAttribute("name", newGradient->name());

        if (dynamic_cast<KoStopGradient*>(newGradient.data())) {
            KoStopGradient *gradient = dynamic_cast<KoStopGradient*>(newGradient.data());
            gradient->toXML(document, gradientElement);
        } else if (dynamic_cast<KoSegmentGradient*>(newGradient.data())) {
            KoSegmentGradient *gradient = dynamic_cast<KoSegmentGradient*>(newGradient.data());
            gradient->toXML(document, gradientElement);
        }

        document.appendChild(gradientElement);
        gradient = document;
    }

    void setDither(bool Dthr) {
        dithered = Dthr;
    }

    void setReverse(bool Rvrs) {
        reverse = Rvrs;
    }

    void setAngle(float Angl) {
        angle = Angl;
    }

    void setType(const QString type) {
        repeat = "none";
        if (type == "Lnr "){
            style = "linear";
        } else if (type == "Rdl "){
            style = "radial";
        } else if (type == "Angl"){
            style = "conical";
        } else if (type == "Rflc"){
            style = "bilinear";
            repeat ="alternate";
        } else {
            style = "square"; // diamond???
        }
    }

    void setAlignWithLayer(bool align) {
        align_with_layer = align;
    }

    void setScale(float Scl) {
        scale = Scl;
    }

    void setOffset(QPointF Ofst) {
        offset = Ofst;
    }

    QDomDocument getFillLayerConfig() {
        KisFilterConfigurationSP cfg;
        cfg = KisGeneratorRegistry::instance()->value("gradient")->defaultConfiguration(KisGlobalResourcesInterface::instance());
        cfg->setProperty("gradient", gradient.toString());
        cfg->setProperty("dither", dithered);
        cfg->setProperty("reverse", reverse);

        cfg->setProperty("shape", style);
        cfg->setProperty("repeat", repeat);

        cfg->setProperty("end_position_coordinate_system", "polar");

        cfg->setProperty("end_position_distance_units", "percent_of_width");
        cfg->setProperty("start_position_x_units", "percent_of_width");
        cfg->setProperty("start_position_y_units", "percent_of_height");

        // angle seems to go from -180 to +180;
        double fixedAngle = fmod(360.0 + angle, 360.0);

        double scaleModifier = 1.0;

        if (style == "square") {
            fixedAngle = fmod((45.0 + fixedAngle), 360.0);

            scaleModifier = cos(kisDegreesToRadians(45.0));
            double halfAngle = fmod(fabs(fixedAngle), 180.0);
            scaleModifier *= 1/cos(kisDegreesToRadians(45.0 - fmod(halfAngle, 45.0) ));
            if (halfAngle >= 45.0 && halfAngle < 135.0) {
                scaleModifier = (scaleModifier) * (imageHeight / imageWidth);
            }

        } else {
            double halfAngle = fmod(fabs(fixedAngle), 180.0);
            scaleModifier *= 1/cos(kisDegreesToRadians(halfAngle));
            if (halfAngle >= 45.0 && halfAngle < 135.0) {
                scaleModifier = (scaleModifier) * (imageHeight / imageWidth);
            }

        }

        cfg->setProperty("end_position_angle", fixedAngle);

        if (style == "linear") {
            // linear has the problem that in Krita it rotates around the top-left,
            // while in psd it rotates around the middle.
            QPointF center(imageWidth*0.5, imageHeight*0.5);

            QTransform rotate;
            rotate.rotate(fixedAngle);
            QTransform tf = QTransform::fromTranslate(-center.x(), -center.y())
                    * rotate * QTransform::fromTranslate(center.x(), center.y());
            QPointF topleft = tf.inverted().map(QPointF(0.0, 0.0));
            double xPercentage = (topleft.x()/double(imageWidth)) * 100.0;
            double yPercentage = (topleft.y()/double(imageHeight)) * 100.0;

            cfg->setProperty("end_position_distance", scale * scaleModifier);
            cfg->setProperty("start_position_x", xPercentage + offset.x());
            cfg->setProperty("start_position_y", yPercentage + offset.y());
        } else {
            cfg->setProperty("end_position_distance", scale * 0.5 * fabs(scaleModifier));
            cfg->setProperty("start_position_x", (50.0)+offset.x());
            cfg->setProperty("start_position_y", (50.0)+offset.y());
        }

        QDomDocument doc;
        doc.setContent(cfg->toXML());
        return doc;
    }

    bool loadFromConfig(KisFilterConfigurationSP cfg) {
        if (cfg->name() != "gradient") {
            return false;
        }

        bool res = false;

        res = gradient.setContent(cfg->getString("gradient", ""));

        dithered = cfg->getBool("dither");
        reverse = cfg->getBool("reverse");
        align_with_layer = false; // not supported.

        style = cfg->getString("shape", "linear");
        repeat = cfg->getString("repeat", "none");

        bool polar = (cfg->getString("end_position_coordinate_system") == "polar");

        QPointF start(cfg->getDouble("start_position_x", 0.0), cfg->getDouble("start_position_y", 0.0));
        if (polar) {
            angle = cfg->getDouble("end_position_angle", 0.0);
            scale = cfg->getDouble("end_position_distance", 100.0);
        } else {
            // assume carthesian
            QPointF end(cfg->getDouble("end_position_x", 1.0), cfg->getDouble("end_position_y", 1.0));
            // calculate angle and scale.
            double width  = start.x() - end.x();
            double height = start.y() - end.y();
            angle = fmod(360.0 + kisRadiansToDegrees(atan2(width, height)), 360.0);
            scale = sqrt((width*width) + (height*height));
        }


        if (style == "linear") {
            QPointF center(imageWidth*0.5, imageHeight*0.5);

            QTransform rotate;
            rotate.rotate(angle);
            QTransform tf = QTransform::fromTranslate(-center.x(), -center.y())
                    * rotate * QTransform::fromTranslate(center.x(), center.y());
            QPointF topleft = tf.inverted().map(QPointF(0.0, 0.0));
            double xPercentage = (topleft.x()/double(imageWidth)) * 100.0;
            double yPercentage = (topleft.y()/double(imageHeight)) * 100.0;
            offset = QPointF((start.x() - xPercentage), (start.y() - yPercentage));
        } else {
            scale = scale*2;
            offset = QPointF((start.x() - 50.0), (start.y() - 50.0));
        }

        double scaleModifier = 1.0;
        if (style == "square") {
            scaleModifier = cos(kisDegreesToRadians(45.0));
            double halfAngle = fmod(fabs(angle), 180.0);
            scaleModifier *= 1/cos(kisDegreesToRadians(45.0 - fmod(halfAngle, 45.0) ));
            if (halfAngle >= 45.0 && halfAngle < 135.0) {
                scaleModifier = (scaleModifier) * (imageHeight / imageWidth);
            }

            angle = angle - 45.0;
            if (angle < 0) {
                angle = 360.0 - angle;
            }

        } else {
            double halfAngle = fmod(fabs(angle), 180.0);
            scaleModifier *= 1/cos(kisDegreesToRadians(halfAngle));
            if (halfAngle >= 45.0 && halfAngle < 135.0) {
                scaleModifier = (scaleModifier) * (imageHeight / imageWidth);
            }

        }

        if (angle > 180) {
            angle = (0.0 - fmod(angle, 180.0));
        }

        scale /= fabs(scaleModifier);


        return res;
    }

    QDomDocument getASLXML() {
        KisAslXmlWriter w;
        w.enterDescriptor("", "", "null");

        if (!gradient.isNull()) {
            const QDomElement gradientElement = gradient.firstChildElement();
            if (!gradientElement.isNull()) {
                const QString gradientType = gradientElement.attribute("type");
                if (gradientType == "stop") {
                    const KoStopGradient *grad = new KoStopGradient(KoStopGradient::fromXML(gradientElement));
                    if (grad && grad->valid()) {
                        w.writeStopGradient("Grad", grad);
                    }
                } else if (gradientType == "segment") {
                    const KoSegmentGradient *grad = new KoSegmentGradient(KoSegmentGradient::fromXML(gradientElement));
                    if (grad && grad->valid()) {
                        w.writeSegmentGradient("Grad", grad);
                    }
                }
            }
        }
        w.writeBoolean("Dthr", dithered);
        w.writeBoolean("Rvrs", reverse);
        w.writeUnitFloat("Angl", "#Ang", angle);

        QString type = "Lnr ";
        if (style == "linear"){
            type = "Lnr ";
        } else if (style == "radial") {
            type = "Rdl ";
        } else if (style == "conical"){
            type = "Angl";
        } else if (style == "bilinear"){
            type = "Rflc";
        } else if (style == "square") {
            type = "Dmnd";
        }

        w.writeEnum("Type", "GrdT", type);
        w.writeBoolean("Algn", align_with_layer);
        w.writeUnitFloat("Scl ", "#Prc", scale);
        w.writePoint("Ofst", offset);

        w.leaveDescriptor();

        return w.document();
    }
};

struct KRITAPSD_EXPORT psd_layer_pattern_fill {
    double angle {0.0};
    double scale {1.0};
    QPointF offset;
    QString patternName;
    QString patternID;
    KoPatternSP pattern;
    bool align_with_layer {false};
    
    void setAngle(float Angl) {
        angle = Angl;
    }
    void setScale(float Scl) {
        scale = Scl;
    }
    void setOffset(QPointF phase) {
        offset = phase;
    }

    void setPatternRef(const QString Idnt, const QString name) {
        patternName = name;
        patternID = Idnt;
    }

    void setAlignWithLayer(bool align) {
        align_with_layer = align;
    }
    
    QDomDocument getFillLayerConfig() const {
        KisFilterConfigurationSP cfg;
        cfg = KisGeneratorRegistry::instance()->value("pattern")->defaultConfiguration(KisGlobalResourcesInterface::instance());

        cfg->setProperty("pattern", patternName);
        cfg->setProperty("fileName", QString(patternID + ".pat"));
        cfg->setProperty("md5", ""); // Zero out MD5, PSD patterns are looked up by UUID in filename

        //angle is flipped for patterns in Krita.
        double fixedAngle = 360.0 - fmod(360.0 + angle, 360.0);

        cfg->setProperty("transform_scale_x", scale / 100);
        cfg->setProperty("transform_scale_y", scale / 100);
        cfg->setProperty("transform_rotation_z", fixedAngle);

        cfg->setProperty("transform_offset_x", offset.x());
        cfg->setProperty("transform_offset_y", offset.y());
        QDomDocument doc;
        doc.setContent(cfg->toXML());
        return doc;
    }

    bool loadFromConfig(KisFilterConfigurationSP cfg) {
        if (cfg->name() != "pattern") {
            return false;
        }

        const QString patternMD5 = cfg->getString("md5", "");
        const QString patternNameTemp = cfg->getString("pattern", "Grid01.pat");
        const QString patternFileName = cfg->getString("fileName", "");

        KoResourceLoadResult res = KisGlobalResourcesInterface::instance()->source(ResourceType::Patterns).bestMatchLoadResult(patternMD5, patternFileName, patternNameTemp);
        pattern = res.resource<KoPattern>();

        patternName = cfg->getString("pattern", "");

        // Pattern ID needs the pattern to be saved first.

        align_with_layer = false;

        scale = cfg->getDouble("transform_scale_x", 1.0) * 100;

        angle = 360.0 - cfg->getDouble("transform_rotation_z", 0.0);
        if (angle > 180) {
            angle = (180.0 - angle);
        }

        offset  = QPointF(cfg->getInt("transform_offset_x", 0), cfg->getInt("transform_offset_y", 0));

        return true;
    }

    QDomDocument getASLXML() {
        KisAslXmlWriter w;
        w.enterDescriptor("", "", "null");

        // pattern ref
        w.enterDescriptor("Ptrn", "", "Ptrn");

        w.writeText("Nm  ", patternName);
        if (patternID.isEmpty()) {
            qWarning() << "This pattern cannot be saved: No pattern UUID available.";
            return QDomDocument();
        }
        w.writeText("Idnt", patternID);
        w.leaveDescriptor();

        // end

        w.writeBoolean("Algn", align_with_layer);
        w.writeUnitFloat("Scl ", "#Prc", scale);
        w.writeUnitFloat("Angl", "#Ang", angle);
        w.writePoint("phase", offset);

        w.leaveDescriptor();

        return w.document();
    }
};

struct KRITAPSD_EXPORT psd_layer_type_face {
    qint8 mark {0}; // Mark value
    qint32 font_type {0}; // Font type data
    qint8 font_name[256]; // Pascal string of font name
    qint8 font_family_name[256]; // Pascal string of font family name
    qint8 font_style_name[256]; // Pascal string of font style name
    qint8 script {0}; // Script value
    qint32 number_axes_vector {0}; // Number of design axes vector to follow
    qint32 *vector {nullptr}; // Design vector value
};

struct KRITAPSD_EXPORT psd_layer_type_style {
    qint8 mark {0}; // Mark value
    qint8 face_mark {0}; // Face mark value
    qint32 size {0}; // Size value
    qint32 tracking {0}; // Tracking value
    qint32 kerning {0}; // Kerning value
    qint32 leading {0}; // Leading value
    qint32 base_shift {0}; // Base shift value
    bool auto_kern {false}; // Auto kern on/off
    bool rotate {false}; // Rotate up/down
};

struct KRITAPSD_EXPORT psd_layer_type_line {
    qint32 char_count {0}; // Character count value
    qint8 orientation {0}; // Orientation value
    qint8 alignment {0}; // Alignment value
    qint8 actual_char {0}; // Actual character as a double byte character
    qint8 style {0}; // Style value
};

struct KRITAPSD_EXPORT psd_layer_type_tool {
    double transform_info[6]; // 6 * 8 double precision numbers for the transform information
    qint8 faces_count {0}; // Count of faces
    psd_layer_type_face *face {nullptr};
    qint8 styles_count {0}; // Count of styles
    psd_layer_type_style *style {nullptr};
    qint8 type {0}; // Type value
    qint32 scaling_factor {0}; // Scaling factor value
    qint32 character_count {0}; // Character count value
    qint32 horz_place {0}; // Horizontal placement
    qint32 vert_place {0}; // Vertical placement
    qint32 select_start {0}; // Select start value
    qint32 select_end {0}; // Select end value
    qint8 lines_count {0}; // Line count
    psd_layer_type_line *line {nullptr};
    QColor color;
    bool anti_alias {false}; // Anti alias on/off
};

/**
 * @brief The PsdAdditionalLayerInfoBlock class implements the Additional Layer Information block
 *
 * See: https://www.adobe.com/devnet-apps/photoshop/fileformatashtml/#50577409_71546
 */
class KRITAPSD_EXPORT PsdAdditionalLayerInfoBlock
{
public:
    PsdAdditionalLayerInfoBlock(const PSDHeader &header);

    using ExtraLayerInfoBlockHandler = std::function<bool(QIODevice &)>;
    using UserMaskInfoBlockHandler = std::function<bool(QIODevice &)>;

    void setExtraLayerInfoBlockHandler(ExtraLayerInfoBlockHandler handler);
    void setUserMaskInfoBlockHandler(UserMaskInfoBlockHandler handler);

    bool read(QIODevice &io);
    bool write(QIODevice &io, KisNodeSP node);

    void writeLuniBlockEx(QIODevice &io, const QString &layerName);
    void writeLsctBlockEx(QIODevice &io, psd_section_type sectionType, bool isPassThrough, const QString &blendModeKey);
    void writeLfx2BlockEx(QIODevice &io, const QDomDocument &stylesXmlDoc, bool useLfxsLayerStyleFormat);
    void writePattBlockEx(QIODevice &io, const QDomDocument &patternsXmlDoc);
    void writeLclrBlockEx(QIODevice &io, const quint16 &labelColor);

    void writeFillLayerBlockEx(QIODevice &io, const QDomDocument &fillConfig, psd_fill_type type);

    bool valid();

    const PSDHeader &m_header;
    QString error;
    QStringList keys; // List of all the keys that we've seen

    QString unicodeLayerName;
    QDomDocument layerStyleXml;
    QVector<QDomDocument> embeddedPatterns;
    quint16 labelColor{0}; // layer color.

    QDomDocument fillConfig;
    psd_fill_type fillType {psd_fill_solid_color};

    psd_section_type sectionDividerType;
    QString sectionDividerBlendMode;

private:
    template<psd_byte_order byteOrder = psd_byte_order::psdBigEndian>
    void readImpl(QIODevice &io);

    template<psd_byte_order byteOrder = psd_byte_order::psdBigEndian>
    void writeLuniBlockExImpl(QIODevice &io, const QString &layerName);

    template<psd_byte_order byteOrder = psd_byte_order::psdBigEndian>
    void writeLsctBlockExImpl(QIODevice &io, psd_section_type sectionType, bool isPassThrough, const QString &blendModeKey);

    template<psd_byte_order byteOrder = psd_byte_order::psdBigEndian>
    void writeLfx2BlockExImpl(QIODevice &io, const QDomDocument &stylesXmlDoc, bool useLfxsLayerStyleFormat);

    template<psd_byte_order byteOrder = psd_byte_order::psdBigEndian>
    void writePattBlockExImpl(QIODevice &io, const QDomDocument &patternsXmlDoc);

    template<psd_byte_order byteOrder = psd_byte_order::psdBigEndian>
    void writeLclrBlockExImpl(QIODevice &io, const quint16 &lclr);

    template<psd_byte_order byteOrder = psd_byte_order::psdBigEndian>
    void writeFillLayerBlockExImpl(QIODevice &io, const QDomDocument &fillConfig, psd_fill_type type);

private:
    ExtraLayerInfoBlockHandler m_layerInfoBlockHandler;
    UserMaskInfoBlockHandler m_userMaskBlockHandler;
};

#endif // PSD_ADDITIONAL_LAYER_INFO_BLOCK_H
