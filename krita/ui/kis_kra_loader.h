/* This file is part of the KDE project
 * Copyright (C) Boudewijn Rempt <boud@valdyas.org>, (C) 2007
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */
#ifndef KIS_KRA_LOADER_H
#define KIS_KRA_LOADER_H

#include <QObject>

class KisDoc2;
class KoXmlElement;
class KoStore;

#include <kis_types.h>

/**
 * Load old-style 1.x .kra files.
 */
class KisKraLoader : public QObject {

    Q_OBJECT;

public:

    KisKraLoader( KisDoc2 * document );

    ~KisKraLoader();

    /**
     * Loading is done in two steps: first all xml is loaded, then, in finishLoading,
     * the actual layer data is loaded.
     */
    KisImageSP loadXML(const KoXmlElement& elem);

    void loadBinaryData( KoStore* store, KisImageSP image, const QString & uri, bool external);
private:

    void loadLayers(const KoXmlElement& element, KisImageSP img, KisGroupLayerSP parent);

    KisLayerSP loadLayer(const KoXmlElement& elem, KisImageSP img);

    KisLayerSP loadPaintLayer(const KoXmlElement& elem, KisImageSP img, const QString & name, qint32 x, qint32 y, qint32 opacity, bool visible, bool locked, const QString & compositeOp);

    KisGroupLayerSP loadGroupLayer(const KoXmlElement& elem, KisImageSP img, const QString & name, qint32 x, qint32 y, qint32 opacity, bool visible, bool locked, const QString &compositeOp);

    KisAdjustmentLayerSP loadAdjustmentLayer(const KoXmlElement& elem, KisImageSP img, const QString & name, qint32 x, qint32 y, qint32 opacity, bool visible, bool locked, const QString & compositeOp);
    
    KisShapeLayerSP loadShapeLayer(const KoXmlElement& elem, KisImageSP img, const QString & name, qint32 x, qint32 y, qint32 opacity, bool visible, bool locked, const QString &compositeOp);

    

private:

    class Private;
    Private * const m_d;

};

#endif
