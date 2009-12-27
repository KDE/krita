/***************************************************************************
 * KoScriptManagerAdd.h
 * This file is part of the KDE project
 * copyright (C) 2006-2007 Sebastian Sauer <mail@dipe.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 * This program is distributed in the hope that it will be useful,
 *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this program; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 ***************************************************************************/

#include "KoScriptManagerAdd.h"

#include <kross/core/manager.h>
#include <kross/core/interpreter.h>
#include <kross/core/actioncollection.h>
#include <kross/ui/view.h>

#include <QtGui/QBoxLayout>
#include <QtGui/QLabel>
#include <QtGui/QRadioButton>

#include <QLineEdit>
#include <QComboBox>

#include <klocale.h>
#include <kfilewidget.h>
#include <kurlrequester.h>
#include <kdebug.h>

/********************************************************************
 * KoScriptManagerAddTypeWidget
 */

KoScriptManagerAddTypeWidget::KoScriptManagerAddTypeWidget(KoScriptManagerAddWizard *wizard)
    : QWidget(wizard),
    m_wizard(wizard)
{
    setObjectName("ScriptManagerAddTypeWidget");
    QVBoxLayout *layout = new QVBoxLayout(this);
    setLayout(layout);
    QLabel *label = new QLabel(i18n("<qt>This wizard will guide you through the proccess of adding a new resource to your scripts.</qt>"), this);
    label->setWordWrap(true);
    layout->addWidget(label);
    layout->addSpacing(10);

    m_scriptCheckbox = new QRadioButton(i18n("Add script file"), this);
    m_scriptCheckbox->setChecked(true);
    connect(m_scriptCheckbox, SIGNAL(toggled(bool)), this, SLOT(slotUpdate()));
    layout->addWidget(m_scriptCheckbox);

    m_collectionCheckbox = new QRadioButton(i18n("Add collection folder"), this);
    layout->addWidget(m_collectionCheckbox);

    m_installCheckBox = new QRadioButton(i18n("Install script package file"), this);
    m_installCheckBox->setEnabled(false);
    layout->addWidget(m_installCheckBox);

    m_onlineCheckbox = new QRadioButton(i18n("Install online script package"), this);
    m_onlineCheckbox->setEnabled(false);
    layout->addWidget(m_onlineCheckbox);

    layout->addStretch(1);
}

KoScriptManagerAddTypeWidget::~KoScriptManagerAddTypeWidget()
{
}

void KoScriptManagerAddTypeWidget::slotUpdate()
{
    m_wizard->setAppropriate(m_wizard->m_fileItem, m_scriptCheckbox->isChecked());
    m_wizard->setAppropriate(m_wizard->m_scriptItem, m_scriptCheckbox->isChecked());
    m_wizard->setAppropriate(m_wizard->m_collectionItem, m_collectionCheckbox->isChecked());
    //m_installCheckBox->isChecked()
    //m_onlineCheckbox->isChecked()
}

/********************************************************************
 * KoScriptManagerAddFileWidget
 */

KoScriptManagerAddFileWidget::KoScriptManagerAddFileWidget(KoScriptManagerAddWizard *wizard, const QString &startDirOrVariable)
    : QWidget(wizard),
    m_wizard(wizard)
{
    setObjectName("ScriptManagerAddFileWidget");
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setMargin(0);
    setLayout(layout);
    m_filewidget = new KFileWidget(KUrl(startDirOrVariable), this);

    QStringList mimetypes;
    foreach (const QString &interpretername, Kross::Manager::self().interpreters()) {
        Kross::InterpreterInfo *info = Kross::Manager::self().interpreterInfo(interpretername);
        Q_ASSERT(info);
        mimetypes.append(info->mimeTypes().join(" ").trimmed());
    }
    m_filewidget->setMimeFilter(mimetypes /*, defaultmime*/);

    layout->addWidget(m_filewidget);
    connect(m_filewidget, SIGNAL(fileHighlighted(const QString&)), this, SLOT(slotFileHighlighted(const QString&)));
    connect(m_filewidget, SIGNAL(fileSelected(const QString&)), this, SLOT(slotUpdate()));
}

KoScriptManagerAddFileWidget::~KoScriptManagerAddFileWidget()
{
}

QString KoScriptManagerAddFileWidget::selectedFile() const
{
    //kDebug(32010)<<m_filewidget->selectedFile();
    return m_file;
}

void KoScriptManagerAddFileWidget::slotFileHighlighted(const QString &file)
{
    //kDebug(32010)<<file;
    m_file = file;
    m_wizard->setValid(m_wizard->m_fileItem, ! file.isEmpty());
}

void KoScriptManagerAddFileWidget::slotUpdate()
{
    //kDebug(32010)<<selectedFile();
    m_wizard->setValid(m_wizard->m_fileItem, ! m_file.isEmpty());
}

/********************************************************************
 * KoScriptManagerAddScriptWidget
 */

KoScriptManagerAddScriptWidget::KoScriptManagerAddScriptWidget(KoScriptManagerAddWizard *wizard)
    : QWidget(wizard),
    m_wizard(wizard),
    m_editor(0)
{
    setObjectName("ScriptManagerAddScriptWidget");
    QVBoxLayout *layout = new QVBoxLayout(this);
    setLayout(layout);
}

KoScriptManagerAddScriptWidget::~KoScriptManagerAddScriptWidget()
{
}

void KoScriptManagerAddScriptWidget::slotUpdate()
{
    //NOTE: idValid() only checks ! nameEdit.isEmpty()
    //d->wizard->setValid(d->wizard->m_scriptItem, d->editor && d->editor->isValid());

    m_wizard->setValid(m_wizard->m_scriptItem,
                         ! (m_editor == 0 || m_editor->nameEdit()->text().isEmpty()
                             || m_editor->textEdit()->text().isEmpty()
                             || m_editor->interpreterEdit()->currentText().isEmpty()
                             || m_editor->fileEdit()->url().fileName().isEmpty() )
                        );
}

void KoScriptManagerAddScriptWidget::showEvent(QShowEvent *event)
{
    Kross::Action *action = 0;
    if (m_editor) {
        action = m_editor->action();
        delete m_editor;
    }
    if (! action)
        action = new Kross::Action(0, QString());

    const QString file = m_wizard->m_filewidget->selectedFile();
    QFileInfo fi(file);
    action->setObjectName(file);
    action->setText(fi.baseName());
    //action->setDescription();
    if (fi.isFile()) {
        action->setIconName(KMimeType::iconNameForUrl(KUrl(file)));
        action->setEnabled(fi.exists());
    }
    action->setFile(file);

    m_editor = new Kross::ActionCollectionEditor(action, this);
    layout()->addWidget(m_editor);
    m_editor->interpreterEdit()->setEnabled(false);
    m_editor->fileEdit()->setEnabled(false);
    connect(m_editor->textEdit(), SIGNAL(textChanged(const QString&)), this, SLOT(slotUpdate()));
    connect(m_editor->interpreterEdit(), SIGNAL(editTextChanged(const QString&)), this, SLOT(slotUpdate()));
    connect(m_editor->fileEdit(), SIGNAL(textChanged(const QString&)), this, SLOT(slotUpdate()));

    QWidget::showEvent(event);
    slotUpdate();
}

bool KoScriptManagerAddScriptWidget::accept()
{
    kDebug(32010);
    Q_ASSERT(m_editor);
    Q_ASSERT(m_editor->action());
    Q_ASSERT(m_wizard);
    Q_ASSERT(m_wizard->m_collection);
    m_editor->commit(); // take over changes done in the editor into the action
    m_wizard->m_collection->addAction(m_editor->action()); // add the action to the collection
    //TODO select new item
    return true;
}

/********************************************************************
 * KoScriptManagerAddCollectionWidget
 */

KoScriptManagerAddCollectionWidget::KoScriptManagerAddCollectionWidget(KoScriptManagerAddWizard *wizard)
    : QWidget(wizard), m_wizard(wizard)
{
    setObjectName("ScriptManagerAddCollectionWidget");
    QVBoxLayout *layout = new QVBoxLayout(this);
    setLayout(layout);
    Kross::ActionCollection *collection = new Kross::ActionCollection(uniqueName());
    m_editor = new Kross::ActionCollectionEditor(collection, this);
    m_editor->textEdit()->setText(QString());
    layout->addWidget(m_editor);
}

KoScriptManagerAddCollectionWidget::~KoScriptManagerAddCollectionWidget()
{
}

QString KoScriptManagerAddCollectionWidget::uniqueName() const
{
    QString s("Collection-%1");
    int i = 1;
    while (m_wizard->m_collection->hasCollection(s.arg(i))) {
        ++i;
    }
    return s.arg(i);
}

void KoScriptManagerAddCollectionWidget::slotUpdate()
{
    m_wizard->setValid(m_wizard->m_collectionItem, m_editor->isValid());
}

bool KoScriptManagerAddCollectionWidget::accept()
{
    kDebug(32010);
    Q_ASSERT(m_wizard);
#ifndef DISABLE_ADD_REMOVE
    m_editor->commit(); // take over changes done in the editor into the action collection
    m_editor->collection()->setParentCollection(m_wizard->m_collection);
#endif
    //TODO select new item
    return true;
}

/********************************************************************
 * KoScriptManagerAddWizard
 */

KoScriptManagerAddWizard::KoScriptManagerAddWizard(QWidget *parent, Kross::ActionCollection *collection)
    : KAssistantDialog(parent), m_collection(collection ? collection : Kross::Manager::self().actionCollection())
{
    Q_ASSERT(m_collection);
    setObjectName("ScriptManagerAddWizard");
    setCaption(i18n("Add"));

    m_typewidget = new KoScriptManagerAddTypeWidget(this);
    m_typeItem = addPage(m_typewidget, i18n("Add"));

    const QString startDirOrVariable("kfiledialog:///scriptmanageraddfile");
    m_filewidget = new KoScriptManagerAddFileWidget(this, startDirOrVariable);
    m_fileItem = addPage(m_filewidget, i18n("Script File"));

    m_scriptwidget = new KoScriptManagerAddScriptWidget(this);
    m_scriptItem = addPage(m_scriptwidget, i18n("Script"));

    m_collectionwidget = new KoScriptManagerAddCollectionWidget(this);
    m_collectionItem = addPage(m_collectionwidget, i18n("Collection"));

    resize(QSize(620, 460).expandedTo(minimumSizeHint()));

    //connect(this, SIGNAL(currentPageChanged(KPageWidgetItem*,KPageWidgetItem*)), this, SLOT(slotUpdate()));
    m_typewidget->slotUpdate();
    m_filewidget->slotUpdate();
    m_scriptwidget->slotUpdate();
    m_collectionwidget->slotUpdate();
}

KoScriptManagerAddWizard::~KoScriptManagerAddWizard()
{
}

int KoScriptManagerAddWizard::exec()
{
    return KAssistantDialog::exec();
}

bool KoScriptManagerAddWizard::invokeWidgetMethod(const char *member)
{
    KPageWidget *pagewidget = pageWidget();
    Q_ASSERT(pagewidget);
    KPageWidgetItem *item = pagewidget->currentPage();
    Q_ASSERT(item);
    bool ok = true;
    QMetaObject::invokeMethod(item->widget(), member, Q_RETURN_ARG(bool,ok));
    kDebug(32010) << "object=" << item->widget()->objectName() << " member=" << member << " ok=" << ok;
    return ok;
}

void KoScriptManagerAddWizard::back()
{
    if (invokeWidgetMethod("back"))
        KAssistantDialog::back();
}

void KoScriptManagerAddWizard::next()
{
    if (invokeWidgetMethod("next"))
        KAssistantDialog::next();
}

void KoScriptManagerAddWizard::accept()
{
    if (invokeWidgetMethod("accept"))
        KAssistantDialog::accept();
}

#include <KoScriptManagerAdd.moc>
