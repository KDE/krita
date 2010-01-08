/*
 *  Copyright (c) 2002 Patrick Julien <freak@codepimps.org>
 *  Copyright (c) 2004-2008 Boudewijn Rempt <boud@valdyas.org>
 *  Copyright (c) 2004 Clarence Dang <dang@kde.org>
 *  Copyright (c) 2004 Adrian Page <adrian@pagenet.plus.com>
 *  Copyright (c) 2004 Cyrille Berger <cberger@cberger.net>
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

#ifndef KIS_DUPLICATEOP_SETTINGS_H_
#define KIS_DUPLICATEOP_SETTINGS_H_

#include <kis_paintop_settings.h>
#include <kis_types.h>
#include <QPointF>

#include "kis_duplicateop_settings_widget.h"

class QDomElement;
class KisDuplicateOpSettings : public KisPaintOpSettings
{

public:
    using KisPaintOpSettings::fromXML;
    using KisPaintOpSettings::toXML;

    KisDuplicateOpSettings(KisImageWSP image);

    virtual ~KisDuplicateOpSettings();
    bool paintIncremental();

    QPointF offset() const;
    void mousePressEvent(KoPointerEvent *e);
    void activate();
    bool healing() const;
    bool perspectiveCorrection() const;

    void fromXML(const QDomElement& elt);
    void toXML(QDomDocument& doc, QDomElement& rootElt) const;

    KisPaintOpSettingsSP clone() const;
    virtual QRectF paintOutlineRect(const QPointF& pos, KisImageWSP image, OutlineMode _mode) const;
    virtual void paintOutline(const QPointF& pos, KisImageWSP image, QPainter &painter, const KoViewConverter &converter, OutlineMode _mode) const;

public:

    KisDuplicateOpSettingsWidget *m_options;
    QPointF m_offset;
    KisImageWSP m_image;
    bool m_isOffsetNotUptodate;
    QPointF m_position; // Give the position of the last alt-click
private:
    QRectF duplicateOutlineRect(const QPointF& pos, KisImageWSP image) const;
};


#endif // KIS_DUPLICATEOP_SETTINGS_H_
