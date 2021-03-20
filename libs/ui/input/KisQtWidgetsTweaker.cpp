/* This file is part of the KDE project
   SPDX-FileCopyrightText: 2017 Nikita Vertikov <kitmouse.nikita@gmail.com>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "KisQtWidgetsTweaker.h"

#include <QBitArray>
#include <QComboBox>
#include <kddockwidgets/DockWidget.h>
#include <QDockWidget>
#include <QDoubleSpinBox>
#include <QEvent>
#include <QKeyEvent>
#include <QKeySequence>
#include <QLineEdit>
#include <QSpinBox>

#include "opengl/kis_opengl_canvas2.h"
#include "canvas/kis_qpainter_canvas.h"
#include "KisMainWindow.h"

Q_GLOBAL_STATIC(KisQtWidgetsTweaker, kqwt_instance)


namespace {

class ShortcutOverriderBase
{
public:
    enum class DecisionOnShortcutOverride {
        overrideShortcut,
        askNext,
        dontOverrideShortcut
    };

    constexpr ShortcutOverriderBase() = default;
    virtual ~ShortcutOverriderBase()
    {}
    virtual bool interestedInEvent(QKeyEvent *event) = 0;
    virtual DecisionOnShortcutOverride handleEvent(QObject *receiver, QKeyEvent *event) = 0;
    virtual DecisionOnShortcutOverride finishedPhysicalKeyPressHandling()
    {
        return DecisionOnShortcutOverride::askNext;
    }
};

class LineTextEditingShortcutOverrider : public ShortcutOverriderBase
{
public:
    constexpr LineTextEditingShortcutOverrider() = default;

    virtual bool interestedInEvent(QKeyEvent *event) override
    {
        static constexpr QKeySequence::StandardKey actionsForQLineEdit[]{
             QKeySequence::MoveToNextChar
            ,QKeySequence::MoveToPreviousChar
            ,QKeySequence::MoveToStartOfLine
            ,QKeySequence::MoveToEndOfLine
            ,QKeySequence::MoveToPreviousWord
            ,QKeySequence::MoveToNextWord
            ,QKeySequence::SelectPreviousChar
            ,QKeySequence::SelectNextChar
            ,QKeySequence::SelectNextWord
            ,QKeySequence::SelectPreviousWord
            ,QKeySequence::SelectStartOfLine
            ,QKeySequence::SelectEndOfLine
            ,QKeySequence::SelectAll
            ,QKeySequence::Deselect
            ,QKeySequence::Backspace
            ,QKeySequence::DeleteStartOfWord
            ,QKeySequence::Delete
            ,QKeySequence::DeleteEndOfWord
            ,QKeySequence::DeleteEndOfLine
            ,QKeySequence::Copy
            ,QKeySequence::Paste
            ,QKeySequence::Cut
            ,QKeySequence::Undo
            ,QKeySequence::Redo
        };
        for (QKeySequence::StandardKey sk : actionsForQLineEdit) {
            if (event->matches(sk)) {
                event->accept();
                return true;
            }
        }
        return false;
    }

    virtual DecisionOnShortcutOverride handleEvent(QObject *receiver, QKeyEvent *event)  override
    {
        Q_UNUSED(event);

        if ((qobject_cast<QLineEdit*>     (receiver) != nullptr)||
            (qobject_cast<QSpinBox*>      (receiver) != nullptr)||
            (qobject_cast<QDoubleSpinBox*>(receiver) != nullptr)) {

            return DecisionOnShortcutOverride::overrideShortcut;
        } else {
            return DecisionOnShortcutOverride::askNext;
        }
    }
};

class SpinboxShortcutOverrider : public ShortcutOverriderBase
{
public:
    constexpr SpinboxShortcutOverrider() = default;

    virtual bool interestedInEvent(QKeyEvent *event) override
    {
        if (event->modifiers() != Qt::NoModifier) {
            return false;
        }
        switch (event->key()) {
        case Qt::Key_Down:
        case Qt::Key_Up:
        case Qt::Key_PageDown:
        case Qt::Key_PageUp:
            event->accept();
            return true;
        default:
            return false;
        }
    }
    virtual DecisionOnShortcutOverride handleEvent(QObject *receiver, QKeyEvent *event)  override
    {
        Q_UNUSED(event);

        if (qobject_cast<QSpinBox*>      (receiver) != nullptr||
            qobject_cast<QDoubleSpinBox*>(receiver) != nullptr) {

            return DecisionOnShortcutOverride::overrideShortcut;
        } else {
            return DecisionOnShortcutOverride::askNext;
        }

    }
};

class TabShortcutOverrider : public ShortcutOverriderBase
{
public:
    constexpr TabShortcutOverrider() = default;

    virtual bool interestedInEvent(QKeyEvent *event) override
    {
        bool tab = event->modifiers() == Qt::NoModifier &&
                    ( event->key() == Qt::Key_Tab ||
                      event->key() == Qt::Key_Backtab);
        bool shiftTab = event->modifiers() == Qt::ShiftModifier &&
                         event->key() == Qt::Key_Backtab;
        if (tab || shiftTab) {
            return true;
        }else{
            return false;
        }
    }
    virtual DecisionOnShortcutOverride handleEvent(QObject *receiver, QKeyEvent *event)  override
    {
        Q_UNUSED(event);

        if (qobject_cast<KisQPainterCanvas*>(receiver) != nullptr||
            qobject_cast<KisOpenGLCanvas2*> (receiver) != nullptr) {

            return DecisionOnShortcutOverride::dontOverrideShortcut;
        } else {
            m_nooverride = true;
            return DecisionOnShortcutOverride::askNext;
        }
    }
    virtual DecisionOnShortcutOverride finishedPhysicalKeyPressHandling() override
    {
        if (m_nooverride){
            m_nooverride = false;
            return DecisionOnShortcutOverride::overrideShortcut;
        }
        return DecisionOnShortcutOverride::askNext;
    }
private:
    bool m_nooverride = false;

};


//for some reason I can't just populate constexpr
//pointer array using "new"
LineTextEditingShortcutOverrider overrider0;
SpinboxShortcutOverrider overrider1;
TabShortcutOverrider overrider2;

constexpr ShortcutOverriderBase *allShortcutOverriders[] = {
    &overrider0, &overrider1, &overrider2
};

constexpr int numOfShortcutOverriders =
                    sizeof(allShortcutOverriders)/
                    sizeof(allShortcutOverriders[0]);



} //namespace

struct KisQtWidgetsTweaker::Private
{
public:
    Private(KisQtWidgetsTweaker *parent)
        : q(parent)
    {
    }

    const KisQtWidgetsTweaker *q;

    QBitArray interestedHandlers = QBitArray(numOfShortcutOverriders);
    ShortcutOverriderBase::DecisionOnShortcutOverride decision = ShortcutOverriderBase::DecisionOnShortcutOverride::askNext;
    bool lastKeyPressProcessingComplete = true;

    void newPhysicalKeyPressed(QKeyEvent *event)
    {
        for (int i=0; i < numOfShortcutOverriders; ++i) {
            if (allShortcutOverriders[i]->interestedInEvent(event)) {
                interestedHandlers.setBit(i);
            }else{
                interestedHandlers.clearBit(i);
            }
        }
        decision = ShortcutOverriderBase::DecisionOnShortcutOverride::askNext;
        lastKeyPressProcessingComplete = false;
    }
};

KisQtWidgetsTweaker::KisQtWidgetsTweaker(QObject *parent)
    :QObject(parent)
    , d(new KisQtWidgetsTweaker::Private(this))
{

}

KisQtWidgetsTweaker::~KisQtWidgetsTweaker()
{
    delete d;
}
bool KisQtWidgetsTweaker::eventFilter(QObject *receiver, QEvent *event)
{
    switch(event->type()) {
    case QEvent::ShortcutOverride:{
        //QLineEdit and other widgets lets qt's shortcut system take away it's keyboard events
        //even is it knows them, such as ctrl+backspace
        //if there is application-wide shortcut, assigned to it.
        //The following code fixes it
        //by accepting ShortcutOverride events.

        //if you press key 'a' and then 'b', qt at first call
        //all handlers for 'a' key press event, and only after that
        //for 'b'
        QKeyEvent *key = static_cast<QKeyEvent*>(event);
        if (d->lastKeyPressProcessingComplete) {
            d->newPhysicalKeyPressed(key);
        }
        for(int i = 0; i < numOfShortcutOverriders; ++i) {
            if (d->decision != ShortcutOverriderBase::DecisionOnShortcutOverride::askNext) {
                break;
            }
            if (d->interestedHandlers.at(i)) {
                d->decision = allShortcutOverriders[i]->handleEvent(receiver, key);
            }
        }
        //if nothing said whether shortcutoverride to be accepted
        //last widget that qt will ask will be kismainwindow or docker
        if (qobject_cast<KisMainWindow*>(receiver)!=nullptr||
            receiver->inherits(QDockWidget::staticMetaObject.className())) {
            for (int i = 0; i < numOfShortcutOverriders; ++i) {
                if (d->decision != ShortcutOverriderBase::DecisionOnShortcutOverride::askNext) {
                    break;
                }
                if (d->interestedHandlers.at(i)) {
                    d->decision = allShortcutOverriders[i]->finishedPhysicalKeyPressHandling();
                }
            }

            d->lastKeyPressProcessingComplete = true;
        }
        bool retval = false;
        switch (d->decision) {
        case ShortcutOverriderBase::DecisionOnShortcutOverride::askNext:
            event->ignore();
            retval = false;
            break;
        case ShortcutOverriderBase::DecisionOnShortcutOverride::dontOverrideShortcut:
            event->ignore();
            retval = true;
            break;
        case ShortcutOverriderBase::DecisionOnShortcutOverride::overrideShortcut:
            event->accept();
            //once shortcutoverride accepted, qt stop asking everyone
            //about it and proceed to handling next event
            d->lastKeyPressProcessingComplete = true;
            retval = true;
            break;
        }

        return retval || QObject::eventFilter(receiver, event);

    }break;

        //other event types
    default:
        break;
    }


    //code for tweaking the behavior of other qt elements will go here


    return QObject::eventFilter(receiver, event);
}


KisQtWidgetsTweaker *KisQtWidgetsTweaker::instance()
{
    return kqwt_instance;
}
