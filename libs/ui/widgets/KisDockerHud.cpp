/*
 *  SPDX-FileCopyrightText: 2024 Freya Lupen <penguinflyer2222@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "KisDockerHud.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QToolButton>
#include <QComboBox>
#include <QLabel>
#include <QDockWidget>

#include <kseparator.h>
#include "KisMainWindow.h"
#include "KisPart.h"
#include "KisDlgListPicker.h"
#include "kis_utility_title_bar.h"

#include "kis_config.h"
#include "kis_icon_utils.h"

QHash<QString, QList<QString>> KisDockerHud::borrowedWidgetOwners = QHash<QString, QList<QString>>();

struct KisDockerHud::Private
{
    QComboBox *dockerComboBox;
    QVBoxLayout *dockerLayout;
    QDockWidget* oldDockerParent {};
    bool isBorrowing {false};
    QLabel* dockerIOULabel {};
    QToolButton* dockerMenu {};

    QString borrowerName;
    QString configId;

    bool isShown {true};
    bool isConnected {false};
};

KisDockerHud::KisDockerHud(QString borrowerName, QString configId)
    : QWidget(),
      m_d(new Private)
{
    this->setObjectName(configId + "DockerHud");
    this->setAutoFillBackground(true); // so that it's not transparent

    m_d->borrowerName = borrowerName;
    m_d->configId = configId;

    QVBoxLayout *layout = new QVBoxLayout();
    QHBoxLayout *menuLayout = new QHBoxLayout();

    // Put the docker config button before the combobox,
    // so it cannot go offscreen in case of a wide widget.
    m_d->dockerMenu = new QToolButton();
    m_d->dockerMenu->setAutoRaise(true);
    m_d->dockerMenu->setToolTip(i18n("Configure docker panel"));
    connect(m_d->dockerMenu, SIGNAL(clicked()), this, SLOT(showDockerConfig()));
    menuLayout->addWidget(m_d->dockerMenu);

    m_d->dockerComboBox = new QComboBox();
    menuLayout->addWidget(m_d->dockerComboBox);

    layout->addLayout(menuLayout);
    layout->addWidget(new KSeparator());
    m_d->dockerLayout = new QVBoxLayout();
    layout->addLayout(m_d->dockerLayout);
    this->setLayout(layout);

    m_d->dockerIOULabel = new QLabel(i18nc("%1 is the name of the widget", "Docker is open in %1. Close %1 to show here.", borrowerName));
    m_d->dockerIOULabel->setWordWrap(true);
    m_d->dockerIOULabel->setAlignment(Qt::AlignCenter);

    tryConnectToDockers();

    slotUpdateIcons();
}

KisDockerHud::~KisDockerHud()
{
    delete m_d->dockerIOULabel;
}

void KisDockerHud::borrowOrReturnDocker()
{
    if (m_d->isBorrowing) { // then return it
        returnDocker();
    }

    // If really visible, borrow it
    if (this->isVisible() && m_d->isShown) {
        borrowDocker();
    }
}

void KisDockerHud::borrowDocker() {
    KisMainWindow *mainWindow = KisPart::instance()->currentMainwindow();


    const QString dockerId = m_d->dockerComboBox->currentData().toString();
    QDockWidget *docker = mainWindow->findChild<QDockWidget*>(dockerId);
    if (docker == nullptr) {
        return;
    }

    QList<QString> &owners = borrowedWidgetOwners[dockerId];

    // If the docker is being borrowed by another widget, request its return
    if (!owners.empty() && owners.last() != this->objectName()) {
        KisDockerHud *prevOwner = mainWindow->findChild<KisDockerHud*>(owners.last());
        prevOwner->returnDocker(true);
    }

    // Notify any waiting borrowers that we own it now
    Q_FOREACH(QString waitingWidgetId, owners) {
        KisDockerHud *waitingWidget = mainWindow->findChild<KisDockerHud*>(waitingWidgetId);
        waitingWidget->showBorrowerLabel(m_d->borrowerName);
    }

    if (!owners.contains(this->objectName())) {
        owners.append(this->objectName());
    }

    // Now actually borrow it.

    m_d->oldDockerParent = docker;

    // Animation dockers have playback controls and such in the titlebar,
    // so we borrow those as well.
    if (docker->objectName() == "AnimationCurvesDocker" || docker->objectName() == "TimelineDocker") {
        KisUtilityTitleBar *titleBar = dynamic_cast<KisUtilityTitleBar*>(docker->titleBarWidget());
        KIS_SAFE_ASSERT_RECOVER_NOOP(titleBar);
        if (titleBar) {
            QWidget *widgetArea = titleBar->widgetArea();
            m_d->dockerLayout->addWidget(widgetArea);
        }
    }

    m_d->dockerLayout->addWidget(docker->widget());
    docker->setWidget(m_d->dockerIOULabel);

    // Hack: somewhat fixes Animation Curves's titlebar being too tall
    m_d->dockerLayout->setStretch(m_d->dockerLayout->count()-1, 1);

    m_d->isBorrowing = true;
}
void KisDockerHud::returnDocker(bool beingTaken) {
    QString dockerId = m_d->oldDockerParent->objectName();
    QList<QString>* owners = &borrowedWidgetOwners[dockerId];

    if (!owners->isEmpty()) {
        const int idx = owners->indexOf(this->objectName());
        const bool wasOwner = idx == owners->count()-1;
        if (!beingTaken) {
            owners->removeAt(idx); // we don't want it anymore
        }

        // If we don't own it, just remove our label
        if (!wasOwner) {
            hideBorrowerLabel();
            return;
        }
    }

    // If we own it, actually return it.

    if (m_d->dockerLayout->count() == 2) {
        // Return the animation dockers' titlebar widget as well
        KisUtilityTitleBar *titleBar = dynamic_cast<KisUtilityTitleBar*>(m_d->oldDockerParent->titleBarWidget());
        KIS_SAFE_ASSERT_RECOVER_NOOP(titleBar);
        if (titleBar) {
            titleBar->setWidgetArea(m_d->dockerLayout->takeAt(0)->widget());
        }
    }

    QWidget* dockerInside = m_d->dockerLayout->takeAt(0)->widget();
    m_d->oldDockerParent->setWidget(dockerInside);
    dockerInside->setParent(m_d->oldDockerParent);

    m_d->oldDockerParent = nullptr;
    m_d->isBorrowing = false;

    // If another borrower is in line, tell it to take it.
    if (!beingTaken && !owners->empty()) {
        KisMainWindow *mainWindow = KisPart::instance()->currentMainwindow();
        KisDockerHud *newOwner = mainWindow->findChild<KisDockerHud*>(owners->last());
        newOwner->borrowDocker();
    }
}

void KisDockerHud::showDockerConfig()
{
    QList<QString> currentDockerNames;
    QList<QVariant> currentDockerIds;
    for(int i = 0; i < m_d->dockerComboBox->count(); i++) {
        currentDockerNames.append(m_d->dockerComboBox->itemText(i));
        currentDockerIds.append(m_d->dockerComboBox->itemData(i));
    }

    KisMainWindow *mainWindow = KisPart::instance()->currentMainwindow();
    const QList<QDockWidget*> dockers = mainWindow->dockWidgets();

    QList<QString> dockerNames;
    QList<QVariant> dockerIds;
    Q_FOREACH(QDockWidget* docker, dockers) {
        if (!currentDockerIds.contains(docker->objectName())) {
            dockerNames.append(docker->windowTitle());
            dockerIds.append(docker->objectName());
        }
    }

    KisDlgListPicker config = KisDlgListPicker(i18nc("%1 is the name of the widget", "Configure %1 dockers", m_d->borrowerName),
                                                    i18n("Available dockers"), i18n("Current dockers"),
                                                    dockerNames, dockerIds, currentDockerNames, currentDockerIds, mainWindow);

    if (config.exec() == QDialog::Accepted) {
        writeDockerList(config.getChosenData());
        const QString currentDocker = readCurrentDocker();
        readDockerList();
        m_d->dockerComboBox->setCurrentIndex(m_d->dockerComboBox->findData(currentDocker, Qt::UserRole));

        // If the current docker was removed from the list, use the first one.
        if (m_d->dockerComboBox->currentData().isNull() && m_d->dockerComboBox->count() != 0) {
            m_d->dockerComboBox->setCurrentIndex(0);
        }
    }
}

void KisDockerHud::writeDockerList(QList<QVariant> currentList)
{
    KisConfig(false).writeList(m_d->configId+"/dockerList", currentList);
}
void KisDockerHud::readDockerList()
{
    m_d->dockerComboBox->clear();
    const QList<QString> defaultDockers = QList<QString>({"BrushHudDocker", "KisLayerBox"});
    const QList<QString> dockerList = KisConfig(true).readList(m_d->configId+"/dockerList", defaultDockers);

    KisMainWindow *mainWindow = KisPart::instance()->currentMainwindow();
    // for an ordered list
    Q_FOREACH(QString objName, dockerList) {
        QDockWidget *docker = mainWindow->findChild<QDockWidget*>(objName);
        if (docker != nullptr) {
            m_d->dockerComboBox->addItem(docker->windowTitle(), docker->objectName());
        }
    }
}

void KisDockerHud::writeCurrentDocker()
{
    KisConfig(false).writeEntry(m_d->configId+"/currentDocker", m_d->dockerComboBox->currentData(Qt::UserRole));
}
QString KisDockerHud::readCurrentDocker()
{
    return KisConfig(true).readEntry(m_d->configId+"/currentDocker", QString("BrushHudDocker"));
}


void KisDockerHud::slotUpdateIcons()
{
    m_d->dockerMenu->setIcon(KisIconUtils::loadIcon("applications-system"));
}

void KisDockerHud::showEvent(QShowEvent *event)
{
    // In case we were loaded before the dockers were (as in the toolbar), 
    // try connecting to them here.
    if (!m_d->isConnected) {
        tryConnectToDockers();
    }

    borrowOrReturnDocker();

    QWidget::showEvent(event);
}

void KisDockerHud::hideEvent(QHideEvent *event)
{
    borrowOrReturnDocker();

    QWidget::hideEvent(event);
}

void KisDockerHud::setIsShown(bool isShown)
{
    m_d->isShown = isShown;
}

void KisDockerHud::tryConnectToDockers()
{
    if (KisPart::instance()->currentMainwindow() == nullptr) {
        return;
    }

    readDockerList();
    m_d->dockerComboBox->setCurrentIndex(m_d->dockerComboBox->findData(readCurrentDocker()));
    connect(m_d->dockerComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(borrowOrReturnDocker()));
    connect(m_d->dockerComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(writeCurrentDocker()));

    m_d->isConnected = true;
}

void KisDockerHud::showBorrowerLabel(QString borrowerName)
{
    m_d->dockerIOULabel->setText(i18nc("%1 is the name of the widget",
                                       "Docker is open in %1. Close %1 to show here.", borrowerName));
    m_d->dockerLayout->addWidget(m_d->dockerIOULabel);
    m_d->dockerLayout->setStretchFactor(m_d->dockerIOULabel, 1); // push the other widgets to the top
    m_d->dockerIOULabel->show();
}

void KisDockerHud::hideBorrowerLabel()
{
    m_d->dockerIOULabel->hide();
    m_d->dockerLayout->takeAt(0)->widget();
    m_d->dockerIOULabel->setText(i18nc("%1 is the name of the widget",
                                        "Docker is open in %1. Close %1 to show here.", m_d->borrowerName));
}
