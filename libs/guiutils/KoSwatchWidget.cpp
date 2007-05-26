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
#include <QSlider>
#include <QHBoxLayout>
#include <QFrame>
#include <QMenu>
#include <QMouseEvent>
#include <QDoubleSpinBox>

#include <kglobal.h>
#include <klocale.h>
#include <kdebug.h>

class KoSwatchContainer : public QFrame
{
public:
    KoSwatchContainer(KoSwatchWidget *parent) : QFrame(parent, Qt::Popup ), m_parent(parent) {}

protected:
    virtual void leaveEvent(QEvent *)
    {
        hide();
    }

private:
    KoSwatchWidget *m_parent;
};

class KoSwatchWidget::KoSwatchWidgetPrivate {
public:
    KoSwatchWidget *thePublic;
    QTimer m_timer;
    KoSwatchContainer *container;
    bool firstShowOfContainer;

    void showPopup();
    void hidePopup();
};

KoSwatchWidget::KoSwatchWidget(QWidget *parent)
   : QPushButton(parent)
    ,d(new KoSwatchWidgetPrivate())
{
    d->thePublic = this;
    d->container = new KoSwatchContainer(this);
    d->container->setAttribute(Qt::WA_WindowPropagation);

    d->firstShowOfContainer = true;

/*
    QHBoxLayout * l = new QHBoxLayout();
    l->setMargin(2);
    l->setSpacing(2);
    l->addWidget(d->slider);
    d->container->setLayout(l);
*/    d->container->resize(200, 200);

    resize(20, 20);
/*
    connect(d->slider, SIGNAL(valueChanged(int)), SLOT(sliderValueChanged(int)));
    connect(d->slider, SIGNAL(sliderReleased()), SLOT(sliderReleased()));
    connect(lineEdit(), SIGNAL(editingFinished()), SLOT(lineEditFinished()));
*/
}

KoSwatchWidget::~KoSwatchWidget()
{
    delete d;
}

void KoSwatchWidget::KoSwatchWidgetPrivate::showPopup()
{
    if(firstShowOfContainer) {
        container->show(); //show container a bit early so the slider can be layout'ed
        firstShowOfContainer = false;
    }

    container->move(thePublic->mapToGlobal(QPoint(0,0)));

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

void KoSwatchWidget::enterEvent(QEvent *)
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
    QPushButton::changeEvent(e);
}

#include "KoSwatchWidget.moc"
