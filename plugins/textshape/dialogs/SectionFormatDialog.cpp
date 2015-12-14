/* This file is part of the KDE project
 * Copyright (C) 2014-2015 Denis Kuplyakov <dener.kup@gmail.com>
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

#include "SectionFormatDialog.h"

#include <KoTextDocument.h>
#include <KoSectionModel.h>
#include <KoSection.h>
#include <KoTextEditor.h>

#include <QIdentityProxyModel>
#include <QToolTip>
#include <kcolorscheme.h>
#include <klocalizedstring.h>

class SectionFormatDialog::ProxyModel : public QIdentityProxyModel
{
public:
    ProxyModel(KoSectionModel *model, QObject *parent = 0)
        : QIdentityProxyModel(parent)
    {
        setSourceModel(model);
    }

    virtual int columnCount(const QModelIndex &parent = QModelIndex()) const
    {
        Q_UNUSED(parent);
        return 1; // We have one column with "Name of section"
    }

    virtual QVariant headerData(int section, Qt::Orientation orientation, int role) const
    {
        if (orientation != Qt::Horizontal || section != 0) {
            return QVariant();
        }

        if (role == Qt::DisplayRole) {
            return i18n("Section name");
        }
        return QVariant();
    }

    virtual QVariant data(const QModelIndex &proxyIndex, int role = Qt::DisplayRole) const
    {
        if (!proxyIndex.isValid() || proxyIndex.column() != 0) {
            return QVariant();
        }

        if (role == Qt::DisplayRole) {
            KoSection *ptr = getSectionByIndex(proxyIndex);
            return ptr->name();
        }
        return QVariant();
    }

    KoSection *getSectionByIndex(const QModelIndex &idx) const
    {
        return sourceModel()->data(
                   mapToSource(idx),
                   KoSectionModel::PointerRole
               ).value<KoSection *>();
    }

private:
    // Make it private. It is intented to be used only with KoSectionModel that is passed through constructor
    virtual void setSourceModel(QAbstractItemModel *sourceModel)
    {
        QAbstractProxyModel::setSourceModel(sourceModel);
    }
};

class SectionFormatDialog::SectionNameValidator : public QValidator
{
public:
    SectionNameValidator(QObject *parent, KoSectionModel *sectionManager, KoSection *section)
        : QValidator(parent)
        , m_sectionModel(sectionManager)
        , m_section(section)
    {
    }

    virtual State validate(QString &input, int &pos) const
    {
        Q_UNUSED(pos);
        if (m_section->name() == input || m_sectionModel->isValidNewName(input)) {
            return QValidator::Acceptable;
        }
        return QValidator::Intermediate;
    }

private:
    KoSectionModel *m_sectionModel;
    KoSection *m_section;
};

SectionFormatDialog::SectionFormatDialog(QWidget *parent, KoTextEditor *editor)
    : KoDialog(parent)
    , m_editor(editor)
{
    setCaption(i18n("Configure sections"));
    setButtons(KoDialog::Ok | KoDialog::Cancel);
    showButtonSeparator(true);
    QWidget *form = new QWidget;
    m_widget.setupUi(form);
    setMainWidget(form);

    m_sectionModel = KoTextDocument(editor->document()).sectionModel();
    m_widget.sectionTree->setModel(new ProxyModel(m_sectionModel, this));
    m_widget.sectionTree->expandAll();

    m_widget.sectionNameLineEdit->setEnabled(false);

    connect(m_widget.sectionTree, SIGNAL(activated(QModelIndex)), this, SLOT(sectionSelected(QModelIndex)));
    connect(m_widget.sectionNameLineEdit, SIGNAL(editingFinished()), this, SLOT(sectionNameChanged()));
    connect(m_widget.sectionNameLineEdit, SIGNAL(textEdited(QString)), this, SLOT(updateTreeState()));

    m_curIdx = m_widget.sectionTree->currentIndex();
}

void SectionFormatDialog::sectionNameChanged()
{
    m_editor->renameSection(sectionFromModel(m_curIdx), m_widget.sectionNameLineEdit->text());
    m_widget.sectionNameLineEdit->setModified(false); // value is set to line edit isn't modified (has new default value)
}

void SectionFormatDialog::sectionSelected(const QModelIndex &idx)
{
    KoSection *curSection = sectionFromModel(idx);
    m_curIdx = m_widget.sectionTree->currentIndex();

    // Update widgets
    m_widget.sectionNameLineEdit->setEnabled(true);
    m_widget.sectionNameLineEdit->setText(curSection->name());
    m_widget.sectionNameLineEdit->setValidator(
        new SectionNameValidator(this, m_sectionModel, curSection));
}

void SectionFormatDialog::updateTreeState()
{
    if (!m_curIdx.isValid()) {
        return;
    }

    bool allOk = true;
    QPalette pal = m_widget.sectionNameLineEdit->palette();
    if (!m_widget.sectionNameLineEdit->hasAcceptableInput()) {
        KColorScheme::adjustBackground(pal, KColorScheme::NegativeBackground);
        m_widget.sectionNameLineEdit->setPalette(pal);

        QToolTip::showText(m_widget.sectionNameLineEdit->mapToGlobal(QPoint()),
                           i18n("Invalid characters or section with such name exists."));

        allOk = false;
    } else {
        KColorScheme::adjustBackground(pal, KColorScheme::NormalBackground);
        m_widget.sectionNameLineEdit->setPalette(pal);
    }

    m_widget.sectionTree->setEnabled(allOk);
    enableButtonOk(allOk);
}

inline KoSection *SectionFormatDialog::sectionFromModel(const QModelIndex &idx)
{
    return dynamic_cast<ProxyModel *>(m_widget.sectionTree->model())->getSectionByIndex(idx);
}
