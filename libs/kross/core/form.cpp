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
#include <QFile>
#include <QDialog>
#include <QVBoxLayout>
#include <QSizePolicy>
#include <QtDesigner/QFormBuilder>
#include <QMetaObject>
#include <QMetaEnum>

#include <kdebug.h>
//#include <klocale.h>
//#include <kicon.h>
//#include <kmimetype.h>
//#include <kstandarddirs.h>

using namespace Kross;

/*********************************************************************************
 * Form
 */

namespace Kross {

    /// \internal d-pointer class.
    class Form::Private
    {
        public:
            QWidget* widget;

            QFormBuilder* formBuilder() {
                if(! m_formbuilder)
                    m_formbuilder = new QFormBuilder();
                return m_formbuilder;
            }

            Private() : widget(0), m_formbuilder(0) {}
            ~Private() { delete m_formbuilder; m_formbuilder = 0; }
        private:
            QFormBuilder* m_formbuilder;
    };

}

Form::Form(QWidget* parent)
    : QWidget(parent)
    , d( new Private() )
{
    QVBoxLayout* boxlayout = new QVBoxLayout(this);
    boxlayout->setSpacing(0);
    boxlayout->setMargin(0);
    setLayout(boxlayout);

    if(parent->layout())
        parent->layout()->addWidget(this);
}

Form::~Form()
{
    delete d;
}

bool Form::loadUiFile(const QString& filename)
{
    QFile file(filename);
    if(! file.exists()) {
        kDebug() << QString("Kross::Form::loadUiFile: There exists no such file \"%1\"").arg(filename) << endl;
        return false;
    }
    if(! file.open(QFile::ReadOnly)) {
        kDebug() << QString("Kross::Form::loadUiFile: Failed to open the file \"%1\"").arg(filename) << endl;
        return false;
    }
    const QString xml = file.readAll();
    file.close();
    return fromUiXml(xml);
}

bool Form::saveUiFile(const QString& filename)
{
    const QString xml = toUiXml();
    if(xml.isEmpty()) {
        kDebug() << QString("Kross::Form::loadUiFile: No UI xml that could be saved to file \"%1\"").arg(filename) << endl;
        return false;
    }
    QFile file(filename);
    if(! file.open(QFile::WriteOnly)) {
        kDebug() << QString("Kross::Form::loadUiFile: Failed to write the file \"%1\"").arg(filename) << endl;
        return false;
    }
    file.write( xml.toUtf8() );
    if(! file.flush()) {
        kDebug() << QString("Kross::Form::loadUiFile: Failed to flush the file \"%1\"").arg(filename) << endl;
        file.close();
        return false;
    }
    file.close();
    return true;
}

QString Form::toUiXml()
{
    if(! d->widget) {
        return "";
    }
    QByteArray ba;
    QBuffer buffer(&ba);
    buffer.open(QIODevice::WriteOnly);
    d->formBuilder()->save(&buffer, d->widget);
    return ba;
}

bool Form::fromUiXml(const QString& xml)
{
    if(d->widget) {
        delete d->widget;
        d->widget = 0;
    }
    if(xml.isEmpty()) {
         return true;
    }
    QByteArray ba = xml.toUtf8();
    QBuffer buffer(&ba);
    buffer.open(QIODevice::ReadOnly);
    d->widget = d->formBuilder()->load(&buffer, this);
    if(! d->widget) {
        return false;
    }
    layout()->addWidget(d->widget);
    return true;
}

/*********************************************************************************
 * Dialog
 */

Dialog::Dialog(const QString& caption)
    : KDialog()
{
    setCaption(caption);
    KDialog::setButtons(KDialog::Ok);

    m_form = new Form( mainWidget() );
    setMainWidget(m_form);

    //m_dialog->setSizePolicy( QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred) );
    setMinimumWidth(380);
}

Dialog::~Dialog()
{
}

bool Dialog::setButtons(QString buttons)
{
    int i = metaObject()->indexOfEnumerator("ButtonCode");
    if(i < 0) {
        kWarning() << "Kross::Dialog::setButtons No such enumerator \"ButtonCode\"" << endl;
        return false;
    }
    QMetaEnum e = metaObject()->enumerator(i);
    int v = e.keysToValue( buttons.toUtf8() );
    if(v < 0) {
        kWarning() << "Kross::Dialog::setButtons Invalid buttons \"" << buttons << "\" defined" << endl;
        return false;
    }
    KDialog::setButtons( (KDialog::ButtonCode)v );
    /*
    QFlags<KDialog::ButtonCode> buttonmask;
    foreach(QString b, buttons.split("|")) {
        b = b.trimmed().toLower();
        if(b == "ok")
            buttonmask |= KDialog::Ok;
        else if(b == "apply")
            buttonmask |= KDialog::Apply;
        else if(b == "try")
            buttonmask |= KDialog::Try;
        else if(b == "cancel")
            buttonmask |= KDialog::Cancel;
        else if(b == "close")
            buttonmask |= KDialog::Close;
        else if(b == "no")
            buttonmask |= KDialog::No;
        else if(b == "yes")
            buttonmask |= KDialog::Yes;
        else if(b == "details")
            buttonmask |= KDialog::Details;
    }
    KDialog::setButtons( buttonmask );
    */
    return true;
}

#include "form.moc"
