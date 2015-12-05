/* This file is part of the KDE project
 * Copyright (C) 2007-2009 Thomas Zander <zander@kde.org>
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
#include "StylesWidget.h"
#include "StylesModel.h"
#include "StylesDelegate.h"
#include "ParagraphGeneral.h"
#include "CharacterGeneral.h"
#include <KoStyleThumbnailer.h>

#include <KoStyleManager.h>
#include <KoCharacterStyle.h>
#include <KoParagraphStyle.h>

#include <QDebug>
#include <QHeaderView>
#include <QFormLayout>
#include <QRadioButton>
#include <QScrollBar>
#include <QHideEvent>
#include <QShowEvent>

#include <QModelIndex>

StylesWidget::StylesWidget(QWidget *parent, bool paragraphMode, Qt::WindowFlags f)
    : QFrame(parent, f)
    , m_styleManager(0)
    , m_styleThumbnailer(0)
    , m_stylesModel(new StylesModel(0, StylesModel::ParagraphStyle))
    , m_stylesDelegate(new StylesDelegate())
    , m_blockSignals(false)
    , m_isHovered(false)
{
    m_styleThumbnailer = new KoStyleThumbnailer();
    m_styleThumbnailer->setThumbnailSize(QSize(250, 48));
    m_stylesModel->setStyleThumbnailer(m_styleThumbnailer);
    widget.setupUi(this);
    widget.stylesView->setModel(m_stylesModel);

    if (paragraphMode) {
        connect(widget.stylesView, SIGNAL(clicked(QModelIndex)), this, SLOT(applyParagraphStyle()));
    } else {
        connect(widget.stylesView, SIGNAL(clicked(QModelIndex)), this, SLOT(applyCharacterStyle()));
    }
}

StylesWidget::~StylesWidget()
{
    delete m_stylesDelegate;
    delete m_stylesModel;
    delete m_styleThumbnailer;
}

QSize StylesWidget::sizeHint() const
{
    return QSize(widget.stylesView->sizeHint().width() + 2 * widget.stylesView->verticalScrollBar()->width(),
                 widget.stylesView->sizeHint().height());
}

void StylesWidget::setStyleManager(KoStyleManager *sm)
{
    m_styleManager = sm;
    m_stylesModel->setStyleManager(sm);
}

void StylesWidget::setCurrentFormat(const QTextBlockFormat &format)
{
    if (format == m_currentBlockFormat) {
        return;
    }
    m_currentBlockFormat = format;
    int id = m_currentBlockFormat.intProperty(KoParagraphStyle::StyleId);
    KoParagraphStyle *usedStyle = 0;
    if (m_styleManager) {
        usedStyle = m_styleManager->paragraphStyle(id);
    }
    if (usedStyle) {
        Q_FOREACH (int property, m_currentBlockFormat.properties().keys()) {
            if (property == QTextFormat::ObjectIndex) {
                continue;
            }
            if (property == KoParagraphStyle::ListStyleId) {
                continue;
            }
            if (m_currentBlockFormat.property(property) != usedStyle->value(property)) {
                break;
            }
        }
    }

    widget.stylesView->setCurrentIndex(m_stylesModel->indexOf(*usedStyle));
}

void StylesWidget::setCurrentFormat(const QTextCharFormat &format)
{
    if (format == m_currentCharFormat) {
        return;
    }
    m_currentCharFormat = format;

    int id = m_currentCharFormat.intProperty(KoCharacterStyle::StyleId);
    KoCharacterStyle *usedStyle = 0;
    if (m_styleManager) {
        usedStyle = m_styleManager->characterStyle(id);
    }
    if (usedStyle) {
        QTextCharFormat defaultFormat;
        usedStyle->unapplyStyle(defaultFormat); // sets the default properties.
        Q_FOREACH (int property, m_currentCharFormat.properties().keys()) {
            if (property == QTextFormat::ObjectIndex) {
                continue;
            }
            if (m_currentCharFormat.property(property) != usedStyle->value(property)
                    && m_currentCharFormat.property(property) != defaultFormat.property(property)) {
                break;
            }
        }
    }

    widget.stylesView->setCurrentIndex(m_stylesModel->indexOf(*usedStyle));
}

void StylesWidget::applyParagraphStyle()
{
    QModelIndex index = widget.stylesView->currentIndex();
    Q_ASSERT(index.isValid());
    KoParagraphStyle *paragraphStyle = m_stylesModel->paragraphStyleForIndex(index);
    if (paragraphStyle) {
        emit paragraphStyleSelected(paragraphStyle);
        emit doneWithFocus();
        return;
    }
}

void StylesWidget::applyCharacterStyle()
{
    QModelIndex index = widget.stylesView->currentIndex();
    Q_ASSERT(index.isValid());
    KoCharacterStyle *characterStyle = m_stylesModel->characterStyleForIndex(index);
    if (characterStyle) {
        emit characterStyleSelected(characterStyle);
        emit doneWithFocus();
        return;
    }
}
