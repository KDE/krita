/* This file is part of the KDE project
 * Copyright (C) Boudewijn Rempt <boud@valdyas.org>, (C) 2008
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

#include "kis_paintop_options_widget.h"
#include "kis_paintop_option.h"

#include <QList>
#include <QLabel>
#include <QHBoxLayout>
#include <QListWidget>
#include <QStackedWidget>

#include <kis_paintop_preset.h>

class KisPaintOpOptionsWidget::Private
{

public:

    QList<KisPaintOpOption*> paintOpOptions;
    QListWidget * optionsList;
    QStackedWidget * optionsStack;
};

KisPaintOpOptionsWidget::KisPaintOpOptionsWidget(QWidget * parent)
        : QWidget(parent)
        , m_d(new Private())
{
    setObjectName("KisPaintOpPresetsWidget");
    QHBoxLayout * layout = new QHBoxLayout(this);
    m_d->optionsList = new QListWidget(this);
    m_d->optionsList->setMinimumWidth(128);
    m_d->optionsStack = new QStackedWidget(this);
    layout->addWidget(m_d->optionsList);
    layout->addWidget(m_d->optionsStack);

    connect(m_d->optionsList,
            SIGNAL(currentItemChanged(QListWidgetItem *, QListWidgetItem *)),
            this, SLOT(changePage(QListWidgetItem *, QListWidgetItem*)));
}


KisPaintOpOptionsWidget::~KisPaintOpOptionsWidget()
{
    delete m_d;
}

void KisPaintOpOptionsWidget::addPaintOpOption(KisPaintOpOption * option)
{
    // XXX
    if (!option->configurationPage()) return;

    m_d->paintOpOptions << option;
    m_d->optionsStack->addWidget(option->configurationPage());
    m_d->paintOpOptions << option;

    QListWidgetItem * item = new QListWidgetItem(m_d->optionsList);
    item->setText(option->label());
    item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
}

void KisPaintOpOptionsWidget::changePage(QListWidgetItem *current, QListWidgetItem *previous)
{
    Q_UNUSED(previous);
    m_d->optionsStack->setCurrentIndex(m_d->optionsList->row(current));
}

#include "kis_paintop_options_widget.moc"
