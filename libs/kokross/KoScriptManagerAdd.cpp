/***************************************************************************
 * scriptmanageradd.h
 * This file is part of the KDE project
 * copyright (C) 2006-2007 Sebastian Sauer <mail@dipe.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 * You should have received a copy of the GNU Library General Public License
 * along with this program; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 ***************************************************************************/

#include "scriptmanageradd.h"
#include "scriptmanager.h"

#include <kross/core/manager.h>
#include <kross/core/interpreter.h>
#include <kross/core/action.h>
#include <kross/core/actioncollection.h>
#include <kross/ui/view.h>

#include <QtCore/QFileInfo>
#include <QtGui/QBoxLayout>
#include <QtGui/QLabel>
#include <QtGui/QRadioButton>

#include <klocale.h>
#include <kurl.h>
#include <kmimetype.h>
#include <kfilewidget.h>

using namespace Kross;

/********************************************************************
 * ScriptManagerAddTypeWidget
 */

ScriptManagerAddTypeWidget::ScriptManagerAddTypeWidget(ScriptManagerAddWizard* wizard)
    : QWidget(wizard), m_wizard(wizard)
{
    setObjectName("ScriptManagerAddTypeWidget");
    QVBoxLayout* layout = new QVBoxLayout(this);
    setLayout(layout);
    QLabel* label = new QLabel(i18n("<qt>This wizard will guide you through the proccess of adding a new resource to your scripts.</qt>"), this);
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

ScriptManagerAddTypeWidget::~ScriptManagerAddTypeWidget()
{
}

void ScriptManagerAddTypeWidget::slotUpdate()
{
    m_wizard->setAppropriate(m_wizard->m_fileItem, m_scriptCheckbox->isChecked());
    m_wizard->setAppropriate(m_wizard->m_scriptItem, m_scriptCheckbox->isChecked());
    m_wizard->setAppropriate(m_wizard->m_collectionItem, m_collectionCheckbox->isChecked());
    //m_installCheckBox->isChecked()
    //m_onlineCheckbox->isChecked()
}

/********************************************************************
 * ScriptManagerAddFileWidget
 */

namespace Kross {
    /// \internal d-pointer class.
    class ScriptManagerAddFileWidget::Private
    {
        public:
            ScriptManagerAddWizard* const wizard;
            KFileWidget* filewidget;
            explicit Private(ScriptManagerAddWizard* const w): wizard(w) {}
    };
}

ScriptManagerAddFileWidget::ScriptManagerAddFileWidget(ScriptManagerAddWizard* wizard, const QString& startDirOrVariable)
    : QWidget(wizard), d(new Private(wizard))
{
    setObjectName("ScriptManagerAddFileWidget");
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setMargin(0);
    setLayout(layout);
    d->filewidget = new KFileWidget(KUrl(startDirOrVariable), this);

    QStringList mimetypes;
    foreach(QString interpretername, Manager::self().interpreters()) {
        InterpreterInfo* info = Manager::self().interpreterInfo(interpretername);
        Q_ASSERT( info );
        mimetypes.append( info->mimeTypes().join(" ").trimmed() );
    }
    d->filewidget->setMimeFilter(mimetypes /*, defaultmime*/);

    layout->addWidget( d->filewidget );
    connect(d->filewidget, SIGNAL(fileHighlighted(const QString&)), this, SLOT(slotUpdate()));
    connect(d->filewidget, SIGNAL(fileSelected(const QString&)), this, SLOT(slotUpdate()));
}

ScriptManagerAddFileWidget::~ScriptManagerAddFileWidget()
{
    delete d;
}

QString ScriptManagerAddFileWidget::selectedFile() const
{
    return d->filewidget->selectedFile();
}

void ScriptManagerAddFileWidget::slotUpdate()
{
    d->wizard->setValid(d->wizard->m_fileItem, ! d->filewidget->selectedFile().isEmpty());
}

/********************************************************************
 * ScriptManagerAddScriptWidget
 */

namespace Kross {
    /// \internal d-pointer class.
    class ScriptManagerAddScriptWidget::Private
    {
        public:
            ScriptManagerAddWizard* const wizard;
            ActionCollectionEditor* editor;
            explicit Private(ScriptManagerAddWizard* const w): wizard(w), editor(0) {}
    };
}

ScriptManagerAddScriptWidget::ScriptManagerAddScriptWidget(ScriptManagerAddWizard* wizard)
    : QWidget(wizard), d(new Private(wizard))
{
    setObjectName("ScriptManagerAddScriptWidget");
    QVBoxLayout* layout = new QVBoxLayout(this);
    setLayout(layout);
}

ScriptManagerAddScriptWidget::~ScriptManagerAddScriptWidget()
{
    delete d;
}

void ScriptManagerAddScriptWidget::slotUpdate()
{
    d->wizard->setValid(d->wizard->m_scriptItem, d->editor && d->editor->isValid());
}

void ScriptManagerAddScriptWidget::showEvent(QShowEvent* event)
{
    Action* action = 0;
    if( d->editor ) {
        action = d->editor->action();
        delete d->editor;
    }
    if( ! action )
        action = new Action(0, "");

    const QString file = d->wizard->m_filewidget->selectedFile();
    QFileInfo fi( file );
    action->setObjectName( file );
    action->setText( fi.baseName() );
    //action->setDescription();
    if( fi.isFile() ) {
        action->setIconName( KMimeType::iconNameForUrl(KUrl(file)) );
        action->setEnabled( fi.exists() );
    }
    action->setFile( file );

    d->editor = new ActionCollectionEditor(action, this);
    layout()->addWidget(d->editor);

    QWidget::showEvent(event);
    slotUpdate();
}

bool ScriptManagerAddScriptWidget::accept()
{
    kDebug()<<"ScriptManagerAddScriptWidget::accept()"<<endl;
    Q_ASSERT( d->editor );
    Q_ASSERT( d->editor->action() );
    Q_ASSERT( d->wizard );
    Q_ASSERT( d->wizard->m_collection );
    d->editor->commit(); // take over changes done in the editor into the action
    d->wizard->m_collection->addAction( d->editor->action() ); // add the action to the collection
    //TODO select new item
    return true;
}

/********************************************************************
 * ScriptManagerAddCollectionWidget
 */

ScriptManagerAddCollectionWidget::ScriptManagerAddCollectionWidget(ScriptManagerAddWizard* wizard)
    : QWidget(wizard), m_wizard(wizard)
{
    setObjectName("ScriptManagerAddCollectionWidget");
    QVBoxLayout* layout = new QVBoxLayout(this);
    setLayout(layout);
    ActionCollection* collection = new ActionCollection("");
    m_editor = new ActionCollectionEditor(collection, this);
    layout->addWidget(m_editor);
}

ScriptManagerAddCollectionWidget::~ScriptManagerAddCollectionWidget()
{
}

void ScriptManagerAddCollectionWidget::slotUpdate()
{
    m_wizard->setValid(m_wizard->m_collectionItem, m_editor->isValid());
}

/********************************************************************
 * ScriptManagerAddWizard
 */

ScriptManagerAddWizard::ScriptManagerAddWizard(QWidget* parent, ActionCollection* collection)
    : KAssistantDialog(parent), m_collection(collection ? collection : Manager::self().actionCollection())
{
    Q_ASSERT(m_collection);
    setObjectName("ScriptManagerAddWizard");
    setCaption( i18n("Add") );

    m_typewidget = new ScriptManagerAddTypeWidget(this);
    m_typeItem = addPage(m_typewidget, i18n("Add"));

    const QString startDirOrVariable = "kfiledialog:///scriptmanageraddfile";
    m_filewidget = new ScriptManagerAddFileWidget(this, startDirOrVariable);
    m_fileItem = addPage(m_filewidget, i18n("Script File"));

    m_scriptwidget = new ScriptManagerAddScriptWidget(this);
    m_scriptItem = addPage(m_scriptwidget, i18n("Script"));

    m_collectionwidget = new ScriptManagerAddCollectionWidget(this);
    m_collectionItem = addPage(m_collectionwidget, i18n("Collection"));

    resize( QSize(620, 460).expandedTo( minimumSizeHint() ) );

    //connect(this, SIGNAL(currentPageChanged(KPageWidgetItem*,KPageWidgetItem*)), this, SLOT(slotUpdate()));
    m_typewidget->slotUpdate();
    m_filewidget->slotUpdate();
    m_scriptwidget->slotUpdate();
    m_collectionwidget->slotUpdate();
}

ScriptManagerAddWizard::~ScriptManagerAddWizard()
{
}

int ScriptManagerAddWizard::exec()
{
    return KAssistantDialog::exec();
}

bool ScriptManagerAddWizard::invokeWidgetMethod(const char* member)
{
    KPageWidget* pagewidget = pageWidget();
    Q_ASSERT( pagewidget );
    KPageWidgetItem* item = pagewidget->currentPage();
    Q_ASSERT( item );
    bool ok = true;
    QMetaObject::invokeMethod(item->widget(), member, Q_RETURN_ARG(bool,ok));
    kDebug()<<"ScriptManagerAddWizard::invokeWidgetMethod object="<<item->widget()->objectName()<<" member="<<member<<" ok="<<ok<<endl;
    return ok;
}

void ScriptManagerAddWizard::back()
{
    if( invokeWidgetMethod("back") )
        KAssistantDialog::back();
}

void ScriptManagerAddWizard::next()
{
    if( invokeWidgetMethod("next") )
        KAssistantDialog::next();
}

void ScriptManagerAddWizard::accept()
{
    if( invokeWidgetMethod("accept") )
        KAssistantDialog::accept();
}

#include "scriptmanageradd.moc"
