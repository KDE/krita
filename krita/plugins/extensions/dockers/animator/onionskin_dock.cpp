/*
 *  Copyright (c) 2013 Somsubhra Bairi <somsubhra.bairi@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; version 2 of the License, or(at you option)
 *  any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "onionskin_dock.h"

#include "KisViewManager.h"
#include "kis_animation.h"
#include "kis_canvas2.h"
#include "kis_animation_doc.h"
#include "KisPart.h"
#include "kis_config.h"
#include "kis_opacity_selector_view.h"

#include <klocale.h>
#include <KColorButton>

#include <QLabel>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QColor>
#include <QCheckBox>
#include <QSpinBox>

OnionSkinDock::OnionSkinDock() : QDockWidget(i18n("Onion Skin")), m_canvas(0), m_animation(0)
{
    m_initialized = false;

    this->setMinimumSize(300, 160);

    QLabel* activeLabel = new QLabel(this);
    activeLabel->setText(i18n("Active: "));
    activeLabel->setGeometry(100, 20, 50, 20);

    KisConfig cfg;

    QCheckBox* activeCheckBox = new QCheckBox(this);
    activeCheckBox->setChecked(cfg.defOnionSkinningEnabled());
    activeCheckBox->setGeometry(150, 20, 20, 20);

    connect(activeCheckBox, SIGNAL(clicked(bool)), this, SLOT(enableOnionSkinning(bool)));

    QLabel* frameLabel = new QLabel(this);
    frameLabel->setText(i18n("Frames"));
    frameLabel->setGeometry(130, 40, 50, 20);

    QSpinBox* previousFramesInput = new QSpinBox(this);
    previousFramesInput->setRange(0, 10);
    previousFramesInput->setValue(3);
    previousFramesInput->setGeometry(60, 40, 50, 20);

    QSpinBox* nextFramesInput = new QSpinBox(this);
    nextFramesInput->setRange(0, 10);
    nextFramesInput->setValue(3);
    nextFramesInput->setGeometry(200, 40, 50, 20);

    QLabel* colorLabel = new QLabel(this);
    colorLabel->setText(i18n("Colors"));
    colorLabel->setGeometry(130, 60, 50, 20);

    KColorButton* previousFramesColor = new KColorButton(this);
    previousFramesColor->setColor(QColor(Qt::red));
    previousFramesColor->setGeometry(60, 60, 50, 20);
    connect(previousFramesColor, SIGNAL(changed(QColor)), this, SLOT(setPrevFramesColor(QColor)));

    KColorButton* nextFramesColor = new KColorButton(this);
    nextFramesColor->setColor(QColor(Qt::blue));
    nextFramesColor->setGeometry(200, 60, 50, 20);
    connect(nextFramesColor, SIGNAL(changed(QColor)), this, SLOT(setNextFramesColor(QColor)));

    QLabel* opacityLabel = new QLabel(this);
    opacityLabel->setText(i18n("Opacity"));
    opacityLabel->setGeometry(130, 80, 50, 20);

    m_previousOpacitySelectorView = new KisOpacitySelectorView(this, KisOpacitySelector::PREV_FRAMES_OPACITY_SELECTOR);
    m_previousOpacitySelectorView->setNumberOfFrames(previousFramesInput->value());
    m_previousOpacitySelectorView->setGeometry(20, 80, 105, 75);
    connect(previousFramesInput, SIGNAL(valueChanged(int)), this, SLOT(setNumberOfPrevFrames(int)));

    m_nextOpacitySelectorView = new KisOpacitySelectorView(this, KisOpacitySelector::NEXT_FRAMES_OPACITY_SELECTOR);
    m_nextOpacitySelectorView->setNumberOfFrames(nextFramesInput->value());
    m_nextOpacitySelectorView->setGeometry(180, 80, 105, 75);
    connect(nextFramesInput, SIGNAL(valueChanged(int)), this, SLOT(setNumberOfNextFrames(int)));

    // Additional signal/slot connections
    connect(m_previousOpacitySelectorView, SIGNAL(opacityValueChanged()), this, SLOT(setPrevFramesOpacityValues()));
    connect(m_nextOpacitySelectorView, SIGNAL(opacityValueChanged()), this, SLOT(setNextFramesOpacityValues()));
}

void OnionSkinDock::setCanvas(KoCanvasBase *canvas)
{
    m_canvas = dynamic_cast<KisCanvas2*>(canvas);
    if (m_canvas && m_canvas->viewManager() && m_canvas->viewManager()->document()) {
        KisAnimationDoc *doc = qobject_cast<KisAnimationDoc*>(m_canvas->viewManager()->document());
        if (doc) {
            m_animation = doc->getAnimation();
            if (m_animation) {
                this->onCavasSet();
            }
        }
    }
}

void OnionSkinDock::onCavasSet()
{
    if (!m_initialized) {
        // Set initial set of opacity values
        m_animation->setPrevOnionSkinOpacityValues(m_previousOpacitySelectorView->opacitySelector()->opacityValues());
        m_animation->setNextOnionSkinOpacityValues(m_nextOpacitySelectorView->opacitySelector()->opacityValues());
        m_initialized = true;
    }
}

void OnionSkinDock::enableOnionSkinning(bool enable)
{
    m_animation->enableOnionSkinning(enable);
    dynamic_cast<KisAnimationDoc*>(m_canvas->viewManager()->document())->onionSkinStateChanged();
}

void OnionSkinDock::setNumberOfPrevFrames(int frames)
{
    m_previousOpacitySelectorView->setNumberOfFrames(frames);
    m_animation->setPrevOnionSkinOpacityValues(m_previousOpacitySelectorView->opacitySelector()->opacityValues());
    dynamic_cast<KisAnimationDoc*>(m_canvas->viewManager()->document())->onionSkinStateChanged();
}

void OnionSkinDock::setNumberOfNextFrames(int frames)
{
    m_nextOpacitySelectorView->setNumberOfFrames(frames);
    m_animation->setNextOnionSkinOpacityValues(m_nextOpacitySelectorView->opacitySelector()->opacityValues());
    dynamic_cast<KisAnimationDoc*>(m_canvas->viewManager()->document())->onionSkinStateChanged();
}

void OnionSkinDock::setPrevFramesOpacityValues()
{
    m_animation->setPrevOnionSkinOpacityValues(m_previousOpacitySelectorView->opacitySelector()->opacityValues());
    dynamic_cast<KisAnimationDoc*>(m_canvas->viewManager()->document())->onionSkinStateChanged();
}

void OnionSkinDock::setNextFramesOpacityValues()
{
    m_animation->setNextOnionSkinOpacityValues(m_nextOpacitySelectorView->opacitySelector()->opacityValues());
    dynamic_cast<KisAnimationDoc*>(m_canvas->viewManager()->document())->onionSkinStateChanged();
}

void OnionSkinDock::setPrevFramesColor(QColor color)
{
    m_animation->setPrevOnionSkinColor(color);
    dynamic_cast<KisAnimationDoc*>(m_canvas->viewManager()->document())->onionSkinStateChanged();
}

void OnionSkinDock::setNextFramesColor(QColor color)
{
    m_animation->setNextOnionSkinColor(color);
    dynamic_cast<KisAnimationDoc*>(m_canvas->viewManager()->document())->onionSkinStateChanged();
}
