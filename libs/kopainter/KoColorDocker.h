/* This file is part of the KDE project
   Made by Tomislav Lukman (tomislav.lukman@ck.tel.hr)
   Copyright (C) 2002 - 2005, The Karbon Developers
   Copyright (C) 2006 Jan Hambecht <jaham@gmx.net>
   Copyright (C) 2006 Boudewijn Rempt <boud@valdyas.org>

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

#ifndef __KO_COLOR_DOCKER_H__
#define __KO_COLOR_DOCKER_H__

#include <QMouseEvent>
#include <KoColor.h>
#include <KoDockFactory.h>

class QDockWidget;
class KarbonView;
class KoUniColorChooser;

/**
   Dock widget that contains a color selector. Currently this is the
   big, unified color selector, but I would prefer a small strip with
   just a band of colors -- and have the unified color selector popup.
   This just takes a tad too much place.
*/
class KoColorDocker : public QDockWidget
{
	Q_OBJECT

public:
	 KoColorDocker();
	 virtual ~KoColorDocker();

	 KoColor color() { return m_color; }

	virtual void update();

signals:
	void fgColorChanged( const QColor &c );
	void bgColorChanged( const QColor &c );

private slots:
	void updateColor( const KoColor &c );
	void updateFgColor(const KoColor &c);
	void updateBgColor(const KoColor &c);

private:
	virtual void mouseReleaseEvent( QMouseEvent *e );

	KoUniColorChooser *m_colorChooser;
	KoColor m_color;
	KoColor m_oldColor;
};


class KoColorDockerFactory : public KoDockFactory
{
public:
    KoColorDockerFactory();

    virtual QString dockId() const;
    virtual Qt::DockWidgetArea defaultDockWidgetArea() const;
    virtual KoColorDocker * createDockWidget();
};

#endif

