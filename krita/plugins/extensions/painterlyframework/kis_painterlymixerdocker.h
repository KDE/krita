/* This file is part of the KDE project

   Copyright (C) 2007 Emanuele Tamponi <emanuele@valinor.it>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#ifndef KIS_PAINTERLY_MIXER_DOCKER_H_
#define KIS_PAINTERLY_MIXER_DOCKER_H_

#include <QDockWidget>
#include <KoDockFactory.h>
#include <KoCanvasObserver.h>

class KoCanvasBase;
class KoColor;
class KisPainterlyMixer;

class KisPainterlyMixerDocker : public QDockWidget, public KoCanvasObserver
{
    Q_OBJECT

public:
    KisPainterlyMixerDocker();
    virtual ~KisPainterlyMixerDocker();

    /// reimplemented from KoCanvasObserver
    virtual void setCanvas(KoCanvasBase *canvas);

private slots:

    void colorChanged(const KoColor& color);
    void resourceChanged(int key, const QVariant& value);

private:
    KisPainterlyMixer *m_painterlyMixer;
    KoCanvasBase *m_currentCanvas;
};


class KisPainterlyMixerDockerFactory : public KoDockFactory
{
public:
    KisPainterlyMixerDockerFactory() {}
    ~KisPainterlyMixerDockerFactory() {}

    QString id() const;

    QDockWidget *createDockWidget()
    {
        KisPainterlyMixerDocker* widget = new KisPainterlyMixerDocker();
        widget->setObjectName(id());
        return widget;
    }

    DockPosition defaultDockPosition() const { return DockMinimized; }
};

#endif // KIS_PAINTERLY_MIXER_DOCKER_H_
