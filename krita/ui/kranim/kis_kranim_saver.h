/*
 *  Copyright (c) 2013 Somsubhra Bairi <somsubhra.bairi@gmail.com>
 *
 *  This library is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; version 2.1 of the License, or (at your option)
 *  any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef KIS_KRANIM_SAVER_H
#define KIS_KRANIM_SAVER_H

#include "kis_types.h"
#include "kranimstore/kis_animation_store.h"
#include "kranimstore/kis_animation_store_writer.h"

class KisAnimationDoc;
class QDomElement;
class QDomNode;
class QDomDocument;
class KoStore;
class QString;
class QRect;

/**
 * This is a helper class to save/dump
 * the animation frames to disk.
 */
class KisKranimSaver
{
public:
    KisKranimSaver(KisAnimationDoc* document);

    ~KisKranimSaver();

    QDomElement saveXML(QDomDocument& doc);

    QDomElement saveMetaData(QDomDocument &doc, QDomNode root);

    void saveFrame(KisAnimationStore* store, KisLayerSP frame, const QRect &framePosition);

    void saveFrame(KisAnimationStore* store, KisPaintDeviceSP device, const QRect &framePosition);

    void deleteFrame(KisAnimationStore* store, int frame, int layer);

    void renameFrame(KisAnimationStore* store, int oldFrame, int oldLayer, int newFrame, int newLayer);

private:
    struct Private;
    Private *const m_d;
    KisAnimationStoreWriter* m_writer;
};

#endif // KIS_KRANIM_SAVER_H
