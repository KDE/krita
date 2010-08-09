/* This file is part of the KOffice libraries
   Copyright (C) 2001 Werner Trobin <trobin@kde.org>
                 2002 Werner Trobin <trobin@kde.org>

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
Boston, MA 02110-1301, USA.
*/

#ifndef __KO_FILTER_H__
#define __KO_FILTER_H__

#include <QtCore/QObject>
#include <QtCore/QMap>
#include <QtCore/QPointer>

#include "komain_export.h"
class QIODevice;
class KoFilterChain;
class KoUpdater;

/**
 * @brief The base class for import and export filters.
 *
 * Derive your filter class from this base class and implement
 * the @ref convert() method. Don't forget to specify the Q_OBJECT
 * macro in your class even if you don't use signals or slots.
 * This is needed as filters are created on the fly.
 * The m_chain member allows access to the @ref KoFilterChain
 * which invokes the filter to query for input/output.
 *
 * @note Take care: The m_chain pointer is invalid while the constructor
 * runs due to the implementation -- @em don't use it in the constructor.
 * After the constructor, when running the @ref convert() method it's
 * guaranteed to be valid, so no need to check against 0.
 * 
 * @note If the code is compiled in debug mode, seting KOFFICE_DEBUG_FILTERS
 * environment variable to any value disables deletion of temporary files while
 * importing/exporting. This is useful for testing purposes.
 *
 * @author Werner Trobin <trobin@kde.org>
 * @todo the class has no constructor and therefore cannot initialize its private class
 */
class KOMAIN_EXPORT KoFilter : public QObject
{
    Q_OBJECT

    friend class KoFilterEntry;  // needed for the filter chain pointer :(
    friend class KoFilterChain;

public:
    /**
     * This enum is used to signal the return state of your filter.
     * Return OK in @ref convert() in case everything worked as expected.
     * Feel free to add some more error conditions @em before the last item
     * if it's needed.
     */
    enum ConversionStatus { OK, StupidError, UsageError, CreationError, FileNotFound,
                            StorageCreationError, BadMimeType, BadConversionGraph,
                            EmbeddedDocError, WrongFormat, NotImplemented,
                            ParsingError, InternalError, UnexpectedEOF,
                            UnexpectedOpcode, UserCancelled, OutOfMemory,
                            PasswordProtected,
                            JustInCaseSomeBrokenCompilerUsesLessThanAByte = 255
                          };

    virtual ~KoFilter();

    /**
     * The filter chain calls this method to perform the actual conversion.
     * The passed mimetypes should be a pair of those you specified in your
     * .desktop file.
     * You @em have to implement this method to make the filter work.
     *
     * @param from The mimetype of the source file/document
     * @param to The mimetype of the destination file/document
     * @return The error status, see the @ref #ConversionStatus enum.
     *         KoFilter::OK means that everything is alright.
     */
    virtual ConversionStatus convert(const QByteArray& from, const QByteArray& to) = 0;

    /**
     * Set the updater to which the filter will report progress.
     * Every emit of the sigProgress signal is reported to the updater.
     */
    void setUpdater(const QPointer<KoUpdater>& updater);

signals:
    /**
     * Emit this signal with a value in the range of 1...100 to have some
     * progress feedback for the user in the statusbar of the application.
     *
     * @param value The actual progress state. Should always remain in
     * the range 1..100.
     */
    void sigProgress(int value);

protected:
    /**
     * This is the constructor your filter has to call, obviously.
     */
    KoFilter(QObject *parent = 0);

    /**
     * Use this pointer to access all information about input/output
     * during the conversion. @em Don't use it in the constructor -
     * it's invalid while constructing the object!
     */
    KoFilterChain *m_chain;

private:
    KoFilter(const KoFilter& rhs);
    KoFilter& operator=(const KoFilter& rhs);

    class Private;
    Private *const d;

private slots:
    void slotProgress(int value);
};

#endif
