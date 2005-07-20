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
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
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
class KisAutoBrush;
class KisTextBrush;
class KisAutoGradient;
class KisHSVWidget;
class KisRGBWidget;
class KisGrayWidget;
class KisPaletteWidget;
class KisResourceMediator;
//class KisControlFrame;

class KritaDefaultDockers : public KParts::Plugin
{
	Q_OBJECT

public:
	KritaDefaultDockers(QObject *parent, const char *name, const QStringList &);
	virtual ~KritaDefaultDockers();

private:
	//void createControlFrame(KisView * view);
	void createBirdEyeBox(KisView * view);
	void createAutoBrush(KisView * view);
	void createTextBrush(KisView * view);
	void createAutoGradient(KisView * view);
	void createHSVWidget(KisView * view);
	void createRGBWidget(KisView * view);
	void createGrayWidget(KisView * view);
	void createPaletteWidget(KisView * view);	
	void createPatternWidget(KisView * view);
	void createBrushesWidget(KisView * view);
	void createGradientsWidget(KisView * view);
#if 0	
public XXXslots:
        void slotBrushChanged(KisBrush * brush);
        void slotGradientChanged(KisGradient * gradient);
        void slotPatternChanged(KisPattern * pattern);
#endif
private:

	KisView * m_view;

	KoPaletteManager * m_paletteManager;
#if 0
	KisControlFrame * m_controlWidget;
#endif
        KisBirdEyeBox * m_birdEyeBox;

        KisAutobrush *m_autobrush;
        KisTextBrush *m_textBrush;
        KisAutogradient* m_autogradient;

        KisHSVWidget *m_hsvwidget;
        KisRGBWidget *m_rgbwidget;
        KisGrayWidget *m_graywidget;
        KisPaletteWidget *m_palettewidget;

        KisResourceMediator *m_brushMediator;
        KisResourceMediator *m_patternMediator;
        KisResourceMediator *m_gradientMediator;

};

#endif
