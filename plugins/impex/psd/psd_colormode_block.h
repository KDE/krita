/*
 *  SPDX-FileCopyrightText: 2009 Boudewijn Rempt <boud@valdyas.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef PSD_COLORMODE_BLOCK_H
#define PSD_COLORMODE_BLOCK_H

#include <psd.h>
#include "psd_header.h"

#include <QByteArray>
#include <QColor>

class PSDColorModeBlock
{
public:

    PSDColorModeBlock(psd_color_mode colormode);

    bool read(QIODevice &io);
    bool write(QIODevice &io);
    bool valid();

    quint32 blocksize;
    psd_color_mode colormode;
    QByteArray data;

    QString error;

    /* to store rgb colormap values of indexed image
    */
    QList<QColor> colormap;
    QByteArray duotoneSpecification; // Krita should save this in an annotation and write it back, if present

};

#endif // PSD_COLORMODE_BLOCK_H
