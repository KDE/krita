/*
 *  kis_sidebar.h - part of Krayon
 *
 *  Copyright (c) 1999 Matthias Elter  <elter@kde.org>
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
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#ifndef __kis_sidebar_h__
#define __kis_sidebar_h__

#include <qframe.h>
#include <qptrlist.h>

#include <kdualcolorbutton.h>

#include <koColor.h>
#include <koFrameButton.h>

#include "kfloatingdialog.h"

class KDualColorButton;
class KisKrayonWidget;
class KisBrushWidget;
class KisPatternWidget;
class KisGradientWidget;
class KisPreviewWidget;
class KisKrayon;
class KisBrush;
class KisPattern;
class KisColorChooser;
class KoColorChooser;

enum ActiveColor { ac_Foreground, ac_Background};

class TopTitleFrame : public QFrame
{
  Q_OBJECT

 public:
    TopTitleFrame( QWidget* parent = 0, const char* name = 0 );

 signals:
    void hideClicked();

 protected:
    virtual void resizeEvent ( QResizeEvent * );

 protected slots:
    void slotHideClicked();

 private:
    KoFrameButton *m_pHideButton, *m_pTitleButton;
    QFrame *m_pEmptyFrame;
};


class TopColorFrame : public QFrame
{
  Q_OBJECT

 public:
    TopColorFrame( QWidget* parent = 0, const char* name = 0 );

 signals:
    void hideClicked();
    void greyClicked();
    void rgbClicked();
    void hsbClicked();
    void cmykClicked();
    void labClicked();

 protected:
    virtual void resizeEvent ( QResizeEvent * );

 protected slots:
    void slotHideClicked();
    void slotGreyClicked();
    void slotRGBClicked();
    void slotHSBClicked();
    void slotCMYKClicked();
    void slotLABClicked();

 private:
    KoFrameButton *m_pHideButton, *m_pGreyButton, *m_pRGBButton, 
    *m_pHSBButton, *m_pCMYKButton, *m_pLABButton;
    QFrame *m_pEmptyFrame;
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


class ControlFrame : public QFrame
{
    Q_OBJECT

 public:
     ControlFrame( QWidget* parent = 0, const char* name = 0 );
     ActiveColor activeColor();

 public slots:
    void slotSetFGColor(const KoColor&);
    void slotSetBGColor(const KoColor&);

    void slotSetKrayon(KisKrayon&);
    void slotSetBrush(KisBrush&);
    void slotSetPattern(KisPattern&);

 signals:
    void fgColorChanged(const KoColor&);
    void bgColorChanged(const KoColor&);
    void activeColorChanged(ActiveColor);

 protected:
    virtual void resizeEvent ( QResizeEvent * );

 protected slots:
    void slotFGColorSelected(const QColor&);
    void slotBGColorSelected(const QColor&);
    void slotActiveColorChanged(KDualColorButton::DualColor );

 private:
    KDualColorButton  *m_pColorButton;
    KisKrayonWidget   *m_pKrayonWidget;  
    KisBrushWidget    *m_pBrushWidget;
    KisPatternWidget  *m_pPatternWidget;
    KisGradientWidget *m_pGradientWidget;  
    KisPreviewWidget  *m_pPreviewWidget;  
};


class DockFrame : public QFrame
{
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

class KisSideBar : public KFloatingDialog
{
    Q_OBJECT

 public:
    KisSideBar( QWidget* parent = 0, const char* name = 0 );

    void plug (QWidget* w) { m_dockFrame->plug(w); }
    void unplug (QWidget* w) { m_dockFrame->unplug(w); }
    QWidget *dockFrame() { return m_dockFrame; }
    
 public slots:
    void slotSetFGColor(const KoColor&);
    void slotSetBGColor(const KoColor&);

    void slotSetKrayon(KisKrayon&);
    void slotSetBrush(KisBrush&);
    void slotSetPattern(KisPattern&);
  
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
    TopTitleFrame       *m_pTopTitleFrame;
    ControlFrame        *m_pControlFrame; 
    TopColorFrame       *m_pTopColorFrame;       
    ColorChooserFrame   *m_pColorChooserFrame;  
    DockFrame *m_dockFrame;
};

#endif
