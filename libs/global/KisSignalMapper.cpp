/****************************************************************************
**
** SPDX-FileCopyrightText: 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtCore module of the Qt Toolkit.
**
** SPDX-License-Identifier: LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KFQF-Accepted-GPL OR LicenseRef-Qt-Commercial
**
****************************************************************************/

#include "KisSignalMapper.h"
#include "qhash.h"

class KisSignalMapper::Private
{
public:
    
    Private(KisSignalMapper *_q)
        :q(_q)
        {}
    
    void _q_senderDestroyed() {
        q->removeMappings(q->sender());
    }
    QHash<QObject *, int> intHash;
    QHash<QObject *, QString> stringHash;
    QHash<QObject *, QWidget*> widgetHash;
    QHash<QObject *, QObject*> objectHash;

    
    KisSignalMapper *q;
};

/*!
    \class KisSignalMapper
    \inmodule QtCore
    \obsolete The recommended solution is connecting the signal to a lambda.
    \brief The KisSignalMapper class bundles signals from identifiable senders.

    \ingroup objectmodel


    This class collects a set of parameterless signals, and re-emits
    them with integer, string or widget parameters corresponding to
    the object that sent the signal.

    The class supports the mapping of particular strings or integers
    with particular objects using setMapping(). The objects' signals
    can then be connected to the map() slot which will emit the
    mapped() signal with the string or integer associated with the
    original signaling object. Mappings can be removed later using
    removeMappings().

    Example: Suppose we want to create a custom widget that contains
    a group of buttons (like a tool palette). One approach is to
    connect each button's \c clicked() signal to its own custom slot;
    but in this example we want to connect all the buttons to a
    single slot and parameterize the slot by the button that was
    clicked.

    Here's the definition of a simple custom widget that has a single
    signal, \c clicked(), which is emitted with the text of the button
    that was clicked:

    \snippet KisSignalMapper/buttonwidget.h 0
    \snippet KisSignalMapper/buttonwidget.h 1

    The only function that we need to implement is the constructor:

    \snippet KisSignalMapper/buttonwidget.cpp 0
    \snippet KisSignalMapper/buttonwidget.cpp 1
    \snippet KisSignalMapper/buttonwidget.cpp 2

    A list of texts is passed to the constructor. A signal mapper is
    constructed and for each text in the list a QPushButton is
    created. We connect each button's \c clicked() signal to the
    signal mapper's map() slot, and create a mapping in the signal
    mapper from each button to the button's text. Finally we connect
    the signal mapper's mapped() signal to the custom widget's \c
    clicked() signal. When the user clicks a button, the custom
    widget will emit a single \c clicked() signal whose argument is
    the text of the button the user clicked.

    This class was mostly useful before lambda functions could be used as
    slots. The example above can be rewritten simpler without KisSignalMapper
    by connecting to a lambda function.

    \snippet KisSignalMapper/buttonwidget.cpp 3

    \sa QObject, QButtonGroup, QActionGroup
*/

/*!
    Constructs a KisSignalMapper with parent \a parent.
*/
KisSignalMapper::KisSignalMapper(QObject* parent)
    : QObject(parent)
    , d(new Private(this))
{
}

/*!
    Destroys the KisSignalMapper.
*/
KisSignalMapper::~KisSignalMapper()
{
}

/*!
    Adds a mapping so that when map() is signalled from the given \a
    sender, the signal mapped(\a id) is emitted.

    There may be at most one integer ID for each sender.

    \sa mapping()
*/
void KisSignalMapper::setMapping(QObject *sender, int id)
{
    d->intHash.insert(sender, id);
    connect(sender, SIGNAL(destroyed()), this, SLOT(_q_senderDestroyed()));
}

/*!
    Adds a mapping so that when map() is signalled from the \a sender,
    the signal mapped(\a text ) is emitted.

    There may be at most one text for each sender.
*/
void KisSignalMapper::setMapping(QObject *sender, const QString &text)
{
    d->stringHash.insert(sender, text);
    connect(sender, SIGNAL(destroyed()), this, SLOT(_q_senderDestroyed()));
}

/*!
    Adds a mapping so that when map() is signalled from the \a sender,
    the signal mapped(\a widget ) is emitted.

    There may be at most one widget for each sender.
*/
void KisSignalMapper::setMapping(QObject *sender, QWidget *widget)
{
    d->widgetHash.insert(sender, widget);
    connect(sender, SIGNAL(destroyed()), this, SLOT(_q_senderDestroyed()));
}

/*!
    Adds a mapping so that when map() is signalled from the \a sender,
    the signal mapped(\a object ) is emitted.

    There may be at most one object for each sender.
*/
void KisSignalMapper::setMapping(QObject *sender, QObject *object)
{
    d->objectHash.insert(sender, object);
    connect(sender, SIGNAL(destroyed()), this, SLOT(_q_senderDestroyed()));
}

/*!
    Returns the sender QObject that is associated with the \a id.

    \sa setMapping()
*/
QObject *KisSignalMapper::mapping(int id) const
{
    return d->intHash.key(id);
}

/*!
    \overload mapping()
*/
QObject *KisSignalMapper::mapping(const QString &id) const
{
    return d->stringHash.key(id);
}

/*!
    \overload mapping()

    Returns the sender QObject that is associated with the \a widget.
*/
QObject *KisSignalMapper::mapping(QWidget *widget) const
{
    return d->widgetHash.key(widget);
}

/*!
    \overload mapping()

    Returns the sender QObject that is associated with the \a object.
*/
QObject *KisSignalMapper::mapping(QObject *object) const
{
    return d->objectHash.key(object);
}

/*!
    Removes all mappings for \a sender.

    This is done automatically when mapped objects are destroyed.

    \note This does not disconnect any signals. If \a sender is not destroyed
    then this will need to be done explicitly if required.
*/
void KisSignalMapper::removeMappings(QObject *sender)
{
    d->intHash.remove(sender);
    d->stringHash.remove(sender);
    d->widgetHash.remove(sender);
    d->objectHash.remove(sender);
}

/*!
    This slot emits signals based on which object sends signals to it.
*/
void KisSignalMapper::map() { map(sender()); }

/*!
    This slot emits signals based on the \a sender object.
*/
void KisSignalMapper::map(QObject *sender)
{
    if (d->intHash.contains(sender))
        emit mapped(d->intHash.value(sender));
    if (d->stringHash.contains(sender))
        emit mapped(d->stringHash.value(sender));
    if (d->widgetHash.contains(sender))
        emit mapped(d->widgetHash.value(sender));
    if (d->objectHash.contains(sender))
        emit mapped(d->objectHash.value(sender));
}


/*!
    \fn void KisSignalMapper::mapped(int i)

    This signal is emitted when map() is signalled from an object that
    has an integer mapping set. The object's mapped integer is passed
    in \a i.

    \sa setMapping()
*/

/*!
    \fn void KisSignalMapper::mapped(const QString &text)

    This signal is emitted when map() is signalled from an object that
    has a string mapping set. The object's mapped string is passed in
    \a text.

    \sa setMapping()
*/

/*!
    \fn void KisSignalMapper::mapped(QWidget *widget)

    This signal is emitted when map() is signalled from an object that
    has a widget mapping set. The object's mapped widget is passed in
    \a widget.

    \sa setMapping()
*/

/*!
    \fn void KisSignalMapper::mapped(QObject *object)

    This signal is emitted when map() is signalled from an object that
    has an object mapping set. The object provided by the map is passed in
    \a object.

    \sa setMapping()
*/

#include "moc_KisSignalMapper.cpp"

