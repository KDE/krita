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

#include "kis_paintop_presets_widget.h"

#include <QList>
#include <QComboBox>
#include <QHBoxLayout>
#include <QToolButton>

#include <kis_paintop_preset.h>

#include <ui_wdgpaintoppresets.h>

class KisPaintOpOption::Private {
public:    
    bool checked;
    QString label;
    QWidget * configurationPage;
};

KisPaintOpOption::KisPaintOpOption( const QString & label, bool checked )
    : m_d(new Private())
{
    m_d->checked = checked;
    m_d->label = label;
    m_d->configurationPage = 0;
}

KisPaintOpOption::~KisPaintOpOption()
{
    delete m_d;
}

void KisPaintOpOption::setConfigurationPage( QWidget * page )
{
    m_d->configurationPage = page;
}

QWidget * KisPaintOpOption::configurationPage() const
{
    return m_d->configurationPage;
}


class KisPaintOpPresetsWidget::Private
{

public:

    QList<KisPaintOpOption*> paintOpOptions;
    Ui_WdgPaintOpPresets uiWdgPaintOpPresets;
    
};

KisPaintOpPresetsWidget::KisPaintOpPresetsWidget( QWidget * parent )
    : QWidget( parent )
    , m_d(new Private())
{
    setObjectName("KisPaintOpPresetsWidget");
    m_d->uiWdgPaintOpPresets.setupUi( this );
}


KisPaintOpPresetsWidget::~KisPaintOpPresetsWidget()
{
    foreach(KisPaintOpOption* option, m_d->paintOpOptions) {
        delete option;
    }
    delete m_d;
}

void KisPaintOpPresetsWidget::addPaintOpOption( KisPaintOpOption * option )
{
    m_d->paintOpOptions << option;
}
