/*
 *  SPDX-FileCopyrightText: 2007 Boudewijn Rempt <boud@valdyas.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef TEST_UTIL
#define TEST_UTIL

#include <QProcessEnvironment>

#include <simpletest.h>
#include <QList>
#include <QTime>
#include <QDir>

#include <KoResource.h>
#include <KoConfig.h>
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
#include "testing_nodes.h"

#ifndef FILES_DATA_DIR
#define FILES_DATA_DIR "."
#endif

#ifndef FILES_DEFAULT_DATA_DIR
#define FILES_DEFAULT_DATA_DIR "."
#endif

#include "qimage_test_util.h"

#define KIS_COMPARE_RF(expr, ref) \
    if ((expr) != (ref)) { \
        qDebug() << "Compared values are not the same at line" << __LINE__; \
        qDebug() << "    Actual  : " << #expr << "=" << (expr); \
        qDebug() << "    Expected: " << #ref << "=" << (ref); \
        return false; \
    }

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

    return KisNodeSP();
}

inline void dumpNodeStack(KisNodeSP node, QString prefix = QString("\t"))
{
    qDebug() << node->name();
    KisNodeSP child = node->firstChild();

    while (child) {

        if (child->childCount() > 0) {
            dumpNodeStack(child, prefix + "\t");
        } else {
            qDebug() << prefix << child->name();
        }
        child = child->nextSibling();
    }
}

class TestProgressBar : public KoProgressProxy {
public:
    TestProgressBar()
        : m_min(0), m_max(0), m_value(0)
    {}

    int maximum() const override {
        return m_max;
    }
    void setValue(int value) override {
        m_value = value;
    }
    void setRange(int min, int max) override {
        m_min = min;
        m_max = max;
    }
    void setFormat(const QString &format) override {
        m_format = format;
    }

    void setAutoNestedName(const QString &name) override {
        m_autoNestedName = name;
        KoProgressProxy::setAutoNestedName(name);
    }

    int min() { return m_min; }
    int max() { return m_max; }
    int value() { return m_value; }
    QString format() { return m_format; }
    QString autoNestedName() { return m_autoNestedName; }


private:
    int m_min;
    int m_max;
    int m_value;
    QString m_format;
    QString m_autoNestedName;
};

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
    //     qDebug() << "comparePaintDevices time elapsed:" << t.elapsed();
    return true;
}

template <typename channel_type>
inline bool comparePaintDevicesClever(const KisPaintDeviceSP dev1, const KisPaintDeviceSP dev2, channel_type alphaThreshold = 0)
{
    QRect rc1 = dev1->exactBounds();
    QRect rc2 = dev2->exactBounds();

    if (rc1 != rc2) {
        qDebug() << "Devices have different size" << ppVar(rc1) << ppVar(rc2);
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

                qDebug() << "Failed compare paint devices:" << iter1->x() << iter1->y();
                qDebug() << "src:" << p1[0] << p1[1] << p1[2] << p1[3];
                qDebug() << "dst:" << p2[0] << p2[1] << p2[2] << p2[3];
                return false;
            }
        } while (iter1->nextPixel() && iter2->nextPixel());

        iter1->nextRow();
        iter2->nextRow();
    }

    return true;
}

#ifdef FILES_OUTPUT_DIR

struct ReferenceImageChecker
{
    enum StorageType {
        InternalStorage = 0,
        ExternalStorage
    };

    ReferenceImageChecker(const QString &prefix, const QString &testName, StorageType storageType = ExternalStorage)
        : m_storageType(storageType),
          m_prefix(prefix),
          m_testName(testName),
          m_success(true),
          m_maxFailingPixels(100),
          m_fuzzy(1)
        {
        }


    void setMaxFailingPixels(int value) {
        m_maxFailingPixels = value;
    }

    void setFuzzy(int fuzzy){
        m_fuzzy = fuzzy;
    }

    bool testPassed() const {
        return m_success;
    }

    inline bool checkDevice(KisPaintDeviceSP device, KisImageSP image, const QString &caseName) {
        bool result = false;


        if (m_storageType == ExternalStorage) {
            result = checkQImageExternal(device->convertToQImage(0, image->bounds()),
                                         m_testName,
                                         m_prefix,
                                         caseName, m_fuzzy, m_fuzzy, m_maxFailingPixels);
        } else {
            result = checkQImage(device->convertToQImage(0, image->bounds()),
                                 m_testName,
                                 m_prefix,
                                 caseName, m_fuzzy, m_fuzzy, m_maxFailingPixels);
        }

        m_success &= result;
        return result;
    }

    inline bool checkImage(KisImageSP image, const QString &testName) {
        bool result = checkDevice(image->projection(), image, testName);

        m_success &= result;
        return result;
    }

private:
    bool m_storageType;

    QString m_prefix;
    QString m_testName;

    bool m_success;
    int m_maxFailingPixels;
    int m_fuzzy;
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

class TestNode : public DefaultNode
{
    Q_OBJECT
public:
    KisNodeSP clone() const override {
        return KisNodeSP(new TestNode(*this));
    }
};

class TestGraphListener : public KisNodeGraphListener
{
public:

    void aboutToAddANode(KisNode *parent, int index) override {
        KisNodeGraphListener::aboutToAddANode(parent, index);
        beforeInsertRow = true;
    }

    void nodeHasBeenAdded(KisNode *parent, int index) override {
        KisNodeGraphListener::nodeHasBeenAdded(parent, index);
        afterInsertRow = true;
    }

    void aboutToRemoveANode(KisNode *parent, int index) override {
        KisNodeGraphListener::aboutToRemoveANode(parent, index);
        beforeRemoveRow  = true;
    }

    void nodeHasBeenRemoved(KisNode *parent, int index) override {
        KisNodeGraphListener::nodeHasBeenRemoved(parent, index);
        afterRemoveRow = true;
    }

    void aboutToMoveNode(KisNode *parent, int oldIndex, int newIndex) override {
        KisNodeGraphListener::aboutToMoveNode(parent, oldIndex, newIndex);
        beforeMove = true;
    }

    void nodeHasBeenMoved(KisNode *parent, int oldIndex, int newIndex) override {
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

#include <QApplication>
#include <kis_paint_layer.h>
#include "kis_undo_stores.h"
#include "kis_layer_utils.h"

namespace TestUtil {

struct MaskParent
{
    MaskParent(const QRect &_imageRect = QRect(0,0,512,512))
        : imageRect(_imageRect) {
        const KoColorSpace * cs = KoColorSpaceRegistry::instance()->rgb8();
        undoStore = new KisSurrogateUndoStore();
        image = new KisImage(undoStore, imageRect.width(), imageRect.height(), cs, "test image");
        layer = KisPaintLayerSP(new KisPaintLayer(image, "paint1", OPACITY_OPAQUE_U8));
        image->addNode(KisNodeSP(layer.data()));
    }

    void waitForImageAndShapeLayers() {
        qApp->processEvents();
        image->waitForDone();
        KisLayerUtils::forceAllDelayedNodesUpdate(image->root());
        /**
         * Shape updates have two channels of compression, 100ms each.
         * One in KoShapeManager, the other one in KisShapeLayerCanvas.
         * Therefore we should wait for a decent amount of time for all
         * of them to land.
         */
        QTest::qWait(500);
        image->waitForDone();
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
            qDebug() << "Val / Total:" << qreal(m_val) / qreal(m_total);
            qDebug() << "Avg. Val:   " << qreal(m_val) / m_cycles;
            qDebug() << "Avg. Total: " << qreal(m_total) / m_cycles;
            qDebug() << ppVar(m_val) << ppVar(m_total) << ppVar(m_cycles);

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

struct MeasureDistributionStats {
    MeasureDistributionStats(int numBins, const QString &name = QString())
        : m_numBins(numBins),
          m_name(name)
    {
        reset();
    }

    void reset() {
        m_values.clear();
        m_values.resize(m_numBins);
    }

    void addValue(int value) {
        addValue(value, 1);
    }

    void addValue(int value, int increment) {
        KIS_SAFE_ASSERT_RECOVER_RETURN(value >= 0);

        if (value >= m_numBins) {
            m_values[m_numBins - 1] += increment;
        } else {
            m_values[value] += increment;
        }
    }

    void print() {
        qCritical() << "============= Stats ==============";

        if (!m_name.isEmpty()) {
            qCritical() << "Name:" << m_name;
        }

        int total = 0;

        for (int i = 0; i < m_numBins; i++) {
            total += m_values[i];
        }

        for (int i = 0; i < m_numBins; i++) {
            if (!m_values[i]) continue;

            const QString lastMarker = i == m_numBins - 1 ? "> " : "  ";

            const QString line =
                QString("  %1%2: %3 (%4%)")
                    .arg(lastMarker)
                    .arg(i, 3)
                    .arg(m_values[i], 5)
                    .arg(qreal(m_values[i]) / total * 100.0, 7, 'g', 2);

            qCritical() << qPrintable(line);
        }
        qCritical() << "----                          ----";
        qCritical() << qPrintable(QString("Total: %1").arg(total));
        qCritical() << "==================================";
    }

private:
    QVector<int> m_values;
    int m_numBins = 0;
    QString m_name;
};

QStringList getHierarchy(KisNodeSP root, const QString &prefix = "");
bool checkHierarchy(KisNodeSP root, const QStringList &expected);

}

#endif
