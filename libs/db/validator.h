/* This file is part of the KDE project
   Copyright (C) 2004-2013 Jarosław Staniek <staniek@kde.org>

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

#ifndef KEXIDB_VALIDATOR_H
#define KEXIDB_VALIDATOR_H

#include "calligradb_export.h"

#include <QValidator>
#include <QVariant>

namespace KexiDB
{

//! @short A validator extending QValidator with offline-checking for value's validity
/*!
 The offline-checking for value's validity is provided by \ref Validator::check() method.
 The validator groups two purposes into one container:
 - string validator for line editors (online checking, "on typing");
 - offline-checking for QVariant values, reimplementing validate().

 It also offers error and warning messages for check() method.
 You may need to reimplement:
 -  QValidator::State IdentifierValidator::validate( QString& input, int& pos ) const;
 -  Result check(const QString &valueName, QVariant v, QString &message, QString &details);
 */
class CALLIGRADB_EXPORT Validator : public QValidator
{
public:
    enum Result { Error = 0, Ok = 1, Warning = 2 };

    Validator(QObject * parent = 0);

    virtual ~Validator();

    /*! Sets accepting empty values on (true) or off (false).
     By default the validator does not accepts empty values. */
    void setAcceptsEmptyValue(bool set);

    /*! \return true if the validator accepts empty values
      @see setAcceptsEmptyValue() */
    bool acceptsEmptyValue() const;

    /*! Checks if value \a v is ok and returns one of \a Result value:
     - \a Error is returned on error;
     - \a Ok on success;
     - \a Warning if there is something to warn about.
     In any case except \a Ok, i18n'ed message will be set in \a message
     and (optionally) datails are set in \a details, e.g. for use in a message box.
     \a valueName can be used to contruct \a message as well, for example:
     "[valueName] is not a valid login name".
     Depending on acceptsEmptyValue(), immediately accepts empty values or not. */
    Result check(const QString &valueName, const QVariant& v, QString &message,
                 QString &details);

    /*! This implementation always returns value QValidator::Acceptable. */
    virtual QValidator::State validate(QString & input, int & pos) const;

    //! A generic error/warning message "... value has to be entered."
    static const QString msgColumnNotEmpty();

    //! Adds a child validator \a v
    void addChildValidator(Validator* v);

    /* @internal
       Used by check(), for reimplementation, by default returns \a Error.*/
    virtual Result internalCheck(const QString &valueName, const QVariant& v,
                                 QString &message, QString &details);

private:
    class Private;
    Private* const d;
};
}

#endif
