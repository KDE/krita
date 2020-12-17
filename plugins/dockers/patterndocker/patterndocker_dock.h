/*
 *  SPDX-FileCopyrightText: 2009 Cyrille Berger <cberger@cberger.net>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef _PATTERN_DOCK_H_
#define _PATTERN_DOCK_H_

#include <QDockWidget>
#include <kis_mainwindow_observer.h>

#include <KoPattern.h>

class KisPatternChooser;

class PatternDockerDock : public QDockWidget, public KisMainwindowObserver {
    Q_OBJECT
public:
    PatternDockerDock( );

    void setViewManager(KisViewManager* kisview) override;
    void setCanvas(KoCanvasBase *canvas) override;
    void unsetCanvas() override;

    QString observerName() override { return "PatternDockerDock"; }
public Q_SLOTS:
    void patternChanged(KoPatternSP pattern);
private Q_SLOTS:

private:
    KisPatternChooser* m_patternChooser;
};


#endif
