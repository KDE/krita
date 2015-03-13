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

class QIODevice;

#include <QVector>

#include <psd.h>

#include "kis_types.h"
#include "krita_export.h"

/**
 * @brief The KisPSDLayerStyle class implements loading, saving and applying
 * the PSD layer effects.
 *
 * See http://www.tonton-pixel.com/Photoshop%20Additional%20File%20Formats/styles-file-format.html
 *
 */
class KRITAIMAGE_EXPORT KisPSDLayerStyle
{

public:
    typedef QVector<KisPSDLayerStyleSP> StylesVector;

public:
    explicit KisPSDLayerStyle();
    virtual ~KisPSDLayerStyle();
    KisPSDLayerStyle(const KisPSDLayerStyle& rhs);
    KisPSDLayerStyle operator=(const KisPSDLayerStyle& rhs);

    KisPSDLayerStyleSP clone() const;

    QString name() const;

    /**
     * \return true if all the styles are disabled
     */
    bool isEmpty() const;

    const psd_layer_effects_context* context() const;
    const psd_layer_effects_drop_shadow* drop_shadow() const;

    psd_layer_effects_context* context();
    psd_layer_effects_drop_shadow* drop_shadow();

    /**
     * Save given styles to the ASL style format. All patterns references in the
     * contained styles will also be saved.
     */
    static bool writeASL(QIODevice *io, StylesVector styles);

    /**
     * Load all style objects in the ASL style format. All patterns contained in the
     * asl file will be added to the global pattern server.
     */
    static StylesVector readASL(QIODevice *io);

    /// Save this style object
    bool write(QIODevice *io) const;

    /// Load this style object
    bool read(QIODevice *io);

    psd_layer_effects_drop_shadow & dropShadow() const;
    void setDropShadow(const psd_layer_effects_drop_shadow &dropShadow);

private:
    struct Private;
    Private * const d;
};

#endif // KIS_PSD_LAYER_STYLE_H
