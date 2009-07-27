/* This file is part of the KDE project
 * Copyright (C) 2007, 2009 Thomas Zander <zander@kde.org>
 * Copyright (C) 2007 Jan Hambrecht <jaham@gmx.net>
 * Copyright (C) 2008 Casper Boemann <cbr@boemann.dk>
 * Copyright (C) 2008 Thorsten Zachmann <zachmann@kde.org>
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

#include "KoImageData_p.h"
#include "KoImageCollection.h"

#include <QIODevice>
#include <KDebug>

KoImageDataPrivate::KoImageDataPrivate()
    : collection(0),
    quality(KoImageData::HighQuality),
    errorCode(KoImageData::Success),
    dataStoreState(StateEmpty)
{
}

KoImageDataPrivate::~KoImageDataPrivate()
{
    if (collection)
        collection->removeOnKey(key);
}

bool KoImageDataPrivate::saveToFile(QIODevice & device)
{
#if 0
    if (!rawData.isEmpty()) {
        return device.write(rawData) == rawData.size();
    }
    else {
        return image.save(&device, "PNG"); // if we only have a image save it as png
    }
#endif
}
