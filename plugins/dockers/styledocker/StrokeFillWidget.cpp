/* This file is part of the KDE project
 * Copyright (C) 2007-2009,2011 Jan Hambrecht <jaham@gmx.net>
 * Copyright (C) 2008-2010 Thorsten Zachmann <zachmann@kde.org>
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

// Own
#include "StrokeFillWidget.h"

// Qt
#include <QGridLayout>
#include <QStackedWidget>
#include <QToolButton>
#include <QLabel>

// KDE
#include <klocale.h>

// Calligra
#include <KoFlake.h>
#include <KoGradientBackground.h>
#include <KoPageApp.h>
#include <KoToolManager.h>
#include <KoCanvasBase.h>
#include <KoCanvasController.h>
#include <KoCanvasResourceManager.h>
#include <KoDocumentResourceManager.h>
#include <KoShapeManager.h>
#include <KoShapeStroke.h>
#include <KoPathShape.h>
#include <KoColorSpaceRegistry.h>
#include <KoColorBackground.h>
#include <KoPatternBackground.h>
#include <KoImageCollection.h>
#include <KoShapeController.h>
#include <KoResourceSelector.h>
#include <KoResourceServerProvider.h>
#include <KoResourceServerAdapter.h>
#include <KoColorPopupAction.h>
#include <KoSliderCombo.h>

// This docker
#include "StylePreview.h"
#include "StyleButtonBox.h"


const int MsecsThresholdForMergingCommands = 2000;

StyleButtonBox::StyleButtons StrokeButtons = (StyleButtonBox::None
                                              | StyleButtonBox::Solid
                                              | StyleButtonBox::Gradient);

StyleButtonBox::StyleButtons FillButtons   = (StyleButtonBox::None
                                              | StyleButtonBox::Solid
                                              | StyleButtonBox::Gradient
                                              | StyleButtonBox::Pattern);

StyleButtonBox::StyleButtons FillRuleButtons = (StyleButtonBox::EvenOdd
                                                | StyleButtonBox::Winding);


StrokeFillWidget::StrokeFillWidget(QWidget * parent)
    : QWidget(parent)
    , m_lastColorFill(0)
{
    // Preview of the stroke/fill
    m_preview = new StylePreview(this);

    m_buttons = new StyleButtonBox(this, 2, 3);
    m_buttons->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);

    // Stack widget that shows one of color/gradient/pattern selector at any given time.
    m_stack = new QStackedWidget(this);
    m_stack->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);

    // Opacity setting
    // FIXME: There is also an opacity setting in the color chooser. How do they interact?
    m_opacity = new KoSliderCombo(this);
    m_opacity->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
    m_opacity->setMinimum(0);
    m_opacity->setMaximum(100);
    m_opacity->setValue(100);
    m_opacity->setDecimals(0);

    m_spacer = new QSpacerItem(0, 0, QSizePolicy::Fixed, QSizePolicy::Fixed);

    // The main layout of this widget
    m_layout = new QGridLayout(this);
    m_layout->addWidget(m_preview, 0, 0, 2, 1);
    m_layout->addWidget(m_buttons, 0, 1, 2, 1);
    m_layout->addWidget(m_stack, 0, 2, 1, 2);
    m_layout->addWidget(new QLabel(i18n("Opacity:")), 1, 2);
    m_layout->addWidget(m_opacity, 1, 3);
    m_layout->addItem(m_spacer, 2, 2);
    m_layout->setMargin(0);
    m_layout->setVerticalSpacing(0);
    m_layout->setSizeConstraint(QLayout::SetMinAndMaxSize);

    // Color selector (part of the stack widget)
    m_colorSelector = new QToolButton(m_stack);
    m_actionColor = new KoColorPopupAction(m_stack);
    m_colorSelector->setDefaultAction(m_actionColor);

    // Gradient selector (part of the stack widget)
    KoResourceServerProvider *serverProvider = KoResourceServerProvider::instance();
    KoAbstractResourceServerAdapter *gradientResourceAdapter = new KoResourceServerAdapter<KoAbstractGradient>(serverProvider->gradientServer(), this);
    KoResourceSelector *gradientSelector = new KoResourceSelector(gradientResourceAdapter, m_stack);
    gradientSelector->setColumnCount(1);
    gradientSelector->setRowHeight(20);
    gradientSelector->setMinimumWidth(100);

    // Pattern selector (part of the stack widget)
    KoAbstractResourceServerAdapter * patternResourceAdapter = new KoResourceServerAdapter<KoPattern>(serverProvider->patternServer(), this);
    KoResourceSelector * patternSelector = new KoResourceSelector(patternResourceAdapter, m_stack);
    patternSelector->setColumnCount(5);
    patternSelector->setRowHeight(30);
    patternSelector->setMinimumWidth(100);

    m_stack->addWidget(m_colorSelector);
    m_stack->addWidget(gradientSelector);
    m_stack->addWidget(patternSelector);
    m_stack->setContentsMargins(0, 0, 0, 0);
    m_stack->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
    m_stack->setMinimumWidth(100);

    // Signals from the preview.
    connect(m_preview, SIGNAL(fillSelected()), this, SLOT(fillSelected()));
    connect(m_preview, SIGNAL(strokeSelected()), this, SLOT(strokeSelected()));

    connect(m_buttons, SIGNAL(buttonPressed(int)), this, SLOT(styleButtonPressed(int)));

    connect(m_actionColor, SIGNAL(colorChanged(const KoColor &)),
            this,          SIGNAL(colorChanged(const KoColor &))); // Yes, this is allowed
    connect(gradientSelector, SIGNAL(resourceSelected(KoResource*)),
            this,             SIGNAL(gradientChanged(KoResource*)));
    connect(gradientSelector, SIGNAL(resourceApplied(KoResource*)),
            this,             SIGNAL(gradientChanged(KoResource*)));
    connect(patternSelector, SIGNAL(resourceSelected(KoResource*)),
            this,            SIGNAL(patternChanged(KoResource*)));
    connect(patternSelector, SIGNAL(resourceApplied(KoResource*)),
            this,            SIGNAL(patternChanged(KoResource*)));
    connect(m_opacity, SIGNAL(valueChanged(qreal, bool)),
            this,      SIGNAL(opacityChanged(qreal)));
}

StrokeFillWidget::~StrokeFillWidget()
{
}


// ----------------------------------------------------------------


void StrokeFillWidget::updateWidget(KoShapeStrokeModel *stroke, KoShapeBackground *fill,
                                    int opacity, QColor &currentColor, int activeStyle)
{
    m_preview->update(stroke, fill);
    updateStyleButtons(activeStyle);

    // We don't want the opacity slider to send any signals when it's only initialized.
    // Otherwise an undo record is created.
    m_opacity->blockSignals (true);
    m_opacity->setValue(opacity);
    m_opacity->blockSignals (false);

    m_actionColor->setCurrentColor(currentColor);
}


void StrokeFillWidget::fillSelected()
{
    updateStyleButtons(KoFlake::Background);
    emit aspectSelected(KoFlake::Background);

}

void StrokeFillWidget::strokeSelected()
{
    updateStyleButtons(KoFlake::Foreground);
    emit aspectSelected(KoFlake::Foreground);
}

void StrokeFillWidget::styleButtonPressed(int buttonId)
{
    // FIXME: We should make it behave similarly no matter which
    //        button is pressed (direct manipulation vs only select mode).
    switch (buttonId) {
    case StyleButtonBox::None:
        // Direct manipulation
        m_stack->setCurrentIndex(0);
        emit noColorSelected();
    case StyleButtonBox::Solid:
        // Only select mode in the widget, don't set actual gradient :/ .
        m_stack->setCurrentIndex(0);
        break;
    case StyleButtonBox::Gradient:
        // Only select mode in the widget, don't set actual gradient :/ .
        m_stack->setCurrentIndex(1);
        break;
    case StyleButtonBox::Pattern:
        // Only select mode in the widget, don't set actual pattern :/ .
        m_stack->setCurrentIndex(2);
        break;
    case StyleButtonBox::EvenOdd:
        // Direct manipulation
        emit fillruleChanged(Qt::OddEvenFill);
        break;
    case StyleButtonBox::Winding:
        // Direct manipulation
        emit fillruleChanged(Qt::WindingFill);
        break;
    }
}


void StrokeFillWidget::updateStyleButtons(int activeStyle)
{
    if (activeStyle == KoFlake::Background) {
        m_buttons->showButtons(FillButtons | FillRuleButtons);
    }
    else {
        m_buttons->showButtons(StrokeButtons);

        // Patterns cannot be used in stroke, so set to solid if pattern was chosen before.
        if (m_stack->currentIndex() == 2)
            m_stack->setCurrentIndex(0);
    }
}

void StrokeFillWidget::setStretchPolicy(StrokeFillWidget::StretchPolicy policy)
{
    switch (policy) {
    case StretchWidth:
        m_spacer->changeSize(0, 0, QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);
        break;
    case StretchHeight:
        m_spacer->changeSize(0, 0, QSizePolicy::Fixed, QSizePolicy::MinimumExpanding);
        break;
    default:
        break;
    }
    m_layout->setSizeConstraint(QLayout::SetMinAndMaxSize);
    m_layout->invalidate();
}


KoShapeBackground *StrokeFillWidget::applyFillGradientStops(KoShape *shape,
                                                            const QGradientStops &stops)
{
    if (! shape || ! stops.count())
        return 0;

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

QBrush StrokeFillWidget::applyStrokeGradientStops(KoShape *shape, const QGradientStops &stops)
{
    if (! shape || ! stops.count())
        return QBrush();

    QBrush gradientBrush;
    KoShapeStroke *stroke = dynamic_cast<KoShapeStroke*>(shape->stroke());
    if (stroke)
        gradientBrush = stroke->lineBrush();

    QGradient *newGradient = 0;
    const QGradient *oldGradient = gradientBrush.gradient();
    if (oldGradient) {
        // just copy the new gradient stops
        newGradient = KoFlake::cloneGradient(oldGradient);
        newGradient->setStops(stops);
    }
    else {
        // no gradient yet, so create a new one
        QLinearGradient *g = new QLinearGradient(QPointF(0, 0), QPointF(1, 1));
        g->setCoordinateMode(QGradient::ObjectBoundingMode);
        g->setStops(stops);
        newGradient = g;
    }

    QBrush brush(*newGradient);
    delete newGradient;

    return brush;
}

#include <StrokeFillWidget.moc>
