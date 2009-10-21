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

#include "widgets/kis_gradient_chooser.h"

#include <QLabel>
#include <QLayout>
#include <QPushButton>
#include <QVBoxLayout>

#include <klocale.h>
#include <kfiledialog.h>
#include <KoAbstractGradient.h>
#include <KoResource.h>
#include <KoResourceItemChooser.h>
#include <KoResourceServerProvider.h>
#include <KoResourceServerAdapter.h>

#include "kis_view2.h"
#include "kis_global.h"
#include "kis_autogradient.h"
#include "kis_canvas_resource_provider.h"

KisCustomGradientDialog::KisCustomGradientDialog(KisView2 * view, QWidget * parent, const char *name)
        : KDialog(parent)
{
    setCaption(i18n("Custom Gradient"));
    setButtons(Close);
    setDefaultButton(Close);
    setObjectName(name);
    setModal(false);
    m_page = new KisAutogradient(this, "autogradient", i18n("Custom Gradient"));
    setMainWidget(m_page);
    connect(m_page, SIGNAL(activatedResource(KoResource *)), view->resourceProvider(), SLOT(slotGradientActivated(KoResource*)));
}

KisGradientChooser::KisGradientChooser(KisView2 * view, QWidget *parent, const char *name)
        : QFrame(parent)
{
    setObjectName(name);
    m_lbName = new QLabel();

    KoResourceServer<KoAbstractGradient> * rserver = KoResourceServerProvider::instance()->gradientServer();
    KoAbstractResourceServerAdapter* adapter = new KoResourceServerAdapter<KoAbstractGradient>(rserver);
    m_itemChooser = new KoResourceItemChooser(adapter, this);
    m_itemChooser->setFixedSize(250, 250);
    m_itemChooser->setColumnCount(1);

    connect(m_itemChooser, SIGNAL(resourceSelected(KoResource *)),
            this, SLOT(update(KoResource *)));

    connect(m_itemChooser, SIGNAL(resourceSelected(KoResource *)),
            this, SIGNAL(resourceSelected(KoResource *)));

    m_customGradient = new QPushButton(i18n("Custom Gradient..."));
    m_customGradient->setObjectName("custom gradient button");
    KisCustomGradientDialog * autogradient = new KisCustomGradientDialog(view, 0, "autogradient");
    connect(m_customGradient, SIGNAL(clicked()), autogradient, SLOT(show()));

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setObjectName("main layout");
    mainLayout->setMargin(2);
    mainLayout->addWidget(m_lbName);
    mainLayout->addWidget(m_itemChooser, 10);
    mainLayout->addWidget(m_customGradient, 10);

    setLayout(mainLayout);
}

KisGradientChooser::~KisGradientChooser()
{
}

KoResource *  KisGradientChooser::currentResource()
{
    return m_itemChooser->currentResource();
}

void KisGradientChooser::setCurrentItem(int row, int column)
{
    m_itemChooser->setCurrentItem(row, column);
    if (currentResource())
        update(currentResource());
}

void KisGradientChooser::update(KoResource * resource)
{
    KoAbstractGradient *gradient = static_cast<KoAbstractGradient *>(resource);
    m_lbName->setText(i18n(gradient->name().toUtf8().data()));
}

#include "kis_gradient_chooser.moc"

