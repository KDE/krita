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

#include "KoFillConfigWidget.h"

#include <QToolButton>
#include <QHBoxLayout>
#include <QButtonGroup>
#include <QLabel>
#include <QSizePolicy>
#include <QBitmap>
#include <QAction>

#include <klocale.h>

#include <KoGroupButton.h>
#include <KoIcon.h>
#include <KoColor.h>
#include <KoColorPopupAction.h>
#include "KoResourceServerProvider.h"
#include "KoResourceServerAdapter.h"
#include "KoResourceSelector.h"
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
#include <KoShapeBackground.h>
#include <KoShapeBackgroundCommand.h>
#include <KoColorBackground.h>
#include <KoGradientBackground.h>
#include <KoPatternBackground.h>
#include <KoImageCollection.h>
#include <KoResourcePopupAction.h>
#include "KoZoomHandler.h"
#include "KoColorPopupButton.h"

static const char* const buttonnone[]={
    "16 16 3 1",
    "# c #000000",
    "e c #ff0000",
    "- c #ffffff",
    "################",
    "#--------------#",
    "#--------------#",
    "#--------------#",
    "#--------------#",
    "#--------------#",
    "#--------------#",
    "#--------------#",
    "#--------------#",
    "#--------------#",
    "#--------------#",
    "#--------------#",
    "#--------------#",
    "#--------------#",
    "#--------------#",
    "################"};

static const char* const buttonsolid[]={
    "16 16 2 1",
    "# c #000000",
    ". c #969696",
    "################",
    "#..............#",
    "#..............#",
    "#..............#",
    "#..............#",
    "#..............#",
    "#..............#",
    "#..............#",
    "#..............#",
    "#..............#",
    "#..............#",
    "#..............#",
    "#..............#",
    "#..............#",
    "#..............#",
    "################"};


// FIXME: Smoother gradient button.

static const char* const buttongradient[]={
    "16 16 15 1",
    "# c #000000",
    "n c #101010",
    "m c #202020",
    "l c #303030",
    "k c #404040",
    "j c #505050",
    "i c #606060",
    "h c #707070",
    "g c #808080",
    "f c #909090",
    "e c #a0a0a0",
    "d c #b0b0b0",
    "c c #c0c0c0",
    "b c #d0d0d0",
    "a c #e0e0e0",
    "################",
    "#abcdefghijklmn#",
    "#abcdefghijklmn#",
    "#abcdefghijklmn#",
    "#abcdefghijklmn#",
    "#abcdefghijklmn#",
    "#abcdefghijklmn#",
    "#abcdefghijklmn#",
    "#abcdefghijklmn#",
    "#abcdefghijklmn#",
    "#abcdefghijklmn#",
    "#abcdefghijklmn#",
    "#abcdefghijklmn#",
    "#abcdefghijklmn#",
    "#abcdefghijklmn#",
    "################"};

static const char* const buttonpattern[]={
    "16 16 4 1",
    ". c #0a0a0a",
    "# c #333333",
    "a c #a0a0a0",
    "b c #ffffffff",
    "################",
    "#aaaaabbbbaaaaa#",
    "#aaaaabbbbaaaaa#",
    "#aaaaabbbbaaaaa#",
    "#aaaaabbbbaaaaa#",
    "#aaaaabbbbaaaaa#",
    "#bbbbbaaaabbbbb#",
    "#bbbbbaaaabbbbb#",
    "#bbbbbaaaabbbbb#",
    "#bbbbbaaaabbbbb#",
    "#aaaaabbbbaaaaa#",
    "#aaaaabbbbaaaaa#",
    "#aaaaabbbbaaaaa#",
    "#aaaaabbbbaaaaa#",
    "#aaaaabbbbaaaaa#",
    "################"};

class KoFillConfigWidget::Private
{
public:
    Private()
    : canvas(0)
    {
    }

    KoColorPopupButton *colorButton;
    QAction *noFillAction;
    KoColorPopupAction *colorAction;
    KoResourcePopupAction *gradientAction;
    KoResourcePopupAction *patternAction;
    QButtonGroup *group;

    QWidget *spacer;
    KoCanvasBase *canvas;
};

KoFillConfigWidget::KoFillConfigWidget(QWidget *parent)
:  QWidget(parent)
, d(new Private())
{
    setObjectName("Fill widget");
    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->setMargin(0);
    layout->setSpacing(0);

    d->group = new QButtonGroup(this);
    d->group->setExclusive(true);

    // The button for no fill
    KoGroupButton *button = new KoGroupButton(KoGroupButton::GroupLeft, this);
    QPixmap noFillButtonIcon((const char **) buttonnone);
    noFillButtonIcon.setMask(QBitmap(noFillButtonIcon));
    button->setIcon(noFillButtonIcon);
    button->setToolTip(i18nc("No stroke or fill", "None"));
    button->setCheckable(true);
    d->group->addButton(button, None);
    layout->addWidget(button);

    // The button for solid fill
    button = new KoGroupButton(KoGroupButton::GroupCenter, this);
    button->setIcon(QPixmap((const char **) buttonsolid));
    button->setToolTip(i18nc("Solid color stroke or fill", "Solid"));
    button->setCheckable(true);
    d->group->addButton(button, Solid);
    layout->addWidget(button);

    // The button for gradient fill
    button = new KoGroupButton(KoGroupButton::GroupCenter, this);
    button->setIcon(QPixmap((const char **) buttongradient));
    button->setToolTip(i18n("Gradient"));
    button->setCheckable(true);
    d->group->addButton(button, Gradient);
    layout->addWidget(button);

    // The button for pattern fill
    button = new KoGroupButton(KoGroupButton::GroupRight, this);
    button->setIcon(QPixmap((const char **) buttonpattern));
    button->setToolTip(i18n("Pattern"));
    button->setCheckable(true);
    d->group->addButton(button, Pattern);
    layout->addWidget(button);

    connect(d->group, SIGNAL(buttonClicked(int)), this, SLOT(styleButtonPressed(int)));

    d->colorButton = new KoColorPopupButton(this);
    d->colorButton->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    layout->addWidget(d->colorButton);

    d->noFillAction = new QAction(0);

    d->colorAction = new KoColorPopupAction(d->colorButton);
    d->colorAction->setToolTip(i18n("Change the filling color"));
    d->colorAction->setCurrentColor(Qt::white);
    d->colorButton->setDefaultAction(d->colorAction);
    d->colorButton->setPopupMode(QToolButton::InstantPopup);
    connect(d->colorAction, SIGNAL(colorChanged(const KoColor &)), this, SLOT(colorChanged()));
    connect(d->colorButton, SIGNAL(iconSizeChanged()), d->colorAction, SLOT(updateIcon()));

    // Gradient selector
    KoResourceServerProvider *serverProvider = KoResourceServerProvider::instance();
    KoAbstractResourceServerAdapter *gradientResourceAdapter = new KoResourceServerAdapter<KoAbstractGradient>(serverProvider->gradientServer(), this);
    d->gradientAction = new KoResourcePopupAction(gradientResourceAdapter, d->colorButton);
    d->gradientAction->setToolTip(i18n("Change the filling color"));
    connect(d->gradientAction, SIGNAL(resourceSelected(KoShapeBackground*)), this, SLOT(gradientChanged(KoShapeBackground*)));
    connect(d->colorButton, SIGNAL(iconSizeChanged()), d->gradientAction, SLOT(updateIcon()));

    // Pattern selector
    KoAbstractResourceServerAdapter *patternResourceAdapter = new KoResourceServerAdapter<KoPattern>(serverProvider->patternServer(), this);
    d->patternAction = new KoResourcePopupAction(patternResourceAdapter, d->colorButton);
    d->patternAction->setToolTip(i18n("Change the filling color"));
    connect(d->patternAction, SIGNAL(resourceSelected(KoShapeBackground*)), this, SLOT(patternChanged(KoShapeBackground*)));
    connect(d->colorButton, SIGNAL(iconSizeChanged()), d->patternAction, SLOT(updateIcon()));

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

KoFillConfigWidget::~KoFillConfigWidget()
{
    delete d;
}

void KoFillConfigWidget::setCanvas( KoCanvasBase *canvas )
{
    KoCanvasController *canvasController = KoToolManager::instance()->activeCanvasController();
    KoSelection *selection = canvasController->canvas()->shapeManager()->selection();

    connect(selection, SIGNAL(selectionChanged()), this, SLOT(shapeChanged()));

    d->canvas = canvas;
}

void KoFillConfigWidget::styleButtonPressed(int buttonId)
{
    d->colorButton->setEnabled(true);
    switch (buttonId) {
        case KoFillConfigWidget::None:
            // Direct manipulation
            d->colorButton->setDefaultAction(d->noFillAction);
             d->colorButton->setDisabled(true);
            noColorSelected();
            break;
        case KoFillConfigWidget::Solid:
            d->colorButton->setDefaultAction(d->colorAction);
            colorChanged();
            break;
        case KoFillConfigWidget::Gradient:
            // Only select mode in the widget, don't set actual gradient :/
            d->colorButton->setDefaultAction(d->gradientAction);
            gradientChanged(d->gradientAction->currentBackground());
            break;
        case KoFillConfigWidget::Pattern:
            // Only select mode in the widget, don't set actual pattern :/
            d->colorButton->setDefaultAction(d->patternAction);
            patternChanged(d->patternAction->currentBackground());
            break;
    }
    d->colorButton->setPopupMode(QToolButton::InstantPopup);
}

void KoFillConfigWidget::noColorSelected()
{
    KoCanvasController *canvasController = KoToolManager::instance()->activeCanvasController();
    KoSelection *selection = canvasController->canvas()->shapeManager()->selection();

    if (!selection || !selection->count()) {
        return;
    }

    QList<KoShape*> selectedShapes = selection->selectedShapes();
    if (selectedShapes.isEmpty()) {
        return;
    }

    canvasController->canvas()->addCommand(new KoShapeBackgroundCommand(selectedShapes, 0));
}

void KoFillConfigWidget::colorChanged()
{
    KoCanvasController *canvasController = KoToolManager::instance()->activeCanvasController();
    KoSelection *selection = canvasController->canvas()->shapeManager()->selection();

    if (!selection || !selection->count()) {
        return;
    }

    KoShapeBackground *fill = new KoColorBackground(d->colorAction->currentColor());

    QList<KoShape*> selectedShapes = selection->selectedShapes();
    if (selectedShapes.isEmpty()) {
        return;
    }

    KUndo2Command *firstCommand = 0;
    foreach (KoShape *shape, selectedShapes) {
        if (! firstCommand) {
            firstCommand = new KoShapeBackgroundCommand(shape, fill);
        } else {
            new KoShapeBackgroundCommand(shape, fill, firstCommand);
        }
    }
    canvasController->canvas()->addCommand(firstCommand);
}

void KoFillConfigWidget::gradientChanged(KoShapeBackground* background)
{
    KoCanvasController *canvasController = KoToolManager::instance()->activeCanvasController();
    KoSelection *selection = canvasController->canvas()->shapeManager()->selection();

    if (!selection || !selection->count()) {
        return;
    }

    QList<KoShape*> selectedShapes = selection->selectedShapes();
    if (selectedShapes.isEmpty()) {
        return;
    }

    KoGradientBackground *gradientBackground = dynamic_cast<KoGradientBackground*>(background);
    if (! gradientBackground) {
        return;
    }

    QGradientStops newStops = gradientBackground->gradient()->stops();
    delete gradientBackground;

    KUndo2Command *firstCommand = 0;
    foreach (KoShape *shape, selectedShapes) {
        KoShapeBackground *fill = applyFillGradientStops(shape, newStops);
        if (! fill) {
            continue;
        }
        if (! firstCommand) {
            firstCommand = new KoShapeBackgroundCommand(shape, fill);
        } else {
            new KoShapeBackgroundCommand(shape, fill, firstCommand);
        }
    }
    canvasController->canvas()->addCommand(firstCommand);
}

void KoFillConfigWidget::patternChanged(KoShapeBackground* background)
{
    KoCanvasController *canvasController = KoToolManager::instance()->activeCanvasController();
    KoSelection *selection = canvasController->canvas()->shapeManager()->selection();

    if (!selection || !selection->count()) {
        return;
    }

    KoPatternBackground *patternBackground = dynamic_cast<KoPatternBackground*>(background);
    if (! patternBackground) {
        return;
    }

    QList<KoShape*> selectedShapes = canvasController->canvas()->shapeManager()->selection()->selectedShapes();
    if (selectedShapes.isEmpty()) {
        return;
    }

    KoImageCollection *imageCollection = canvasController->canvas()->shapeController()->resourceManager()->imageCollection();
    if (imageCollection) {
        KoPatternBackground *fill = new KoPatternBackground(imageCollection);
        fill->setPattern(patternBackground->pattern());
        canvasController->canvas()->addCommand(new KoShapeBackgroundCommand(selectedShapes, fill));
    }
}

void KoFillConfigWidget::shapeChanged()
{
    KoCanvasController *canvasController = KoToolManager::instance()->activeCanvasController();
    KoSelection *selection = canvasController->canvas()->shapeManager()->selection();
    KoShape *shape = selection->firstSelectedShape();
    if (! shape) {
        d->group->button(KoFillConfigWidget::None)->setChecked(false);
        d->group->button(KoFillConfigWidget::Solid)->setChecked(false);
        d->group->button(KoFillConfigWidget::Gradient)->setChecked(false);
        d->group->button(KoFillConfigWidget::Pattern)->setChecked(false);
        d->colorButton->setDisabled(true);
        return;
    }

    updateWidget(shape);
}


void KoFillConfigWidget::updateWidget(KoShape *shape)
{
    if (! shape) {
        return;
    }

    KoZoomHandler zoomHandler;
    const qreal realWidth = zoomHandler.resolutionX() * width();
    const qreal realHeight = zoomHandler.resolutionX() * height();

    const qreal zoom = (realWidth > realHeight) ? realHeight : realWidth;
    zoomHandler.setZoom(zoom);

    shape->waitUntilReady(zoomHandler, false);

    d->colorButton->setEnabled(true);
    KoShapeBackground *background = shape->background();
    if (! background) {
        // No Fill
        d->group->button(KoFillConfigWidget::None)->setChecked(true);
        d->colorButton->setDefaultAction(d->noFillAction);
        d->colorButton->setDisabled(true);
        d->colorButton->setPopupMode(QToolButton::InstantPopup);
        return;
    }

    KoColorBackground *colorBackground = dynamic_cast<KoColorBackground*>(background);
    KoGradientBackground *gradientBackground = dynamic_cast<KoGradientBackground*>(background);
    KoPatternBackground *patternBackground = dynamic_cast<KoPatternBackground*>(background);

    if (colorBackground) {
        d->colorAction->setCurrentColor(colorBackground->color());
        d->group->button(KoFillConfigWidget::Solid)->setChecked(true);
        d->colorButton->setDefaultAction(d->colorAction);
    } else if (gradientBackground) {
        d->gradientAction->setCurrentBackground(background);
        d->group->button(KoFillConfigWidget::Gradient)->setChecked(true);
        d->colorButton->setDefaultAction(d->gradientAction);
    } else if (patternBackground) {
        d->patternAction->setCurrentBackground(background);
        d->group->button(KoFillConfigWidget::Pattern)->setChecked(true);
        d->colorButton->setDefaultAction(d->patternAction);
    } else {
        // No Fill
        d->group->button(KoFillConfigWidget::None)->setChecked(true);
        d->colorButton->setDefaultAction(d->noFillAction);
        d->colorButton->setDisabled(true);
    }
    d->colorButton->setPopupMode(QToolButton::InstantPopup);
}

KoShapeBackground *KoFillConfigWidget::applyFillGradientStops(KoShape *shape, const QGradientStops &stops)
{
    if (! shape || ! stops.count()) {
        return 0;
    }

    KoGradientBackground *newGradient = 0;
    KoGradientBackground *oldGradient = dynamic_cast<KoGradientBackground*>(shape->background());
    if (oldGradient) {
        // just copy the gradient and set the new stops
        QGradient *g = KoFlake::cloneGradient(oldGradient->gradient());
        g->setStops(stops);
        newGradient = new KoGradientBackground(g);
        newGradient->setTransform(oldGradient->transform());
    }
    else {
        // No gradient yet, so create a new one.
        QLinearGradient *g = new QLinearGradient(QPointF(0, 0), QPointF(1, 1));
        g->setCoordinateMode(QGradient::ObjectBoundingMode);
        g->setStops(stops);
        newGradient = new KoGradientBackground(g);
    }
    return newGradient;
}

void KoFillConfigWidget::blockChildSignals(bool block)
{
    d->colorButton->blockSignals(block);
    d->colorAction->blockSignals(block);
    d->gradientAction->blockSignals(block);
    d->patternAction->blockSignals(block);
    d->group->blockSignals(block);
}


#include <KoFillConfigWidget.moc>
