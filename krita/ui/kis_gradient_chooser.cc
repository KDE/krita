/*
 *  Copyright (c) 2004 Adrian Page <adrian@pagenet.plus.com>
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

#include <QLabel>
#include <QLayout>
#include <QPushButton>
#include <QVBoxLayout>

#include <klocale.h>

#include "kis_view.h"
#include "kis_global.h"
#include "kis_icon_item.h"
#include "kis_gradient.h"
#include "kis_autogradient.h"

#include "kis_gradient_chooser.h"

KisCustomGradientDialog::KisCustomGradientDialog(KisView * view, QWidget * parent, const char *name)
    : KDialog(parent)
{
    setCaption( i18n("Custom Gradient") );
    setButtons(  Close);
    setDefaultButton( Close );
    setObjectName(name);
    setModal(false);
    m_page = new KisAutogradient(this, "autogradient", i18n("Custom Gradient"));
    setMainWidget(m_page);
    connect(m_page, SIGNAL(activatedResource(KisResource *)), view, SLOT(gradientActivated(KisResource*)));
}

KisGradientChooser::KisGradientChooser(KisView * view, QWidget *parent, const char *name) : super(parent, name)
{
    m_lbName = new QLabel();

    m_customGradient = new QPushButton(i18n("Custom Gradient..."));
    m_customGradient->setObjectName("custom gradient button");

    KisCustomGradientDialog * autogradient = new KisCustomGradientDialog(view, 0, "autogradient");
    connect(m_customGradient, SIGNAL(clicked()), autogradient, SLOT(show()));

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setObjectName("main layout");
    mainLayout->setMargin(2);
    mainLayout->addWidget(m_lbName);
    mainLayout->addWidget(chooserWidget(), 10);
    mainLayout->addWidget(m_customGradient, 10);
    setLayout(mainLayout);
}

KisGradientChooser::~KisGradientChooser()
{
}

void KisGradientChooser::update(QTableWidgetItem *item)
{
    KisIconItem *kisItem = static_cast<KisIconItem *>(item);

    if (item) {
        KisGradient *gradient = static_cast<KisGradient *>(kisItem->resource());

        m_lbName->setText(gradient->name());
    }
}


#include "kis_gradient_chooser.moc"

