/*
 *  SPDX-FileCopyrightText: 2020 Anna Medonosová <anna.medonosova@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "KisSPenSettings.h"

#include <QAction>
#include <QStandardItem>
#include <QStandardItemModel>
#include <QList>
#include <QMap>
#include <QModelIndex>

#include <kactioncollection.h>
#include <KisPart.h>
#include <kactioncategory.h>
#include <kconfiggroup.h>
#include <ksharedconfig.h>
#include <klocalizedstring.h>
#include <KisActionsSnapshot.h>
#include <kis_icon_utils.h>

KisSPenSettings::KisSPenSettings(QWidget *parent)
    : KisPreferenceSet(parent)
    , m_model(new QStandardItemModel())
{
    mUi = new WdgSPenSettings(this);

    m_model->setColumnCount(2);

    // TODO - popup palette is different action mechanism, thus is missing in this list; maybe we could create a fake KisAction for it
    // Thanks to the KisActionSnapshot, we can list all actions even when no document is open
    QScopedPointer<KisActionsSnapshot> actionsSnapshot(new KisActionsSnapshot());

    KActionCollection *actionCollection = KisPart::instance()->currentMainwindow()->actionCollection();
    for (QAction *action: actionCollection->actions()) {
        actionsSnapshot->addAction(action->objectName(), action);
    }

    QMap<QString, KActionCollection*> sortedCollections = actionsSnapshot->actionCollections();
    for (KActionCollection* collection: sortedCollections) {
        for (QAction* action: collection->actions()) {
            QString actionName = KLocalizedString::removeAcceleratorMarker(action->text());
            QStandardItem* item = new QStandardItem(action->icon(), actionName);
            QStandardItem* actionNameItem = new QStandardItem(action->objectName());
            m_model->appendRow(QList<QStandardItem*>() << item << actionNameItem);
        }
    }

    m_model->sort(m_ACTION_TEXT_COLUMN);
    m_model->insertRow(0, new QStandardItem(i18n("Do nothing")));

    mUi->cmbClickAction->setModel(m_model);
    mUi->cmbDoubleClickAction->setModel(m_model);
    mUi->cmbGestureSwipeUp->setModel(m_model);
    mUi->cmbGestureSwipeDown->setModel(m_model);
    mUi->cmbGestureSwipeLeft->setModel(m_model);
    mUi->cmbGestureSwipeRight->setModel(m_model);
    mUi->cmbGestureCircleCW->setModel(m_model);
    mUi->cmbGestureCircleCCW->setModel(m_model);

    loadPreferences();
}

KisSPenSettings::~KisSPenSettings()
{
    delete mUi;
    delete m_model;
}

QString KisSPenSettings::id()
{
    return QString("SPenSettings");
}

QString KisSPenSettings::name()
{
    return header();
}

QString KisSPenSettings::header()
{
    return QString(i18n("S-Pen Actions"));
}

QIcon KisSPenSettings::icon()
{
    return KisIconUtils::loadIcon("spen-remote");
}

void KisSPenSettings::savePreferences() const
{
    KConfigGroup cfg = KSharedConfig::openConfig()->group("SPenSettings");

    cfg.writeEntry("actionButtonClick", actionNameForIndex(mUi->cmbClickAction->currentIndex()));
    cfg.writeEntry("actionButtonDoubleClick", actionNameForIndex(mUi->cmbDoubleClickAction->currentIndex()));
    cfg.writeEntry("actionGestureSwipeUp", actionNameForIndex(mUi->cmbGestureSwipeUp->currentIndex()));
    cfg.writeEntry("actionGestureSwipeDown", actionNameForIndex(mUi->cmbGestureSwipeDown->currentIndex()));
    cfg.writeEntry("actionGestureSwipeLeft", actionNameForIndex(mUi->cmbGestureSwipeLeft->currentIndex()));
    cfg.writeEntry("actionGestureSwipeRight", actionNameForIndex(mUi->cmbGestureSwipeRight->currentIndex()));
    cfg.writeEntry("actionGestureCircleCW", actionNameForIndex(mUi->cmbGestureCircleCW->currentIndex()));
    cfg.writeEntry("actionGestureCircleCCW", actionNameForIndex(mUi->cmbGestureCircleCCW->currentIndex()));

    emit settingsChanged();
}

void KisSPenSettings::loadPreferences()
{
    KConfigGroup cfg = KSharedConfig::openConfig()->group("SPenSettings");

    mUi->cmbClickAction->setCurrentIndex(indexFromActionName(cfg.readEntry("actionButtonClick", QString())));
    mUi->cmbDoubleClickAction->setCurrentIndex(indexFromActionName(cfg.readEntry("actionButtonDoubleClick", QString())));
    mUi->cmbGestureSwipeUp->setCurrentIndex(indexFromActionName(cfg.readEntry("actionGestureSwipeUp", QString())));
    mUi->cmbGestureSwipeDown->setCurrentIndex(indexFromActionName(cfg.readEntry("actionGestureSwipeDown", QString())));
    mUi->cmbGestureSwipeLeft->setCurrentIndex(indexFromActionName(cfg.readEntry("actionGestureSwipeLeft", QString())));
    mUi->cmbGestureSwipeRight->setCurrentIndex(indexFromActionName(cfg.readEntry("actionGestureSwipeRight", QString())));
    mUi->cmbGestureCircleCW->setCurrentIndex(indexFromActionName(cfg.readEntry("actionGestureCircleCW", QString())));
    mUi->cmbGestureCircleCCW->setCurrentIndex(indexFromActionName(cfg.readEntry("actionGestureCircleCCW", QString())));

}

void KisSPenSettings::loadDefaultPreferences()
{
    mUi->cmbClickAction->setCurrentIndex(indexFromActionName(QString("fake_show_popup_palette")));
    mUi->cmbDoubleClickAction->setCurrentIndex(indexFromActionName(QString("erase_action")));
    mUi->cmbGestureSwipeUp->setCurrentIndex(indexFromActionName(QString("make_brush_color_lighter")));
    mUi->cmbGestureSwipeDown->setCurrentIndex(indexFromActionName(QString("make_brush_color_darker")));
    mUi->cmbGestureSwipeLeft->setCurrentIndex(indexFromActionName(QString("KritaShape/KisToolBrush")));
    mUi->cmbGestureSwipeRight->setCurrentIndex(indexFromActionName(QString("KritaSelected/KisToolColorSampler")));
    mUi->cmbGestureCircleCW->setCurrentIndex(indexFromActionName(QString("shift_brush_color_clockwise")));
    mUi->cmbGestureCircleCCW->setCurrentIndex(indexFromActionName(QString("shift_brush_color_counter_clockwise")));
}

QString KisSPenSettings::actionNameForIndex(int index) const
{
    QModelIndex modelIndex = m_model->index(index, m_ACTION_NAME_COLUMN);
    QString actionName = m_model->itemFromIndex(modelIndex)->data(Qt::DisplayRole).toString();
    return actionName;
}

int KisSPenSettings::indexFromActionName(QString actionName) const
{
    if (actionName.isEmpty()) {
        return 0;
    } else {
        QList<QStandardItem*> itemsFound = m_model->findItems(actionName, Qt::MatchExactly, m_ACTION_NAME_COLUMN);
        if (itemsFound.size() == 0) {
            return 1;
        } else {
            return itemsFound[0]->index().row();
        }
    }
}
