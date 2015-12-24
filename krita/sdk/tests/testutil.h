/*
 *  Copyright (c) 2007 Boudewijn Rempt <boud@valdyas.org>
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

#ifndef TEST_UTIL
#define TEST_UTIL

#include <QProcessEnvironment>

#include <QList>
#include <QTime>
#include <QDir>

#include <KoColorSpace.h>
#include <KoColorSpaceRegistry.h>
#include <KoColorProfile.h>
#include <KoProgressProxy.h>
#include <kis_paint_device.h>
#include <kis_node.h>
#include <kis_undo_adapter.h>
#include "kis_node_graph_listener.h"
#include "kis_iterator_ng.h"
#include "kis_image.h"


#ifndef FILES_DATA_DIR
#define FILES_DATA_DIR "."
#endif

#ifndef FILES_DEFAULT_DATA_DIR
#define FILES_DEFAULT_DATA_DIR "."
#endif


/**
 * Routines that are useful for writing efficient tests
 */

namespace TestUtil
{

inline KisNodeSP findNode(KisNodeSP root, const QString &name) {
    if(root->name() == name) return root;

    KisNodeSP child = root->firstChild();
    while (child) {
        if((root = findNode(child, name))) return root;
        child = child->nextSibling();
    }

    return 0;
}


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

inline void dumpNodeStack(KisNodeSP node, QString prefix = QString("\t"))
{
    dbgKrita << node->name();
    KisNodeSP child = node->firstChild();

    while (child) {

        if (child->childCount() > 0) {
            dumpNodeStack(child, prefix + "\t");
        } else {
            dbgKrita << prefix << child->name();
        }
        child = child->nextSibling();
    }
}

class TestProgressBar : public KoProgressProxy {
public:
    TestProgressBar()
        : m_min(0), m_max(0), m_value(0)
    {}

    int maximum() const {
        return m_max;
    }
    void setValue(int value) {
        m_value = value;
    }
    void setRange(int min, int max) {
        m_min = min;
        m_max = max;
    }
    void setFormat(const QString &format) {
        m_format = format;
    }

    int min() { return m_min; }
    int max() { return m_max; }
    int value() { return m_value; }
    QString format() { return m_format; }

private:
    int m_min;
    int m_max;
    int m_value;
    QString m_format;
};


inline bool compareQImages(QPoint & pt, const QImage & image1, const QImage & image2, int fuzzy = 0, int fuzzyAlpha = 0, int maxNumFailingPixels = 0)
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
        dbgKrita << "Images have different sizes" << image1.size() << image2.size();
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
                const bool same = qAbs(qRed(a) - qRed(b)) <= fuzzy
                                  && qAbs(qGreen(a) - qGreen(b)) <= fuzzy
                                  && qAbs(qBlue(a) - qBlue(b)) <= fuzzy;
                const bool sameAlpha = qAlpha(a) - qAlpha(b) <= fuzzyAlpha;
                const bool bothTransparent = sameAlpha && qAlpha(a)==0;

                if (!bothTransparent && (!same || !sameAlpha)) {
                    pt.setX(x);
                    pt.setY(y);
                    numFailingPixels++;

                    dbgKrita << " Different at" << pt
                             << "source" << qRed(a) << qGreen(a) << qBlue(a) << qAlpha(a)
                             << "dest" << qRed(b) << qGreen(b) << qBlue(b) << qAlpha(b)
                             << "fuzzy" << fuzzy
                             << "fuzzyAlpha" << fuzzyAlpha
                             << "(" << numFailingPixels << "of" << maxNumFailingPixels << "allowed )";


                    if (numFailingPixels > maxNumFailingPixels) {
                        return false;
                    }
                }
            }
        }
    }
    //     dbgKrita << "compareQImages time elapsed:" << t.elapsed();
    //    dbgKrita << "Images are identical";
    return true;
}

inline bool comparePaintDevices(QPoint & pt, const KisPaintDeviceSP dev1, const KisPaintDeviceSP dev2)
{
    //     QTime t;
    //     t.start();

    QRect rc1 = dev1->exactBounds();
    QRect rc2 = dev2->exactBounds();

    if (rc1 != rc2) {
        pt.setX(-1);
        pt.setY(-1);
    }

    KisHLineConstIteratorSP iter1 = dev1->createHLineConstIteratorNG(0, 0, rc1.width());
    KisHLineConstIteratorSP iter2 = dev2->createHLineConstIteratorNG(0, 0, rc1.width());

    int pixelSize = dev1->pixelSize();

    for (int y = 0; y < rc1.height(); ++y) {

        do {
            if (memcmp(iter1->oldRawData(), iter2->oldRawData(), pixelSize) != 0)
                return false;
        } while (iter1->nextPixel() && iter2->nextPixel());

        iter1->nextRow();
        iter2->nextRow();
    }
    //     dbgKrita << "comparePaintDevices time elapsed:" << t.elapsed();
    return true;
}

template <typename channel_type>
inline bool comparePaintDevicesClever(const KisPaintDeviceSP dev1, const KisPaintDeviceSP dev2, channel_type alphaThreshold = 0)
{
    QRect rc1 = dev1->exactBounds();
    QRect rc2 = dev2->exactBounds();

    if (rc1 != rc2) {
        dbgKrita << "Devices have different size" << ppVar(rc1) << ppVar(rc2);
        return false;
    }

    KisHLineConstIteratorSP iter1 = dev1->createHLineConstIteratorNG(0, 0, rc1.width());
    KisHLineConstIteratorSP iter2 = dev2->createHLineConstIteratorNG(0, 0, rc1.width());

    int pixelSize = dev1->pixelSize();

    for (int y = 0; y < rc1.height(); ++y) {

        do {
            if (memcmp(iter1->oldRawData(), iter2->oldRawData(), pixelSize) != 0) {
                const channel_type* p1 = reinterpret_cast<const channel_type*>(iter1->oldRawData());
                const channel_type* p2 = reinterpret_cast<const channel_type*>(iter2->oldRawData());

                if (p1[3] < alphaThreshold && p2[3] < alphaThreshold) continue;

                dbgKrita << "Failed compare paint devices:" << iter1->x() << iter1->y();
                dbgKrita << "src:" << p1[0] << p1[1] << p1[2] << p1[3];
                dbgKrita << "dst:" << p2[0] << p2[1] << p2[2] << p2[3];
                return false;
            }
        } while (iter1->nextPixel() && iter2->nextPixel());

        iter1->nextRow();
        iter2->nextRow();
    }

    return true;
}

#ifdef FILES_OUTPUT_DIR

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

                dbgKrita << "--- Saving reference image:" << name << path;
                image.save(path);
                saveStandardResults = false;

            } else {
                dbgKrita << "--- External image not found. Skipping..." << name;
            }
        } else {
            dbgKrita << "--- Wrong image:" << name;
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

struct ExternalImageChecker
{
    ExternalImageChecker(const QString &prefix, const QString &testName)
        : m_prefix(prefix),
          m_testName(testName),
          m_success(true),
          m_maxFailingPixels(100)
        {
        }


    void setMaxFailingPixels(int value) {
        m_maxFailingPixels = value;
    }

    bool testPassed() const {
        return m_success;
    }

    inline bool checkDevice(KisPaintDeviceSP device, KisImageSP image, const QString &caseName) {
        bool result =
            checkQImageExternal(device->convertToQImage(0, image->bounds()),
                                m_testName,
                                m_prefix,
                                caseName, 1, 1, m_maxFailingPixels);

        m_success &= result;
        return result;
    }

    inline bool checkImage(KisImageSP image, const QString &testName) {
        bool result = checkDevice(image->projection(), image, testName);

        m_success &= result;
        return result;
    }

private:
    QString m_prefix;
    QString m_testName;

    bool m_success;
    int m_maxFailingPixels;
};


#endif

inline quint8 alphaDevicePixel(KisPaintDeviceSP dev, qint32 x, qint32 y)
{
    KisHLineConstIteratorSP iter = dev->createHLineConstIteratorNG(x, y, 1);
    const quint8 *pix = iter->oldRawData();
    return *pix;
}

inline void alphaDeviceSetPixel(KisPaintDeviceSP dev, qint32 x, qint32 y, quint8 s)
{
    KisHLineIteratorSP iter = dev->createHLineIteratorNG(x, y, 1);
    quint8 *pix = iter->rawData();
    *pix = s;
}

inline bool checkAlphaDeviceFilledWithPixel(KisPaintDeviceSP dev, const QRect &rc, quint8 expected)
{
    KisHLineIteratorSP it = dev->createHLineIteratorNG(rc.x(), rc.y(), rc.width());

    for (int y = rc.y(); y < rc.y() + rc.height(); y++) {
        for (int x = rc.x(); x < rc.x() + rc.width(); x++) {

            if(*((quint8*)it->rawData()) != expected) {
                errKrita << "At point:" << x << y;
                errKrita << "Expected pixel:" << expected;
                errKrita << "Actual pixel:  " << *((quint8*)it->rawData());
                return false;
            }
            it->nextPixel();
        }
        it->nextRow();
    }
    return true;
}

class TestNode : public KisNode
{
    Q_OBJECT
public:
    KisNodeSP clone() const;
    bool allowAsChild(KisNodeSP) const;
    const KoColorSpace * colorSpace() const;
    const KoCompositeOp * compositeOp() const;
};

class TestGraphListener : public KisNodeGraphListener
{
public:

    virtual void aboutToAddANode(KisNode *parent, int index) {
        KisNodeGraphListener::aboutToAddANode(parent, index);
        beforeInsertRow = true;
    }

    virtual void nodeHasBeenAdded(KisNode *parent, int index) {
        KisNodeGraphListener::nodeHasBeenAdded(parent, index);
        afterInsertRow = true;
    }

    virtual void aboutToRemoveANode(KisNode *parent, int index) {
        KisNodeGraphListener::aboutToRemoveANode(parent, index);
        beforeRemoveRow  = true;
    }

    virtual void nodeHasBeenRemoved(KisNode *parent, int index) {
        KisNodeGraphListener::nodeHasBeenRemoved(parent, index);
        afterRemoveRow = true;
    }

    virtual void aboutToMoveNode(KisNode *parent, int oldIndex, int newIndex) {
        KisNodeGraphListener::aboutToMoveNode(parent, oldIndex, newIndex);
        beforeMove = true;
    }

    virtual void nodeHasBeenMoved(KisNode *parent, int oldIndex, int newIndex) {
        KisNodeGraphListener::nodeHasBeenMoved(parent, oldIndex, newIndex);
        afterMove = true;
    }

    bool beforeInsertRow;
    bool afterInsertRow;
    bool beforeRemoveRow;
    bool afterRemoveRow;
    bool beforeMove;
    bool afterMove;

    void resetBools() {
        beforeRemoveRow = false;
        afterRemoveRow = false;
        beforeInsertRow = false;
        afterInsertRow = false;
        beforeMove = false;
        afterMove = false;
    }
};

}

#include <kis_paint_layer.h>
#include <kis_image.h>
#include "kis_undo_stores.h"

namespace TestUtil {

struct MaskParent
{
    MaskParent(const QRect &_imageRect = QRect(0,0,512,512))
        : imageRect(_imageRect) {
        const KoColorSpace * cs = KoColorSpaceRegistry::instance()->rgb8();
        undoStore = new KisSurrogateUndoStore();
        image = new KisImage(undoStore, imageRect.width(), imageRect.height(), cs, "test image");
        layer = new KisPaintLayer(image, "paint1", OPACITY_OPAQUE_U8);
        image->addNode(layer);
    }

    KisSurrogateUndoStore *undoStore;
    const QRect imageRect;
    KisImageSP image;
    KisPaintLayerSP layer;
};

}

namespace TestUtil {

class MeasureAvgPortion
{
public:
    MeasureAvgPortion(int period)
        : m_period(period),
        m_val(0),
        m_total(0),
        m_cycles(0)
    {
    }

    ~MeasureAvgPortion() {
        printValues(true);
    }

    void addVal(int x) {
        m_val += x;
    }

    void addTotal(int x) {
        m_total += x;
        m_cycles++;
        printValues();
    }

private:
    void printValues(bool force = false) {
        if (m_cycles > m_period || force) {
            dbgKrita << "Val / Total:" << qreal(m_val) / qreal(m_total);
            dbgKrita << "Avg. Val:   " << qreal(m_val) / m_cycles;
            dbgKrita << "Avg. Total: " << qreal(m_total) / m_cycles;
            dbgKrita << ppVar(m_val) << ppVar(m_total) << ppVar(m_cycles);

            m_val = 0;
            m_total = 0;
            m_cycles = 0;
        }
    }

private:
    int m_period;
    qint64 m_val;
    qint64 m_total;
    qint64 m_cycles;
};

}

#endif
