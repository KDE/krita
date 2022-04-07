/*
 *  kis_control_frame.h - part of Krita
 *
 *  SPDX-FileCopyrightText: 1999 Matthias Elter <elter@kde.org>
 *  SPDX-FileCopyrightText: 2003 Patrick Julien <freak@codepimps.org>
 *  SPDX-FileCopyrightText: 2004 Sven Langkamp <sven.langkamp@gmail.com>
 *  SPDX-FileCopyrightText: 2003-2008 Boudewijn Rempt <boud@valdyas.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef __kis_control_frame_h__
#define __kis_control_frame_h__

#include <QMenu>
#include <QObject>

#include <KoPattern.h>
#include <KoAbstractGradient.h>
#include <KoCheckerBoardPainter.h>

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

    KisViewManager *m_viewManager {nullptr};

    QTabWidget *m_gradientTab {nullptr};
    QTabWidget *m_patternsTab {nullptr};

    KisIconWidget *m_patternWidget {nullptr};
    KisIconWidget *m_gradientWidget {nullptr};

    QWidget *m_patternChooserPopup {nullptr};
    QWidget *m_gradientChooserPopup {nullptr};

    KisGradientChooser *m_gradientChooser {nullptr};
    KisPatternChooser *m_patternChooser {nullptr};

    KisPaintopBox *m_paintopBox {nullptr};

    KoDualColorButton *m_dual {nullptr};
    KoCheckerBoardPainter m_checkersPainter;

};

#endif

