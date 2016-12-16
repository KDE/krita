/*
 *  Copyright (c) 2016 Boudewijn Rempt <boud@valdyas.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
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
#ifndef LIBKIS_CHANNEL_H
#define LIBKIS_CHANNEL_H

#include <QObject>

#include "kritalibkis_export.h"
#include "libkis.h"

#include <KoChannelInfo.h>
#include <kis_node.h>

/**
 * Channel
 */
class KRITALIBKIS_EXPORT Channel : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(Channel)

    Q_PROPERTY(bool visible READ visible WRITE setvisible)

public:
    explicit Channel(KisNodeSP node, KoChannelInfo *channel, QObject *parent = 0);
    virtual ~Channel();

    bool visible() const;
    void setvisible(bool value);

    /**
     * @return the name of the channel
     */
    QString name() const;

    /**
     * @returns the position of the first byte of the channel in the pixel
     */
    int position() const;

    /**
     * @return the number of bytes this channel takes
     */
    int channelSize() const;

    /**
     * @return the exact bounds of the channel. This can be smaller than the bounds of the Node this channel is part of.
     */
    QRect bounds() const;

    /**
     * Read the values of the channel into the a byte array for each pixel in the rect from the Node this channel is part of, and returns it,
     */
    QByteArray pixelData(const QRect &rect) const;

    /**
     * @brief setPixelData writes the given data to the relevant channel in the Node. This is only possible for Nodes
     * that have a paintDevice, so nothing will happen when trying to write to e.g. a group layer.
     * @param value a byte array with exactly enough bytes.
     * @param rect the rectangle to write the bytes into
     */
    void setPixelData(QByteArray value, const QRect &rect);

private:

    struct Private;
    Private *const d;

};

#endif // LIBKIS_CHANNEL_H
