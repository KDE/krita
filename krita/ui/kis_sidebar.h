/*
 *  kis_sidebar.h - part of Krayon
 *
 *  Copyright (c) 1999 Matthias Elter  <elter@kde.org>
 *  Copyright (c) 2003 Patrick Julien  <freak@codepimps.org>
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
#ifndef __kis_sidebar_h__
#define __kis_sidebar_h__

#include <qframe.h>
#include <qptrlist.h>
#include <kdockwidget.h>
#include <kdualcolorbutton.h>
#include <koColor.h>
#include <koFrameButton.h>
#include "kfloatingdialog.h"

class KDualColorButton;
class KoIconItem;
class KisIconWidget;
class KisGradientWidget;

class KisBrush;
class KisPattern;
class KisColorChooser;
class KoColorChooser;
class ControlFrame;

enum ActiveColor { ac_Foreground, ac_Background};

/*
    Control Frame - status display with access to
    color selector, brushes, patterns, and preview
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

signals:
	void fgColorChanged(const KoColor& c);
	void bgColorChanged(const KoColor& c);
	void activeColorChanged(ActiveColor ac);

protected:
	virtual void resizeEvent(QResizeEvent *e);

protected slots:
	void slotFGColorSelected(const QColor& c);
	void slotBGColorSelected(const QColor& c);
	void slotActiveColorChanged(KDualColorButton::DualColor dc);

private:
	KDualColorButton  *m_pColorButton;
	KisIconWidget    *m_pBrushWidget;
	KisIconWidget  *m_pPatternWidget;
	KisGradientWidget *m_pGradientWidget;  
};

class ColorChooserFrame : public QFrame
{
  Q_OBJECT

 public:
    ColorChooserFrame( QWidget* parent = 0, const char* name = 0 );

 public slots:
    void slotSetFGColor(const KoColor&);
    void slotSetBGColor(const KoColor&);
    void slotSetActiveColor( ActiveColor );

 signals:
    void colorChanged(const KoColor&);

 protected:
    virtual void resizeEvent ( QResizeEvent * );

 protected slots:
    void slotColorSelected(const KoColor&);

 private:
    KoColorChooser   *m_pColorChooser;
    KoColor m_fg;
    KoColor m_bg;
};


class DockFrame : public QFrame
{
	typedef QFrame super;
  Q_OBJECT

 public:
    DockFrame( QWidget* parent = 0, const char* name = 0 );

 public:
    void plug (QWidget* w);
    void unplug (QWidget* w);

 public slots:
    void slotActivateTab(const QString& tab);

 protected:
    virtual void resizeEvent ( QResizeEvent * );

 private:
    QPtrList<QWidget>         m_wlst;
    QPtrList<KoFrameButton>  m_blst;
};

class KisSideBar : public KFloatingDialog {
	typedef KFloatingDialog super;
	Q_OBJECT

public:
	KisSideBar(QWidget *parent = 0, const char *name = 0);
	virtual ~KisSideBar();

public:
	void plug(QWidget *w);
	void unplug(QWidget *w);
	QWidget *dockFrame();
    
public slots:
	void slotSetFGColor(const KoColor&);
	void slotSetBGColor(const KoColor&);

	void slotSetBrush(KoIconItem *item);
	void slotSetPattern(KoIconItem *item);

	void slotActivateTab(const QString& tab) { m_dockFrame->slotActivateTab(tab); }
	void slotHideChooserFrame();

signals:
	void fgColorChanged(const KoColor&);
	void bgColorChanged(const KoColor&);

protected:
	virtual void resizeEvent ( QResizeEvent * );
	virtual void closeEvent ( QCloseEvent * );

protected slots:
	void slotColorChooserColorSelected(const KoColor&);
	void slotControlFGColorSelected(const KoColor&);
	void slotControlBGColorSelected(const KoColor&);
	void slotControlActiveColorChanged(ActiveColor);

private:
	ControlFrame *m_pControlFrame; 
	ColorChooserFrame *m_pColorChooserFrame;  
	DockFrame *m_dockFrame;
};

#endif

