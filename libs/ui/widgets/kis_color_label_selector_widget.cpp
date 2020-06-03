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
#include <QStyleOption>

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QButtonGroup>
#include <QSpacerItem>

#include "kis_color_label_button.h"
#include "kis_node_view_color_scheme.h"

struct Private
{
    Private(KisColorLabelSelectorWidget *_q)
        : q(_q)
        , buttonSize(24)
    {
    }

    KisColorLabelSelectorWidget *q;
    QVector<QColor> colors;
    QButtonGroup* colorButtonGroup;
    QSpacerItem* menuAlignmentOffset;
    const int buttonSize;
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
    layout->setAlignment(Qt::AlignLeft);
    m_d->menuAlignmentOffset = new QSpacerItem(0,0);
    layout->addItem(m_d->menuAlignmentOffset);

    {
        m_d->colorButtonGroup = new QButtonGroup(this);
        m_d->colorButtonGroup->setExclusive(true);

        for (int id = 0; id < m_d->colors.count(); id++) {
            KisColorLabelButton* btn = new KisColorLabelButton(m_d->colors[id], m_d->buttonSize, this);
            btn->setChecked(false);
            btn->setSelectionVisType(KisColorLabelButton::Outline);
            m_d->colorButtonGroup->addButton(btn, id);
            layout->addWidget(btn);
        }

        connect(m_d->colorButtonGroup, SIGNAL(buttonToggled(int,bool)), this, SLOT(groupButtonChecked(int,bool)));
    }
}

KisColorLabelSelectorWidget::~KisColorLabelSelectorWidget()
{
    delete m_d->menuAlignmentOffset;
}

int KisColorLabelSelectorWidget::currentIndex() const
{
    return m_d->colorButtonGroup->checkedId();
}

QSize KisColorLabelSelectorWidget::sizeHint() const
{
    return QSize(calculateMenuOffset() + m_d->buttonSize * m_d->colors.count(), m_d->buttonSize);
}

void KisColorLabelSelectorWidget::resizeEvent(QResizeEvent *e) {
    int menuOffset = calculateMenuOffset();

    m_d->menuAlignmentOffset->changeSize(menuOffset, height());
    layout()->invalidate();

    QMenu *menu = qobject_cast<QMenu*>(parent());

    if(menu) {
        menu->resize(menu->width() + menuOffset, menu->height());
    }

    QWidget::resizeEvent(e);
}

int KisColorLabelSelectorWidget::calculateMenuOffset() const
{
    bool hasWideItems = false;
    QMenu *menu = qobject_cast<QMenu*>(parent());
    int menuOffset = 0;

    if (menu) {
        Q_FOREACH(QAction *action, menu->actions()) {
            if (action->isCheckable() ||
                !action->icon().isNull()) {

                hasWideItems = true;
                break;
            }
        }
    }

    if (hasWideItems) {
        QStyleOption opt;
        opt.init(this);
        // some copy-pasted code from QFusionStyle style
        const int hmargin = style()->pixelMetric(QStyle::PM_MenuHMargin, &opt, this);
        const int icone = style()->pixelMetric(QStyle::PM_SmallIconSize, &opt, this);
        menuOffset = hmargin + icone + 6;
    }

    return menuOffset;
}

void KisColorLabelSelectorWidget::groupButtonChecked(int index, bool state)
{
    if (state == true) {
        emit currentIndexChanged(index);
    }
}

void KisColorLabelSelectorWidget::setCurrentIndex(int index)
{
    if (index != m_d->colorButtonGroup->checkedId()) {
        QAbstractButton* btn = m_d->colorButtonGroup->button(index);
        if (btn) {
            btn->setChecked(true);
        }
    }

    emit currentIndexChanged(index);
}
