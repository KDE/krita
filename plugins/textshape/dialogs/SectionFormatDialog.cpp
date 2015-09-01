/* This file is part of the KDE project
 * Copyright (C) 2014 Denis Kuplyakov <dener.kup@gmail.com>
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
#include <KoSection.h>

#include <QToolTip>
#include <KColorScheme>

#include <klocale.h>

class SectionFormatDialog::SectionNameValidator : public QValidator
{
public:
    SectionNameValidator(QObject *parent, KoSectionManager *sectionManager, KoSection *section)
    : QValidator(parent)
    , m_sectionManager(sectionManager)
    , m_section(section)
    {
    }

    virtual State validate(QString &input, int &pos) const
    {
        Q_UNUSED(pos);
        if (m_section->name() == input || m_sectionManager->isValidNewName(input)) {
            return QValidator::Acceptable;
        }
        return QValidator::Intermediate;
    }

private:
    KoSectionManager *m_sectionManager;
    KoSection *m_section;
};

SectionFormatDialog::SectionFormatDialog(QWidget *parent, KoTextEditor *editor)
    : KDialog(parent)
    , m_editor(editor)
{
    setCaption(i18n("Configure sections"));
    setButtons(KDialog::Ok | KDialog::Cancel);
    showButtonSeparator(true);
    QWidget *form = new QWidget;
    m_widget.setupUi(form);
    setMainWidget(form);

    m_sectionManager = KoTextDocument(editor->document()).sectionManager();
    QStandardItemModel *model = m_sectionManager->update(true);
    model->setColumnCount(1);
    QStringList header;
    header << i18n("Section name");
    model->setHorizontalHeaderLabels(header);
    m_widget.sectionTree->setModel(model);
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
    m_widget.sectionTree->model()->setData(m_curIdx, m_widget.sectionNameLineEdit->text(), Qt::DisplayRole);
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
        new SectionNameValidator(this, m_sectionManager, curSection));
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

inline KoSection* SectionFormatDialog::sectionFromModel(const QModelIndex &idx)
{
    return m_widget.sectionTree->model()->itemData(idx)[Qt::UserRole + 1].value<KoSection *>();
}
