/*
 *  Copyright (c) 2002 Patrick Julien <freak@codepimps.org>
 *  Copyright (c) 2004 Boudewijn Rempt <boud@valdyas.org>
 *  Copyright (c) 2004 Clarence Dang <dang@kde.org>
 *  Copyright (c) 2004 Adrian Page <adrian@pagenet.plus.com>
 *  Copyright (c) 2004,2007 Cyrille Berger <cberger@cberger.net>
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

#ifndef KIS_DUPLICATEOP_H_
#define KIS_DUPLICATEOP_H_

#include "kis_paintop.h"
#include <QString>
#include <klocale.h>

class QPointF;
class KisPainter;
class Ui_DuplicateOpOptionsWidget;

class KisDuplicateOpFactory  : public KisPaintOpFactory  {

public:
    KisDuplicateOpFactory() {}
    virtual ~KisDuplicateOpFactory() {}

    virtual KisPaintOp * createOp(const KisPaintOpSettings *settings, KisPainter * painter, KisImageSP image);
    virtual QString id() const { return "duplicate"; }
    virtual QString name() const { return i18n("Duplicate"); }
    virtual QString pixmap() { return "duplicate.png"; }
    virtual KisPaintOpSettings *settings(QWidget * parent, const KoInputDevice& inputDevice, KisImageSP image);
};

class KisDuplicateOpSettings : public QObject, public KisPaintOpSettings {
    public:
        KisDuplicateOpSettings(QWidget* parent, KisImageSP image);
        virtual ~KisDuplicateOpSettings();
        virtual QWidget *widget() const { return m_optionsWidget; }
        virtual void mousePressEvent(KoPointerEvent *e);
        virtual void activate();
        QPointF offset() const { return m_offset; }
        bool healing() const;
        bool perspectiveCorrection() const;
    private:
        QWidget* m_optionsWidget;
        Ui_DuplicateOpOptionsWidget* m_uiOptions;
        KisImageSP m_image;
        bool m_isOffsetNotUptodate;
        QPointF m_position; // Give the position of the last alt-click
        QPointF m_offset;
};

class KisDuplicateOp : public KisPaintOp {

    typedef KisPaintOp super;


public:

    KisDuplicateOp(const KisDuplicateOpSettings* settings, KisPainter * painter, KisImageSP image);
    virtual ~KisDuplicateOp();

    void paintAt(const KisPaintInformation& info);
private:
    double minimizeEnergy(const double* m, double* sol, int w, int h);
private:

    KisPaintDeviceSP m_srcdev;
    KisPaintDeviceSP m_target;
    KisImageSP m_image;
    const KisDuplicateOpSettings* m_settings;
    QPointF m_duplicateStart;
    bool m_duplicateStartIsSet;
};

#endif // KIS_DUPLICATEOP_H_
