/*
 *  SPDX-FileCopyrightText: 2010 Cyrille Berger <cberger@cberger.net>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KIS_SVG_BRUSH_
#define KIS_SVG_BRUSH_

#include "kis_scaling_size_brush.h"

class BRUSH_EXPORT KisSvgBrush : public KisScalingSizeBrush
{
public:
    /// Construct brush to load filename later as brush
    KisSvgBrush(const QString &filename);
    KisSvgBrush(const KisSvgBrush &rhs);
    KisSvgBrush &operator=(const KisSvgBrush &rhs) = delete;

    KoResourceSP clone() const override;

    bool loadFromDevice(QIODevice *dev, KisResourcesInterfaceSP resourcesInterface) override;
    bool saveToDevice(QIODevice *dev) const override;

    QPair<QString, QString> resourceType() const override {
        return QPair<QString, QString>(ResourceType::Brushes, ResourceSubType::SvgBrushes);
    }

    QString defaultFileExtension() const override;
    void toXML(QDomDocument& d, QDomElement& e) const override;
private:
    QByteArray m_svg;
};

#endif
