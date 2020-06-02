/*
 *  Copyright (c) 2015 Dmitry Kazakov <dimula73@gmail.com>
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

#include "kis_color_label_selector_widget.h"

#include "kis_debug.h"
#include "kis_global.h"

#include <QAction>
#include <QIcon>
#include <QMenu>

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QButtonGroup>

#include "kis_color_label_button.h"
#include "kis_node_view_color_scheme.h"

struct Private
{
    Private(KisColorLabelSelectorWidget *_q)
        : q(_q)
    {
    }

    KisColorLabelSelectorWidget *q;
    QVector<QColor> colors;
    QButtonGroup* group;
};

KisColorLabelSelectorWidget::KisColorLabelSelectorWidget(QWidget *parent)
    : QWidget(parent)
    , m_d(new Private(this))
{
    KisNodeViewColorScheme scm;
    m_d->colors = scm.allColorLabels();

    QHBoxLayout *layout = new QHBoxLayout(this);

    this->setLayout(layout);
    layout->setContentsMargins(0,0,0,0);
    layout->setAlignment(Qt::AlignCenter);

    {
        m_d->group = new QButtonGroup(this);
        m_d->group->setExclusive(true);

        for (int id = 0; id < m_d->colors.count(); id++) {
            KisColorLabelButton* btn = new KisColorLabelButton(m_d->colors[id], 24, this);
            btn->setChecked(false);
            btn->setSelectionVisType(KisColorLabelButton::Outline);
            m_d->group->addButton(btn, id);
            layout->addWidget(btn);
        }

        connect(m_d->group, SIGNAL(buttonToggled(int,bool)), this, SLOT(groupButtonChecked(int,bool)));
    }
}

KisColorLabelSelectorWidget::~KisColorLabelSelectorWidget()
{
}

int KisColorLabelSelectorWidget::currentIndex() const
{
    return m_d->group->checkedId();
}

void KisColorLabelSelectorWidget::groupButtonChecked(int index, bool state)
{
    if (state == true) {
        emit currentIndexChanged(index);
    }
}

void KisColorLabelSelectorWidget::setCurrentIndex(int index)
{
    if (index != m_d->group->checkedId()) {
        QAbstractButton* btn = m_d->group->button(index);
        if (btn) {
            btn->setChecked(true);
        }
    }

    emit currentIndexChanged(index);
}
