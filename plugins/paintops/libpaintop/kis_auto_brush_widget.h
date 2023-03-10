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

#include <lager/cursor.hpp>
#include <KisBrushModel.h>

class KisSignalCompressor;
class KisAspectRatioLocker;
class KisAutoBrushModel;

class PAINTOP_EXPORT KisWdgAutoBrush : public QWidget, public Ui::KisWdgAutoBrush
{
    Q_OBJECT

public:
    KisWdgAutoBrush(QWidget *parent, const char *name)
        : QWidget(parent)
    {
        setObjectName(name);
        setupUi(this);
    }

};

class PAINTOP_EXPORT KisAutoBrushWidget : public KisWdgAutoBrush
{
    Q_OBJECT

public:

    KisAutoBrushWidget(int maxBrushSize,
                       KisAutoBrushModel *model,
                       QWidget *parent, const char* name);
    ~KisAutoBrushWidget() override;

    KisBrushSP brush();

private Q_SLOTS:
    void setStackedWidget(int);

    void slotCurveWidgetChanged();
    void slotCurvePropertyChanged(const QString &value);

    void slotUpdateBrushPreview();

protected:
    void resizeEvent(QResizeEvent *) override;

private:
    QScopedPointer<KisAspectRatioLocker> m_fadeAspectLocker;

    struct Private;
    const QScopedPointer<Private> m_d;
};


#endif
