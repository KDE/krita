/*
 *  Copyright (c) 2014 Boudewijn Rempt <boud@valdyas.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */
#ifndef KIS_PSD_LAYER_STYLE_H
#define KIS_PSD_LAYER_STYLE_H

#include <QObject>

class QIODevice;

#include <krita_export.h>

/**
 * @brief The KisPSDLayerStyle class implements loading, saving and applying
 * the PSD layer effects.
 *
 */
class KRITAIMAGE_EXPORT KisPSDLayerStyle : public QObject
{
    Q_OBJECT
public:
    explicit KisPSDLayerStyle(QObject *parent = 0);

    /// Save the ASL style format. See http://www.tonton-pixel.com/Photoshop%20Additional%20File%20Formats/styles-file-format.html
    bool writeASL(QIODevice *io) const;

    /// Load the ASL style format. See http://www.tonton-pixel.com/Photoshop%20Additional%20File%20Formats/styles-file-format.html
    bool readASL(QIODevice *io);

    bool write(QIODevice *io) const;
    bool read(QIODevice *io);

signals:

public slots:

private:
    struct Private;
    Private * const d;
};

#endif // KIS_PSD_LAYER_STYLE_H
