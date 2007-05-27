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
#include <QFrame>
//#include <QMenu>
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
    bool firstShowOfContainer;
    QHBoxLayout *recentsLayout;
    KoColorPatch *recentPatches[6];
    int numRecents;
    KoColorPatch *lastSelected;

    void colorTriggered(KoColorPatch *patch);
    void showPopup();
    void hidePopup();
    void addRecent(KoColor &);
};

void KoSwatchWidget::KoSwatchWidgetPrivate::addRecent(KoColor &color)
{
    if(numRecents<6) {
        recentPatches[numRecents] = new KoColorPatch(container);
        recentPatches[numRecents]->setSizePolicy(QSizePolicy::Fixed,QSizePolicy::Fixed);
        recentPatches[numRecents]->resize(22,22);
        recentPatches[numRecents]->setFrameShape(QFrame::Box);
        recentsLayout->insertWidget(numRecents, recentPatches[numRecents]);
        connect(recentPatches[numRecents], SIGNAL(triggered(KoColorPatch *)), thePublic, SLOT(colorTriggered(KoColorPatch *)));
        numRecents++;
    }
    // shift colors to the right 
    for (int i = numRecents- 1; i >1; i--) {
        kDebug() << "set " << i << " to value of " << i-1 << endl;
    }

    //Finally set the recent color
    recentPatches[0]->setColor(color);
}

KoSwatchWidget::KoSwatchWidget(QWidget *parent)
   : QToolButton(parent)
    ,d(new KoSwatchWidgetPrivate())
{
    d->thePublic = this;
    d->container = new KoSwatchContainer(this);
    d->container->setAttribute(Qt::WA_WindowPropagation);

    d->firstShowOfContainer = true;

    QVBoxLayout * l = new QVBoxLayout();
    l->setMargin(2);
    l->setSpacing(2);

    l->addWidget(new QLabel("Recent:"));

    d->numRecents = 0;
    d->recentsLayout = new QHBoxLayout();
    l->addLayout(d->recentsLayout);
    d->recentsLayout->setMargin(2);
    d->recentsLayout->setSpacing(2);
    d->recentsLayout->addStretch(1);

    KoColor color(KoColorSpaceRegistry::instance()->rgb8());
    color.fromQColor(QColor(128,0,0));
    d->addRecent(color);
    d->lastSelected = d->recentPatches[0];

    l->addWidget(new QLabel("Suggestions:"));
        QHBoxLayout *hl = new QHBoxLayout();
        hl->setMargin(2);
        hl->setSpacing(2);
        l->addLayout(hl);

        for(int i = 0; i<6; i++)
        {
            KoColorPatch *patch = new KoColorPatch(d->container);
            KoColor color(KoColorSpaceRegistry::instance()->rgb8());
            color.fromQColor(QColor(0,128+15*i,0));
            patch->setColor(color);
            patch->setSizePolicy(QSizePolicy::Fixed,QSizePolicy::Fixed);
            patch->resize(22,22);
            patch->setFrameShape(QFrame::Box);
            connect(patch, SIGNAL(triggered(KoColorPatch *)), SLOT(colorTriggered(KoColorPatch *)));
            hl->addWidget(patch);
        }
    d->container->setLayout(l);

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
    lastSelected = patch;
}

void KoSwatchWidget::KoSwatchWidgetPrivate::showPopup()
{
    if(firstShowOfContainer) {
        container->show(); //show container a bit early so the slider can be layout'ed
        firstShowOfContainer = false;
    }

    

    container->move(thePublic->mapToGlobal( QPoint(0,0) - lastSelected->pos()));

    container->raise();
    container->show();
}

void KoSwatchWidget::KoSwatchWidgetPrivate::hidePopup()
{
    container->hide();
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
