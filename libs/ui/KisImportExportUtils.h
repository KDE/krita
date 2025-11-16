/*
 *  SPDX-FileCopyrightText: 2017 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KISIMPORTEXPORTUTILS_H
#define KISIMPORTEXPORTUTILS_H

#include <mutex>

#include <QFlags>
#include <QString>

#include <kritaui_export.h>
#include "KisImportExportErrorCode.h"
#include <KisImageBarrierLock.h>

class KisImportUserFeedbackInterface;


namespace KritaUtils {

enum SaveFlag {
    SaveNone = 0,
    SaveShowWarnings = 0x1,
    SaveIsExporting = 0x2,
    SaveInAutosaveMode = 0x4
};

enum BackgroudSavingStartResult {
    Success = 0,
    Failure = 1,
    AnotherSavingInProgress = 2,
    ImageLockFailure = 3,
    Cancelled = 4
};

Q_DECLARE_FLAGS(SaveFlags, SaveFlag)
Q_DECLARE_OPERATORS_FOR_FLAGS(SaveFlags)

struct ExportFileJob {
    ExportFileJob()
        : flags(SaveNone)
    {
    }

    ExportFileJob(QString _filePath, QByteArray _mimeType, SaveFlags _flags = SaveNone)
        : filePath(_filePath), mimeType(_mimeType), flags(_flags)
    {
    }

    bool isValid() const {
        return !filePath.isEmpty();
    }

    QString filePath;
    QByteArray mimeType;
    SaveFlags flags;
};

/**
 * When the image has a colorspace that is not suitable for displaying,
 * Krita should convert that into something more useful. This tool function
 * asks the user about the desired working color space and converts into
 * it.
 */
KisImportExportErrorCode KRITAUI_EXPORT
workaroundUnsuitableImageColorSpace(KisImageSP image,
                                    KisImportUserFeedbackInterface *feedbackInterface,
                                    KisImageBarrierLock &lock);

}

#endif // KISIMPORTEXPORTUTILS_H
