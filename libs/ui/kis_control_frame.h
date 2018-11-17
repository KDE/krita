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

#include <KoPattern.h>
#include <KoAbstractGradient.h>

class QWidget;
class QTabWidget;

class KisGradientChooser;
class KisPatternChooser;
class KisPaintopBox;
class KisViewManager;
class KisIconWidget;
class KoDualColorButton;

/**
 *   Control Frame - status display with access to
 *   color selector, gradient, patterns, and paintop presets
 */
class KisControlFrame : public QObject
{
    Q_OBJECT

public:

    KisControlFrame(KisViewManager *view, QWidget *parent = 0, const char *name = 0);
    ~KisControlFrame() override {}
    void setup(QWidget *parent);

    KisPaintopBox* paintopBox() {
        return m_paintopBox;
    }

private Q_SLOTS:

    void slotSetPattern(KoPatternSP pattern);
    void slotSetGradient(KoAbstractGradientSP gradient);
    void slotUpdateDisplayRenderer();

private:

    void createPatternsChooser(KisViewManager * view);
    void createGradientsChooser(KisViewManager * view);

private:

    QFont m_font;
    KisViewManager *m_viewManager;

    QTabWidget *m_gradientTab;
    QTabWidget *m_patternsTab;

    KisIconWidget *m_patternWidget;
    KisIconWidget *m_gradientWidget;

    QWidget *m_patternChooserPopup;
    QWidget *m_gradientChooserPopup;

    KisGradientChooser *m_gradientChooser;
    KisPatternChooser *m_patternChooser;

    KisPaintopBox *m_paintopBox;

    KoDualColorButton *m_dual;

};

#endif

