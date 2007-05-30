/* This file is part of the KDE project
   Copyright (c) 2007 Casper Boemann <cbr@boemann.dk>

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
#include "KoSwatchWidget.h"

#include <QTimer>
#include <QApplication>
#include <QSize>
#include <QPushButton>
#include <QHBoxLayout>
#include <QCheckBox>
#include <QFrame>
#include <QLabel>
#include <QMouseEvent>

#include <kglobal.h>
#include <klocale.h>
#include <kdebug.h>
#include <kicon.h>

#include <KoColorPatch.h>
#include "KoColorSpaceRegistry.h"

class KoSwatchContainer : public QFrame
{
public:
    KoSwatchContainer(KoSwatchWidget *parent) : QFrame(parent, Qt::Popup ), m_parent(parent) {}

protected:

private:
    KoSwatchWidget *m_parent;
};

class KoSwatchWidget::KoSwatchWidgetPrivate {
public:
    KoSwatchWidget *thePublic;
    QTimer m_timer;
    KoSwatchContainer *container;
    QVBoxLayout *mainLayout;
    bool firstShowOfContainer;
    QCheckBox *filterCheckBox;
    QWidget *swatchContainer;
    QGridLayout *swatchLayout;
    QHBoxLayout *recentsLayout;
    KoColorPatch *recentPatches[6];
    int numRecents;

    void colorTriggered(KoColorPatch *patch);
    void showPopup();
    void hidePopup();
    void addRecent(const KoColor &);
    void activateRecent(int i);
    void filter(int state);
};

void KoSwatchWidget::KoSwatchWidgetPrivate::filter(int state)
{
    bool hide = (state ==QCheckBox::On);

    if (swatchContainer)
        delete swatchContainer;
    swatchContainer = new QWidget();
    swatchLayout = new QGridLayout();
    swatchLayout->setMargin(0);
    swatchLayout->setSpacing(1);
    for(int i = 0; i<16; i++) {
        swatchLayout->setColumnMinimumWidth(i, 16);
    }
    swatchContainer->setLayout(swatchLayout);

    for(int i = 0; i<75; i++) {
        if(!hide || i%3!=0 && i%16!=5) {
            KoColorPatch *patch = new KoColorPatch(swatchContainer);
            KoColor color(KoColorSpaceRegistry::instance()->rgb8());
            color.fromQColor(QColor(0,3*i,250-3*i));
            patch->setColor(color);
            patch->setSizePolicy(QSizePolicy::Fixed,QSizePolicy::Fixed);
            patch->setFrameShape(QFrame::Box);
            connect(patch, SIGNAL(triggered(KoColorPatch *)), thePublic, SLOT(colorTriggered(KoColorPatch *)));
            swatchLayout->addWidget(patch, i/16, i%16);
        }
    }
    mainLayout->insertWidget(2, swatchContainer);
}


void KoSwatchWidget::KoSwatchWidgetPrivate::addRecent(const KoColor &color)
{
    if(numRecents<6) {
        recentPatches[numRecents] = new KoColorPatch(container);
        recentPatches[numRecents]->setSizePolicy(QSizePolicy::Fixed,QSizePolicy::Fixed);
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

void KoSwatchWidget::KoSwatchWidgetPrivate::activateRecent(int i)
{
    KoColor color = recentPatches[i]->color();

    while (i >0) {
        recentPatches[i]->setColor(recentPatches[i-1]->color());
        i--;
    }
    recentPatches[0]->setColor(color);
}

KoSwatchWidget::KoSwatchWidget(QWidget *parent)
   : QToolButton(parent)
    ,d(new KoSwatchWidgetPrivate())
{
    d->thePublic = this;
    d->container = new KoSwatchContainer(this);
    d->container->setAttribute(Qt::WA_WindowPropagation);
    d->container->setFrameShape(QFrame::Box);

    d->firstShowOfContainer = true;

    d->mainLayout = new QVBoxLayout();
    d->mainLayout->setMargin(4);
    d->mainLayout->setSpacing(2);

    d->swatchContainer = 0;

    d->numRecents = 0;
    d->recentsLayout = new QHBoxLayout();
    d->mainLayout->addLayout(d->recentsLayout);
    d->recentsLayout->setMargin(0);
    d->recentsLayout->setSpacing(2);
    d->recentsLayout->addWidget(new QLabel("Recent:"));
    d->recentsLayout->addStretch(1);
    d->recentsLayout->addWidget(new QPushButton("Add / Remove Colors..."));

    KoColor color(KoColorSpaceRegistry::instance()->rgb8());
    color.fromQColor(QColor(128,0,0));
    d->addRecent(color);

    d->filterCheckBox = new QCheckBox("Hide colors with bad contrast");
    d->filterCheckBox->setChecked(true);
    d->mainLayout->addWidget(d->filterCheckBox);
    connect(d->filterCheckBox, SIGNAL(stateChanged(int)), SLOT(filter(int)));

    d->filter(QCheckBox::On);

    d->container->setLayout(d->mainLayout);

    setIcon(KIcon("textcolor"));

/*    connect(d->slider, SIGNAL(sliderReleased()), SLOT(sliderReleased()));
    connect(lineEdit(), SIGNAL(editingFinished()), SLOT(lineEditFinished()));
*/
}

KoSwatchWidget::~KoSwatchWidget()
{
    delete d;
}

void KoSwatchWidget::KoSwatchWidgetPrivate::colorTriggered(KoColorPatch *patch)
{
    hidePopup();
    int i;

    for(i = 0; i <numRecents; i++)
        if(patch == recentPatches[i]) {
            activateRecent(i);
            break;
        }

    if(i == numRecents) // we didn't find it above
        addRecent(patch->color());
}

void KoSwatchWidget::KoSwatchWidgetPrivate::showPopup()
{
    if(firstShowOfContainer) {
        container->show(); //show container a bit early so the slider can be layout'ed
        firstShowOfContainer = false;
    }

    

    container->move(thePublic->mapToGlobal( QPoint(6 - recentPatches[0]->pos().x(), 24)));

    container->raise();
    container->show();
}

void KoSwatchWidget::KoSwatchWidgetPrivate::hidePopup()
{
    container->hide();
}

void KoSwatchWidget::setOppositeColor(const KoColor &color)
{
}

void KoSwatchWidget::hideEvent(QHideEvent *)
{
    d->hidePopup();
}

void KoSwatchWidget::mousePressEvent(QMouseEvent *)
{
    d->showPopup();
}


void KoSwatchWidget::changeEvent(QEvent *e)
{
    switch (e->type())
    {
        case QEvent::EnabledChange:
            if (!isEnabled())
                d->hidePopup();
            break;
        case QEvent::PaletteChange:
            d->container->setPalette(palette());
            break;
        default:
            break;
    }
    QToolButton::changeEvent(e);
}

#include "KoSwatchWidget.moc"
