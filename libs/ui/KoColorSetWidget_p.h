/* This file is part of the KDE project
   Copyright (c) 2007, 2012 C. Boemann <cbo@boemann.dk>
   Copyright (c) 2007-2008 Fredy Yanardi <fyanardi@gmail.com>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
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
#include <QPointer>
#include <QScrollArea>
#include <QComboBox>

#include <klocalizedstring.h>
#include <WidgetsDebug.h>

#include <resources/KoColorSet.h>
#include <KoColorDisplayRendererInterface.h>

class KoColorPatch;

class Q_DECL_HIDDEN KoColorSetWidget::KoColorSetWidgetPrivate {
public:
    KoColorSetWidget *thePublic;
    QPointer<KoColorSet> colorSet;
    QTimer m_timer;
    QVBoxLayout *mainLayout;
    bool firstShowOfContainer;
    QWidget *colorSetContainer;
    QScrollArea *scrollArea;
    QVBoxLayout *colorSetLayout;
    QHBoxLayout *recentsLayout;
    KoColorPatch *recentPatches[6];
    QToolButton *addRemoveButton;
    QComboBox *colorNameCmb;
    QStringList colornames;
    int numRecents;

    void colorTriggered(KoColorPatch *patch);
    void addRecent(const KoColor &);
    void activateRecent(int i);
    void fillColors();
    void addRemoveColors();
    void setColorFromString(QString s);

    QList<KoColorPatch*> patchWidgetList;
    const KoColorDisplayRendererInterface *displayRenderer;

};

#endif
