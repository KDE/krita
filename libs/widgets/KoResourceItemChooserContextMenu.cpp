
 /* This file is part of the KDE project
 *    Copyright (c) 2013 Sascha Suelzer <s_suelzer@lavabit.com>
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Library General Public
 *    License as published by the Free Software Foundation; either
 *    version 2 of the License, or (at your option) any later version.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Library General Public License for more details.
 *
 *    You should have received a copy of the GNU Library General Public License
 *    along with this library; see the file COPYING.LIB.  If not, write to
 *    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  * Boston, MA 02110-1301, USA.
 * */
#include <QDebug>
#include <QLabel>
#include <QGridLayout>

#include <KoIcon.h>
#include <klocale.h>

#include "KoResourceItemChooserContextMenu.h"

ContextMenuExistingTagAction::ContextMenuExistingTagAction(KoResource* resource, QString tag, QObject* parent)
: QAction(parent)
, m_resource(resource)
, m_tag(tag)
{
    setText(tag);
    connect (this, SIGNAL(triggered()),
             this, SLOT(onTriggered()));
}

ContextMenuExistingTagAction::~ContextMenuExistingTagAction()
{
}

void ContextMenuExistingTagAction::onTriggered()
{
    emit triggered(m_resource,m_tag);
}
ContextMenuNewTagAction::~ContextMenuNewTagAction()
{
}

ContextMenuNewTagAction::ContextMenuNewTagAction(KoResource* resource, QObject* parent)
    :QWidgetAction (parent)
    ,m_resource(resource)
{

    QWidget* pWidget = new QWidget (NULL);
    QHBoxLayout* pLayout = new QHBoxLayout();
    QLabel * label = new QLabel(NULL);
    QIcon icon = koIcon("document-new");
    QPixmap pixmap = QPixmap(icon.pixmap(16,16));
    label->setPixmap(pixmap);
    m_editBox = new KLineEdit(NULL);
    pLayout->addWidget(label);
    pLayout->addWidget(m_editBox);
    pWidget->setLayout(pLayout);
    m_resource = resource;

    setDefaultWidget(pWidget);
    m_editBox->setClickMessage(i18n("New tag"));
    connect (m_editBox, SIGNAL(returnPressed(QString)),
             this, SLOT(onTriggered(QString)));
}

void ContextMenuNewTagAction::onTriggered(const QString & tagName)
{
    if (!tagName.isEmpty()) {
        m_tag = tagName;
        emit triggered(m_resource,m_tag);
        this->parentWidget()->close();
    }
}

KoResourceItemChooserContextMenu::KoResourceItemChooserContextMenu(QWidget* parent)
: QMenu(parent)
{

}

