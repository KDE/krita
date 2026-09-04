/*
 *  SPDX-FileCopyrightText: 2026 Luna Lovecraft <ciubix8514@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "recorder_export_preprocessor.h"
#include "recorder_const.h"
#include "recorder_format.h"
#include <QRegularExpression>
#include <algorithm>

struct RecorderExportPreprocessor::Private {
    QDir directory;
    RecorderFormat format;

    Private(QDir directory, RecorderFormat format)
        : directory(directory)
        , format(format)
    {
    }

    ~Private()
    {
    }
};

RecorderExportPreprocessor::RecorderExportPreprocessor(QDir directory, RecorderFormat format)
    : d(new Private(directory, format))
{
}

RecorderExportPreprocessor::~RecorderExportPreprocessor()
{
}

void RecorderExportPreprocessor::updateSettings(QDir directory, RecorderFormat format)
{
    d->directory = directory;
    d->format = format;
}

// Double check that the files are sequential and have no holes, if there are any, fill them in
void RecorderExportPreprocessor::doPreprocessing()
{
    if (!d->directory.exists()) {
        return;
    }

    QStringList fileNames = d->directory.entryList({"*." % RecorderFormatInfo::fileExtension(d->format)});

    QRegularExpression fileRegex = RecorderConst::snapshotFilePatternFor(RecorderFormatInfo::fileExtension(d->format));
    QHash<int, QString> files;
    QVector<int> indecies;

    Q_FOREACH (QString file, fileNames) {
        QRegularExpressionMatch match = fileRegex.match(file);

        if (match.hasMatch()) {
            int index = match.captured(1).toInt();
            files.insert(index, file);
            indecies.push_back(index);
        }
    }

    // It should be sorted, but just in case
    std::sort(indecies.begin(), indecies.end());

    QHash<int, int> holes;
    int prevIndex = 0;

    for (auto it = indecies.begin(); it != indecies.end(); ++it) {
        int curIndex = *it;
        int diff = curIndex - prevIndex;

        if (diff > 1) {
            holes.insert(curIndex, diff - 1);
        }

        prevIndex = curIndex;
    }

    // No holes, so we don't need to do anything
    if (holes.isEmpty()) {
        return;
    }

    int offset = 0;
    for (auto it = indecies.begin(); it != indecies.end(); ++it) {
        int ind = *it;

        if (holes.contains(ind)) {
            offset += holes[ind];
        }

        if (offset == 0) {
            continue;
        }

        QString oldName = files[ind];
        QString newName = QString("%2.%3").arg(QString("%1").arg(ind - offset, 7, 10, QLatin1Char('0')),
                                               RecorderFormatInfo::fileExtension(d->format));

        // Hopefully nothing should go wrong
        d->directory.rename(oldName, newName);
    }
}
