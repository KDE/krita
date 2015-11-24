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

#include "kis_lod_availability_widget.h"

#include <QPushButton>
#include <QCheckBox>
#include <QHBoxLayout>
#include <QToolTip>


#include <kis_canvas_resource_provider.h>

struct KisLodAvailabilityWidget::Private
{
    Private() : chkLod(0), resourceManager(0) {}

    QCheckBox *chkLod;
    QPushButton *btnLod;
    KoCanvasResourceManager *resourceManager;

    void updateLodStatus();
};

KisLodAvailabilityWidget::KisLodAvailabilityWidget(QWidget *parent)
    : QWidget(parent),
      m_d(new Private)
{
    m_d->chkLod = new QCheckBox(this);

    m_d->btnLod = new QPushButton(this);
    m_d->btnLod->setFlat(true);

    connect(m_d->btnLod, SIGNAL(clicked()), SLOT(showLodToolTip()));

    QHBoxLayout *layout = new QHBoxLayout;
    layout->addWidget(m_d->chkLod);
    layout->addWidget(m_d->btnLod);

    layout->setSpacing(0);

    setLayout(layout);

    // set no limitations
    KisPaintopLodLimitations l;
    setLimitations(l);

    connect(m_d->chkLod, SIGNAL(toggled(bool)), SIGNAL(sigUserChangedLodAvailability(bool)));
}

KisLodAvailabilityWidget::~KisLodAvailabilityWidget()
{
}

void KisLodAvailabilityWidget::showLodToolTip()
{
    QToolTip::showText(QCursor::pos(), m_d->btnLod->toolTip(), m_d->btnLod);
}

void KisLodAvailabilityWidget::setLimitations(const KisPaintopLodLimitations &l)
{
    QString limitationsText;
    foreach (const KoID &id, l.limitations) {
        limitationsText.append("<item>");
        limitationsText.append(id.name());
        limitationsText.append("</item>");
    }

    QString blockersText;
    foreach (const KoID &id, l.blockers) {
        blockersText.append("<item>");
        blockersText.append(id.name());
        blockersText.append("</item>");
    }

    bool isBlocked = !l.blockers.isEmpty();
    bool isLimited = !l.limitations.isEmpty();
    QString text = !isBlocked && isLimited ? i18n("(Instant Preview)*") : i18n("Instant Preview");

    QString toolTip;

    if (isBlocked) {
        toolTip.append(i18nc("@info:tooltip",
                             "<para>Instant Preview Mode is "
                             "disabled by the following options:"
                             "<list>%1</list></para>", blockersText));

    } else if (isLimited) {
        toolTip.append(i18nc("@info:tooltip",
                             "<para>Instant Preview may look different "
                             "from the final result. In case of troubles "
                             "try disabling the following options:"
                             "<list>%1</list></para>", limitationsText));
    } else {
        toolTip = i18nc("@info:tooltip", "<para>Instant Preview Mode is available</para>");
    }

    {
        QFont font;
        font.setStrikeOut(isBlocked);
        m_d->chkLod->setEnabled(!isBlocked);

        m_d->btnLod->setEnabled(!isBlocked);
        m_d->btnLod->setFont(font);
        m_d->btnLod->setText(text);
        m_d->btnLod->setToolTip(toolTip);
    }

    m_d->updateLodStatus();
}

void KisLodAvailabilityWidget::slotUserChangedLodAvailability(bool value)
{
    m_d->chkLod->setChecked(value);
    m_d->updateLodStatus();
}

void KisLodAvailabilityWidget::setCanvasResourceManager(KoCanvasResourceManager *resourceManager)
{
    m_d->resourceManager = resourceManager;
}

void KisLodAvailabilityWidget::Private::updateLodStatus()
{
    if (!resourceManager) return;

    bool value = chkLod->isChecked() && chkLod->isEnabled();
    resourceManager->setResource(KisCanvasResourceProvider::PresetAllowsLod, value);
}
