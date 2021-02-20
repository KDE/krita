/* This file is part of the KDE project
   SPDX-FileCopyrightText: 2007, 2012 C. Boemann <cbo@boemann.dk>
   SPDX-FileCopyrightText: 2007-2008 Fredy Yanardi <fyanardi@gmail.com>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/
#include "KoColorSetWidget.h"
#include "KoColorSetWidget_p.h"

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
#include <QGroupBox>
#include <QVBoxLayout>

#include <klocalizedstring.h>
#include <ksharedconfig.h>

#include <resources/KoColorSet.h>
#include <KoColorPatch.h>
#include <KoColorSpaceRegistry.h>
#include <KoResourceServer.h>
#include <KoResourceServerProvider.h>

#include <kis_palette_view.h>
#include <KisPaletteDelegate.h>
#include <KisPaletteModel.h>
#include <kis_icon_utils.h>

void KoColorSetWidget::KoColorSetWidgetPrivate::addRecent(const KoColor &color)
{
    if(numRecents < 6) {
        recentPatches[numRecents] = new KoColorPatch(thePublic);
        recentPatches[numRecents]->setFrameShape(QFrame::StyledPanel);
        recentPatches[numRecents]->setDisplayRenderer(displayRenderer);
        recentsLayout->insertWidget(numRecents + 1, recentPatches[numRecents]);
        connect(recentPatches[numRecents], SIGNAL(triggered(KoColorPatch*)), thePublic, SLOT(slotPatchTriggered(KoColorPatch*)));
        numRecents++;
    }
    // shift colors to the right
    for (int i = numRecents- 1; i >0; i--) {
        recentPatches[i]->setColor(recentPatches[i-1]->color());
    }

    //Finally set the recent color
    recentPatches[0]->setColor(color);
}

void KoColorSetWidget::KoColorSetWidgetPrivate::activateRecent(int i)
{
    KoColor color = recentPatches[i]->color();

    while (i >0) {
        recentPatches[i]->setColor(recentPatches[i-1]->color());
        i--;
    }
    recentPatches[0]->setColor(color);
}

KoColorSetWidget::KoColorSetWidget(QWidget *parent)
   : QFrame(parent)
   , d(new KoColorSetWidgetPrivate())
{
    d->thePublic = this;

    d->numRecents = 0;
    d->recentsLayout = new QHBoxLayout;
    d->recentsLayout->setMargin(0);
    d->recentsLayout->addWidget(new QLabel(i18n("Recent:")));
    d->recentsLayout->addStretch(1);

    KoColor color(KoColorSpaceRegistry::instance()->rgb8());
    color.fromQColor(QColor(128,0,0));
    d->addRecent(color);

    d->paletteView = new KisPaletteView(this);
    KisPaletteModel *paletteModel = new KisPaletteModel(d->paletteView);
    d->paletteView->setPaletteModel(paletteModel);
    d->paletteView->setDisplayRenderer(d->displayRenderer);

    d->paletteChooser = new KisPaletteChooser(this);
    d->paletteChooserButton = new KisPopupButton(this);
    d->paletteChooserButton->setPopupWidget(d->paletteChooser);
    d->paletteChooserButton->setIcon(KisIconUtils::loadIcon("hi16-palette_library"));
    d->paletteChooserButton->setToolTip(i18n("Choose palette"));

    d->colorNameCmb = new KisPaletteComboBox(this);
    d->colorNameCmb->setCompanionView(d->paletteView);

    d->bottomLayout = new QHBoxLayout;
    d->bottomLayout->addWidget(d->paletteChooserButton);
    d->bottomLayout->addWidget(d->colorNameCmb);
    d->bottomLayout->setStretch(0, 0); // minimize chooser button
    d->bottomLayout->setStretch(1, 1); // maximize color name cmb

    d->mainLayout = new QVBoxLayout(this);
    d->mainLayout->setMargin(4);
    d->mainLayout->setSpacing(2);
    d->mainLayout->addLayout(d->recentsLayout);
    d->mainLayout->addWidget(d->paletteView);
    d->mainLayout->addLayout(d->bottomLayout);

    connect(d->paletteChooser, SIGNAL(sigPaletteSelected(KoColorSetSP)), SLOT(slotPaletteChoosen(KoColorSetSP)));
    connect(d->paletteView, SIGNAL(sigColorSelected(KoColor)), SLOT(slotColorSelectedByPalette(KoColor)));
    connect(d->colorNameCmb, SIGNAL(sigColorSelected(KoColor)), SLOT(slotNameListSelection(KoColor)));

    d->rServer = KoResourceServerProvider::instance()->paletteServer();
    KoColorSetSP defaultColorSet = d->rServer->resourceByName("Default");
    if (!defaultColorSet && d->rServer->resourceCount() > 0) {
        defaultColorSet = d->rServer->firstResource();
    }
    setColorSet(defaultColorSet);
}

KoColorSetWidget::~KoColorSetWidget()
{
    delete d;
}

void KoColorSetWidget::setColorSet(KoColorSetSP colorSet)
{
    if (!colorSet) return;
    if (colorSet == d->colorSet) return;

    d->paletteView->paletteModel()->setPalette(colorSet);
    d->colorSet = colorSet;
}

KoColorSetSP KoColorSetWidget::colorSet()
{
    return d->colorSet;
}

void KoColorSetWidget::setDisplayRenderer(const KoColorDisplayRendererInterface *displayRenderer)
{
    if (displayRenderer) {
        d->displayRenderer = displayRenderer;
        for (int i=0; i<6; i++) {
            if (d->recentPatches[i]) {
                d->recentPatches[i]->setDisplayRenderer(displayRenderer);
            }
        }
    }
}

void KoColorSetWidget::resizeEvent(QResizeEvent *event)
{
    emit widgetSizeChanged(event->size());
    QFrame::resizeEvent(event);
}

void KoColorSetWidget::slotColorSelectedByPalette(const KoColor &color)
{
    emit colorChanged(color, true);
    d->addRecent(color);
}

void KoColorSetWidget::slotPatchTriggered(KoColorPatch *patch)
{
    emit colorChanged(patch->color(), true);

    int i;

    for (i = 0; i < d->numRecents; i++) {
        if(patch == d->recentPatches[i]) {
            d->activateRecent(i);
            break;
        }
    }

    if (i == d->numRecents) { // we didn't find it above
        d->addRecent(patch->color());
    }
}

void KoColorSetWidget::slotPaletteChoosen(KoColorSetSP colorSet)
{
    d->colorSet = colorSet;
    d->paletteView->paletteModel()->setPalette(colorSet);
}

void KoColorSetWidget::slotNameListSelection(const KoColor &color)
{
    emit colorChanged(color, true);
}

//have to include this because of Q_PRIVATE_SLOT
#include "moc_KoColorSetWidget.cpp"
