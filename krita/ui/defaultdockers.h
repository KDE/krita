/*
 * This file is part of Krita
 *
 * Copyright (c) 2004 Cyrille Berger <cberger@cberger.net>
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

#ifndef DEFAULT_DOCKERS_H
#define DEFAULT_DOCKERS_H

#include <qobject.h>
#include <qstringlist.h>
#include <kparts/plugin.h>

class KisView;
class KisBrush;
class KisPattern;
class KisBirdEyeBox;
class KisHSVWidget;
class KisRGBWidget;
class KisGrayWidget;
class KisPaletteWidget;
class KisResourceMediator;
class KisControlFrame;

class KritaDefaultDockers : public KParts::Plugin
{
    Q_OBJECT

public:
    KritaDefaultDockers(QObject *parent, const char *name, const QStringList &);
    virtual ~KritaDefaultDockers();

private:
    void createControlFrame(KisView * view);
    void createBirdEyeBox(KisView * view);
    void createHSVWidget(KisView * view);
    void createRGBWidget(KisView * view);
    void createPaletteWidget(KisView * view);
    void createGrayWidget(KisView * view);
private:

    KisView * m_view;

    KoPaletteManager * m_paletteManager;

    KisControlFrame * m_controlWidget;

    KisBirdEyeBox * m_birdEyeBox;

    KisHSVWidget *m_hsvwidget;
    KisRGBWidget *m_rgbwidget;
    KisGrayWidget *m_graywidget;
    KisPaletteWidget *m_palettewidget;

};

#endif
