/*
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

#ifndef _KIS_AUTO_BRUSH_WIDGET_H_
#define _KIS_AUTO_BRUSH_WIDGET_H_

#include <QObject>
#include <QResizeEvent>
#include "kritapaintop_export.h"
#include "ui_wdgautobrush.h"
#include <kis_auto_brush.h>

class KisSignalCompressor;
class KisAspectRatioLocker;


class PAINTOP_EXPORT KisWdgAutoBrush : public QWidget, public Ui::KisWdgAutoBrush
{
    Q_OBJECT

public:
    KisWdgAutoBrush(QWidget *parent, const char *name) : QWidget(parent) {
        setObjectName(name); setupUi(this);
    }

};

class PAINTOP_EXPORT KisAutoBrushWidget : public KisWdgAutoBrush
{
    Q_OBJECT

public:

    KisAutoBrushWidget(QWidget *parent, const char* name);
    ~KisAutoBrushWidget() override;

    void activate();

    KisBrushSP brush();

    void setBrush(KisBrushSP brush);

    void setBrushSize(qreal dxPixels, qreal dyPixels);
    QSizeF brushSize() const;

    void drawBrushPreviewArea();

private Q_SLOTS:
    void paramChanged();
    void setStackedWidget(int);

Q_SIGNALS:

    void sigBrushChanged();

protected:
    void resizeEvent(QResizeEvent *) override;

private:
    QImage m_brush;
    KisBrushSP m_autoBrush;
    QScopedPointer<KisSignalCompressor> m_updateCompressor;
    QScopedPointer<KisAspectRatioLocker> m_fadeAspectLocker;
};


#endif
