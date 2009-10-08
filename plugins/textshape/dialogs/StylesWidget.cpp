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
#include "ParagraphGeneral.h"
#include "CharacterGeneral.h"

#include <KoStyleManager.h>
#include <KoCharacterStyle.h>
#include <KoParagraphStyle.h>

#include <KDebug>
#include <KInputDialog>
#include <QHeaderView>
#include <QFormLayout>
#include <QRadioButton>

StylesWidget::StylesWidget(QWidget *parent)
        : QWidget(parent),
        m_styleManager(0),
        m_stylesModel(new StylesModel(0, this)),
        m_blockSignals(false)
{
    widget.setupUi(this);
    widget.stylesView->setModel(m_stylesModel);
    widget.stylesView->header()->swapSections(0, 1);
    widget.stylesView->header()->resizeSection(1, 16);
    widget.stylesView->header()->hide();

    widget.newStyle->setIcon(KIcon("list-add"));
    widget.deleteStyle->setIcon(KIcon("list-remove"));
    widget.modifyStyle->setIcon(KIcon("configure"));
    widget.applyStyle->setIcon(KIcon("dialog-ok-apply"));

    setCurrent(QModelIndex()); // register that we don't have a selection at startup

    connect(widget.newStyle, SIGNAL(clicked()), this, SLOT(newStyleClicked()));
    connect(widget.deleteStyle, SIGNAL(clicked()), this, SLOT(deleteStyleClicked()));
    connect(widget.modifyStyle, SIGNAL(clicked()), this, SLOT(editStyle()));
    connect(widget.applyStyle, SIGNAL(clicked()), this, SLOT(applyStyle()));
    connect(widget.stylesView, SIGNAL(clicked(const QModelIndex&)), this, SLOT(setCurrent(const QModelIndex&)));
}

void StylesWidget::setEmbedded(bool embed)
{
    m_isEmbedded = embed;

    widget.newStyle->setVisible(!embed);
    widget.deleteStyle->setVisible(!embed);
    widget.modifyStyle->setVisible(!embed);
    widget.applyStyle->setVisible(!embed);
}

void StylesWidget::setStyleManager(KoStyleManager *sm)
{
    m_styleManager = sm;
    m_stylesModel->setStyleManager(sm);
}

void StylesWidget::setCurrentFormat(const QTextBlockFormat &format)
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

    m_blockSignals = true;
    m_stylesModel->setCurrentParagraphStyle(id, unchanged);
    m_blockSignals = false;
}

void StylesWidget::setCurrentFormat(const QTextCharFormat &format)
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

    m_blockSignals = true;
    m_stylesModel->setCurrentCharacterStyle(id, unchanged);
    m_blockSignals = false;
}

void StylesWidget::newStyleClicked()
{
    KDialog *dialog = new KDialog(this);
    QWidget *root = new QWidget(dialog);
    QFormLayout *lay = new QFormLayout(root);
    QLineEdit *name = new QLineEdit(root);
    name->setText(i18n("new style"));
    lay->addRow(i18n("Name:"), name);
    QRadioButton *pr = new QRadioButton(i18n("Paragraph style"), root);
    pr->setChecked(true);
    lay->addRow(i18n("Type:"), pr);
    QRadioButton *cr = new QRadioButton(i18n("Character style"), root);
    lay->addRow(0, cr);
    root->setLayout(lay);

    dialog->setCaption(i18n("Create New Style"));
    dialog->setMainWidget(root);
    if (dialog->exec() == KDialog::Accepted) {
        QString styleName = name->text();
        if (styleName.isEmpty())
            styleName = i18n("new style");
        if (cr->isChecked()) {
            KoCharacterStyle *style = new KoCharacterStyle();
            style->setName(styleName);
            m_styleManager->add(style);
        } else {
            KoParagraphStyle *style = new KoParagraphStyle();
            style->setName(styleName);
            m_styleManager->add(style);
        }
    }
}

void StylesWidget::deleteStyleClicked()
{
    QModelIndex index = widget.stylesView->currentIndex();
    Q_ASSERT(index.isValid());
    widget.stylesView->clearSelection();
    KoParagraphStyle *paragraphStyle = m_stylesModel->paragraphStyleForIndex(index);
    if (paragraphStyle) {
        KoCharacterStyle *s = paragraphStyle->characterStyle();
        m_styleManager->remove(paragraphStyle);
        bool inUse = false;
        foreach(KoParagraphStyle *ps, m_styleManager->paragraphStyles()) {
            if (ps->characterStyle() == s) {
                inUse = true;
                break;
            }
        }
        if (!inUse)
            m_styleManager->remove(s);
    } else
        m_styleManager->remove(m_stylesModel->characterStyleForIndex(index));
}

void StylesWidget::editStyle()
{
    QModelIndex index = widget.stylesView->currentIndex();
    Q_ASSERT(index.isValid());
    KoParagraphStyle *paragraphStyle = m_stylesModel->paragraphStyleForIndex(index);

    QWidget *widget = 0;
    if (paragraphStyle) {
        ParagraphGeneral *p = new ParagraphGeneral;
        p->setParagraphStyles(m_styleManager->paragraphStyles());
        p->setStyle(paragraphStyle);
        connect(p, SIGNAL(styleAltered(const KoParagraphStyle*)),
                m_styleManager, SLOT(alteredStyle(const KoParagraphStyle*)));
        // TODO get KoUnit from somewhere and set that on p
        widget = p;
    } else {
        KoCharacterStyle *characterStyle = m_stylesModel->characterStyleForIndex(index);
        if (characterStyle) {
            CharacterGeneral *c = new CharacterGeneral;
            c->setStyle(characterStyle);
            connect(c, SIGNAL(styleAltered(const KoCharacterStyle*)),
                    m_styleManager, SLOT(alteredStyle(const KoCharacterStyle*)));
            widget = c;
        }
    }

    if (widget) {
        KDialog *dialog = new KDialog(this);
        dialog->setCaption(paragraphStyle ? i18n("Edit Paragraph Style") : i18n("Edit Character Style"));
        dialog->setMainWidget(widget);
        connect(dialog, SIGNAL(okClicked()), widget, SLOT(save()));
        dialog->exec();
        delete dialog;
    }
}

void StylesWidget::applyStyle()
{
    QModelIndex index = widget.stylesView->currentIndex();
    Q_ASSERT(index.isValid());
    KoParagraphStyle *paragraphStyle = m_stylesModel->paragraphStyleForIndex(index);
    if (paragraphStyle) {
        emit paragraphStyleSelected(paragraphStyle);
        emit doneWithFocus();
        return;
    }

    KoCharacterStyle *characterStyle = m_stylesModel->characterStyleForIndex(index);
    if (characterStyle) {
        emit characterStyleSelected(characterStyle);
        emit doneWithFocus();
        return;
    }
}

void StylesWidget::setCurrent(const QModelIndex &index)
{
    widget.modifyStyle->setEnabled(index.isValid());
    widget.applyStyle->setEnabled(index.isValid());

    bool canDelete = index.isValid();
    if (canDelete) {
        canDelete = !index.parent().isValid();
        KoParagraphStyle *paragraphStyle = m_stylesModel->paragraphStyleForIndex(index);
        if (!canDelete) // there is one other way its deletable, if its a parag style
            canDelete = paragraphStyle;
        // but not if its the default paragraph style.
        if (canDelete && (paragraphStyle && paragraphStyle->styleId() == 100))
            canDelete = false;
    }
    widget.deleteStyle->setEnabled(canDelete);

    if (index.isValid() && m_isEmbedded) {
        KoParagraphStyle *paragraphStyle = m_stylesModel->paragraphStyleForIndex(index);
        if (paragraphStyle) {
            emit paragraphStyleSelected(paragraphStyle, canDelete);
            return;
        }

        KoCharacterStyle *characterStyle = m_stylesModel->characterStyleForIndex(index);
        if (characterStyle) {
            emit characterStyleSelected(characterStyle, canDelete);
            return;
        }
    }
}

#include <StylesWidget.moc>
