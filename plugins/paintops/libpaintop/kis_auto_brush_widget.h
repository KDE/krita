/*
 *  SPDX-FileCopyrightText: 2004, 2007 Cyrille Berger <cberger@cberger.net>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
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

    void reset();

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
