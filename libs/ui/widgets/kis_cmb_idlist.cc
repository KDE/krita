/*
 *  kis_cmb_idlist.cc - part of KImageShop/Krayon/Krita
 *
 *  SPDX-FileCopyrightText: 2005 Boudewijn Rempt (boud@valdyas.org)
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "widgets/kis_cmb_idlist.h"

#include <klocalizedstring.h>
#include <kis_debug.h>
#include <KoID.h>

KisCmbIDList::KisCmbIDList(QWidget * parent, const char * name)
    : QComboBox(parent)
{
    setObjectName(name);
    setEditable(false);
    connect(this, SIGNAL(activated(int)), this, SLOT(slotIDActivated(int)));
    connect(this, SIGNAL(highlighted(int)), this, SLOT(slotIDHighlighted(int)));
}

KisCmbIDList::~KisCmbIDList()
{
}

const KoID KisCmbIDList::AutoOptionID = KoID("AUTO", ki18nc("Automatic", "Auto"));

void KisCmbIDList::setIDList(const QList<KoID>  & list, bool sorted)
{
    m_idList = list;
    m_sorted = sorted;

    buildItems();
}

void KisCmbIDList::allowAuto(bool setAuto)
{
    m_autoOption = setAuto;

    buildItems();
}

void KisCmbIDList::setAutoHint(const QString & hint)
{
    m_autoHint = hint;

    buildItems();
}

KoID KisCmbIDList::currentItem() const
{
    qint32 index = QComboBox::currentIndex();

    if (index > m_idList.count() - 1 || index < 0) return KoID();

    return m_idList[index];
}

void KisCmbIDList::setCurrent(const KoID id)
{
    qint32 index = m_idList.indexOf(id);

    if (index >= 0) {
        QComboBox::setCurrentIndex(index);
    } else if (id != KoID()) {
        m_idList.push_back(id);
        buildItems();
        QComboBox::setCurrentIndex(m_idList.indexOf(id));
    }
}

void KisCmbIDList::setCurrent(const QString & id)
{
    for (qint32 index = 0; index < m_idList.count(); ++index) {
        if (m_idList.at(index).id() == id) {
            QComboBox::setCurrentIndex(index);
            break;
        }
    }
}

void KisCmbIDList::slotIDActivated(int index)
{
    if (index > m_idList.count() - 1 || index < 0) return;

    emit activated(m_idList[index]);
}

void KisCmbIDList::slotIDHighlighted(int index)
{
    if (index > m_idList.count() - 1 || index < 0) return;

    emit highlighted(m_idList[index]);
}

void KisCmbIDList::buildItems()
{
    const KoID selectedID = currentItem();

    clear();

    // m_idList has auto, remove it temporarily.
    if (m_idList.contains(AutoOptionID)) {
        m_idList.removeAll(AutoOptionID);
    }

    // Sort id list..
    if (m_sorted) {
        std::sort(m_idList.begin(), m_idList.end(), KoID::compareNames);
    }

    // Re-add "Auto" option to front of list if auto enabled..
    if (m_autoOption) {
        m_idList.insert(0, AutoOptionID);
    }

    // Populate GUI item list..
    for (qint32 i = 0; i < m_idList.count(); ++i) {
        const KoID &id = m_idList.at(i);
        if (id == AutoOptionID && !m_autoHint.isEmpty()) {
            addItem(QString("%1 (%2)").arg(id.name(), m_autoHint));
        } else {
            addItem(m_idList.at(i).name());
        }
    }

    setCurrent(selectedID);
}


