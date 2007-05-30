/* This file is part of the KDE project
   Made by Emanuele Tamponi (emanuele@valinor.it)
   Copyright (C) 2007 Emanuele Tamponi

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

#ifndef __KIS_PAINTERLY_MIXER_DOCKER_H__
#define __KIS_PAINTERLY_MIXER_DOCKER_H__

#include <QDockWidget>

#include <KoDockFactory.h>
#include "koguiutils_export.h"

class KisPainterlyMixer;

class KOGUIUTILS_EXPORT KisPainterlyMixerDocker : public QDockWidget
{
    Q_OBJECT

public:
    KisPainterlyMixerDocker();
    virtual ~KisPainterlyMixerDocker();

private:
    KisPainterlyMixer *m_painterlyMixer;
};


class KOGUIUTILS_EXPORT KisPainterlyMixerDockerFactory : public KoDockFactory
{
public:
    KisPainterlyMixerDockerFactory() {}
    ~KisPainterlyMixerDockerFactory() {}

    QString id() const;
    Qt::DockWidgetArea defaultDockWidgetArea() const;
    QDockWidget* createDockWidget();
};

#endif
