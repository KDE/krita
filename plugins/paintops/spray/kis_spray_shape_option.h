/*
 *  SPDX-FileCopyrightText: 2008, 2009, 2010 Lukáš Tvrdý <lukast.dev@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KIS_SPRAY_SHAPE_OPTION_H
#define KIS_SPRAY_SHAPE_OPTION_H

#include <kis_paintop_option.h>

const QString SPRAYSHAPE_ENABLED = "SprayShape/enabled";
const QString SPRAYSHAPE_SHAPE = "SprayShape/shape";
const QString SPRAYSHAPE_PROPORTIONAL = "SprayShape/proportional";
const QString SPRAYSHAPE_WIDTH = "SprayShape/width";
const QString SPRAYSHAPE_HEIGHT = "SprayShape/height";
const QString SPRAYSHAPE_IMAGE_URL = "SprayShape/imageUrl";
const QString SPRAYSHAPE_USE_ASPECT = "SprayShape/useAspect";


class KisShapeOptionsWidget;
class KisAspectRatioLocker;

class KisSprayShapeOption : public KisPaintOpOption
{
    Q_OBJECT
public:
    KisSprayShapeOption();
    ~KisSprayShapeOption() override;

    /// 0 - ellipse, 1 - rectangle, 2 - anti-aliased pixel, 2 - pixel
    int shape() const;

    void writeOptionSetting(KisPropertiesConfigurationSP setting) const override;
    void readOptionSetting(const KisPropertiesConfigurationSP setting) override;

private:
    KisShapeOptionsWidget * m_options;
    int m_maxSize;
    KisAspectRatioLocker *m_sizeRatioLocker;

private Q_SLOTS:
    void prepareImage();
    void changeSizeUI(bool proportionalSize);
};

#include <QImage>

class KisShapeProperties
{
public:
    // particle type size
    quint8 shape;
    quint16 width;
    quint16 height;
    bool enabled;
    bool proportional;
    // rotation
    QImage image;

public:

    void loadSettings(const KisPropertiesConfigurationSP settings, qreal proportionalWidth, qreal proportionalHeight) {
        enabled = settings->getBool(SPRAYSHAPE_ENABLED, true);

        width = settings->getInt(SPRAYSHAPE_WIDTH);
        height = settings->getInt(SPRAYSHAPE_HEIGHT);

        proportional = settings->getBool(SPRAYSHAPE_PROPORTIONAL);

        if (proportional) {
            width = (width / 100.0) * proportionalWidth;
            height = (height / 100.0) * proportionalHeight;
        }
        // particle type size
        shape = settings->getInt(SPRAYSHAPE_SHAPE);
        // you have to check if the image is null in client
        image = QImage(settings->getString(SPRAYSHAPE_IMAGE_URL));
    }
};

#endif // KIS_SPRAY_SHAPE_OPTION_H

