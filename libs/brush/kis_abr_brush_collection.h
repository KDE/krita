/*
 *  SPDX-FileCopyrightText: 2010 Boudewijn Rempt <boud@valdyas.org>
 *  SPDX-FileCopyrightText: 2010 Lukáš Tvrdý <lukast.dev@gmail.com>
 *  SPDX-FileCopyrightText: 2007 Eric Lamarque <eric.lamarque@free.fr>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KIS_ABR_BRUSH_COLLECTION_H
#define KIS_ABR_BRUSH_COLLECTION_H

#include <QImage>
#include <QVector>
#include <QDataStream>
#include <QString>
#include <kis_debug.h>

#include <kis_scaling_size_brush.h>
#include <kis_types.h>
#include <kis_shared.h>
#include <brushengine/kis_paint_information.h>
#include <kis_abr_brush.h>


class QString;
class QIODevice;


struct AbrInfo;

/**
 * load a collection of brushes from an abr file
 */
class BRUSH_EXPORT KisAbrBrushCollection
{

protected:

public:

    /// Construct brush to load filename later as brush
    KisAbrBrushCollection(const QString& filename);

    ~KisAbrBrushCollection() {}

    bool load();

    bool loadFromDevice(QIODevice *dev);

    bool save();

    bool saveToDevice(QIODevice* dev) const;

    bool isLoaded() const;

    /**
     * @return a preview of the brush
     */
    QImage image() const;

    /**
     * @return default file extension for saving the brush
     */
    QString defaultFileExtension() const;

    QList<KisAbrBrushSP> brushes() const {
        return m_abrBrushes->values();
    }

    QSharedPointer<QMap<QString, KisAbrBrushSP>> brushesMap() const {
        return m_abrBrushes;
    }

    KisAbrBrushSP brushByName(QString name) const {
        if (m_abrBrushes->contains(name)) {
            return m_abrBrushes.data()->operator[](name);
        }
        return KisAbrBrushSP();
    }

    QDateTime lastModified() const {
        return m_lastModified;
    }

    QString filename() const {
        return m_filename;
    }

protected:
    KisAbrBrushCollection(const KisAbrBrushCollection& rhs);

    void toXML(QDomDocument& d, QDomElement& e) const;

private:

    qint32 abr_brush_load(QDataStream & abr, AbrInfo *abr_hdr, const QString filename, qint32 image_ID, qint32 id);
    qint32 abr_brush_load_v12(QDataStream & abr, AbrInfo *abr_hdr, const QString filename, qint32 image_ID, qint32 id);
    quint32 abr_brush_load_v6(QDataStream & abr, AbrInfo *abr_hdr, const QString filename, qint32 image_ID, qint32 id);

    bool m_isLoaded;
    QDateTime m_lastModified;
    QString m_filename;
    QSharedPointer<QMap<QString, KisAbrBrushSP>> m_abrBrushes;
};

typedef QSharedPointer<KisAbrBrushCollection> KisAbrBrushCollectionSP;

#endif

