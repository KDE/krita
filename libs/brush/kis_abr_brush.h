/*
 *  SPDX-FileCopyrightText: 2010 Boudewijn Rempt <boud@valdyas.org>
 *  SPDX-FileCopyrightText: 2010 Lukáš Tvrdý <lukast.dev@gmail.com>
 *  SPDX-FileCopyrightText: 2007 Eric Lamarque <eric.lamarque@free.fr>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KIS_ABR_BRUSH_
#define KIS_ABR_BRUSH_

#include <QImage>
#include <QVector>

#include <kis_scaling_size_brush.h>
#include <kis_types.h>
#include <kis_shared.h>

#include "kritabrush_export.h"

class KisQImagemask;
class KisAbrBrushCollection;
typedef KisSharedPtr<KisQImagemask> KisQImagemaskSP;

class QString;
class QIODevice;


class BRUSH_EXPORT KisAbrBrush : public KisScalingSizeBrush
{

public:

    /// Construct brush to load filename later as brush
    KisAbrBrush(const QString& filename, KisAbrBrushCollection *parent);
    KisAbrBrush(const KisAbrBrush& rhs);
    KisAbrBrush(const KisAbrBrush& rhs, KisAbrBrushCollection *parent);
    KisAbrBrush &operator=(const KisAbrBrush &rhs) = delete;
    KoResourceSP clone() const override;

    bool isSerializable() const override;
    bool loadFromDevice(QIODevice *dev, KisResourcesInterfaceSP resourcesInterface) override;
    bool saveToDevice(QIODevice *dev) const override;

    QPair<QString, QString> resourceType() const override {
        return QPair<QString, QString>(ResourceType::Brushes, ResourceSubType::AbrBrushes);
    }

    /**
     * @return default file extension for saving the brush
     */
    QString defaultFileExtension() const override;

    QImage brushTipImage() const override;

    friend class KisAbrBrushCollection;

    void setBrushTipImage(const QImage& image) override;

    void toXML(QDomDocument& d, QDomElement& e) const override;

private:
    KisAbrBrushCollection *m_parent;
};

typedef QSharedPointer<KisAbrBrush> KisAbrBrushSP;

#endif // KIS_ABR_BRUSH_

