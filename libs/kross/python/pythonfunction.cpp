/***************************************************************************
 * pythonfunction.cpp
 * This file is part of the KDE project
 * copyright (C)2006 by Sebastian Sauer (mail@dipe.org)
 *
 * Parts of the code are from kjsembed4 SlotProxy
 * Copyright (C) 2005, 2006 KJSEmbed Authors.
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

#include "pythonfunction.h"
#include "pythonvariant.h"
#include "pythoninterpreter.h"
#include "pythonextension.h"
#include <kross/core/metatype.h>

#include <QByteArray>
#include <QPointer>
#include <QVariant>
#include <QMetaMethod>
#include <QMetaObject>

using namespace Kross;

namespace Kross {

    /// \internal d-pointer class.
    class PythonFunction::Private
    {
        public:
            QPointer<QObject> sender;
            QByteArray signature;
            Py::Callable callable;
            QByteArray stringData;
            uint data[21];
            QVariant tmpResult;
    };

}

PythonFunction::PythonFunction(QObject* sender, const QByteArray& signal, const Py::Callable& callable)
    : QObject()
    , d( new Private() )
{
    d->sender = sender;
    d->signature = QMetaObject::normalizedSignature( signal );
    d->callable = callable;

    //krossdebug(QString("PythonFunction::PythonFunction sender=\"%1\" signal=\"%2\"").arg(sender->objectName()).arg(d->signature.constData()));

    uint signatureSize = d->signature.size() + 1;

    // content
    d->data[0] = 1;  // revision
    d->data[1] = 0;  // classname
    d->data[2] = 0;  // classinfo
    d->data[3] = 0;  // classinfo
    d->data[4] = 1;  // methods
    d->data[5] = 15; // methods
    d->data[6] = 0;  // properties
    d->data[7] = 0;  // properties
    d->data[8] = 0;  // enums/sets
    d->data[9] = 0;  // enums/sets

    // slots
    d->data[15] = 15;  // signature start
    d->data[16] = 15 + signatureSize;  // parameters start
    d->data[17] = 15 + signatureSize;  // type start
    d->data[18] = 15 + signatureSize;  // tag start
    d->data[19] = 0x0a; // flags
    d->data[20] = 0;    // eod

    // data
    d->stringData = QByteArray("PythonFunction\0", 15);
    d->stringData += d->signature;
    d->stringData += QByteArray("\0\0", 2);

    // static metaobject
    staticMetaObject.d.superdata = &QObject::staticMetaObject;
    staticMetaObject.d.stringdata = d->stringData.data();
    staticMetaObject.d.data = d->data;
    staticMetaObject.d.extradata = 0;
}

PythonFunction::~PythonFunction()
{
    //krossdebug("PythonFunction::~PythonFunction");
    delete d;
}

const QMetaObject* PythonFunction::metaObject() const
{
    return &staticMetaObject;
}

void* PythonFunction::qt_metacast(const char *_clname)
{
    if (! _clname)
        return 0;
    if (! strcmp(_clname, d->stringData))
        return static_cast<void*>( const_cast<PythonFunction*>(this) );
    return QObject::qt_metacast(_clname);
}

int PythonFunction::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    //krossdebug(QString("PythonFunction::qt_metacall id=%1").arg(_id));
    if(_id >= 0 && _c == QMetaObject::InvokeMetaMethod) {
        switch(_id) {
            case 0: {
                // convert the arguments
                QMetaMethod method = metaObject()->method( metaObject()->indexOfMethod(d->signature) );
                QList<QByteArray> params = method.parameterTypes();
                Py::Tuple args( params.size() );
                int idx = 1;
                foreach(QByteArray param, params) {
                    int tp = QVariant::nameToType( param.constData() );
                    switch(tp) {
                        case QVariant::Invalid: // fall through
                        case QVariant::UserType: {
                            tp = QMetaType::type( param.constData() );
                            #ifdef KROSS_PYTHON_FUNCTION_DEBUG
                                krossdebug( QString("PythonFunction::qt_metacall: metatypeId=%1").arg(tp) );
                            #endif
                            switch( tp ) {
                                case QMetaType::QObjectStar: {
                                    QObject* obj = (*reinterpret_cast< QObject*(*)>( _a[idx] ));
                                    args[idx-1] = Py::asObject(new PythonExtension(obj));
                                } break;
                                case QMetaType::QWidgetStar: {
                                    QWidget* obj = (*reinterpret_cast< QWidget*(*)>( _a[idx] ));
                                    args[idx-1] = Py::asObject(new PythonExtension(obj));
                                } break;
                                default: {
                                    args[idx-1] = Py::None();
                                } break;
                            }
                        } break;
                        default: {
                            QVariant v(tp, _a[idx]);
                            #ifdef KROSS_PYTHON_FUNCTION_DEBUG
                                krossdebug( QString("PythonFunction::qt_metacall argument param=%1 typeId=%2").arg(param.constData()).arg(tp) );
                            #endif
                            args[idx-1] = PythonType<QVariant>::toPyObject(v);
                        } break;
                    }
                    ++idx;
                }

                Py::Object result;
                try {
                    // call the python function
                    result = d->callable.apply(args);
                }
                catch(Py::Exception& e) {
                    QStringList trace;
                    int lineno;
                    PythonInterpreter::extractException(trace, lineno);
                    krosswarning( QString("PythonFunction::qt_metacall exception on line %1:\n%2 \n%3").arg(lineno).arg(Py::value(e).as_string().c_str()).arg(trace.join("\n")) );
                    PyErr_Print(); //e.clear();
                    return -1;
                }

                // finally set the returnvalue
                d->tmpResult = PythonType<QVariant>::toVariant(result);
                #ifdef KROSS_PYTHON_FUNCTION_DEBUG
                    QObject* sender = QObject::sender();
                    krossdebug( QString("PythonFunction::qt_metacall sender.objectName=%1 sender.className=%2 pyresult=%3 variantresult=%4").arg(sender->objectName()).arg(sender->metaObject()->className()).arg(result.as_string().c_str()).arg(d->tmpResult.toString()) );
                #endif
                //_a[0] = Kross::MetaTypeVariant<QVariant>(d->tmpResult).toVoidStar();
                _a[0] = &(d->tmpResult);
            } break;
        }
        _id -= 1;
    }
    return _id;
}
