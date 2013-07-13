/*
 *  Copyright (c) 2013 Somsubhra Bairi <somsubhra.bairi@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; version 2 of the License, or(at you option)
 *  any later version..
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef KIS_KRANIM_LOADER_H
#define KIS_KRANIM_LOADER_H


class QString;
class KoStore;

#include <kis_animation_doc.h>
#include <kis_types.h>

class KisKranimLoader
{
public:
    KisKranimLoader(KisAnimationDoc* doc);

    ~KisKranimLoader();

    KisImageWSP loadXML(const KoXmlElement &elem);

    void loadBinaryData(KoStore* store, KisImageWSP image, const QString &uri, bool external);

private:
    struct Private;
    Private* const m_d;
};

#endif // KIS_KRANIM_LOADER_H
