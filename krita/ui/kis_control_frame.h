/*
 *  kis_control_frame.h - part of Krita
 *
 *  Copyright (c) 1999 Matthias Elter  <elter@kde.org>
 *  Copyright (c) 2003 Patrick Julien  <freak@codepimps.org>
 *  Copyright (c) 2004 Sven Langkamp  <sven.langkamp@gmail.com>
 *  Copyright (c) 2003-2008 Boudewijn Rempt <boud@valdyas.org>
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
#ifndef __kis_control_frame_h__
#define __kis_control_frame_h__

#include <QMenu>
#include <QKeyEvent>
#include <QObject>

class QWidget;
class QTabWidget;
class QTableWidgetItem;
class QPushButton;

class KToolBar;

class KoResourceItem;

class KoAbstractGradient;
class KisGradientChooser;
class KoResourceItemChooser;
class KisPaintopBox;
class KisView2;
class KisIconWidget;
class KisPattern;
class KXmlGuiWindow;

/**
 *   Control Frame - status display with access to
 *   color selector, gradient, patterns, and paintop presets
 */
class KisControlFrame : public QObject
{
    Q_OBJECT

public:

    KisControlFrame(KisView2 * view,  const char *name = 0);
    virtual ~KisControlFrame() {}

    KisPaintopBox* paintopBox() {
        return m_paintopBox;
    }

public slots:

    void slotSetPattern(KisPattern * pattern);
    void slotSetGradient(KoAbstractGradient * gradient);
    void slotSaveToFavouriteBrushes();

private:

    void createPatternsChooser(KisView2 * view);
    void createGradientsChooser(KisView2 * view);

private:

    QFont m_font;
    KisView2 * m_view;

    QTabWidget * m_gradientTab;
    QTabWidget * m_patternsTab;

    KisIconWidget *m_patternWidget;
    KisIconWidget *m_gradientWidget;

    QWidget * m_patternChooserPopup;
    QWidget * m_gradientChooserPopup;

    KisGradientChooser * m_gradientChooser;

    KisPaintopBox * m_paintopBox;
    QPushButton* m_paletteButton;
};

#endif

