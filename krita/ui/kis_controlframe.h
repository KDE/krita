/*
 *  kis_controlframe.h - part of Krita
 *
 *  Copyright (c) 1999 Matthias Elter  <elter@kde.org>
 *  Copyright (c) 2003 Patrick Julien  <freak@codepimps.org>
 *  Copyright (c) 2004 Sven Langkamp  <longamp@reallygood.de>
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
#ifndef __kis_controlframe_h__
#define __kis_controlframe_h__

#include <qframe.h>
#include <qtabwidget.h>
#include <qdockwindow.h>
#include <kdualcolorbutton.h>
#include <koColor.h>
#include <koFrameButton.h>

class KDualColorButton;
class KoIconItem;
class KisIconWidget;
class KisGradientWidget;

class KisBrush;
class KisPattern;
class KisGradient;
class KoColorChooser;
class ControlFrame;
class KisRGBWidget;
class KisHSVWidget;
class KisGrayWidget;

enum ActiveColor { ac_Foreground, ac_Background};

/**
 *   Control Frame - status display with access to
 *   color selector, brushes, patterns, and preview
 */
class ControlFrame : public QFrame {
	Q_OBJECT

public:
	ControlFrame(QWidget *parent = 0, const char *name = 0 );
	ActiveColor activeColor();

public slots:
	void slotSetFGColor(const KoColor& c);
	void slotSetBGColor(const KoColor& c);

	void slotSetBrush(KoIconItem *item);
	void slotSetPattern(KoIconItem *item);
	void slotSetGradient(KoIconItem *item);

signals:
	void fgColorChanged(const KoColor& c);
	void bgColorChanged(const KoColor& c);
	void activeColorChanged(ActiveColor ac);

protected slots:
	void slotFGColorSelected(const QColor& c);
	void slotBGColorSelected(const QColor& c);
	void slotActiveColorChanged(KDualColorButton::DualColor dc);

private:
	KDualColorButton  *m_pColorButton;
	KisIconWidget    *m_pBrushWidget;
	KisIconWidget  *m_pPatternWidget;
	KisIconWidget  *m_pGradientWidget;
};

#endif

