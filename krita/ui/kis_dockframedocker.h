/*
 *  kis_dockframedocker.h - part of Krita
 *
 *  Copyright (c) 1999 Matthias Elter  <elter@kde.org>
 *  Copyright (c) 2003 Patrick Julien  <freak@codepimps.org>
 *  Copyright (c) 2004 Sven Langkamp  <longamp@reallygood.de>
 *  Copyright (c) 2004 Boudewijn Remot <boud@valdyas.org>
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
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */
#ifndef __kis_dockframedocker_h__
#define __kis_dockframedocker_h__

#include <qdockwindow.h>

#include "kis_basedocker.h"

class WdgDockerTab;

class KisDockFrameDocker : public KisBaseDocker
{
        Q_OBJECT

public:
        KisDockFrameDocker ( QWidget* parent = 0, const char* name = 0 );
        ~KisDockFrameDocker();

        void plug(QWidget *w);
        void unplug(QWidget *w);
        void showPage(QWidget *w);
	void setCaption(const QString & caption);

public slots:

	void shade(bool toggle);
	void slotPlaceChanged(QDockWindow::Place p);

private:
        WdgDockerTab * m_page;
	bool m_docked;
};


#endif // __kis_dockframedocker_h__
