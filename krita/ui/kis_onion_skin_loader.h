/*
 *  Copyright (c) 2014 Somsubhra Bairi <somsubhra.bairi@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; version 2 of the License, or(at you option)
 *  any later version.
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

#ifndef KIS_ONION_SKIN_LOADER_H
#define KIS_ONION_SKIN_LOADER_H

#include <QObject>
#include <kis_animation_doc.h>
#include <kis_types.h>

class KisKranimLoader;

class KisOnionSkinLoader : public QObject
{
    Q_OBJECT
public:
    KisOnionSkinLoader(KisAnimationDoc* doc, QObject *parent = 0);

    void loadOnionSkins();

    void setNextFramesNumber();
    void setPrevFramesNumber();

    void setNextFramesOpacity();
    void setPrevFramesOpacity();

    void setNextFramesColor();
    void setPrevFramesColor();

private:
    QBitArray prevFramesChannelFlags();
    QBitArray nextFramesChannelFlags();

    int normalizeOpacityValue(int val);

private:
    KisAnimationDoc* m_doc;
};

#endif // KIS_ONION_SKIN_LOADER_H
