/*
 *  SPDX-FileCopyrightText: 2010 Cyrille Berger <cberger@cberger.net>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KIS_PNG_BRUSH_
#define KIS_PNG_BRUSH_

#include "KisColorfulBrush.h"

class BRUSH_EXPORT  KisPngBrush : public KisColorfulBrush
{
public:
    /// Construct brush to load filename later as brush
    KisPngBrush(const QString& filename);
    KisPngBrush(const KisPngBrush &rhs);
    KoResourceSP clone() const override;
    KisPngBrush &operator=(const KisPngBrush &rhs) = delete;

    bool loadFromDevice(QIODevice *dev, KisResourcesInterfaceSP resourcesInterface) override;
    bool saveToDevice(QIODevice *dev) const override;

    QString defaultFileExtension() const override;
    void toXML(QDomDocument& d, QDomElement& e) const override;

    QPair<QString, QString> resourceType() const override {
        return QPair<QString, QString>(ResourceType::Brushes, ResourceSubType::PngBrushes);
    }

};

#endif
