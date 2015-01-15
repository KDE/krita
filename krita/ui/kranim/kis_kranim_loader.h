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

#ifndef KIS_KRANIM_LOADER_H
#define KIS_KRANIM_LOADER_H


class QString;
class KoStore;

#include "kis_animation_doc.h"
#include "kis_types.h"
#include "kranimstore/kis_animation_store.h"

/**
 * This is a helper class for loading
 * the animation frames from disk
 */
class KisKranimLoader
{
public:
    KisKranimLoader(KisAnimationDoc* doc);

    ~KisKranimLoader();

    KisImageWSP loadXML(const KoXmlElement &elem);

    void loadBinaryData(KoStore* store, KisImageWSP image, const QString &uri, bool external);

    void loadFrame(KisNodeSP layer, KisAnimationStore* store, QString location);

private:
    struct Private;
    Private* const m_d;
};

#endif // KIS_KRANIM_LOADER_H
