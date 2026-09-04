/*
 *  SPDX-FileCopyrightText: 2026 Luna Lovecraft <ciubix8514@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef KRITA_RECORDER_EXPORT_PREPROCESSOR_H
#define KRITA_RECORDER_EXPORT_PREPROCESSOR_H

#include "recorder_format.h"
#include <QDir>
#include <QScopedPointer>

class RecorderExportPreprocessor
{
public:
    RecorderExportPreprocessor(QDir directory, RecorderFormat format);
    ~RecorderExportPreprocessor();

    void updateSettings(QDir directory, RecorderFormat format);

    void doPreprocessing();

private:
    struct Private;
    QScopedPointer<Private> d;
};

#endif
