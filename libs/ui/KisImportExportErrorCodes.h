/*
 *  Copyright (c) 2019 Agata Cacko <cacko.azh@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */
#ifndef KIS_IMPORT_EXPORT_ERROR_CODES_H
#define KIS_IMPORT_EXPORT_ERROR_CODES_H


#include <QFile>
#include <QFileDevice>
#include <QString>
#include <kritaui_export.h>
#include <QDebug>


namespace ImportExport
{

    enum KRITAUI_EXPORT ErrorCodeID
    {
        InternalError, // error that shouldn't happen; only inside ASSERTS

        // Reading
        FileNotExist, // there is no file with that name in that location,
        NoAccessToRead, // Krita has no reading access to the file,
        ErrorWhileReading, // there was an error that occured during reading,
        FileFormatIncorrect, // file format cannot be parsed,
        FormatFeaturesUnsupported, // file format can be parsed, but some features are unsupported,
        FormatColorSpaceUnsupported, // file format can be parsed, but color space of the image is unsupported


        // Writing
        CannotCreateFile, // file cannot be created
        NoAccessToWrite, // Krita has no writing access to the file
        ErrorWhileWriting, // there was an error that occured during writing (can be insufficient memory, too, just we don't know)
        InsufficientMemory, // there is not enough memory left

        // Both
        Cancelled, // cancelled by a user

        // Other
        Failure, // unspecified error

        // OK
        OK, // everything went ok

    };

    struct KRITAUI_EXPORT ComplexErrorCode {
        virtual QString errorMessage() const = 0;
        ComplexErrorCode(QFileDevice::FileError error);
        bool isOk() const;
    protected:
        QString qtErrorMessage() const;
        QFileDevice::FileError m_error;
        virtual ~ComplexErrorCode() {}
    };

    struct KRITAUI_EXPORT ErrorCodeCannotWrite : ComplexErrorCode {

        ErrorCodeCannotWrite(QFileDevice::FileError error);
        QString errorMessage() const;

        ~ErrorCodeCannotWrite() { }
    };

    struct KRITAUI_EXPORT ErrorCodeCannotRead : ComplexErrorCode {

        ErrorCodeCannotRead(QFileDevice::FileError error);
        QString errorMessage() const;

        ~ErrorCodeCannotRead() { }
    };



    struct KRITAUI_EXPORT ErrorCode
    {
    public:
        // required by kis_async_action_feedback
        ErrorCode();

        ErrorCode(ErrorCodeID code);
        ErrorCode(ErrorCodeCannotRead code);
        ErrorCode(ErrorCodeCannotWrite code);

        QString errorMessage() const;
        bool isOk() const;
        bool isCancelled() const;
        bool isInternalError() const;

    private:
        enum ErrorFieldUsed
        {
            None,
            CodeId,
            CannotRead,
            CannotWrite
        };

        ErrorFieldUsed errorFieldUsed;

        ErrorCodeID codeId;
        ErrorCodeCannotRead cannotRead;
        ErrorCodeCannotWrite cannotWrite;
    };





};


KRITAUI_EXPORT QDebug operator<<(QDebug d, const ImportExport::ErrorCode& errorCode);

#endif // KIS_IMPORT_EXPORT_ERROR_CODES_H

