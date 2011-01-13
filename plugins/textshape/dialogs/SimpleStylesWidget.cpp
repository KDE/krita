/* This file is part of the KDE project
 * Copyright (C) 2010 Casper Boemann <cbo@boemann.dk>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */
#include "SimpleStylesWidget.h"
#include "TextTool.h"
#include "StylesWidget.h"

#include <KoStyleManager.h>
#include <KoCharacterStyle.h>
#include <KoParagraphStyle.h>

#include <KAction>
#include <KDebug>

#include <QWidget>
#include <QFrame>
#include <QHBoxLayout>

class SpecialButton : public QFrame
{
public:
    SpecialButton();

    void setStylesWidget(StylesWidget *stylesWidget);

protected:
    virtual void enterEvent(QEvent *event);
    virtual void leaveEvent(QEvent *event);

    StylesWidget *m_stylesWidget;
};

SpecialButton::SpecialButton()
 : QFrame()
{
    setFrameShape(QFrame::StyledPanel);
    setFrameShadow(QFrame::Sunken);
}

void SpecialButton::setStylesWidget(StylesWidget *stylesWidget)
{
    m_stylesWidget = stylesWidget;
}

void SpecialButton::enterEvent(QEvent *event)
{
    m_stylesWidget->show();
}

void SpecialButton::leaveEvent(QEvent *event)
{
}


SimpleStylesWidget::SimpleStylesWidget(QWidget *parent)
        : QWidget(parent)
        ,m_blockSignals(false)
{
    setObjectName("simplestyleswidget");
    m_popupForBlock = new StylesWidget;
    m_popupForChar = new StylesWidget;

    SpecialButton *blockFrame = new SpecialButton;
    blockFrame->setStylesWidget(m_popupForBlock);
    QWidget *blockPreview = new QWidget();
    blockPreview->setAutoFillBackground(true);
    blockPreview->setBackgroundRole(QPalette::Base);
    blockPreview->setMinimumWidth(50);
    blockPreview->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
    QHBoxLayout *l = new QHBoxLayout;
    l->addWidget(blockPreview);
    l->setMargin(0);
    blockFrame->setLayout(l);

    SpecialButton *charFrame = new SpecialButton;
    QWidget *charPreview = new QWidget();
    charPreview->setAutoFillBackground(true);
    charPreview->setBackgroundRole(QPalette::Base);
    charPreview->setMinimumWidth(50);
    charPreview->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
    l = new QHBoxLayout;
    l->addWidget(charPreview);
    l->setMargin(0);
    charFrame->setLayout(l);

    l = new QHBoxLayout;
    l->addWidget(blockFrame);
    l->addWidget(charFrame);
    l->setMargin(0);
    setLayout(l);
}

void SimpleStylesWidget::setStyleManager(KoStyleManager *sm)
{
    m_styleManager = sm;
    m_popupForBlock->setStyleManager(sm);
    m_popupForChar->setStyleManager(sm);
}

void SimpleStylesWidget::setCurrentFormat(const QTextBlockFormat &format)
{
    if (format == m_currentBlockFormat)
        return;
    m_currentBlockFormat = format;
    int id = m_currentBlockFormat.intProperty(KoParagraphStyle::StyleId);
    bool unchanged = true;
    KoParagraphStyle *usedStyle = 0;
    if (m_styleManager)
        usedStyle = m_styleManager->paragraphStyle(id);
    if (usedStyle) {
        foreach(int property, m_currentBlockFormat.properties().keys()) {
            if (property == QTextFormat::ObjectIndex)
                continue;
            if (property == KoParagraphStyle::ListStyleId)
                continue;
            if (m_currentBlockFormat.property(property) != usedStyle->value(property)) {
                unchanged = false;
                break;
            }
        }
    }
}

void SimpleStylesWidget::setCurrentFormat(const QTextCharFormat &format)
{
    if (format == m_currentCharFormat)
        return;
    m_currentCharFormat = format;

    int id = m_currentCharFormat.intProperty(KoCharacterStyle::StyleId);
    bool unchanged = true;
    KoCharacterStyle *usedStyle = 0;
    if (m_styleManager)
        usedStyle = m_styleManager->characterStyle(id);
    if (usedStyle) {
        QTextCharFormat defaultFormat;
        usedStyle->unapplyStyle(defaultFormat); // sets the default properties.
        foreach(int property, m_currentCharFormat.properties().keys()) {
            if (property == QTextFormat::ObjectIndex)
                continue;
            if (m_currentCharFormat.property(property) != usedStyle->value(property)
                    && m_currentCharFormat.property(property) != defaultFormat.property(property)) {
                unchanged = false;
                break;
            }
        }
    }
}



#include <SimpleStylesWidget.moc>
