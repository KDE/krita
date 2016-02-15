/*
 *  Copyright (c) 2016 Dmitry Kazakov <dimula73@gmail.com>
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

#include "kis_multinode_property.h"

/******************************************************************/
/*               MultinodePropertyConnectorInterface              */
/******************************************************************/

MultinodePropertyConnectorInterface::~MultinodePropertyConnectorInterface()
{
}

void MultinodePropertyConnectorInterface::connectValueChangedSignal(const QObject *receiver, const char *method, Qt::ConnectionType type) {
    connect(this, SIGNAL(sigValueChanged()), receiver, method, type);
    notifyValueChanged();
}

void MultinodePropertyConnectorInterface::notifyValueChanged() {
    emit sigValueChanged();
}

/******************************************************************/
/*               MultinodePropertyBaseConnector                   */
/******************************************************************/

MultinodePropertyBaseConnector::MultinodePropertyBaseConnector(KisMultinodePropertyInterface *parent)
    : m_parent(parent)
{
}

void MultinodePropertyBaseConnector::connectIgnoreCheckBox(QCheckBox *ignoreBox) {
    m_ignoreBox = ignoreBox;

    if (!m_parent->isIgnored() && !m_parent->savedValuesDiffer()) {
        m_ignoreBox->setEnabled(false);
        m_ignoreBox->setChecked(true);

        if (m_parent->haveTheOnlyNode()) {
            m_ignoreBox->setVisible(false);
        }
    } else {
        connect(m_ignoreBox, SIGNAL(stateChanged(int)), SLOT(slotIgnoreCheckBoxChanged(int)));
        m_ignoreBox->setEnabled(true);
        m_ignoreBox->setChecked(!m_parent->isIgnored());
    }
}

void MultinodePropertyBaseConnector::slotIgnoreCheckBoxChanged(int state) {
    m_parent->setIgnored(state != Qt::Checked);
}

void MultinodePropertyBaseConnector::notifyIgnoreChanged() {
    if (!m_ignoreBox) return;

    if (m_ignoreBox->isChecked() != !m_parent->isIgnored()) {
        m_ignoreBox->setChecked(!m_parent->isIgnored());
    }
}

/******************************************************************/
/*               KisMultinodePropertyInterface                    */
/******************************************************************/

KisMultinodePropertyInterface::KisMultinodePropertyInterface()
{
}

KisMultinodePropertyInterface::~KisMultinodePropertyInterface()
{
}
