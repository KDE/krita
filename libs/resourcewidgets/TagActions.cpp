/*
 * Copyright (c) 2018 Boudewijn Rempt <boud@valdyas.org>
 * Copyright (c) 2020 Agata Cacko <cacko.azh@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */
#include "TagActions.h"

#include <QDebug>
#include <QLabel>
#include <QGridLayout>

#include <KoIcon.h>
#include <klocalizedstring.h>
#include <KoResource.h>
#include <KisTagModelProvider.h>


#include <KisTag.h>

#include "kis_debug.h"

ExistingTagAction::ExistingTagAction(KoResourceSP resource, KisTagSP tag, QObject* parent)
    : QAction(parent)
    , m_resource(resource)
    , m_tag(tag)
{
    if (tag) {
        setText(tag->name());
    }
    connect (this, SIGNAL(triggered()),
             this, SLOT(onTriggered()));
}

ExistingTagAction::~ExistingTagAction()
{
}

void ExistingTagAction::onTriggered()
{
    ENTER_FUNCTION();
    if (!m_tag) return;
    emit triggered(m_resource, m_tag);
}


TagEditAction::TagEditAction(KoResourceSP resource, KisTagSP tag, QObject* parent)
    : QWidgetAction(parent)
    , m_closeParentOnTrigger(false)
    , m_tag(tag)
    , m_resource(resource)
{
    QWidget* pWidget = new QWidget (0);
    QHBoxLayout* pLayout = new QHBoxLayout();
    m_label = new QLabel(0);
    m_editBox = new QLineEdit(0);
    m_editBox->setClearButtonEnabled(true);
    m_AddButton = new QPushButton();
    m_AddButton->setIcon(koIcon("list-add"));
    pLayout->addWidget(m_label);
    pLayout->addWidget(m_editBox);
    pLayout->addWidget(m_AddButton);
    pWidget->setLayout(pLayout);
    setDefaultWidget(pWidget);

    connect(m_editBox, &QLineEdit::returnPressed, this, &TagEditAction::onTriggered);
    connect(m_AddButton, &QPushButton::clicked, this, &TagEditAction::onTriggered);
}

TagEditAction::~TagEditAction()
{
}

void TagEditAction::setIcon(const QIcon &icon)
{
    QPixmap pixmap = QPixmap(icon.pixmap(16,16));
    m_label->setPixmap(pixmap);
}

void TagEditAction::closeParentOnTrigger(bool closeParent)
{
    m_closeParentOnTrigger = closeParent;
}

bool TagEditAction::closeParentOnTrigger()
{
    return m_closeParentOnTrigger;
}

void TagEditAction::onTriggered()
{
    if (!m_editBox->text().isEmpty()) {
        if (m_tag) {
            m_tag->setName(m_editBox->text());
            emit triggered(m_tag);
            m_editBox->text().clear();
        }
        else if (m_resource) {
            emit triggered(m_resource, m_editBox->text());
        }
        else {
            emit triggered(m_editBox->text());
        }
        if (m_closeParentOnTrigger) {
            this->parentWidget()->close();
            m_editBox->clearFocus();
        }
    }
}

void TagEditAction::setPlaceholderText(const QString& clickMessage)
{
    m_editBox->setPlaceholderText(clickMessage);
}

void TagEditAction::setText(const QString& text)
{
    ENTER_FUNCTION();
    m_editBox->setText(text);
}

void TagEditAction::setVisible(bool showAction)
{
    QLayout* currentLayout = defaultWidget()->layout();

    this->QAction::setVisible(showAction);

    for(int i=0;i<currentLayout->count();i++) {
        currentLayout->itemAt(i)->widget()->setVisible(showAction);
    }
    defaultWidget()->setVisible(showAction);
}

void TagEditAction::setTag(KisTagSP tag)
{
    m_tag = tag;
}


NewTagAction::~NewTagAction()
{
}

NewTagAction::NewTagAction(KoResourceSP resource, QMenu* parent)
    : TagEditAction(resource, 0, parent)
{
    setIcon(koIcon("document-new"));
    setPlaceholderText(i18n("New tag"));
    closeParentOnTrigger(true);
}


CompareWithOtherTagFunctor::CompareWithOtherTagFunctor(KisTagSP referenceTag)
{
    m_referenceTag = referenceTag;
}

bool CompareWithOtherTagFunctor::operator()(KisTagSP otherTag)
{
    ENTER_FUNCTION() << "refTag: " << (m_referenceTag.isNull() ? "null" : m_referenceTag->name())
                     << " other: " << (otherTag.isNull() ? "null" : otherTag->name())
                     << " result = " << (!otherTag.isNull() && !m_referenceTag.isNull() && otherTag->url() == m_referenceTag->url());
    return !otherTag.isNull() && !m_referenceTag.isNull() && otherTag->url() == m_referenceTag->url();
}

void CompareWithOtherTagFunctor::setReferenceTag(KisTagSP referenceTag) {
    m_referenceTag = referenceTag;
}

KisTagSP CompareWithOtherTagFunctor::referenceTag() {
    return m_referenceTag;
}

