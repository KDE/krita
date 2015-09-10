/*
 *  Copyright (c) 2004 Adrian Page <adrian@pagenet.plus.com>
 *  Copyright (C) 2011 Srikanth Tiyyagura <srikanth.tulasiram@gmail.com>
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

#include <klocalizedstring.h>
#include <KoAbstractGradient.h>
#include <KoResource.h>
#include <KoSegmentGradient.h>
#include <KoResourceItemChooser.h>
#include <KoResourceServerProvider.h>
#include <KoResourceServerAdapter.h>
#include <KoIcon.h>

#include "KisViewManager.h"
#include "kis_global.h"
#include "kis_autogradient.h"
#include "kis_canvas_resource_provider.h"

KisCustomGradientDialog::KisCustomGradientDialog(KoSegmentGradient* gradient, QWidget * parent, const char *name)
        : KoDialog(parent)
{
    setCaption(i18n("Custom Gradient"));
    setButtons(Close);
    setDefaultButton(Close);
    setObjectName(name);
    setModal(false);
    m_page = new KisAutogradient(gradient, this, "autogradient", i18n("Custom Gradient"));
    setMainWidget(m_page);
}

KisGradientChooser::KisGradientChooser(QWidget *parent, const char *name)
        : QFrame(parent)
{
    setObjectName(name);
    m_lbName = new QLabel();

    KoResourceServer<KoAbstractGradient> * rserver = KoResourceServerProvider::instance()->gradientServer(false);
    QSharedPointer<KoAbstractResourceServerAdapter> adapter (new KoResourceServerAdapter<KoAbstractGradient>(rserver));
    m_itemChooser = new KoResourceItemChooser(adapter, this);
    QString knsrcFile = "kritagradients.knsrc";
    m_itemChooser->setKnsrcFile(knsrcFile);
    m_itemChooser->showGetHotNewStuff(true, true);
    m_itemChooser->showTaggingBar(true);
    m_itemChooser->setFixedSize(250, 250);
    m_itemChooser->setColumnCount(1);

    connect(m_itemChooser, SIGNAL(resourceSelected(KoResource *)),
            this, SLOT(update(KoResource *)));

    connect(m_itemChooser, SIGNAL(resourceSelected(KoResource *)),
            this, SIGNAL(resourceSelected(KoResource *)));

    QWidget* buttonWidget = new QWidget(this);
    QHBoxLayout* buttonLayout = new QHBoxLayout(buttonWidget);

    QPushButton* addGradient = new QPushButton(themedIcon("list-add"), i18n("Add..."), this);
    connect(addGradient, SIGNAL(clicked()), this, SLOT(addGradient()));
    buttonLayout->addWidget(addGradient);

    m_editGradient = new QPushButton(themedIcon("configure"), i18n("Edit..."));
    m_editGradient->setEnabled(false);
    connect(m_editGradient, SIGNAL(clicked()), this, SLOT(editGradient()));
    buttonLayout->addWidget(m_editGradient);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setObjectName("main layout");
    mainLayout->setMargin(2);
    mainLayout->addWidget(m_lbName);
    mainLayout->addWidget(m_itemChooser, 10);
    mainLayout->addWidget(buttonWidget);

    setLayout(mainLayout);
}

KisGradientChooser::~KisGradientChooser()
{
}

KoResource *KisGradientChooser::currentResource()
{
    return m_itemChooser->currentResource();
}

void KisGradientChooser::setCurrentResource(KoResource *resource)
{
    m_itemChooser->setCurrentResource(resource);
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

    KoSegmentGradient *segmentGradient = dynamic_cast<KoSegmentGradient*>(gradient);
    m_editGradient->setEnabled(segmentGradient && segmentGradient->removable());
}

void KisGradientChooser::addGradient()
{
    KoResourceServer<KoAbstractGradient> * rserver = KoResourceServerProvider::instance()->gradientServer();
    QString saveLocation = rserver->saveLocation();

    KoSegmentGradient* gradient = new KoSegmentGradient("");
    gradient->createSegment(INTERP_LINEAR, COLOR_INTERP_RGB, 0.0, 1.0, 0.5, Qt::black, Qt::white);
    gradient->setName(i18n("unnamed"));

    KisCustomGradientDialog dialog(gradient, this, "autogradient");
    dialog.exec();

    gradient->setFilename(saveLocation + gradient->name() + gradient->defaultFileExtension());
    gradient->setValid(true);
    rserver->addResource(gradient);
    m_itemChooser->setCurrentResource(gradient);
}

void KisGradientChooser::editGradient()
{
    KoSegmentGradient* gradient = dynamic_cast<KoSegmentGradient*>(currentResource());
    if (gradient) {
        KisCustomGradientDialog dialog(gradient, this, "autogradient");
        dialog.exec();
    }
}




