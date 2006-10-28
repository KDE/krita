/***************************************************************************
 * form.cpp
 * This file is part of the KDE project
 * copyright (C)2006 by Sebastian Sauer (mail@dipe.org)
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

#include "form.h"

#include <QByteArray>
#include <QBuffer>
#include <QRegExp>
#include <QFile>
#include <QMetaObject>
#include <QMetaEnum>
#include <QKeyEvent>
#include <QDialog>
#include <QVBoxLayout>
#include <QSizePolicy>
#include <QApplication>
#include <QProgressDialog>
#include <QtDesigner/QFormBuilder>

#include <kdebug.h>
#include <kurl.h>
#include <kpushbutton.h>
#include <kurlcombobox.h>
#include <kdiroperator.h>
#include <kshell.h>
#include <kicon.h>
#include <kurlbar.h>
#include <kmessagebox.h>
//#include <kio/netaccess.h>
//#include <klocale.h>
//#include <kicon.h>
//#include <kmimetype.h>
//#include <kstandarddirs.h>
#include <kaction.h>
#include <kactioncollection.h>

extern "C"
{
    QObject* krossmodule()
    {
        return new Kross::FormModule();
    }
}

using namespace Kross;

/*********************************************************************************
 * FormFileWidget
 */

namespace Kross {

    /// \internal implementation of the customized KFileDialog
    class FormFileWidgetImpl : public KFileDialog
    {
        public:
            FormFileWidgetImpl(QWidget* parent, const QString& startDirOrVariable)
                : KFileDialog(KUrl(startDirOrVariable), "", parent, 0)
            {
                setModal( false );
                setParent( parent, windowFlags() & ~Qt::WindowType_Mask );
                setGeometry(0, 0, width(), height());
                setFocusProxy( locationEdit );
                //setMinimumSize( QSize(width(), height()) );
                setMinimumSize( QSize(480,360) );

                if( layout() )
                    layout()->setMargin(0);
                if( parent->layout() )
                    parent->layout()->addWidget(this);

                if( KFileDialog::okButton() )
                    KFileDialog::okButton()->setVisible(false);
                if( KFileDialog::cancelButton() )
                    KFileDialog::cancelButton()->setVisible(false);

                KFileDialog::setMode( KFile::File | KFile::LocalOnly );

                if( actionCollection() ) {
                    KAction* a = actionCollection()->action("toggleSpeedbar");
                    if( a && a->isCheckable() && a->isChecked() ) a->toggle();
                    a = actionCollection()->action("toggleBookmarks");
                    if( a && a->isCheckable() && a->isChecked() ) a->toggle();
                }
            }

            virtual ~FormFileWidgetImpl() {}

            QString selectedFile() const
            {
                KUrl selectedUrl;
                QString locationText = locationEdit->currentText();
                if( locationText.contains( '/' ) ) { // relative path? -> prepend the current directory
                    KUrl u( ops->url(), KShell::tildeExpand(locationText) );
                    selectedUrl = u.isValid() ? u : selectedUrl = ops->url();
                }
                else { // simple filename -> just use the current URL
                    selectedUrl = ops->url();
                }

                QFileInfo fi( selectedUrl.path(), locationEdit->currentText() );
                return fi.absoluteFilePath();
            }

            virtual void accept()
            {
                kDebug() << "FormFileWidget::accept m_file=" << selectedFile() << endl;
                setResult( QDialog::Accepted );
                //emit fileSelected( selectedFile() );
                //KFileDialog::accept();
            }

            virtual void reject()
            {
                kDebug() << "FormFileWidget::reject" << endl;
                for(QWidget* parent = parentWidget(); parent; parent = parent->parentWidget()) {
                    FormDialog* dialog = qobject_cast<FormDialog*>(parent);
                    if( dialog ) { dialog->reject(); break; }
                    if( parent == QApplication::activeModalWidget() || parent == QApplication::activeWindow() ) break;
                }
                //KFileDialog::reject();
            }
    };

    /// \internal d-pointer class.
    class FormFileWidget::Private
    {
        public:
            FormFileWidgetImpl* impl;

            QString startDirOrVariable;
            KFileDialog::OperationMode mode;
            QString currentFilter;
            QString filter;
            QString currentMimeFilter;
            QStringList mimeFilter;
            QString selectedFile;

            Private(const QString& startDirOrVariable) : impl(0), startDirOrVariable(startDirOrVariable) {}
    };

}

FormFileWidget::FormFileWidget(QWidget* parent, const QString& startDirOrVariable)
    : QWidget(parent)
    , d( new Private(startDirOrVariable) )
{
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setSpacing(0);
    layout->setMargin(0);
    setLayout(layout);
}

FormFileWidget::~FormFileWidget()
{
    delete d;
}

void FormFileWidget::setMode(const QString& mode)
{
    QMetaEnum e = metaObject()->enumerator( metaObject()->indexOfEnumerator("Mode") );
    d->mode = (KFileDialog::OperationMode) e.keysToValue(mode.toLatin1());
    if( d->impl )
        d->impl->setOperationMode(d->mode);
}

QString FormFileWidget::currentFilter() const
{
    return d->impl ? d->impl->currentFilter() : d->currentFilter;
}

void FormFileWidget::setFilter(QString filter)
{
    filter.replace(QRegExp("([^\\\\]{1,1})/"), "\\1\\/"); // escape '/' chars else KFileDialog assumes they are mimetypes :-/
    d->filter = filter;
    if( d->impl )
        d->impl->setFilter(d->filter);
}

QString FormFileWidget::currentMimeFilter() const
{
    return d->impl ? d->impl->currentMimeFilter() : d->currentMimeFilter;
}

void FormFileWidget::setMimeFilter(const QStringList& filter)
{
    d->mimeFilter = filter;
    if( d->impl )
        d->impl->setMimeFilter(d->mimeFilter);
}

QString FormFileWidget::selectedFile() const
{
    return d->impl ? d->impl->selectedFile() : d->selectedFile;
}

void FormFileWidget::showEvent(QShowEvent* event)
{
    QWidget::showEvent(event);
    if( ! d->impl ) {
        d->impl = new FormFileWidgetImpl(this, d->startDirOrVariable);
        d->impl->setOperationMode(d->mode);
        if( d->mimeFilter.count() > 0 )
            d->impl->setMimeFilter(d->mimeFilter);
        else if( ! d->filter.isEmpty() )
            d->impl->setFilter(d->filter);
        d->impl->show();
    }
}

void FormFileWidget::hideEvent(QHideEvent* event)
{
    QWidget::hideEvent(event);
    /*
    if( d->impl ) {
        d->selectedFile = selectedFile();
        d->currentFilter = currentFilter();
        d->currentMimeFilter = currentMimeFilter();
        delete d->impl; d->impl = 0;
    }
    */
}

/*********************************************************************************
 * FormDialog
 */

namespace Kross {

    /// \internal d-pointer class.
    class FormDialog::Private
    {
        public:
            KDialog::ButtonCode buttoncode;
            QHash<QString, KPageWidgetItem*> items;
    };

}

FormDialog::FormDialog(const QString& caption)
    : KPageDialog( /*0, Qt::WShowModal | Qt::WDestructiveClose*/ )
    , d( new Private() )
{
    setCaption(caption);
    KDialog::setButtons(KDialog::Ok);
    setSizePolicy( QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding) );

    connect(this, SIGNAL(currentPageChanged(KPageWidgetItem*,KPageWidgetItem*)),
            this, SLOT(slotCurrentPageChanged(KPageWidgetItem*)));
}

FormDialog::~FormDialog()
{
    delete d;
}

bool FormDialog::setButtons(const QString& buttons)
{
    int i = metaObject()->indexOfEnumerator("ButtonCode");
    Q_ASSERT( i >= 0 );
    QMetaEnum e = metaObject()->enumerator(i);
    int v = e.keysToValue( buttons.toUtf8() );
    if( v < 0 )
        return false;
    KDialog::setButtons( (KDialog::ButtonCode) v );
    return true;
}

bool FormDialog::setFaceType(const QString& facetype)
{
    int i = KPageView::staticMetaObject.indexOfEnumerator("FaceType");
    Q_ASSERT( i >= 0 );
    QMetaEnum e = KPageView::staticMetaObject.enumerator(i);
    int v = e.keysToValue( facetype.toUtf8() );
    if( v < 0 )
        return false;
    KPageDialog::setFaceType( (KPageDialog::FaceType) v );
    return true;
}

QString FormDialog::currentPage() const
{
    KPageWidgetItem* item = KPageDialog::currentPage();
    return item ? item->name() : QString::null;
}

bool FormDialog::setCurrentPage(const QString& name)
{
    if( ! d->items.contains(name) )
        return false;
    KPageDialog::setCurrentPage( d->items[name] );
    return true;
}

QWidget* FormDialog::page(const QString& name) const
{
    return d->items.contains(name) ? d->items[name]->widget() : 0;
}

QWidget* FormDialog::addPage(const QString& name, const QString& header, const QString& iconname)
{
    QWidget* widget = new QWidget( mainWidget() );
    QVBoxLayout* boxlayout = new QVBoxLayout(widget);
    boxlayout->setSpacing(0);
    boxlayout->setMargin(0);
    widget->setLayout(boxlayout);

    KPageWidgetItem* item = KPageDialog::addPage(widget, name);
    item->setHeader(header);
    if( ! iconname.isEmpty() )
        item->setIcon( KIcon(iconname) );
    d->items.insert(name, item);

    return item->widget();
}

QString FormDialog::result()
{
    int i = metaObject()->indexOfEnumerator("ButtonCode");
    if( i < 0 ) {
        kWarning() << "Kross::FormDialog::setButtons No such enumerator \"ButtonCode\"" << endl;
        return QString::null;
    }
    QMetaEnum e = metaObject()->enumerator(i);
    return e.valueToKey(d->buttoncode);
}

void FormDialog::slotButtonClicked(int button)
{
    d->buttoncode = (KDialog::ButtonCode) button;
    KDialog::slotButtonClicked(button);
}

void FormDialog::slotCurrentPageChanged(KPageWidgetItem* current)
{
    Q_UNUSED(current);
    //kDebug() << "FormDialog::slotCurrentPageChanged current=" << current->name() << endl;
    //foreach(QWidget* widget, current->widget()->findChildren< QWidget* >("")) widget->setFocus();
}

/*********************************************************************************
 * FormModule
 */

namespace Kross {

    /// \internal d-pointer class.
    class FormModule::Private
    {
        public:
            QFormBuilder* builder;
            Private() : builder( new QFormBuilder() ) {}
            ~Private() { delete builder; }
    };

}

FormModule::FormModule()
    : QObject()
    , d( new Private() )
{
}

FormModule::~FormModule()
{
    delete d;
}

QWidget* FormModule::activeModalWidget()
{
    return QApplication::activeModalWidget();
}

QWidget* FormModule::activeWindow()
{
    return QApplication::activeWindow();
}

QString FormModule::showMessageBox(const QString& dialogtype, const QString& caption, const QString& message)
{
    KMessageBox::DialogType type;
    if(dialogtype == "QuestionYesNo") type = KMessageBox::QuestionYesNo;
    else if(dialogtype == "WarningYesNo") type = KMessageBox::WarningYesNo;
    else if(dialogtype == "WarningContinueCancel") type = KMessageBox::WarningContinueCancel;
    else if(dialogtype == "WarningYesNoCancel") type = KMessageBox::WarningYesNoCancel;
    else if(dialogtype == "Sorry") type = KMessageBox::Sorry;
    else if(dialogtype == "Error") type = KMessageBox::Error;
    else if(dialogtype == "QuestionYesNoCancel") type = KMessageBox::QuestionYesNoCancel;
    else /*if(dialogtype == "Information")*/ type = KMessageBox::Information;

    switch( KMessageBox::messageBox(0, type, message, caption) ) {
        case KMessageBox::Ok: return "Ok";
        case KMessageBox::Cancel: return "Cancel";
        case KMessageBox::Yes: return "Yes";
        case KMessageBox::No: return "No";
        case KMessageBox::Continue: return "Continue";
        default: return QString();
    }
}

QWidget* FormModule::showProgressDialog(const QString& caption, const QString& labelText)
{
    QProgressDialog* progress = new QProgressDialog();
    //progress->setWindowModality(Qt::WindowModal);
    progress->setModal(true);
    progress->setWindowTitle(caption);
    progress->setLabelText(labelText);
    progress->setAutoClose(true);
    progress->setAutoReset(true);
    progress->setCancelButtonText(QString());
    progress->setMinimumWidth(300);
    progress->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding));
    progress->show();
    return progress;
}

QWidget* FormModule::createDialog(const QString& caption)
{
    return new FormDialog(caption);
}

#if 0
QWidget* FormModule::createWidget(QWidget* parent, const QString& classname)
{
    //return new FormWidget(parent);
}
#endif

QWidget* FormModule::createFileWidget(QWidget* parent, const QString& startDirOrVariable)
{
    if( ! parent )
        return 0;
    FormFileWidget* widget = new FormFileWidget(parent, startDirOrVariable);
    if( parent->layout() )
        parent->layout()->addWidget(widget);
    return widget;
}

QWidget* FormModule::createWidgetFromUI(QWidget* parent, const QString& xml)
{
    if( ! parent )
        return 0;
    QByteArray ba = xml.toUtf8();
    QBuffer buffer(&ba);
    buffer.open(QIODevice::ReadOnly);
    QWidget* widget = d->builder->load(&buffer, parent);
    if( widget && parent->layout() )
        parent->layout()->addWidget(widget);
    return widget;
}

QWidget* FormModule::createWidgetFromUIFile(QWidget* parent, const QString& filename)
{
    if( ! parent )
        return 0;
    QFile file(filename);
    if( ! file.exists() ) {
        kDebug() << QString("Kross::FormModule::createWidgetFromUIFile: There exists no such file \"%1\"").arg(filename) << endl;
        return false;
    }
    if( ! file.open(QFile::ReadOnly) ) {
        kDebug() << QString("Kross::FormModule::createWidgetFromUIFile: Failed to open the file \"%1\"").arg(filename) << endl;
        return false;
    }
    const QString xml = file.readAll();
    file.close();
    return createWidgetFromUI(parent, xml);
}

#include "form.moc"
