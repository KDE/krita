/*
 *  SPDX-FileCopyrightText: 2015 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
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
        , colorButtonGroup(0)
        , menuAlignmentOffset(0)
        , buttonSize(26)
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

    layout->setContentsMargins(0,0,0,0);
    layout->setSpacing(0);
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

KisColorLabelSelectorWidget::~KisColorLabelSelectorWidget(){
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
    bool hasCheckable = false;
    bool hasIcon = false;

    int menuOffset = 0;

    if (menu) {
        Q_FOREACH(QAction *action, menu->actions()) {
            hasCheckable |= action->isCheckable();
            hasIcon |= action->icon().isNull();
            hasWideItems |= (hasCheckable || hasIcon);

            if (hasWideItems) {
                break;
            }
        }
    }

    if (hasWideItems) {
        QStyleOption opt;
        opt.init(this);
        // some copy-pasted code from QFusionStyle style
        const int hMargin = style()->pixelMetric(QStyle::PM_MenuHMargin, &opt, this);
        const int iconSize = style()->pixelMetric(QStyle::PM_SmallIconSize, &opt, this);
        menuOffset = (hMargin + iconSize + 6);
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
    if (index == -1) {
        QAbstractButton* btn = m_d->colorButtonGroup->checkedButton();
        if (btn) {
            btn->group()->setExclusive(false);
            btn->setChecked(false);
            btn->group()->setExclusive(true);
        }
    } else if (index != m_d->colorButtonGroup->checkedId()) {
        QAbstractButton* btn = m_d->colorButtonGroup->button(index);
        if (btn) {
            btn->setChecked(true);
        }
    }

    emit currentIndexChanged(index);
}
