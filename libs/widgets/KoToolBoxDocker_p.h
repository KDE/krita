/*
 * Copyright (c) 2005 Boudewijn Rempt <boud@valdyas.org>
 * Copyright (c) 2005-2008 Thomas Zander <zander@kde.org>
 * Copyright (c) 2009 Peter Simonsson <peter.simonsson@gmail.com>
 * Copyright (c) 2010 Cyrille Berger <cberger@cberger.net>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
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
    void resizeEvent(QResizeEvent *event);

protected Q_SLOTS:
    void updateToolBoxOrientation(Qt::DockWidgetArea area);
    void updateFloating(bool);

private:
    KoToolBox *m_toolBox;
    KoToolBoxScrollArea *m_scrollArea;
};

#endif // _KO_TOOLBOX_DOCKER_H_
