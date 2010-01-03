/* This file is part of the KDE project
   Copyright (c) 2007 Casper Boemann <cbr@boemann.dk>
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
#include <QtGui/QScrollArea>

#include <kglobal.h>
#include <kstandarddirs.h>
#include <klocale.h>
#include <kdebug.h>
#include <kicon.h>

#include <KoColorSet.h>
#include <KoColorPatch.h>
#include <KoEditColorSetDialog.h>
#include <KoColorSpaceRegistry.h>
#include <KoResourceServer.h>
#include <KoResourceServerProvider.h>

class KoColorSetWidget::KoColorSetWidgetPrivate {
public:
    KoColorSetWidget *thePublic;
    KoColorSet *colorSet;
    QTimer m_timer;
    QVBoxLayout *mainLayout;
    bool firstShowOfContainer;
    QCheckBox *filterCheckBox;
    QWidget *colorSetContainer;
    QScrollArea *scrollArea;
    QGridLayout *colorSetLayout;
    QHBoxLayout *recentsLayout;
    KoColorPatch *recentPatches[6];
    QToolButton *addRemoveButton;
    int numRecents;

    void colorTriggered(KoColorPatch *patch);
    void addRecent(const KoColor &);
    void activateRecent(int i);
    void filter(int state);
    void addRemoveColors();
};

void KoColorSetWidget::KoColorSetWidgetPrivate::filter(int state)
{
    bool hide = (state == Qt::Checked);

    if (colorSetContainer)
        delete colorSetContainer;
    colorSetContainer = new QWidget();
    colorSetLayout = new QGridLayout();
    colorSetLayout->setMargin(3);
    colorSetLayout->setSpacing(1);
    colorSetContainer->setBackgroundRole(QPalette::Dark);
    for(int i = 0; i<16; i++) {
        colorSetLayout->setColumnMinimumWidth(i, 12);
    }
    colorSetContainer->setLayout(colorSetLayout);

    if (colorSet) {
        for( int i = 0, p= 0; i < colorSet->nColors(); i++) {
            if(!hide || (i % 3 != 0 && i % 16 != 5)) {
                KoColorPatch *patch = new KoColorPatch(colorSetContainer);
                patch->setFrameStyle(QFrame::NoFrame);
                patch->setColor(colorSet->getColor(i).color);
                connect(patch, SIGNAL(triggered(KoColorPatch *)), thePublic, SLOT(colorTriggered(KoColorPatch *)));
                colorSetLayout->addWidget(patch, p/16, p%16);
                ++p;
            }
        }
    }

    scrollArea->setWidget(colorSetContainer);
}

void KoColorSetWidget::KoColorSetWidgetPrivate::addRemoveColors()
{
    KoResourceServer<KoColorSet>* srv = KoResourceServerProvider::instance()->paletteServer();
    QList<KoColorSet*> palettes = srv->resources();

    Q_ASSERT(colorSet);
    KoEditColorSetDialog *dlg = new KoEditColorSetDialog(palettes, colorSet->name(), thePublic);
    if (dlg->exec() == KDialog::Accepted ) { // always reload the color set
        KoColorSet * cs = dlg->activeColorSet();
        // check if the selected colorset is predefined
        if( ! palettes.contains( cs ) ) {
            int i = 1;
            QFileInfo fileInfo;
            QString savePath = srv->saveLocation();

            do {
                fileInfo.setFile( savePath + QString("%1.gpl").arg( i++, 4, 10, QChar('0') ) );
            }
            while( fileInfo.exists() );

            cs->setFilename( fileInfo.filePath() );
            cs->setValid( true );

            // add new colorset to predefined colorsets
            if( ! srv->addResource( cs ) ) {
                delete cs;
                cs = 0;
            }
        }
        if( cs )
            thePublic->setColorSet(cs);
        // colorSetContainer->setFixedSize(colorSetLayout->sizeHint());
        // thePublic->setFixedSize(mainLayout->sizeHint());
    }
    delete dlg;
}

void KoColorSetWidget::KoColorSetWidgetPrivate::addRecent(const KoColor &color)
{
    if(numRecents<6) {
        recentPatches[numRecents] = new KoColorPatch(thePublic);
        recentPatches[numRecents]->setFrameShape(QFrame::Box);
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
    d->recentsLayout->setSpacing(KDialog::spacingHint());
    d->recentsLayout->addWidget(new QLabel(i18n("Recent:")));
    d->recentsLayout->addStretch(1);

    KoColor color(KoColorSpaceRegistry::instance()->rgb8());
    color.fromQColor(QColor(128,0,0));
    d->addRecent(color);

    d->filterCheckBox = new QCheckBox(i18n("Hide colors with bad contrast"));
    d->filterCheckBox->setChecked(true);
    d->mainLayout->addWidget(d->filterCheckBox);
    connect(d->filterCheckBox, SIGNAL(stateChanged(int)), SLOT(filter(int)));

    d->scrollArea = new QScrollArea();
    d->scrollArea->setBackgroundRole(QPalette::Dark);
    d->mainLayout->addWidget(d->scrollArea);
    d->filter(Qt::Checked);

    d->addRemoveButton = new QToolButton(this);
    d->addRemoveButton->setText(i18n("Add / Remove Colors..."));
    d->addRemoveButton->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    connect(d->addRemoveButton, SIGNAL(clicked()), SLOT(addRemoveColors()));
    d->mainLayout->addWidget(d->addRemoveButton);

    setLayout(d->mainLayout);

    // Use 40_Colors.gpl for testing
    // TODO don't depend on Krita data in libs
    QString defaultPalette("krita/palettes/40_Colors.gpl");
    QString dir = KGlobal::dirs()->findResourceDir("data", defaultPalette);
    KoColorSet *colorSet = new KoColorSet(dir.append(defaultPalette));
    colorSet->load();
    setColorSet(colorSet);

/*    connect(d->slider, SIGNAL(sliderReleased()), SLOT(sliderReleased()));
    connect(lineEdit(), SIGNAL(editingFinished()), SLOT(lineEditFinished()));
*/

}

KoColorSetWidget::~KoColorSetWidget()
{
    delete d->colorSet;
    delete d;
}

void KoColorSetWidget::addRecentColor(const KoColor &color)
{
    d->addRecent(color);
}

void KoColorSetWidget::KoColorSetWidgetPrivate::colorTriggered(KoColorPatch *patch)
{
    int i;

    emit thePublic->colorChanged(patch->color(), true);

    for(i = 0; i <numRecents; i++)
        if(patch == recentPatches[i]) {
            activateRecent(i);
            break;
        }

    if(i == numRecents) // we didn't find it above
        addRecent(patch->color());
}

void KoColorSetWidget::setOppositeColor(const KoColor &color)
{
    Q_UNUSED(color);
}

void KoColorSetWidget::setColorSet(KoColorSet *colorSet)
{
    d->colorSet = colorSet;
    d->filter(d->filterCheckBox->checkState());
}

void KoColorSetWidget::resizeEvent(QResizeEvent *event)
{
    emit widgetSizeChanged(event->size());
    QFrame::resizeEvent(event);
}

#include <KoColorSetWidget.moc>
