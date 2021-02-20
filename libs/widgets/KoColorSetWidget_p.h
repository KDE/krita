/* This file is part of the KDE project
   SPDX-FileCopyrightText: 2007, 2012 C. Boemann <cbo@boemann.dk>
   SPDX-FileCopyrightText: 2007-2008 Fredy Yanardi <fyanardi@gmail.com>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/
#ifndef KoColorSetWidget_p_h
#define KoColorSetWidget_p_h

#include "KoColorSetWidget.h"

#include <QTimer>
#include <QApplication>
#include <QSize>
#include <QToolButton>
#include <QHBoxLayout>
#include <QCheckBox>
#include <QFrame>
#include <QLabel>
#include <QMouseEvent>
#include <QMenu>
#include <QWidgetAction>
#include <QDir>
#include <QScrollArea>
#include <QComboBox>

#include <klocalizedstring.h>
#include <WidgetsDebug.h>
#include <KoResourceServer.h>
#include <KisPopupButton.h>
#include <KisPaletteChooser.h>
#include <KisPaletteComboBox.h>

#include <resources/KoColorSet.h>
#include <KoColorDisplayRendererInterface.h>

class KoColorPatch;
class KisPaletteView;

class Q_DECL_HIDDEN KoColorSetWidget::KoColorSetWidgetPrivate {
public:
    KoColorSetWidget *thePublic;
    KoColorSetSP colorSet;

    KisPaletteView *paletteView;
    KisPaletteChooser *paletteChooser;
    KisPopupButton *paletteChooserButton;

    QVBoxLayout *mainLayout;
    QVBoxLayout *colorSetLayout;
    QHBoxLayout *recentsLayout;
    QHBoxLayout *bottomLayout;

    KoColorPatch *recentPatches[6];
    QToolButton *addRemoveButton;
    KisPaletteComboBox *colorNameCmb;
    int numRecents;

    const KoColorDisplayRendererInterface *displayRenderer;
    KoResourceServer<KoColorSet> *rServer;

    void addRecent(const KoColor &);
    void activateRecent(int i);
    void addRemoveColors();
};

#endif
