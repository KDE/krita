/*
 *  SPDX-FileCopyrightText: 2016 Boudewijn Rempt <boud@valdyas.org>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */
#ifndef LIBKIS_CHANNEL_H
#define LIBKIS_CHANNEL_H

#include <QObject>

#include "kritalibkis_export.h"
#include "libkis.h"

#include <KoChannelInfo.h>
#include <kis_node.h>

/**
 * A Channel represents a singel channel in a Node. Krita does not
 * use channels to store local selections: these are strictly the
 * color and alpha channels.
 */
class KRITALIBKIS_EXPORT Channel : public QObject
{
    Q_OBJECT

public:
    explicit Channel(KisNodeSP node, KoChannelInfo *channel, QObject *parent = 0);
    ~Channel() override;

    bool operator==(const Channel &other) const;
    bool operator!=(const Channel &other) const;
    
    /**
     * @brief visible checks whether this channel is visible in the node
     * @return the status of this channel
     */
    bool visible() const;

    /**
     * @brief setvisible set the visibility of the channel to the given value.
     */
    void setVisible(bool value);

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
     * Read the values of the channel into the a byte array for each pixel in the rect from the Node this channel is part of, and returns it.
     *
     * Note that if Krita is built with OpenEXR and the Node has the 16 bits floating point channel depth type, Krita returns
     * 32 bits float for every channel; the libkis scripting API does not support half.
     */
    QByteArray pixelData(const QRect &rect) const;

    /**
     * @brief setPixelData writes the given data to the relevant channel in the Node. This is only possible for Nodes
     * that have a paintDevice, so nothing will happen when trying to write to e.g. a group layer.
     *
     * Note that if Krita is built with OpenEXR and the Node has the 16 bits floating point channel depth type, Krita expects
     * to be given a 4 byte, 32 bits float for every channel; the libkis scripting API does not support half.
     *
     * @param value a byte array with exactly enough bytes.
     * @param rect the rectangle to write the bytes into
     */
    void setPixelData(QByteArray value, const QRect &rect);

private:

    struct Private;
    Private *const d;

};

#endif // LIBKIS_CHANNEL_H
