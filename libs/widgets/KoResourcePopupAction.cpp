/* This file is part of the KDE project
 * Made by Tomislav Lukman (tomislav.lukman@ck.tel.hr)
 * Copyright (C) 2012 Jean-Nicolas Artaud <jeannicolasartaud@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "KoResourcePopupAction.h"

#include "KoResourceServerProvider.h"
#include "KoResourceServerAdapter.h"
#include "KoResourceItemView.h"
#include "KoResourceModel.h"
#include "KoResourceItemDelegate.h"
#include "KoResource.h"
#include "KoCheckerBoardPainter.h"
#include "KoShapeBackground.h"
#include <KoGradientBackground.h>
#include <KoPatternBackground.h>
#include <KoImageCollection.h>

#include <QMenu>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QPainter>
#include <QGradient>
#include <QToolButton>
#include <QRect>

class KoResourcePopupAction::Private
{
public:
    Private() : resourceList(0), background(0), checkerPainter(4)
    {}
    QMenu *menu;
    KoResourceItemView *resourceList;
    QSharedPointer<KoShapeBackground> background;
    KoCheckerBoardPainter checkerPainter;
};

KoResourcePopupAction::KoResourcePopupAction(KoAbstractResourceServerAdapter *resourceAdapter, QObject *parent)
:  KAction(parent)
, d(new Private())
{
    Q_ASSERT(resourceAdapter);

    d->menu = new QMenu();
    QWidget *widget = new QWidget(d->menu);
    QWidgetAction *wdgAction = new QWidgetAction(widget);

    d->resourceList = new KoResourceItemView(widget);
    d->resourceList->setModel(new KoResourceModel(resourceAdapter, widget));
    d->resourceList->setItemDelegate(new KoResourceItemDelegate(widget));
    KoResourceModel * resourceModel = qobject_cast<KoResourceModel*>(d->resourceList->model());
    if (resourceModel) {
        resourceModel->setColumnCount(1);
    }

    KoResource *resource = 0;
    if (resourceAdapter->resources().count() > 0) {
        resource = resourceAdapter->resources().at(0);
    }

    KoAbstractGradient *gradient = dynamic_cast<KoAbstractGradient*>(resource);
    KoPattern *pattern = dynamic_cast<KoPattern*>(resource);
    if (gradient) {
        QGradient *qg = gradient->toQGradient();
        qg->setCoordinateMode(QGradient::ObjectBoundingMode);
        d->background = QSharedPointer<KoShapeBackground>(new KoGradientBackground(qg));
    } else if (pattern) {
        KoImageCollection *collection = new KoImageCollection();
        d->background = QSharedPointer<KoShapeBackground>(new KoPatternBackground(collection));
        static_cast<KoPatternBackground*>(d->background.data())->setPattern(pattern->image());
    }

    QHBoxLayout *layout = new QHBoxLayout(widget);
    layout->addWidget(d->resourceList);
    widget->setLayout(layout);

    wdgAction->setDefaultWidget(widget);
    d->menu->addAction(wdgAction);
    setMenu(d->menu);
    new QHBoxLayout(d->menu);
    d->menu->layout()->addWidget(widget);
    d->menu->layout()->setMargin(0);

    connect(d->resourceList, SIGNAL(clicked(QModelIndex)), this, SLOT(indexChanged(QModelIndex)));

    updateIcon();
}

KoResourcePopupAction::~KoResourcePopupAction()
{
    delete d;
}

QSharedPointer<KoShapeBackground> KoResourcePopupAction::currentBackground() const
{
    return d->background;
}

void KoResourcePopupAction::setCurrentBackground(QSharedPointer<KoShapeBackground>  background)
{
    d->background = background;

    updateIcon();
}


void KoResourcePopupAction::indexChanged(const QModelIndex &modelIndex)
{
    if (! modelIndex.isValid()) {
        return;
    }

    d->menu->hide();

    KoResource *resource = static_cast<KoResource*>(modelIndex.internalPointer());
    if(resource) {
        KoAbstractGradient *gradient = dynamic_cast<KoAbstractGradient*>(resource);
        KoPattern *pattern = dynamic_cast<KoPattern*>(resource);
        if (gradient) {
            QGradient *qg = gradient->toQGradient();
            qg->setCoordinateMode(QGradient::ObjectBoundingMode);
            d->background = QSharedPointer<KoShapeBackground>(new KoGradientBackground(qg));
        } else if (pattern) {
            KoImageCollection *collection = new KoImageCollection();
            d->background = QSharedPointer<KoShapeBackground>(new KoPatternBackground(collection));
            qSharedPointerDynamicCast<KoPatternBackground>(d->background)->setPattern(pattern->image());
        }

        emit resourceSelected(d->background);

        updateIcon();
    }
}

void KoResourcePopupAction::updateIcon()
{
    QSize iconSize;
    QToolButton *toolButton = dynamic_cast<QToolButton*>(parentWidget());
    if (toolButton) {
        iconSize = QSize(toolButton->iconSize());
    } else {
        iconSize = QSize(16, 16);
    }

    // This must be a QImage, as drawing to a QPixmap outside the
    // UI thread will cause sporadic crashes.
    QImage pm = QImage(iconSize, QImage::Format_ARGB32_Premultiplied);

    pm.fill(Qt::transparent);

    QPainter p(&pm);
    QSharedPointer<KoGradientBackground> gradientBackground = qSharedPointerDynamicCast<KoGradientBackground>(d->background);
    QSharedPointer<KoPatternBackground> patternBackground = qSharedPointerDynamicCast<KoPatternBackground>(d->background);

    if (gradientBackground) {
        QRect innerRect(0, 0, iconSize.width(), iconSize.height());
        QLinearGradient paintGradient;
        paintGradient.setStops(gradientBackground->gradient()->stops());
        paintGradient.setStart(innerRect.topLeft());
        paintGradient.setFinalStop(innerRect.topRight());

        d->checkerPainter.paint(p, innerRect);
        p.fillRect(innerRect, QBrush(paintGradient));
    } else if (patternBackground) {
        d->checkerPainter.paint(p, QRect(QPoint(),iconSize));
        p.fillRect(0, 0, iconSize.width(), iconSize.height(), patternBackground->pattern());
    }

    p.end();

    setIcon(QIcon(QPixmap::fromImage(pm)));
}

#include <KoResourcePopupAction.moc>
