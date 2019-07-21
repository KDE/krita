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


namespace ImportExportCodes
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

};


class KisImportExportErrorCode;

struct KRITAUI_EXPORT KisImportExportComplexError
{
    virtual QString errorMessage() const = 0;
    KisImportExportComplexError(QFileDevice::FileError error);

    friend QDebug operator<<(QDebug d, const KisImportExportErrorCode& errorCode);


protected:
    QString qtErrorMessage() const;
    QFileDevice::FileError m_error;
    virtual ~KisImportExportComplexError() {}
};

struct KRITAUI_EXPORT KisImportExportErrorCannotWrite : KisImportExportComplexError
{

    KisImportExportErrorCannotWrite(QFileDevice::FileError error);
    QString errorMessage() const override;

    ~KisImportExportErrorCannotWrite() { }

    bool operator==(KisImportExportErrorCannotWrite other);

private:
    KisImportExportErrorCannotWrite();

    //friend KisImportExportErrorCode::KisImportExportErrorCode(KisImportExportErrorCannotWrite code);
    friend class KisImportExportErrorCode;

};

struct KRITAUI_EXPORT KisImportExportErrorCannotRead : KisImportExportComplexError
{

    KisImportExportErrorCannotRead(QFileDevice::FileError error);
    QString errorMessage() const override;

    ~KisImportExportErrorCannotRead() { }

    bool operator==(KisImportExportErrorCannotRead other);

private:
    KisImportExportErrorCannotRead();

    //friend KisImportExportErrorCode::KisImportExportErrorCode(KisImportExportErrorCannotRead code);
    friend class KisImportExportErrorCode;

};



class KRITAUI_EXPORT KisImportExportErrorCode
{
public:
    // required by kis_async_action_feedback
    KisImportExportErrorCode();

    KisImportExportErrorCode(ImportExportCodes::ErrorCodeID code);
    KisImportExportErrorCode(KisImportExportErrorCannotRead code);
    KisImportExportErrorCode(KisImportExportErrorCannotWrite code);

    QString errorMessage() const;
    bool isOk() const;
    bool isCancelled() const;
    bool isInternalError() const;

    friend QDebug operator<<(QDebug d, const KisImportExportErrorCode& errorCode);

    bool operator==(KisImportExportErrorCode errorCode);

private:
    enum ErrorFieldUsed
    {
        None,
        CodeId,
        CannotRead,
        CannotWrite
    };

    ErrorFieldUsed errorFieldUsed;

    ImportExportCodes::ErrorCodeID codeId;
    KisImportExportErrorCannotRead cannotRead;
    KisImportExportErrorCannotWrite cannotWrite;
};






KRITAUI_EXPORT QDebug operator<<(QDebug d, const KisImportExportErrorCode& errorCode);

#endif // KIS_IMPORT_EXPORT_ERROR_CODES_H

