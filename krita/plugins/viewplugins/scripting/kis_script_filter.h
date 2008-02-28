/*
 * This file is part of Krita
 *
 * Copyright (c) 2007 Cyrille Berger <cberger@cberger.net>
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

#ifndef _KIS_SCRIPT_FILTER_H_
#define _KIS_SCRIPT_FILTER_H_

#include <QObject>
#include "filter/kis_filter.h"
#include "kis_processing_information.h"

#include "kritacore/krs_paint_device.h"

namespace Kross {
    class Action;
}

class KisScriptFilter : public QObject, public KisFilter {
    Q_OBJECT
    public:
        KisScriptFilter(Kross::Action* action);
        virtual ~KisScriptFilter();

        using KisFilter::process;
        
        virtual void process(KisConstProcessingInformation srcInfo, KisProcessingInformation dstInfo, const QSize& size, const KisFilterConfiguration* config, KoUpdater*) const;
    public Q_SLOTS:
        QString category() const;
    signals:
        void scriptProcess( QObject* src, const QPoint& srcTopLeft, QObject* dst, const QPoint& dstTopLeft, const QSize& size, QObject* config ) const;
    public:
        virtual bool supportsPainting() const;
        virtual bool supportsPreview() const;
        virtual bool supportsAdjustmentLayers() const;
        virtual bool supportsIncrementalPainting() const;
        virtual bool supportsThreading() const;
    private:
        class Private;
        Private* const d;
};


#endif
