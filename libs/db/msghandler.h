/* This file is part of the KDE project
   Copyright (C) 2004-2005 Jarosław Staniek <staniek@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#ifndef KEXIDB_MSGHANDLER_H
#define KEXIDB_MSGHANDLER_H

#include "object.h"
#include <QPointer>
#include <QWidget>

namespace KexiDB
{

/*! A helper class for setting temporary message title for an KexiDB::Object.
 Message title is a text prepended to error or warning messages.
 Use it this way:
 \code
 KexiDB::MessageTitle title(myKexiDBObject, i18n("Terrible error occurred"));
 \endcode
 After leaving current from code block, object's message title will be reverted
 to previous value.
*/
class CALLIGRADB_EXPORT MessageTitle
{
public:
    explicit MessageTitle(KexiDB::Object* o, const QString& msg = QString());
    ~MessageTitle();

protected:
    Object* m_obj;
    QString m_prevMsgTitle;
};

/*! A prototype for Message Handler usable
 for reacting on messages sent by KexiDB::Object object(s).
*/
class CALLIGRADB_EXPORT MessageHandler
{
public:
    enum MessageType { Error, Sorry, Warning };

    /*! Constructs message handler, \a parent is a widget that will be a parent
     for displaying gui elements (e.g. message boxes). Can be 0 for non-gui usage. */
    explicit MessageHandler(QWidget *parent = 0);
    virtual ~MessageHandler();

    /*! This method can be used to block/unblock messages.
     Sometimes you are receiving both lower- and higher-level messages,
     but you do not need to display two message boxes but only one (higher level with details).
     All you need is to call enableMessages(false) before action that can fail
     and restore messages by enableMessages(true) after the action.
     See KexiMainWindow::renameObject() implementation for example. */
    inline void enableMessages(bool enable) {
        m_enableMessages = enable;
    }

    /*! Shows error message with \a title (it is not caption) and details. */
    virtual void showErrorMessage(const QString &title,
                                  const QString &details = QString()) = 0;

    /*! Shows error message with \a msg text. Existing error message from \a obj object
     is also copied, if present. */
    virtual void showErrorMessage(KexiDB::Object *obj, const QString& msg = QString()) = 0;

    /*! Interactively asks a question. For GUI version, KMessageBox class is used.
     See KMessageBox documentation for explanation of the parameters.
     \a defaultResult is returned in case when no message handler is installed.
     \a message should be i18n'd string.
     Value from KMessageBox::ButtonCode enum is returned.
     Reimplement this. This implementation does nothing, just returns \a defaultResult. */
    virtual int askQuestion(const QString& message,
                            KMessageBox::DialogType dlgType, KMessageBox::ButtonCode defaultResult,
                            const KGuiItem &buttonYes = KStandardGuiItem::yes(),
                            const KGuiItem &buttonNo = KStandardGuiItem::no(),
                            const QString &dontShowAskAgainName = QString(),
                            KMessageBox::Options options = KMessageBox::Notify);

protected:
    QPointer<QWidget> m_messageHandlerParentWidget;
    bool m_enableMessages : 1;
};

}

#endif
