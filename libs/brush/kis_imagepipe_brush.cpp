/*
 *  Copyright (c) 2004 Boudewijn Rempt <boud@valdyas.org>
 *  Copyright (c) 2005 Bart Coppens <kde@bartcoppens.be>
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
#include "kis_imagepipe_brush.h"
#include "kis_pipebrush_parasite.h"
#include "kis_brushes_pipe.h"


class KisImageBrushesPipe : public KisBrushesPipe<KisGbrBrush>
{
public:
    KisImageBrushesPipe()
        : m_isInitialized(false)
    {
    }


    /*
       pre and post are split because:

    21:12:20 < dmitryK> boud: i guess it was somehow related to the fact that the maskWidth/maskHeight should
                        correspond to the size of the mask returned by paintDevice()
    21:13:33 < dmitryK> boud: the random stuff is called once per brush->paintDevice() call, after the device is
                        returned to the paint op, that is "preparing the randomness for the next call"
    21:14:16 < dmitryK> boud: and brushesPipe->currentBrush() always returning the same brush for any particular
                        paintInfo.
    */
protected:
    static int selectPre(KisParasite::SelectionMode mode,
                         int index, int rank,
                         const KisPaintInformation& info) {

        qreal angle;

        switch (mode) {
        case KisParasite::Constant:
        case KisParasite::Incremental:
        case KisParasite::Random:
            break;
        case KisParasite::Pressure:
            index = static_cast<int>(info.pressure() * (rank - 1) + 0.5);
            break;
        case KisParasite::Angular:
            // + m_d->PI_2 to be compatible with the gimp
            angle = info.drawingAngle() + M_PI_2;
            angle = normalizeAngle(angle);

            index = static_cast<int>(angle / (2.0 * M_PI) * rank);
            break;
        case KisParasite::TiltX:
            index = qRound(info.xTilt() / 2.0 * rank) + rank / 2;
            break;
        case KisParasite::TiltY:
            index = qRound(info.yTilt() / 2.0 * rank) + rank / 2;
            break;
        default:
            warnImage << "Parasite" << mode << "is not implemented";
            index = 0;
        }

        return index;
    }

    static int selectPost(KisParasite::SelectionMode mode,
                          int index, int rank,
                          const KisPaintInformation& info,
                          int seqNo) {

        switch (mode) {
        case KisParasite::Constant: break;
        case KisParasite::Incremental:
            index = (seqNo >= 0 ? seqNo : (index + 1)) % rank;
            break;
        case KisParasite::Random:
            index = info.randomSource()->generate(0, rank);
            break;
        case KisParasite::Pressure:
        case KisParasite::Angular:
            break;
        case KisParasite::TiltX:
        case KisParasite::TiltY:
            break;
        default:
            warnImage << "Parasite" << mode << "is not implemented";
            index = 0;
        }

        return index;
    }

    int chooseNextBrush(const KisPaintInformation& info) override {
        quint32 brushIndex = 0;

        if (!m_isInitialized) {
            /**
             * Reset all the indexes to the initial values and do the
             * generation based on parameters.
             */
            for (int i = 0; i < m_parasite.dim; i++) {
                m_parasite.index[i] = 0;
            }
            updateBrushIndexes(info, 0);
            m_isInitialized = true;
        }

        for (int i = 0; i < m_parasite.dim; i++) {
            int index = selectPre(m_parasite.selection[i],
                                  m_parasite.index[i],
                                  m_parasite.rank[i], info);

            brushIndex += m_parasite.brushesCount[i] * index;
        }
        brushIndex %= m_brushes.size();
        return brushIndex;
    }

    void updateBrushIndexes(const KisPaintInformation& info, int seqNo) override {
        for (int i = 0; i < m_parasite.dim; i++) {
            m_parasite.index[i] = selectPost(m_parasite.selection[i],
                                             m_parasite.index[i],
                                             m_parasite.rank[i],
                                             info,
                                             seqNo);
        }
    }

public:
    using KisBrushesPipe<KisGbrBrush>::addBrush;

    void setParasite(const KisPipeBrushParasite& parasite) {
        m_parasite = parasite;
    }

    const KisPipeBrushParasite& parasite() const {
        return m_parasite;
    }

    void setUseColorAsMask(bool useColorAsMask) {
        Q_FOREACH (KisGbrBrushSP brush, m_brushes) {
            brush->setUseColorAsMask(useColorAsMask);
        }
    }

    void makeMaskImage() {
        Q_FOREACH (KisGbrBrushSP brush, m_brushes) {
            brush->makeMaskImage();
        }
    }

    bool saveToDevice(QIODevice* dev) const {
        Q_FOREACH (KisGbrBrushSP brush, m_brushes) {
            if (!brush->saveToDevice(dev)) {
                return false;
            }
        }
        return true;
    }

    void notifyStrokeStarted() override {
        m_isInitialized = false;
    }

private:
    KisPipeBrushParasite m_parasite;
    bool m_isInitialized;
};


struct KisImagePipeBrush::Private {
public:
    KisImageBrushesPipe brushesPipe;
};

KisImagePipeBrush::KisImagePipeBrush(const QString& filename)
    : KisGbrBrush(filename)
    , m_d(new Private())
{
}

KisImagePipeBrush::KisImagePipeBrush(const QString& name, int w, int h,
                                     QVector< QVector<KisPaintDevice*> > devices,
                                     QVector<KisParasite::SelectionMode > modes)
    : KisGbrBrush(QString())
    , m_d(new Private())
{
    Q_ASSERT(devices.count() == modes.count());
    Q_ASSERT(devices.count() > 0);
    Q_ASSERT(devices.count() < 2); // XXX Multidimensionals not supported yet, change to MaxDim!

    setName(name);

    KisPipeBrushParasite parasite;

    parasite.dim = devices.count();
    // XXX Change for multidim! :
    parasite.ncells = devices.at(0).count();
    parasite.rank[0] = parasite.ncells; // ### This can masquerade some bugs, be careful here in the future
    parasite.selection[0] = modes.at(0);


    // XXX needsmovement!

    parasite.setBrushesCount();

    setParasite(parasite);
    setDevices(devices, w, h);
    setBrushTipImage(m_d->brushesPipe.firstBrush()->brushTipImage());
}

KisImagePipeBrush::KisImagePipeBrush(const KisImagePipeBrush& rhs)
    : KisGbrBrush(rhs),
      m_d(new Private(*rhs.m_d))
{
}


KisImagePipeBrush::~KisImagePipeBrush()
{
    delete m_d;
}

bool KisImagePipeBrush::load()
{
    QFile file(filename());
    file.open(QIODevice::ReadOnly);
    bool res = loadFromDevice(&file);
    file.close();
    return res;
}

bool KisImagePipeBrush::loadFromDevice(QIODevice *dev)
{
    QByteArray data = dev->readAll();
    return initFromData(data);
}

bool KisImagePipeBrush::initFromData(const QByteArray &data)
{
    if (data.size() == 0) return false;
    // XXX: this doesn't correctly load the image pipe brushes yet.

    // XXX: This stuff is in utf-8, too.
    // The first line contains the name -- this means we look until we arrive at the first newline
    QByteArray line1;

    qint32 i = 0;

    while (data[i] != '\n' && i < data.size()) {
        line1.append(data[i]);
        i++;
    }
    setName(QString::fromUtf8(line1, line1.size()));

    i++; // Skip past the first newline

    // The second line contains the number of brushes, separated by a space from the parasite

    // XXX: This stuff is in utf-8, too.
    QByteArray line2;
    while (data[i] != '\n' && i < data.size()) {
        line2.append(data[i]);
        i++;
    }

    QString paramline = QString::fromUtf8(line2, line2.size());
    qint32 numOfBrushes = paramline.left(paramline.indexOf(' ')).toUInt();
    QString parasiteString = paramline.mid(paramline.indexOf(' ') + 1);

    KisPipeBrushParasite parasite = KisPipeBrushParasite(parasiteString);
    parasite.sanitize();

    parasiteSelectionString = parasite.selectionMode; // selection mode to return to UI

    m_d->brushesPipe.setParasite(parasite);
    i++; // Skip past the second newline

    for (int brushIndex = 0;
            brushIndex < numOfBrushes && i < data.size(); brushIndex++) {

        KisGbrBrushSP brush = KisGbrBrushSP(new KisGbrBrush(name() + '_' + QString().setNum(brushIndex),
                                             data,
                                             i));

        m_d->brushesPipe.addBrush(brush);
    }

    if (numOfBrushes > 0) {
        setValid(true);
        setSpacing(m_d->brushesPipe.lastBrush()->spacing());
        setWidth(m_d->brushesPipe.firstBrush()->width());
        setHeight(m_d->brushesPipe.firstBrush()->height());
        setBrushTipImage(m_d->brushesPipe.firstBrush()->brushTipImage());
    }

    return true;
}

bool KisImagePipeBrush::save()
{
    QFile file(filename());
    file.open(QIODevice::WriteOnly | QIODevice::Truncate);
    bool ok = saveToDevice(&file);
    file.close();
    return ok;
}

bool KisImagePipeBrush::saveToDevice(QIODevice* dev) const
{
    QByteArray utf8Name = name().toUtf8(); // Names in v2 brushes are in UTF-8
    char const* name = utf8Name.data();
    int len = qstrlen(name);

    if (m_d->brushesPipe.parasite().dim != 1) {
        warnImage << "Save to file for pipe brushes with dim != not yet supported!";
        return false;
    }

    // Save this pipe brush: first the header, and then all individual brushes consecutively
    // XXX: this needs some care for when we have > 1 dimension)

    // Gimp Pipe Brush header format: Name\n<number of brushes> <parasite>\n

    // The name\n
    if (dev->write(name, len) == -1)
        return false;

    if (!dev->putChar('\n'))
        return false;

    // Write the parasite (also writes number of brushes)
    if (!m_d->brushesPipe.parasite().saveToDevice(dev))
        return false;

    if (!dev->putChar('\n'))
        return false;

    KoResource::saveToDevice(dev);

    // <gbr brushes>
    return m_d->brushesPipe.saveToDevice(dev);
}

void KisImagePipeBrush::notifyStrokeStarted()
{
    m_d->brushesPipe.notifyStrokeStarted();
}

void KisImagePipeBrush::notifyCachedDabPainted(const KisPaintInformation& info)
{
    m_d->brushesPipe.notifyCachedDabPainted(info);
}

void KisImagePipeBrush::prepareForSeqNo(const KisPaintInformation &info, int seqNo)
{
    m_d->brushesPipe.prepareForSeqNo(info, seqNo);
}

void KisImagePipeBrush::generateMaskAndApplyMaskOrCreateDab(KisFixedPaintDeviceSP dst, KisBrush::ColoringInformation* coloringInformation,
        KisDabShape const& shape,
        const KisPaintInformation& info,
        double subPixelX , double subPixelY,
        qreal softnessFactor) const
{
    m_d->brushesPipe.generateMaskAndApplyMaskOrCreateDab(dst, coloringInformation, shape, info, subPixelX, subPixelY, softnessFactor);
}

QVector<KisGbrBrushSP> KisImagePipeBrush::brushes() const
{
    return m_d->brushesPipe.brushes();
}

KisFixedPaintDeviceSP KisImagePipeBrush::paintDevice(
    const KoColorSpace * colorSpace,
    KisDabShape const& shape,
    const KisPaintInformation& info, double subPixelX, double subPixelY) const
{
    return m_d->brushesPipe.paintDevice(colorSpace, shape, info, subPixelX, subPixelY);
}

enumBrushType KisImagePipeBrush::brushType() const
{
    return !hasColor() || useColorAsMask() ? PIPE_MASK : PIPE_IMAGE;
}

QString KisImagePipeBrush::parasiteSelection()
{
    return parasiteSelectionString;
}

bool KisImagePipeBrush::hasColor() const
{
    return m_d->brushesPipe.hasColor();
}

void KisImagePipeBrush::makeMaskImage()
{
    m_d->brushesPipe.makeMaskImage();
    setUseColorAsMask(false);
}

void KisImagePipeBrush::setUseColorAsMask(bool useColorAsMask)
{
    KisGbrBrush::setUseColorAsMask(useColorAsMask);
    m_d->brushesPipe.setUseColorAsMask(useColorAsMask);
}

const KisBoundary* KisImagePipeBrush::boundary() const
{
    KisGbrBrushSP brush = m_d->brushesPipe.firstBrush();
    Q_ASSERT(brush);

    return brush->boundary();
}

bool KisImagePipeBrush::canPaintFor(const KisPaintInformation& info)
{
    return (!m_d->brushesPipe.parasite().needsMovement || info.drawingDistance() >= 0.5);
}

KisBrushSP KisImagePipeBrush::clone() const
{
    return KisBrushSP(new KisImagePipeBrush(*this));
}

QString KisImagePipeBrush::defaultFileExtension() const
{
    return QString(".gih");
}

quint32 KisImagePipeBrush::brushIndex(const KisPaintInformation& info) const
{
    return m_d->brushesPipe.brushIndex(info);
}

qint32 KisImagePipeBrush::maskWidth(KisDabShape const& shape, double subPixelX, double subPixelY, const KisPaintInformation& info) const
{
    return m_d->brushesPipe.maskWidth(shape, subPixelX, subPixelY, info);
}

qint32 KisImagePipeBrush::maskHeight(KisDabShape const& shape, double subPixelX, double subPixelY, const KisPaintInformation& info) const
{
    return m_d->brushesPipe.maskHeight(shape, subPixelX, subPixelY, info);
}

void KisImagePipeBrush::setAngle(qreal _angle)
{
    KisGbrBrush::setAngle(_angle);
    m_d->brushesPipe.setAngle(_angle);
}

void KisImagePipeBrush::setScale(qreal _scale)
{
    KisGbrBrush::setScale(_scale);
    m_d->brushesPipe.setScale(_scale);
}

void KisImagePipeBrush::setSpacing(double _spacing)
{
    KisGbrBrush::setSpacing(_spacing);
    m_d->brushesPipe.setSpacing(_spacing);
}

void KisImagePipeBrush::setBrushType(enumBrushType type)
{
    Q_UNUSED(type);
    qFatal("FATAL: protected member setBrushType has no meaning for KisImagePipeBrush");
    // brushType() is a function of hasColor() and useColorAsMask()
}

void KisImagePipeBrush::setHasColor(bool hasColor)
{
    Q_UNUSED(hasColor);
    qFatal("FATAL: protected member setHasColor has no meaning for KisImagePipeBrush");
    // hasColor() is a function of the underlying brushes
}

KisGbrBrushSP KisImagePipeBrush::testingGetCurrentBrush(const KisPaintInformation& info) const
{
    return m_d->brushesPipe.currentBrush(info);
}


void KisImagePipeBrush::testingSelectNextBrush(const KisPaintInformation& info) const
{
    return m_d->brushesPipe.testingSelectNextBrush(info);
}

const KisPipeBrushParasite& KisImagePipeBrush::parasite() const {
    return m_d->brushesPipe.parasite();
}

void KisImagePipeBrush::setParasite(const KisPipeBrushParasite &parasite)
{
    m_d->brushesPipe.setParasite(parasite);
}

void KisImagePipeBrush::setDevices(QVector<QVector<KisPaintDevice *> > devices, int w, int h)
{

    for (int i = 0; i < devices.at(0).count(); i++) {
        m_d->brushesPipe.addBrush(KisGbrBrushSP(new KisGbrBrush(devices.at(0).at(i), 0, 0, w, h)));
    }
}
