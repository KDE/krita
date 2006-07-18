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
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */
#ifndef __kis_controlframe_h__
#define __kis_controlframe_h__

#include <QMenu>
#include <QKeyEvent>

#include <ktoolbar.h>

class QWidget;
class QTimer;
class QTabWidget;

class KToolBar;

class QTableWidgetItem;
class KisIconWidget;
class KisGradientWidget;

class KisAutobrush;
class KisAutogradient;
class KisBrush;
class KisBrushChooser;
class KisGradient;
class KisGradientChooser;
class KisItemChooser;
class KisPattern;
class KisResourceMediator;
class KisPaintopBox;
class KisView;

class KisPopupFrame : public QMenu {

    Q_OBJECT

public:

    KisPopupFrame(QWidget * parent, const char * name = 0);
    virtual void keyPressEvent(QKeyEvent *);

public:

    void setChooser(KisItemChooser * chooser) { m_chooser = chooser; };
    KisItemChooser * chooser() { return m_chooser; };

private:
    KisItemChooser * m_chooser;
};


/**
 *   Control Frame - status display with access to
 *   color selector, brushes, patterns, and preview
 */
class KisControlFrame : public QObject  //: public KToolBar
{
    Q_OBJECT

public:
    KisControlFrame(KMainWindow * window, KisView * view, const char *name = 0 );
    virtual ~KisControlFrame() {};

public slots:

    void slotSetBrush(QTableWidgetItem *item);
    void slotSetPattern(QTableWidgetItem *item);
    void slotSetGradient(QTableWidgetItem *item);

    void slotBrushChanged(KisBrush * brush);
    void slotPatternChanged(KisPattern * pattern);
    void slotGradientChanged(KisGradient * gradient);

private:

    void createBrushesChooser(KisView * view);
    void createPatternsChooser(KisView * view);
    void createGradientsChooser(KisView * view);


private:
    QFont m_font;
    KisView * m_view;

    QTabWidget * m_brushesTab;
    QTabWidget * m_gradientTab;
    QTabWidget * m_patternsTab;

    KisIconWidget *m_brushWidget;
    KisIconWidget *m_patternWidget;
    KisIconWidget *m_gradientWidget;

    KisPopupFrame * m_brushChooserPopup;
    KisPopupFrame * m_patternChooserPopup;
    KisPopupFrame * m_gradientChooserPopup;

    KisResourceMediator *m_brushMediator;
    KisResourceMediator *m_patternMediator;
    KisResourceMediator *m_gradientMediator;


    KisAutobrush * m_autobrush;
    KisBrushChooser * m_brushChooser;
    KisGradientChooser * m_gradientChooser;

    KisPaintopBox * m_paintopBox;
};

#endif

