/* This file is part of the KDE project
   Copyright (C) 2003-2005 Jarosław Staniek <staniek@kde.org>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#ifndef KEXIDB_OBJECT_H
#define KEXIDB_OBJECT_H

#include "error.h"
#include <kmessagebox.h>
#include <kstandardguiitem.h>
#include <QString>

namespace KexiDB
{

class MessageHandler;

/*! Prototype of KexiDB object, handles result of last operation.
*/
class CALLIGRADB_EXPORT Object
{
public:
    /*! \return true if there was error during last operation on the object. */
    bool error() const {
        return m_hasError;
    }

    /*! \return (localized) error message if there was error during last operation on the object,
      else: 0. */
    const QString& errorMsg() const {
        return m_errMsg;
    }

    /*! \return error number of if there was error during last operation on the object,
      else: 0. */
    int errorNum() const {
        return m_errno;
    }

    //! \return previous server result number, for error displaying purposes.
    int previousServerResult() const {
        return m_previousServerResultNum;
    }

    QString previousServerResultName() const {
        return m_previousServerResultName;
    }

    /*! Sends errorMsg() to debug output. */
    void debugError();

    /*! Clears error flag.
     Also calls drv_clearServerResult().
     You can reimplement this method in subclasses to clear even more members,
     but remember to also call Object::clearError(). */
    virtual void clearError();

    /*! KexiDB library offers detailed error numbers using errorNum()
     and detailed error i18n'd messages using errorMsg() -
     this information is not engine-dependent (almost).
     Use this in your application to give users more information on what's up.

     This method returns (non-i18n'd !) engine-specific error message,
     if there was any error during last server-side operation,
     otherwise null string.
     Reimplement this for your driver
     - default implementation just returns null string.
     \sa serverErrorMsg()
    */
    virtual QString serverErrorMsg();

    /*! \return engine-specific last server-side operation result number.
     Use this in your application to give users more information on what's up.

     Reimplement this for your driver - default implementation just returns 0.
     Note that this result value is not the same as the one returned
     by errorNum() (Object::m_errno member)
     \sa serverErrorMsg(), drv_clearServerResult()
    */
    virtual int serverResult();

    /*! \return engine-specific last server-side operation result name,
     (name for serverResult()).
     Use this in your application to give users more information on what's up.

     Reimplement this for your driver - default implementation
     just returns null string.
     Note that this result name is not the same as the error message returned
     by serverErorMsg() or erorMsg()
     \sa serverErrorMsg(), drv_clearServerResult()
    */
    virtual QString serverResultName();

    /*! \return message title that sometimes is provided and prepended
     to the main warning/error message. Used by MessageHandler. */
    QString msgTitle() const {
        return m_msgTitle;
    }

    /*! \return sql string of actually executed SQL statement,
     usually using drv_executeSQL(). If there was error during executing SQL statement,
     before, that string is returned instead. */
    const QString recentSQLString() const {
        return m_errorSql.isEmpty() ? m_sql : m_errorSql;
    }

protected:
    /* Constructs a new object.
     \a handler can be provided to receive error messages. */
    Object(MessageHandler* handler = 0);

    virtual ~Object();

    /*! Sets the (localized) error code to \a code and message to \a msg.
     You have to set at least nonzero error code \a code,
     although it is also adviced to set descriptive message \a msg.
     Eventually, if you omit all parameters, ERR_OTHER code will be set
     and default message for this will be set.
     Use this in KexiDB::Object subclasses to inform the world about your
     object's state. */
    virtual void setError(int code = ERR_OTHER, const QString &msg = QString());

    /*! \overload void setError(int code,  const QString &msg = QString() )
      Sets error code to ERR_OTHER. Use this if you don't care about
      setting error code.
    */
    virtual void setError(const QString &msg);

    /*! \overload void setError(const QString &msg)
      Also sets \a title. */
    virtual void setError(const QString &title, const QString &msg);

    /*! Copies the (localized) error message and code from other KexiDB::Object. */
    void setError(KexiDB::Object *obj, const QString& prependMessage = QString());

    /*! Copies the (localized) error message and code from other KexiDB::Object
     with custom error \a code. */
    virtual void setError(KexiDB::Object *obj, int code,
                          const QString& prependMessage = QString());

    /*! Interactively asks a question. Console or GUI can be used for this,
     depending on installed message handler. For GUI version, KMessageBox class is used.
     See KexiDB::MessageHandler::askQuestion() for details. */
    virtual int askQuestion(const QString& message,
                            KMessageBox::DialogType dlgType, KMessageBox::ButtonCode defaultResult,
                            const KGuiItem &buttonYes = KStandardGuiItem::yes(),
                            const KGuiItem &buttonNo = KStandardGuiItem::no(),
                            const QString &dontShowAskAgainName = QString(),
                            KMessageBox::Options options = KMessageBox::Notify,
                            MessageHandler* msgHandler = 0);

    /*! Clears number of last server operation's result stored
     as a single integer. Formally, this integer should be set to value
     that means "NO ERRORS" or "OK". This method is called by clearError().
     For reimplementation. By default does nothing.
     \sa serverErrorMsg()
    */
    virtual void drv_clearServerResult() {}

    //! used to store of actually executed SQL statement
    QString m_sql, m_errorSql;
    int m_serverResult;
    QString m_serverResultName, m_serverErrorMsg;
    QString m_errMsg;

private:
    int m_errno;
    bool m_hasError;

    //! previous server result number, for error displaying purposes.
    int m_previousServerResultNum, m_previousServerResultNum2;
    //! previous server result name, for error displaying purposes.
    QString m_previousServerResultName, m_previousServerResultName2;

    QString m_msgTitle;
    MessageHandler *m_msgHandler;

    class Private;
    Private * const d; //!< for future extensions

    friend class MessageTitle;
};

} //namespace KexiDB

#endif
