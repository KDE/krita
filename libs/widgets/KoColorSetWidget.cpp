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
 * Boston, MA 02110-1301, USA.
*/
#include "KoColorSetWidget.h"
#include "KoColorSetWidget_p.h"

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
#include <QGroupBox>
#include <QVBoxLayout>

#include <klocalizedstring.h>
#include <ksharedconfig.h>

#include <resources/KoColorSet.h>
#include <KoColorPatch.h>
#include <KoEditColorSetDialog.h>
#include <KoColorSpaceRegistry.h>
#include <KoResourceServer.h>
#include <KoResourceServerProvider.h>

void KoColorSetWidget::KoColorSetWidgetPrivate::fillColors()
{
    delete colorSetContainer;
    colorSetContainer = new QWidget();
    colorSetLayout = new QVBoxLayout();
    colorSetLayout->setMargin(3);
    colorSetLayout->setSpacing(0); // otherwise the use can click where there is none
    colorSetContainer->setBackgroundRole(QPalette::Dark);

    int patchSize = 12;

    int columns = 16;
    if (colorSet) {
        columns = colorSet->columnCount();
    }
    colorSetContainer->setMinimumWidth(columns*patchSize+6);
    colorSetContainer->setLayout(colorSetLayout);
    patchWidgetList.clear();
    colornames.clear();
    colorNameCmb->clear();

    QWidget *defaultGroupContainer = new QWidget();
    QGridLayout *colorGroupLayout = new QGridLayout();
    for(int i = 0; i<columns; i++) {
        colorGroupLayout->setColumnMinimumWidth(i, patchSize);
    }
    defaultGroupContainer->setMinimumWidth(columns*patchSize);
    defaultGroupContainer->setMaximumWidth(columns*patchSize);
    colorGroupLayout->setSpacing(0);
    colorGroupLayout->setMargin(0);
    defaultGroupContainer->setLayout(colorGroupLayout);
    if (colorSet) {
        for( quint32 i = 0, p= 0; i < colorSet->nColorsGroup(); i++) {
            KoColorPatch *patch = new KoColorPatch(colorSetContainer);
            patch->setFrameStyle(QFrame::Plain | QFrame::Box);
            patch->setLineWidth(1);
            KoColorSetEntry c = colorSet->getColorGlobal(i);
            patch->setColor(c.color);
            patch->setToolTip(c.name);
            connect(patch, SIGNAL(triggered(KoColorPatch *)), thePublic, SLOT(colorTriggered(KoColorPatch *)));
            colorGroupLayout->addWidget(patch, p/columns, p%columns);
            patch->setDisplayRenderer(displayRenderer);
            patchWidgetList.append(patch);
            colornames.append(c.name);
            QPixmap colorsquare = QPixmap(12,12);
            colorsquare.fill(c.color.toQColor());
            colorNameCmb->addItem(QIcon(colorsquare), c.name);
            ++p;
        }
        colorSetLayout->addWidget(defaultGroupContainer);
        Q_FOREACH(QString groupName, colorSet->getGroupNames()) {
            QGroupBox *groupbox = new QGroupBox();
            groupbox->setTitle(groupName);
            QGridLayout *groupLayout = new QGridLayout();
            for(int i = 0; i<columns; i++) {
                groupLayout->setColumnMinimumWidth(i, patchSize);
            }
            groupLayout->setSpacing(0);
            groupLayout->setMargin(0);
            groupbox->setMinimumWidth(columns*patchSize);
            groupbox->setMaximumWidth(columns*patchSize);
            groupbox->setLayout(groupLayout);
            groupbox->setFlat(true);
            for( quint32 i = 0, p= 0; i < colorSet->nColorsGroup(groupName); i++) {
                KoColorPatch *patch = new KoColorPatch(colorSetContainer);
                patch->setFrameStyle(QFrame::Plain | QFrame::Box);
                patch->setLineWidth(1);
                KoColorSetEntry c = colorSet->getColorGroup(i, groupName);
                patch->setColor(c.color);
                patch->setToolTip(c.name);
                connect(patch, SIGNAL(triggered(KoColorPatch *)), thePublic, SLOT(colorTriggered(KoColorPatch *)));
                groupLayout->addWidget(patch, p/columns, p%columns);
                patch->setDisplayRenderer(displayRenderer);
                patchWidgetList.append(patch);
                colornames.append(c.name);
                QPixmap colorsquare = QPixmap(12,12);
                colorsquare.fill(c.color.toQColor());
                colorNameCmb->addItem(QIcon(colorsquare), c.name);
                ++p;
            }
            colorSetLayout->addWidget(groupbox);
        }
    }

    scrollArea->setWidget(colorSetContainer);
    connect(colorNameCmb, SIGNAL(activated(QString)), thePublic, SLOT(setColorFromString(QString)), Qt::UniqueConnection);
}

void KoColorSetWidget::KoColorSetWidgetPrivate::addRemoveColors()
{
    KoResourceServer<KoColorSet>* srv = KoResourceServerProvider::instance()->paletteServer();
    QList<KoColorSet*> palettes = srv->resources();

    Q_ASSERT(colorSet);
    KoEditColorSetDialog *dlg = new KoEditColorSetDialog(palettes, colorSet->name(), thePublic);
    if (dlg->exec() == KoDialog::Accepted ) { // always reload the color set
        KoColorSet * cs = dlg->activeColorSet();
        // check if the selected colorset is predefined
        if( cs && !palettes.contains( cs ) ) {
            int i = 1;
            QFileInfo fileInfo;
            QString savePath = srv->saveLocation();

            do {
                fileInfo.setFile(savePath + QString("%1.%2").arg(i++, 4, 10, QChar('0')).arg(colorSet->defaultFileExtension()));
            }
            while (fileInfo.exists());

            cs->setFilename( fileInfo.filePath() );
            cs->setValid( true );

            // add new colorset to predefined colorsets
            if (!srv->addResource(cs)) {

                delete cs;
                cs = 0;
            }
        }
        if (cs) {
            thePublic->setColorSet(cs);
        }
    }
    delete dlg;
}

void KoColorSetWidget::KoColorSetWidgetPrivate::addRecent(const KoColor &color)
{
    if(numRecents<6) {
        recentPatches[numRecents] = new KoColorPatch(thePublic);
        recentPatches[numRecents]->setFrameShape(QFrame::StyledPanel);
        recentPatches[numRecents]->setDisplayRenderer(displayRenderer);
        recentsLayout->insertWidget(numRecents+1, recentPatches[numRecents]);
        connect(recentPatches[numRecents], SIGNAL(triggered(KoColorPatch *)), thePublic, SLOT(colorTriggered(KoColorPatch *)));
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
    ,d(new KoColorSetWidgetPrivate())
{
    d->thePublic = this;
    d->colorSet = 0;

    d->firstShowOfContainer = true;

    d->mainLayout = new QVBoxLayout();
    d->mainLayout->setMargin(4);
    d->mainLayout->setSpacing(2);

    d->colorSetContainer = 0;

    d->numRecents = 0;
    d->recentsLayout = new QHBoxLayout();
    d->mainLayout->addLayout(d->recentsLayout);
    d->recentsLayout->setMargin(0);
    d->recentsLayout->addWidget(new QLabel(i18n("Recent:")));
    d->recentsLayout->addStretch(1);

    KoColor color(KoColorSpaceRegistry::instance()->rgb8());
    color.fromQColor(QColor(128,0,0));
    d->addRecent(color);

    d->scrollArea = new QScrollArea();
    d->scrollArea->setBackgroundRole(QPalette::Dark);
    d->mainLayout->addWidget(d->scrollArea);
    d->colorNameCmb = new QComboBox(this);
    d->colorNameCmb->setEditable(true);
    d->colorNameCmb->setInsertPolicy(QComboBox::NoInsert);
    d->mainLayout->addWidget(d->colorNameCmb);
    d->fillColors();


    d->addRemoveButton = new QToolButton(this);
    d->addRemoveButton->setText(i18n("Add / Remove Colors..."));
    d->addRemoveButton->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    connect(d->addRemoveButton, SIGNAL(clicked()), SLOT(addRemoveColors()));
    d->mainLayout->addWidget(d->addRemoveButton);

    setLayout(d->mainLayout);

    KoColorSet *colorSet = new KoColorSet();
    d->colorSet = colorSet;
    d->fillColors();
}

KoColorSetWidget::~KoColorSetWidget()
{
    KoResourceServer<KoColorSet>* srv = KoResourceServerProvider::instance()->paletteServer();
    QList<KoColorSet*> palettes = srv->resources();
    if (!palettes.contains(d->colorSet)) {
        delete d->colorSet;
    }
    delete d;
}

void KoColorSetWidget::KoColorSetWidgetPrivate::colorTriggered(KoColorPatch *patch)
{
    int i;

    emit thePublic->colorChanged(patch->color(), true);

    colorNameCmb->setCurrentIndex(colornames.indexOf(QRegExp(patch->toolTip()+"|Fixed")));

    for(i = 0; i <numRecents; i++)
        if(patch == recentPatches[i]) {
            activateRecent(i);
            break;
        }

    if(i == numRecents) // we didn't find it above
        addRecent(patch->color());
}

void KoColorSetWidget::KoColorSetWidgetPrivate::setColorFromString(QString s)
{
    int i = colornames.indexOf(QRegExp(s+"|Fixed"));
    i = qMax(i,0);
    colorTriggered(patchWidgetList.at(i));
}

void KoColorSetWidget::setColorSet(KoColorSet *colorSet)
{
    if (colorSet == d->colorSet) return;

    KoResourceServer<KoColorSet>* srv = KoResourceServerProvider::instance()->paletteServer();
    QList<KoColorSet*> palettes = srv->resources();
    if (!palettes.contains(d->colorSet)) {
        delete d->colorSet;
    }

    d->colorSet = colorSet;
    d->fillColors();
}

KoColorSet* KoColorSetWidget::colorSet()
{
    return d->colorSet;
}

void KoColorSetWidget::setDisplayRenderer(const KoColorDisplayRendererInterface *displayRenderer)
{
    if (displayRenderer) {
        d->displayRenderer = displayRenderer;
        Q_FOREACH(KoColorPatch *p, d->patchWidgetList) {
            p->setDisplayRenderer(displayRenderer);
        }
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

//have to include this because of Q_PRIVATE_SLOT
#include "moc_KoColorSetWidget.cpp"

