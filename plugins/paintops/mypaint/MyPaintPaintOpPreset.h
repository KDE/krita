/*
 * SPDX-FileCopyrightText: 2020 Ashwin Dhakaita <ashwingpdhakaita@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KIS_MYPAINT_BRUSH_H
#define KIS_MYPAINT_BRUSH_H

#include <QObject>
#include <libmypaint/mypaint-brush.h>
#include <KoColor.h>
#include <kis_paintop_settings.h>
#include <kis_painter.h>
#include <KoResource.h>
#include <KisResourceTypes.h>
#include <kis_paintop_preset.h>

class KisMyPaintPaintOpPreset : public QObject, public KisPaintOpPreset
{
    Q_OBJECT

public:

    KisMyPaintPaintOpPreset(const QString &fileName="");
    virtual ~KisMyPaintPaintOpPreset();

    void setColor(const KoColor color, const KoColorSpace *colorSpace);
    void apply(KisPaintOpSettingsSP settings);
    MyPaintBrush* brush();

    bool loadFromDevice(QIODevice *dev, KisResourcesInterfaceSP resourcesInterface) override;
    bool save() override;

    QPair<QString, QString> resourceType() const override {
        return QPair<QString, QString>(ResourceType::PaintOpPresets, ResourceSubType::MyPaintPaintOpPresets);
    }

    void updateThumbnail() override;
    QString thumbnailPath() const override;

    QByteArray getJsonData();
    float getSize();
    float getHardness();
    float getOpacity();
    float getOffset();
    float isEraser();

private:

    class Private;
    Private* const d;
    bool firstLoad = true;
};

#endif // KIS_MYPAINT_BRUSH_H
