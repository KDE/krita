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
#include <KoStyleThumbnailer.h>
#include <KoCharacterStyle.h>
#include <KoParagraphStyle.h>

#include <KAction>
#include <KDebug>

#include <QWidget>
#include <QFrame>
#include <QComboBox> // just to query style
#include <QHBoxLayout>
#include <QDesktopWidget>
#include <QPixmap>
#include <QLabel>

class SpecialButton : public QFrame
{
public:
    SpecialButton(QWidget *parent);

    void setStylesWidget(StylesWidget *stylesWidget);
    void setStylePreview(const QPixmap &pm);

    void showPopup();
protected:
    virtual void mousePressEvent(QMouseEvent *event);

    StylesWidget *m_stylesWidget;
    QLabel *m_preview;
};

SpecialButton::SpecialButton(QWidget *parent)
    : QFrame(parent)
{
    setFrameShape(QFrame::StyledPanel);
    setFrameShadow(QFrame::Sunken);

    setMinimumSize(50,32);
    setMaximumHeight(25);

    m_preview = new QLabel();
    m_preview->setAutoFillBackground(true);
    m_preview->setBackgroundRole(QPalette::Base);
    m_preview->setMinimumWidth(50);
    m_preview->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
    QHBoxLayout *l = new QHBoxLayout(this);
    l->addWidget(m_preview);
    l->setMargin(0);
    setLayout(l);
}

void SpecialButton::setStylePreview(const QPixmap &pm)
{
    m_preview->setPixmap(pm);
}

void SpecialButton::showPopup()
{
    if (!m_stylesWidget) {
        return;
    }

    QRect popupRect(mapToGlobal(QPoint(0, height())), m_stylesWidget->sizeHint());

    // Make sure the popup is not drawn outside the screen area
    QRect screenRect = QApplication::desktop()->availableGeometry(this);
    if (popupRect.right() > screenRect.right())
        popupRect.translate(screenRect.right() - popupRect.right(), 0);
    if (popupRect.left() < screenRect.left())
        popupRect.translate(screenRect.left() - popupRect.left(), 0);
    if (popupRect.bottom() > screenRect.bottom())
        popupRect.translate(0, -(height() + m_stylesWidget->height()));

    m_stylesWidget->setGeometry(popupRect);
    m_stylesWidget->raise();
    m_stylesWidget->show();
}

void SpecialButton::setStylesWidget(StylesWidget *stylesWidget)
{
    m_stylesWidget = stylesWidget;
}

void SpecialButton::mousePressEvent(QMouseEvent *)
{
    showPopup();
}

SimpleStylesWidget::SimpleStylesWidget(QWidget *parent)
        : QWidget(parent)
        ,m_styleManager(0)
        ,m_blockSignals(false)
        ,m_popupForBlock(0)
        ,m_popupForChar(0)
        ,m_thumbnailer(0)
        ,m_blockFrame(0)
        ,m_charFrame(0)
{
    setObjectName("simplestyleswidget");
    m_popupForBlock = new StylesWidget(this, true, Qt::Popup);
    m_popupForBlock->setFrameShape(QFrame::StyledPanel);
    m_popupForBlock->setFrameShadow(QFrame::Raised);
    m_popupForChar = new StylesWidget(this, false, Qt::Popup);
    m_popupForChar->setFrameShape(QFrame::StyledPanel);
    m_popupForChar->setFrameShadow(QFrame::Raised);

    m_blockFrame = new SpecialButton(this);
    m_blockFrame->setStylesWidget(m_popupForBlock);

    m_charFrame = new SpecialButton(this);
    m_charFrame->setStylesWidget(m_popupForChar);

    QHBoxLayout *l = new QHBoxLayout(this);
    l->addWidget(m_blockFrame);
    l->addWidget(m_charFrame);
    l->setMargin(0);
    setLayout(l);

    connect(m_popupForBlock, SIGNAL(paragraphStyleSelected(KoParagraphStyle *)), this, SIGNAL(paragraphStyleSelected(KoParagraphStyle *)));
    connect(m_popupForBlock, SIGNAL(paragraphStyleSelected(KoParagraphStyle *)), this, SIGNAL(doneWithFocus()));
    connect(m_popupForBlock, SIGNAL(paragraphStyleSelected(KoParagraphStyle *)), this, SLOT(hidePopups()));
    connect(m_popupForChar, SIGNAL(characterStyleSelected(KoCharacterStyle *)), this, SIGNAL(characterStyleSelected(KoCharacterStyle *)));
    connect(m_popupForChar, SIGNAL(characterStyleSelected(KoCharacterStyle *)), this, SIGNAL(doneWithFocus()));
    connect(m_popupForChar, SIGNAL(characterStyleSelected(KoCharacterStyle *)), this, SLOT(hidePopups()));

    m_thumbnailer = new KoStyleThumbnailer();
}

SimpleStylesWidget::~SimpleStylesWidget()
{
    delete m_thumbnailer;
}

void SimpleStylesWidget::setStyleManager(KoStyleManager *sm)
{
    m_styleManager = sm;
    m_popupForBlock->setStyleManager(sm);
    m_popupForChar->setStyleManager(sm);
}

void SimpleStylesWidget::hidePopups()
{
    m_popupForBlock->hide();
    m_popupForChar->hide();
}


void SimpleStylesWidget::setCurrentFormat(const QTextBlockFormat &format)
{
    if (format == m_currentBlockFormat)
        return;

    m_currentBlockFormat = format;

    int id = m_currentBlockFormat.intProperty(KoParagraphStyle::StyleId);
    KoParagraphStyle *style(m_styleManager->paragraphStyle(id));
    if (style) {
        m_blockFrame->setStylePreview(m_thumbnailer->thumbnail(style, m_blockFrame->size()));
    }
    m_popupForBlock->setCurrentFormat(format);
}

void SimpleStylesWidget::setCurrentFormat(const QTextCharFormat &format)
{
    if (format == m_currentCharFormat)
        return;
    m_currentCharFormat = format;

    int id = m_currentCharFormat.intProperty(KoCharacterStyle::StyleId);
    KoCharacterStyle *style(m_styleManager->characterStyle(id));
    if (style) {
        m_charFrame->setStylePreview(m_thumbnailer->thumbnail(m_styleManager->characterStyle(id), m_charFrame->size()));
    }
    m_popupForChar->setCurrentFormat(format);
}

#include <SimpleStylesWidget.moc>
