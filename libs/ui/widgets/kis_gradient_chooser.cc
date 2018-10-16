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
#include <QMenu>

#include <klocalizedstring.h>
#include <resources/KoAbstractGradient.h>
#include <KoResource.h>
#include <resources/KoSegmentGradient.h>
#include <KoResourceItemView.h>
#include <KisKineticScroller.h>
#include <KoStopGradient.h>
#include <KoColorSpaceRegistry.h>
#include <KoResourceItemChooser.h>
#include <KoResourceServerProvider.h>
#include <KoResourceServerAdapter.h>
#include <kis_icon.h>
#include <kis_config.h>

#include "KisViewManager.h"
#include "kis_global.h"
#include "kis_autogradient.h"
#include "kis_canvas_resource_provider.h"
#include "kis_stopgradient_editor.h"

KisCustomGradientDialog::KisCustomGradientDialog(KoAbstractGradient* gradient, QWidget * parent, const char *name)
    : KoDialog(parent)
{
    setCaption(i18n("Custom Gradient"));
    setButtons(Close);
    setDefaultButton(Close);
    setObjectName(name);
    setModal(false);

    KoStopGradient* stopGradient = dynamic_cast<KoStopGradient*>(gradient);
    if (stopGradient) {
        m_page = new KisStopGradientEditor(stopGradient, this, "autogradient", i18n("Custom Gradient"));
    }
    KoSegmentGradient* segmentedGradient = dynamic_cast<KoSegmentGradient*>(gradient);
    if (segmentedGradient) {
        m_page = new KisAutogradient(segmentedGradient, this, "autogradient", i18n("Custom Gradient"));
    }
    setMainWidget(m_page);
}

KisGradientChooser::KisGradientChooser(QWidget *parent, const char *name)
    : QFrame(parent)
{
    setObjectName(name);
    m_lbName = new QLabel();

    KoResourceServer<KoAbstractGradient> * rserver = KoResourceServerProvider::instance()->gradientServer();
    QSharedPointer<KoAbstractResourceServerAdapter> adapter (new KoResourceServerAdapter<KoAbstractGradient>(rserver));
    m_itemChooser = new KoResourceItemChooser(adapter, this);

    m_itemChooser->showTaggingBar(true);
    m_itemChooser->setFixedSize(250, 250);
    m_itemChooser->setColumnCount(1);

    connect(m_itemChooser, SIGNAL(resourceSelected(KoResource *)),
            this, SLOT(update(KoResource *)));

    connect(m_itemChooser, SIGNAL(resourceSelected(KoResource *)),
            this, SIGNAL(resourceSelected(KoResource *)));

    QWidget* buttonWidget = new QWidget(this);
    QHBoxLayout* buttonLayout = new QHBoxLayout(buttonWidget);

    m_addGradient = new QToolButton(this);

    m_addGradient->setText(i18n("Add..."));
    m_addGradient->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    connect(m_addGradient, SIGNAL(clicked()), this, SLOT(addStopGradient()));
    buttonLayout->addWidget(m_addGradient);

    QMenu *menuAddGradient = new QMenu(m_addGradient);

    QAction* addStopGradient = new QAction(i18n("Stop gradient"), this);
    connect(addStopGradient, SIGNAL(triggered(bool)), this, SLOT(addStopGradient()));
    menuAddGradient->addAction(addStopGradient);

    QAction* addSegmentedGradient = new QAction(i18n("Segmented gradient"), this);
    connect(addSegmentedGradient, SIGNAL(triggered(bool)), this, SLOT(addSegmentedGradient()));
    menuAddGradient->addAction(addSegmentedGradient);

    m_addGradient->setMenu(menuAddGradient);
    m_addGradient->setPopupMode(QToolButton::MenuButtonPopup);

    m_editGradient = new QPushButton();
    m_editGradient->setText(i18n("Edit..."));
    m_editGradient->setEnabled(false);
    connect(m_editGradient, SIGNAL(clicked()), this, SLOT(editGradient()));
    buttonLayout->addWidget(m_editGradient);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setObjectName("main layout");
    mainLayout->setMargin(2);
    mainLayout->addWidget(m_lbName);
    mainLayout->addWidget(m_itemChooser, 10);
    mainLayout->addWidget(buttonWidget);

    slotUpdateIcons();
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

void KisGradientChooser::slotUpdateIcons()
{
    if (m_addGradient && m_editGradient) {
        m_addGradient->setIcon(KisIconUtils::loadIcon("list-add"));
        m_editGradient->setIcon(KisIconUtils::loadIcon("configure"));
    }
}

void KisGradientChooser::update(KoResource * resource)
{
    KoAbstractGradient *gradient = static_cast<KoAbstractGradient *>(resource);
    m_lbName->setText(gradient ? i18n(gradient->name().toUtf8().data()) : "");
    m_editGradient->setEnabled(gradient && gradient->removable());
}

void KisGradientChooser::addStopGradient()
{
    KoStopGradient* gradient = new KoStopGradient("");

    QList<KoGradientStop> stops;
    stops << KoGradientStop(0.0, KoColor(QColor(250, 250, 0), KoColorSpaceRegistry::instance()->rgb8())) << KoGradientStop(1.0,  KoColor(QColor(255, 0, 0, 255), KoColorSpaceRegistry::instance()->rgb8()));
    gradient->setType(QGradient::LinearGradient);
    gradient->setStops(stops);
    addGradient(gradient);
}

void KisGradientChooser::addSegmentedGradient()
{
    KoSegmentGradient* gradient = new KoSegmentGradient("");
    gradient->createSegment(INTERP_LINEAR, COLOR_INTERP_RGB, 0.0, 1.0, 0.5, Qt::black, Qt::white);
    gradient->setName(i18n("unnamed"));
    addGradient(gradient);
}

void KisGradientChooser::addGradient(KoAbstractGradient* gradient)
{
    KoResourceServer<KoAbstractGradient> * rserver = KoResourceServerProvider::instance()->gradientServer();
    QString saveLocation = rserver->saveLocation();

    KisCustomGradientDialog dialog(gradient, this, "autogradient");
    dialog.exec();

    gradient->setFilename(saveLocation + gradient->name() + gradient->defaultFileExtension());
    gradient->setValid(true);
    rserver->addResource(gradient);
    m_itemChooser->setCurrentResource(gradient);
}

void KisGradientChooser::editGradient()
{
    KisCustomGradientDialog dialog(static_cast<KoAbstractGradient*>(currentResource()), this, "autogradient");
    dialog.exec();
}




