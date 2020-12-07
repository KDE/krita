/*
 * Copyright (c) 2005 Boudewijn Rempt <boud@valdyas.org>
 * Copyright (c) 2005-2008 Thomas Zander <zander@kde.org>
 * Copyright (c) 2009 Peter Simonsson <peter.simonsson@gmail.com>
 * Copyright (c) 2010 Cyrille Berger <cberger@cberger.net>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */
#ifndef _KO_TOOLBOX_DOCKER_H_
#define _KO_TOOLBOX_DOCKER_H_

#include <KoCanvasObserverBase.h>

#include <QDockWidget>

class KoCanvasBase;
class KoToolBox;
class KoToolBoxScrollArea;

class KoToolBoxDocker : public QDockWidget, public KoCanvasObserverBase
{
    Q_OBJECT
public:
    explicit KoToolBoxDocker(KoToolBox *toolBox);

    /// reimplemented from KoCanvasObserverBase
    void setCanvas(KoCanvasBase *canvas) override;
    void unsetCanvas() override;
    QString observerName() override { return "KoToolBoxDocker"; }

protected:
    void resizeEvent(QResizeEvent *event) override;

protected Q_SLOTS:
    void updateToolBoxOrientation(Qt::DockWidgetArea area);
    void updateFloating(bool);

private:
    KoToolBox *m_toolBox;
    KoToolBoxScrollArea *m_scrollArea;
};

#endif // _KO_TOOLBOX_DOCKER_H_
