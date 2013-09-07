/*
 *  Copyright (c) 2013 Boudewijn Rempt <boud@valdyas.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */
#ifndef KIS_FILE_LAYER_H
#define KIS_FILE_LAYER_H

#include "kis_external_layer_iface.h"

#include <QFileSystemWatcher>

class KisDoc2;
class KisPart2;

/**
 * @brief The KisFileLayer class loads a particular file as a layer into the layer stack.
 */
class KisFileLayer : public KisExternalLayer
{
    Q_OBJECT
public:
    explicit KisFileLayer(KisImageWSP image, const QString& basePath, const QString &filename, bool scaleToImageResolution, const QString &name, quint8 opacity);
    ~KisFileLayer();
    KisFileLayer(const KisFileLayer& rhs);

    void resetCache();

    virtual const KoColorSpace *colorSpace() const;

    KisPaintDeviceSP original() const;
    KisPaintDeviceSP paintDevice() const;
    KoDocumentSectionModel::PropertyList sectionModelProperties() const;

    void setFileName(const QString &basePath, const QString &filename);
    QString fileName() const;
    QString path() const;
    void setScaleToImageResolution(bool scale);
    bool scaleToImageResolution() const;
    
    KisNodeSP clone() const;
    bool allowAsChild(KisNodeSP) const;

    bool accept(KisNodeVisitor&);
    void accept(KisProcessingVisitor &visitor, KisUndoAdapter *undoAdapter);

public slots:

    void reloadImage();

private:
    KisDoc2 *m_doc;

    QString m_basePath;
    QString m_filename;
    bool m_scaleToImageResolution;

    KisPaintDeviceSP m_image;

    QFileSystemWatcher m_fileWatcher;

};

#endif // KIS_FILE_LAYER_H
