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

#include "KoOpacityConfigWidget.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QSizePolicy>

#include <klocale.h>

#include <KoSliderCombo.h>
#include <KoCanvasController.h>
#include <KoSelection.h>
#include <KoToolManager.h>
#include <KoCanvasBase.h>
#include <KoCanvasController.h>
#include <KoCanvasResourceManager.h>
#include <KoDocumentResourceManager.h>
#include <KoShape.h>
#include <KoShapeManager.h>
#include <KoShapeController.h>
#include <KoShapeTransparencyCommand.h>

class KoOpacityConfigWidget::Private
{
public:
    Private()
    : canvas(0)
    {
    }
    KoSliderCombo *opacity;

    QWidget *spacer;
    KoCanvasBase *canvas;
};

KoOpacityConfigWidget::KoOpacityConfigWidget(QWidget *parent)
:  QWidget(parent)
, d(new Private())
{
    setObjectName("Opacity widget");
    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->setMargin(0);
    layout->setSpacing(0);

    // Opacity setting
    d->opacity = new KoSliderCombo(this);
    d->opacity->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
    d->opacity->setMinimum(0);
    d->opacity->setMaximum(100);
    d->opacity->setValue(100);
    d->opacity->setDecimals(0);
    connect(d->opacity, SIGNAL(valueChanged(qreal, bool)), this, SLOT(updateOpacity(qreal)));

    layout->addWidget(d->opacity);

    // Spacer
    d->spacer = new QWidget();
    d->spacer->setObjectName("SpecialSpacer");
    layout->addWidget(d->spacer);

    KoCanvasController *canvasController = KoToolManager::instance()->activeCanvasController();
    KoSelection *selection = canvasController->canvas()->shapeManager()->selection();
    if (selection) {
        d->canvas = canvasController->canvas();
        connect(selection, SIGNAL(selectionChanged()), this, SLOT(shapeChanged()));
    }
}

KoOpacityConfigWidget::~KoOpacityConfigWidget()
{
    delete d;
}

void KoOpacityConfigWidget::setCanvas( KoCanvasBase *canvas )
{
    KoCanvasController *canvasController = KoToolManager::instance()->activeCanvasController();
    KoSelection *selection = canvasController->canvas()->shapeManager()->selection();

    connect(selection, SIGNAL(selectionChanged()), this, SLOT(shapeChanged()));

    d->canvas = canvas;
}


void KoOpacityConfigWidget::updateOpacity(qreal opacity)
{
    KoCanvasController *canvasController = KoToolManager::instance()->activeCanvasController();
    KoSelection *selection = canvasController->canvas()->shapeManager()->selection();

    if (!selection || !selection->count()) {
        return;
    }

    QList<KoShape*> selectedShapes = selection->selectedShapes(KoFlake::TopLevelSelection);
    if (!selectedShapes.count()){
        return;
    }

    canvasController->canvas()->addCommand(new KoShapeTransparencyCommand(selectedShapes, 1.0 - opacity / 100));
}

void KoOpacityConfigWidget::shapeChanged()
{
    KoCanvasController *canvasController = KoToolManager::instance()->activeCanvasController();
    KoSelection *selection = canvasController->canvas()->shapeManager()->selection();
    KoShape *shape = selection->firstSelectedShape();

    if (! shape) {
        return;
    }

    // We don't want the opacity slider to send any signals when it's only initialized.
    // Otherwise an undo record is created.
    d->opacity->blockSignals(true);
    d->opacity->setValue(100 - shape->transparency() * 100);
    d->opacity->blockSignals(false);
}


#include <KoOpacityConfigWidget.moc>
