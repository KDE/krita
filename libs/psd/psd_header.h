/*
 *  SPDX-FileCopyrightText: 2009 Boudewijn Rempt <boud@valdyas.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef PSD_HEADER_H
#define PSD_HEADER_H

#include "kritapsd_export.h"

#include <QtGlobal>
#include <kis_debug.h>
#include <psd.h>

class QIODevice;

class KRITAPSD_EXPORT PSDHeader
{
public:
    PSDHeader();

    /**
     * Reads a psd header from the given device.
     *
     * @return false if:
     *   <li>reading failed
     *   <li>if the 8BPS signature is not found
     *   <li>if the version is not 1 or 2
     */
    bool read(QIODevice *device);

    /**
     * write the header data to the given device
     *
     * @return false if writing failed or if this is not a valid header
     */
    bool write(QIODevice *device);

    bool valid();

    QString signature; // 8PBS
    quint16 version; // 1 or 2
    quint16 nChannels; // 1 - 56
    quint32 height; // 1-30,000 or 1 - 300,000
    quint32 width; // 1-30,000 or 1 - 300,000
    quint16 channelDepth; // 1, 8, 16. XXX: check whether 32 is used!
    psd_color_mode colormode;

    QString error;
};

KRITAPSD_EXPORT QDebug operator<<(QDebug dbg, const PSDHeader &header);

#endif // PSD_HEADER_H
