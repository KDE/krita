/*
 *  SPDX-FileCopyrightText: 2022 Deif Lou <ginoba@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include <QButtonGroup>
#include <QHBoxLayout>

#include <KoGroupButton.h>

#include <kis_assert.h>
#include "KisOptionButtonStrip.h"

class KisOptionButtonStrip::Private
{
public:
    int numberOfButtons{0};
    QButtonGroup *buttonGroup{nullptr};
};

KisOptionButtonStrip::KisOptionButtonStrip(QWidget *parent)
    : QWidget(parent)
    , m_d(new Private)
{
    setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Preferred);

    QHBoxLayout *mainLayout = new QHBoxLayout;
    mainLayout->setSpacing(0);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->addStretch();
    setLayout(mainLayout);

    m_d->buttonGroup = new QButtonGroup(this);
    m_d->buttonGroup->setExclusive(true);

    connect(
        m_d->buttonGroup,
        QOverload<QAbstractButton *, bool>::of(&QButtonGroup::buttonToggled),
        [this](QAbstractButton *button, bool checked) {
            Q_EMIT buttonToggled(dynamic_cast<KoGroupButton *>(button), checked);
            Q_EMIT buttonToggled(m_d->buttonGroup->id(button), checked);
        });
}

KisOptionButtonStrip::~KisOptionButtonStrip()
{
}

KoGroupButton *KisOptionButtonStrip::addButton(const QIcon &icon,
                                               const QString &text)
{
    KoGroupButton *newButton = new KoGroupButton(this);
    newButton->setCheckable(true);
    newButton->setIcon(icon);
    newButton->setText(text);
    newButton->setMinimumSize(28, 28);

    if (m_d->numberOfButtons > 0) {
        KoGroupButton *prevButton = dynamic_cast<KoGroupButton *>(
            m_d->buttonGroup->button(m_d->numberOfButtons - 1));
        prevButton->setGroupPosition(m_d->numberOfButtons == 1
                                         ? KoGroupButton::GroupLeft
                                         : KoGroupButton::GroupCenter);
        newButton->setGroupPosition(KoGroupButton::GroupRight);
    }

    m_d->buttonGroup->addButton(newButton, m_d->numberOfButtons);

    QHBoxLayout *mainLayout = dynamic_cast<QHBoxLayout *>(layout());
    KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(mainLayout, newButton);
    mainLayout->insertWidget(m_d->numberOfButtons, newButton);

    ++m_d->numberOfButtons;

    return newButton;
}

KoGroupButton *KisOptionButtonStrip::addButton(const QString &text)
{
    return addButton(QIcon(), text);
}

KoGroupButton *KisOptionButtonStrip::addButton()
{
    return addButton(QIcon(), QString());
}

KoGroupButton *KisOptionButtonStrip::button(int index) const
{
    return dynamic_cast<KoGroupButton *>(m_d->buttonGroup->button(index));
}

QList<KoGroupButton *> KisOptionButtonStrip::buttons() const
{
    QList<KoGroupButton *> list;
    for (QAbstractButton *b : m_d->buttonGroup->buttons()) {
        list.append(dynamic_cast<KoGroupButton *>(b));
    }
    return list;
}

bool KisOptionButtonStrip::exclusive() const
{
    return m_d->buttonGroup->exclusive();
}

void KisOptionButtonStrip::setExclusive(bool exclusive)
{
    m_d->buttonGroup->setExclusive(exclusive);
}

KoGroupButton *KisOptionButtonStrip::checkedButton() const
{
    return dynamic_cast<KoGroupButton *>(m_d->buttonGroup->checkedButton());
}

int KisOptionButtonStrip::checkedButtonIndex() const
{
    return m_d->buttonGroup->checkedId();
}
