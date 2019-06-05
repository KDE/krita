/*
 *  Copyright (c) 2016 Dmitry Kazakov <dimula73@gmail.com>
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
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef QIMAGE_TEST_UTIL_H
#define QIMAGE_TEST_UTIL_H

#ifdef FILES_OUTPUT_DIR

#include <QProcessEnvironment>
#include <QDir>

namespace TestUtil {

inline QString fetchExternalDataFileName(const QString relativeFileName)
{
    static QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    static QString unittestsDataDirPath = "KRITA_UNITTESTS_DATA_DIR";

    QString path;
    if (!env.contains(unittestsDataDirPath)) {
        warnKrita << "Environment variable" << unittestsDataDirPath << "is not set";
        return QString();
    } else {
        path = env.value(unittestsDataDirPath, "");
    }

    QString filename  =
        path +
        QDir::separator() +
        relativeFileName;

    return filename;
}

inline QString fetchDataFileLazy(const QString relativeFileName, bool externalTest = false)
{
    if (externalTest) {
        return fetchExternalDataFileName(relativeFileName);
    } else {
        QString filename  =
            QString(FILES_DATA_DIR) +
            QDir::separator() +
            relativeFileName;

        if (QFileInfo(filename).exists()) {
            return filename;
        }

        filename  =
            QString(FILES_DEFAULT_DATA_DIR) +
            QDir::separator() +
            relativeFileName;

        if (QFileInfo(filename).exists()) {
            return filename;
        }
    }

    return QString();
}

// quint8 arguments are automatically converted into int
inline bool compareChannels(int ch1, int ch2, int fuzzy)
{
    return qAbs(ch1 - ch2) <= fuzzy;
}

inline bool compareQImages(QPoint & pt, const QImage & image1, const QImage & image2, int fuzzy = 0, int fuzzyAlpha = 0, int maxNumFailingPixels = 0, bool showDebug = true)
{
    //     QTime t;
    //     t.start();

    const int w1 = image1.width();
    const int h1 = image1.height();
    const int w2 = image2.width();
    const int h2 = image2.height();
    const int bytesPerLine = image1.bytesPerLine();

    if (w1 != w2 || h1 != h2) {
        pt.setX(-1);
        pt.setY(-1);
        qDebug() << "Images have different sizes" << image1.size() << image2.size();
        return false;
    }

    int numFailingPixels = 0;

    for (int y = 0; y < h1; ++y) {
        const QRgb * const firstLine = reinterpret_cast<const QRgb *>(image2.scanLine(y));
        const QRgb * const secondLine = reinterpret_cast<const QRgb *>(image1.scanLine(y));

        if (memcmp(firstLine, secondLine, bytesPerLine) != 0) {
            for (int x = 0; x < w1; ++x) {
                const QRgb a = firstLine[x];
                const QRgb b = secondLine[x];
                const bool same =
                        compareChannels(qRed(a), qRed(b), fuzzy) &&
                        compareChannels(qGreen(a), qGreen(b), fuzzy) &&
                        compareChannels(qBlue(a), qBlue(b), fuzzy);
                const bool sameAlpha = compareChannels(qAlpha(a), qAlpha(b), fuzzyAlpha);
                const bool bothTransparent = sameAlpha && qAlpha(a)==0;

                if (!bothTransparent && (!same || !sameAlpha)) {
                    pt.setX(x);
                    pt.setY(y);
                    numFailingPixels++;

                    if (showDebug) {
                        qDebug() << " Different at" << pt
                                 << "source" << qRed(a) << qGreen(a) << qBlue(a) << qAlpha(a)
                                 << "dest" << qRed(b) << qGreen(b) << qBlue(b) << qAlpha(b)
                                 << "fuzzy" << fuzzy
                                 << "fuzzyAlpha" << fuzzyAlpha
                                 << "(" << numFailingPixels << "of" << maxNumFailingPixels << "allowed )";
                    }

                    if (numFailingPixels > maxNumFailingPixels) {
                        return false;
                    }
                }
            }
        }
    }
    //     qDebug() << "compareQImages time elapsed:" << t.elapsed();
    //    qDebug() << "Images are identical";
    return true;
}

inline bool checkQImageImpl(bool externalTest,
                            const QImage &srcImage, const QString &testName,
                            const QString &prefix, const QString &name,
                            int fuzzy, int fuzzyAlpha, int maxNumFailingPixels)
{
    QImage image = srcImage.convertToFormat(QImage::Format_ARGB32);

    if (fuzzyAlpha == -1) {
        fuzzyAlpha = fuzzy;
    }

    QString filename(prefix + "_" + name + ".png");
    QString dumpName(prefix + "_" + name + "_expected.png");

    const QString standardPath =
        testName + QDir::separator() +
        prefix + QDir::separator() + filename;

    QString fullPath = fetchDataFileLazy(standardPath, externalTest);

    if (fullPath.isEmpty() || !QFileInfo(fullPath).exists()) {
        // Try without the testname subdirectory
        fullPath = fetchDataFileLazy(prefix + QDir::separator() +
                                     filename,
                                     externalTest);
    }

    if (fullPath.isEmpty() || !QFileInfo(fullPath).exists()) {
        // Try without the prefix subdirectory
        fullPath = fetchDataFileLazy(testName + QDir::separator() +
                                     filename,
                                     externalTest);
    }

    if (!QFileInfo(fullPath).exists()) {
        fullPath = "";
    }

    bool canSkipExternalTest = fullPath.isEmpty() && externalTest;
    QImage ref(fullPath);

    bool valid = true;
    QPoint t;
    if(!compareQImages(t, image, ref, fuzzy, fuzzyAlpha, maxNumFailingPixels)) {
        bool saveStandardResults = true;

        if (canSkipExternalTest) {
            static QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
            static QString writeUnittestsVar = "KRITA_WRITE_UNITTESTS";

            int writeUnittests = env.value(writeUnittestsVar, "0").toInt();
            if (writeUnittests) {
                QString path = fetchExternalDataFileName(standardPath);

                QFileInfo pathInfo(path);
                QDir directory;
                directory.mkpath(pathInfo.path());

                qDebug() << "--- Saving reference image:" << name << path;
                image.save(path);
                saveStandardResults = false;

            } else {
                qDebug() << "--- External image not found. Skipping..." << name;
            }
        } else {
            qDebug() << "--- Wrong image:" << name;
            valid = false;
        }

        if (saveStandardResults) {
            image.save(QString(FILES_OUTPUT_DIR) + QDir::separator() + filename);
            ref.save(QString(FILES_OUTPUT_DIR) + QDir::separator() + dumpName);
        }
    }

    return valid;
}

inline bool checkQImage(const QImage &image, const QString &testName,
                        const QString &prefix, const QString &name,
                        int fuzzy = 0, int fuzzyAlpha = -1, int maxNumFailingPixels = 0)
{
    return checkQImageImpl(false, image, testName,
                           prefix, name,
                           fuzzy, fuzzyAlpha, maxNumFailingPixels);
}

inline bool checkQImageExternal(const QImage &image, const QString &testName,
                                const QString &prefix, const QString &name,
                                int fuzzy = 0, int fuzzyAlpha = -1, int maxNumFailingPixels = 0)
{
    return checkQImageImpl(true, image, testName,
                           prefix, name,
                           fuzzy, fuzzyAlpha, maxNumFailingPixels);
}

}

#endif // FILES_OUTPUT_DIR

#endif // QIMAGE_TEST_UTIL_H

