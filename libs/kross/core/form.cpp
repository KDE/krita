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
#include <QtDesigner/QFormBuilder>

#include <kdebug.h>
#include <kdialog.h>
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
    return true;
}

/*********************************************************************************
 * FormDialog
 */

FormDialog::FormDialog(KDialog* dialog)
    : Form(dialog->mainWidget())
    , m_dialog(dialog)
{
}

FormDialog::~FormDialog()
{
}

int FormDialog::exec()
{
    return m_dialog->exec();
}

int FormDialog::exec_loop()
{
    exec();
}

#include "form.moc"
