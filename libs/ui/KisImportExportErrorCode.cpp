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

#include "KisImportExportErrorCodes.h"
#include <KLocalizedString>
#include <kis_assert.h>



KisImportExportComplexError::KisImportExportComplexError(QFileDevice::FileError error) : m_error(error) { }


QString KisImportExportComplexError::qtErrorMessage() const
{
    // Error descriptions in most cases taken from https://doc.qt.io/qt-5/qfiledevice.html
    QString unspecifiedError = i18n("An unspecified error occurred.");
    switch (m_error) {
        case QFileDevice::FileError::NoError :
            // Returning this file error may mean that something is wrong in our code.
            // Successful operation should return ImportExportCodes::OK instead.
            return i18n("The action has been completed successfully.");
        case QFileDevice::FileError::ReadError :
            return i18n("An error occurred when reading from the file.");
        case QFileDevice::FileError::WriteError :
            return i18n("An error occurred when writing to the file.");
        case QFileDevice::FileError::FatalError :
            return i18n("A fatal error occurred.");
        case QFileDevice::FileError::ResourceError :
            return i18n("Out of resources (e.g. out of memory).");
        case QFileDevice::FileError::OpenError :
            return i18n("The file could not be opened.");
        case QFileDevice::FileError::AbortError :
            return i18n("The operation was aborted.");
        case QFileDevice::FileError::TimeOutError :
            return i18n("A timeout occurred.");
        case QFileDevice::FileError::UnspecifiedError :
            return unspecifiedError;
        case QFileDevice::FileError::RemoveError :
            return i18n("The file could not be removed.");
        case QFileDevice::FileError::RenameError :
            return i18n("The file could not be renamed.");
        case QFileDevice::FileError::PositionError :
            return i18n("The position in the file could not be changed.");
        case QFileDevice::FileError::ResizeError :
            return i18n("The file could not be resized.");
        case QFileDevice::FileError::PermissionsError :
            return i18n("Permission denied. Krita is not allowed to read or write to the file.");
        case QFileDevice::FileError::CopyError :
            return i18n("The file could not be copied.");
    }
    return unspecifiedError;
}


KisImportExportErrorCannotRead::KisImportExportErrorCannotRead(QFileDevice::FileError error) : KisImportExportComplexError(error) {
    KIS_ASSERT_RECOVER_NOOP(error != QFileDevice::NoError);
}

QString KisImportExportErrorCannotRead::errorMessage() const
{
    return i18n("Cannot open file for reading. Reason: %1", qtErrorMessage());
}


KisImportExportErrorCannotWrite::KisImportExportErrorCannotWrite(QFileDevice::FileError error) : KisImportExportComplexError(error) {
    KIS_ASSERT_RECOVER_NOOP(error != QFileDevice::NoError);
}

QString KisImportExportErrorCannotWrite::errorMessage() const
{
    return i18n("Cannot open file for writing. Reason: %1", qtErrorMessage());
}

KisImportExportErrorCode::KisImportExportErrorCode() : errorFieldUsed(None), cannotRead(QFileDevice::FileError()), cannotWrite(QFileDevice::FileError()) { }

KisImportExportErrorCode::KisImportExportErrorCode(ImportExportCodes::ErrorCodeID id) : errorFieldUsed(CodeId), codeId(id),  cannotRead(QFileDevice::FileError()), cannotWrite(QFileDevice::FileError()) { }

KisImportExportErrorCode::KisImportExportErrorCode(KisImportExportErrorCannotRead error) : errorFieldUsed(CannotRead), cannotRead(error), cannotWrite(QFileDevice::FileError()) { }
KisImportExportErrorCode::KisImportExportErrorCode(KisImportExportErrorCannotWrite error) : errorFieldUsed(CannotWrite), cannotRead(QFileDevice::FileError()), cannotWrite(error) { }





bool KisImportExportErrorCode::isOk() const
{
    // if cannotRead or cannotWrite is "NoError", it means that something is wrong in our code
    return errorFieldUsed == CodeId && codeId == ImportExportCodes::OK;
}

bool KisImportExportErrorCode::isCancelled() const
{
    return errorFieldUsed == CodeId && codeId == ImportExportCodes::Cancelled;
}

bool KisImportExportErrorCode::isInternalError() const
{
    return errorFieldUsed == CodeId && codeId == ImportExportCodes::InternalError;
}

QString KisImportExportErrorCode::errorMessage() const
{
    QString internal = i18n("Unexpected error. Please contact developers.");
    if (errorFieldUsed == CannotRead) {
        return cannotRead.errorMessage();
    } else if (errorFieldUsed == CannotWrite) {
        return cannotWrite.errorMessage();
    } else if (errorFieldUsed == CodeId) {
        switch (codeId) {
            // Reading
            case ImportExportCodes::FileNotExist:
                return i18n("The file doesn't exists.");
            case ImportExportCodes::NoAccessToRead:
                return i18n("Permission denied: Krita is not allowed to read the file.");
            case ImportExportCodes::FileFormatIncorrect:
                return i18n("The file format cannot be parsed.");
            case ImportExportCodes::FormatFeaturesUnsupported:
                return i18n("The file format contains unsupported features.");
            case ImportExportCodes::FormatColorSpaceUnsupported:
                return i18n("The file format contains unsupported color space.");

            // Writing
            case ImportExportCodes::CannotCreateFile:
                return i18n("The file cannot be created.");
            case ImportExportCodes::NoAccessToWrite:
                return i18n("Permission denied: Krita is not allowed to write to the file.");
            case ImportExportCodes::InsufficientMemory:
                return i18n("There is not enough memory left to save the file.");


            // Both
            case ImportExportCodes::Cancelled:
                return i18n("The action was cancelled by the user.");

            // Other
            case ImportExportCodes::Failure:
                return i18n("Unknown error.");
            case ImportExportCodes::InternalError:
                return internal;

            // OK
            case ImportExportCodes::OK:
                return i18n("The action has been completed successfully.");
            default:
                return internal;

        }
    }
    return internal; // errorFieldUsed = None
}



QDebug operator<<(QDebug d, const KisImportExportErrorCode& errorCode)
{
    switch(errorCode.errorFieldUsed) {
    case KisImportExportErrorCode::None:
            d << "None of the error fields is in use.";
        break;
    case KisImportExportErrorCode::CannotRead:
            d << "Cannot read: " << errorCode.cannotRead.m_error;
        break;
    case KisImportExportErrorCode::CannotWrite:
        d << "Cannot read: " << errorCode.cannotRead.m_error;
        break;
    case KisImportExportErrorCode::CodeId:
        d << "Error code = " << errorCode.codeId;
    }
    d << " " << errorCode.errorMessage();
    return d;
}


