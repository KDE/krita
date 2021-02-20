/*
 * SPDX-FileCopyrightText: 2018 Boudewijn Rempt <boud@valdyas.org>
 * SPDX-FileCopyrightText: 2020 Agata Cacko <cacko.azh@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "TagActions.h"

#include <QDebug>
#include <QLabel>
#include <QGridLayout>

#include <KoIcon.h>
#include <klocalizedstring.h>
#include <KoResource.h>

#include <KisTag.h>

#include "kis_debug.h"

// ############ Simple Existing Tag Action ##############

SimpleExistingTagAction::SimpleExistingTagAction(KoResourceSP resource, KisTagSP tag, QObject* parent)
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

SimpleExistingTagAction::~SimpleExistingTagAction()
{
}

void SimpleExistingTagAction::onTriggered()
{
    ENTER_FUNCTION();
    if (!m_tag) return;
    emit triggered(m_tag, m_resource);
}


// ############ Line Edit ##############

LineEditAction::LineEditAction(QObject* parent)
    : QWidgetAction(parent)
    , m_closeParentOnTrigger(false)
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

    connect(m_editBox, &QLineEdit::returnPressed, this, &LineEditAction::slotActionTriggered);
    connect(m_AddButton, &QPushButton::clicked, this, &LineEditAction::slotActionTriggered);

}

LineEditAction::~LineEditAction()
{
}

void LineEditAction::setIcon(const QIcon &icon)
{
    QPixmap pixmap = QPixmap(icon.pixmap(16,16));
    m_label->setPixmap(pixmap);
}

void LineEditAction::setCloseParentOnTrigger(bool closeParent)
{
    m_closeParentOnTrigger = closeParent;
}

bool LineEditAction::closeParentOnTrigger()
{
    return m_closeParentOnTrigger;
}


void LineEditAction::slotActionTriggered()
{
    onTriggered();
    if (!m_editBox->text().isEmpty()) {
        if (m_closeParentOnTrigger) {
            this->parentWidget()->close();
            m_editBox->clearFocus();
            m_editBox->clear();
        }
    }
}

void LineEditAction::setPlaceholderText(const QString& clickMessage)
{
    m_editBox->setPlaceholderText(clickMessage);
}

void LineEditAction::setText(const QString& text)
{
    ENTER_FUNCTION();
    m_editBox->setText(text);
}

void LineEditAction::setVisible(bool showAction)
{
    QLayout* currentLayout = defaultWidget()->layout();

    this->QAction::setVisible(showAction);

    for(int i=0;i<currentLayout->count();i++) {
        currentLayout->itemAt(i)->widget()->setVisible(showAction);
    }
    defaultWidget()->setVisible(showAction);
}

QString LineEditAction::userText()
{
    return m_editBox->text();
}

// ############ New Tag ##############

UserInputTagAction::UserInputTagAction(QObject* parent)
    : LineEditAction(parent)
{
    setIcon(koIcon("document-new"));
    setPlaceholderText(i18n("New tag"));
    setCloseParentOnTrigger(true);
}

UserInputTagAction::~UserInputTagAction()
{
}

void UserInputTagAction::onTriggered()
{
    emit triggered(userText());
}

// ############ New Tag Resource ##############

NewTagResourceAction::NewTagResourceAction(KoResourceSP resource, QObject *parent)
    : LineEditAction(parent)
{
    setIcon(koIcon("document-new"));
    setPlaceholderText(i18n("New tag"));
    setCloseParentOnTrigger(true);
    m_resource = resource;
}

NewTagResourceAction::~NewTagResourceAction()
{
}

void NewTagResourceAction::setResource(KoResourceSP resource)
{
    m_resource = resource;
}

void NewTagResourceAction::onTriggered()
{
    emit triggered(userText(), m_resource);
}

// ############ TAG COMPARER ##############


CompareWithOtherTagFunctor::CompareWithOtherTagFunctor(KisTagSP referenceTag)
{
    m_referenceTag = referenceTag;
}

bool CompareWithOtherTagFunctor::operator()(KisTagSP otherTag)
{
//    ENTER_FUNCTION() << "refTag: " << (m_referenceTag.isNull() ? "null" : m_referenceTag->name())
//                     << " other: " << (otherTag.isNull() ? "null" : otherTag->name())
//                     << " result = " << (!otherTag.isNull() && !m_referenceTag.isNull() && otherTag->url() == m_referenceTag->url());
    return !otherTag.isNull() && !m_referenceTag.isNull() && otherTag->url() == m_referenceTag->url();
}

void CompareWithOtherTagFunctor::setReferenceTag(KisTagSP referenceTag) {
    m_referenceTag = referenceTag;
}

KisTagSP CompareWithOtherTagFunctor::referenceTag() {
    return m_referenceTag;
}

