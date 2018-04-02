/*
 *  Copyright (c) 2005 Boudewijn Rempt <boud@valdyas.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "kis_selection_options.h"

#include <QWidget>
#include <QRadioButton>
#include <QComboBox>
#include <QVBoxLayout>
#include <QLayout>
#include <QButtonGroup>

#include <kis_icon.h>
#include "kis_types.h"
#include "kis_layer.h"
#include "kis_image.h"
#include "kis_selection.h"
#include "kis_paint_device.h"
#include "canvas/kis_canvas2.h"
#include "KisViewManager.h"

KisSelectionOptions::KisSelectionOptions(KisCanvas2 * /*canvas*/)
{
    m_page = new WdgSelectionOptions(this);
    Q_CHECK_PTR(m_page);

    QVBoxLayout * l = new QVBoxLayout(this);
    l->addWidget(m_page);
    l->addSpacerItem(new QSpacerItem(0,0, QSizePolicy::Preferred, QSizePolicy::Expanding));
    l->setContentsMargins(0,0,0,0);

    m_mode = new QButtonGroup(this);
    m_mode->addButton(m_page->pixel, PIXEL_SELECTION);
    m_mode->addButton(m_page->shape, SHAPE_PROTECTION);

    m_action = new QButtonGroup(this);
    m_action->addButton(m_page->add, SELECTION_ADD);
    m_action->addButton(m_page->subtract, SELECTION_SUBTRACT);
    m_action->addButton(m_page->replace, SELECTION_REPLACE);
    m_action->addButton(m_page->intersect, SELECTION_INTERSECT);

    m_page->pixel->setGroupPosition(KoGroupButton::GroupLeft);
    m_page->shape->setGroupPosition(KoGroupButton::GroupRight);
    m_page->pixel->setIcon(KisIconUtils::loadIcon("select_pixel"));
    m_page->shape->setIcon(KisIconUtils::loadIcon("select_shape"));

    m_page->add->setGroupPosition(KoGroupButton::GroupCenter);
    m_page->subtract->setGroupPosition(KoGroupButton::GroupRight);
    m_page->replace->setGroupPosition(KoGroupButton::GroupLeft);
    m_page->intersect->setGroupPosition(KoGroupButton::GroupCenter);
    m_page->add->setIcon(KisIconUtils::loadIcon("selection_add"));
    m_page->subtract->setIcon(KisIconUtils::loadIcon("selection_subtract"));
    m_page->replace->setIcon(KisIconUtils::loadIcon("selection_replace"));
    m_page->intersect->setIcon(KisIconUtils::loadIcon("selection_intersect"));

    connect(m_mode, SIGNAL(buttonClicked(int)), this, SIGNAL(modeChanged(int)));
    connect(m_action, SIGNAL(buttonClicked(int)), this, SIGNAL(actionChanged(int)));
    connect(m_mode, SIGNAL(buttonClicked(int)), this, SLOT(hideActionsForSelectionMode(int)));

}

KisSelectionOptions::~KisSelectionOptions()
{
}

int KisSelectionOptions::action()
{
    return m_action->checkedId();
}

void KisSelectionOptions::setAction(int action) {
    QAbstractButton* button = m_action->button(action);
    Q_ASSERT(button);
    if(button) button->setChecked(true);
}

void KisSelectionOptions::setMode(int mode) {
    QAbstractButton* button = m_mode->button(mode);
    Q_ASSERT(button);
    if(button) button->setChecked(true);
    hideActionsForSelectionMode(mode);
}

//hide action buttons and antialiasing, if shape selection is active (actions currently don't work on shape selection)
void KisSelectionOptions::hideActionsForSelectionMode(int mode) {
    bool isPixelSelection = (mode == (int)PIXEL_SELECTION);

    m_page->lblAction->setVisible(isPixelSelection);
    m_page->add->setVisible(isPixelSelection);
    m_page->subtract->setVisible(isPixelSelection);
    m_page->replace->setVisible(isPixelSelection);
    m_page->intersect->setVisible(isPixelSelection);
    m_page->chkAntiAliasing->setVisible(isPixelSelection);
}

bool KisSelectionOptions::antiAliasSelection()
{
    return m_page->chkAntiAliasing->isChecked();
}

void KisSelectionOptions::disableAntiAliasSelectionOption()
{
    m_page->chkAntiAliasing->hide();
    disconnect(m_page->pixel, SIGNAL(clicked()), m_page->chkAntiAliasing, SLOT(show()));
}

void KisSelectionOptions::disableSelectionModeOption()
{
    m_page->lblMode->hide();
    m_page->pixel->hide();
    m_page->shape->hide();
}

