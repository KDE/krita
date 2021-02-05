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
        : m_currentBrushIndex(0)
        , m_isInitialized(false)
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
        qreal velocity;
        qreal capSpeed = 3;

        switch (mode) {
        case KisParasite::Constant:
        case KisParasite::Incremental:
        case KisParasite::Random:
            break;
        case KisParasite::Pressure:
            index = static_cast<int>(info.pressure() * (rank - 1) + 0.5);
            break;
        case KisParasite::Angular:
            // + M_PI_2 + M_PI_4 to be compatible with the gimp
            angle = info.drawingAngle() + M_PI_2 + M_PI_4;
            angle = normalizeAngle(angle);

            index = static_cast<int>(angle / (2.0 * M_PI) * rank);
            break;
        case KisParasite::TiltX:
            index = qRound(info.xTilt() / 2.0 * rank) + rank / 2;
            break;
        case KisParasite::TiltY:
            index = qRound(info.yTilt() / 2.0 * rank) + rank / 2;
            break;
        case KisParasite::Velocity:
            // log is slow, but allows for nicer dab transition
            velocity = log(info.drawingSpeed() + 1);
            if (velocity > capSpeed) {
                velocity = capSpeed;
            }
            velocity /= capSpeed;
            velocity *= (rank - 1) + 0.5;
            index = qRound(velocity);
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
            index = info.randomSource()->generate(0, rank-1);
            break;
        case KisParasite::Pressure:
        case KisParasite::Angular:
            break;
        case KisParasite::TiltX:
        case KisParasite::TiltY:
        case KisParasite::Velocity:
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
        brushIndex %= (quint32)m_brushes.size();
        m_currentBrushIndex = brushIndex;
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
    using KisBrushesPipe<KisGbrBrush>::sizeBrush;

    int currentBrushIndex() override {
        return m_currentBrushIndex;
    }

    void setParasite(const KisPipeBrushParasite& parasite) {
        m_parasite = parasite;
    }

    const KisPipeBrushParasite& parasite() const {
        return m_parasite;
    }

    void setAdjustmentMidPoint(quint8 value) {
        Q_FOREACH (KisGbrBrush * brush, m_brushes) {
            brush->setAdjustmentMidPoint(value);
        }
    }

    void setBrightnessAdjustment(qreal value) {
        Q_FOREACH (KisGbrBrush * brush, m_brushes) {
            brush->setBrightnessAdjustment(value);
        }
    }

    void setContrastAdjustment(qreal value) {
        Q_FOREACH (KisGbrBrush * brush, m_brushes) {
            brush->setContrastAdjustment(value);
        }
    }

    void makeMaskImage(bool preserveAlpha) {
        Q_FOREACH (KisGbrBrush * brush, m_brushes) {
            brush->makeMaskImage(preserveAlpha);
        }
    }

    bool saveToDevice(QIODevice* dev) const {
        Q_FOREACH (KisGbrBrush * brush, m_brushes) {
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
    int m_currentBrushIndex;
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

    while (i < data.size() && data[i] != '\n') {
        line1.append(data[i]);
        i++;
    }
    setName(QString::fromUtf8(line1, line1.size()));

    i++; // Skip past the first newline

    // The second line contains the number of brushes, separated by a space from the parasite

    // XXX: This stuff is in utf-8, too.
    QByteArray line2;
    while (i < data.size() && data[i] != '\n') {
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

    for (int brushIndex = m_d->brushesPipe.sizeBrush();
            brushIndex < numOfBrushes && i < data.size(); brushIndex++) {

        KisGbrBrush* brush = new KisGbrBrush(name() + '_' + QString().setNum(brushIndex),
                                             data,
                                             i);

        m_d->brushesPipe.addBrush(brush);
    }

    if (numOfBrushes > 0) {
        setValid(true);
        setSpacing(m_d->brushesPipe.lastBrush()->spacing());
        setWidth(m_d->brushesPipe.firstBrush()->width());
        setHeight(m_d->brushesPipe.firstBrush()->height());
        setBrushTipImage(m_d->brushesPipe.firstBrush()->brushTipImage());
        setBrushApplication(m_d->brushesPipe.firstBrush()->brushApplication());
        setBrushType(m_d->brushesPipe.isImageType() ? PIPE_IMAGE : PIPE_MASK);
        setHasColorAndTransparency(m_d->brushesPipe.hasColorAndTransparency());
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

    if (m_d->brushesPipe.parasite().dim >= KisPipeBrushParasite::MaxDim) {
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

void KisImagePipeBrush::prepareForSeqNo(const KisPaintInformation &info, int seqNo)
{
    m_d->brushesPipe.prepareForSeqNo(info, seqNo);
}

void KisImagePipeBrush::generateMaskAndApplyMaskOrCreateDab(KisFixedPaintDeviceSP dst, KisBrush::ColoringInformation* coloringInformation,
        KisDabShape const& shape,
        const KisPaintInformation& info,
        double subPixelX , double subPixelY,
        qreal softnessFactor, qreal lightnessStrength) const
{
    m_d->brushesPipe.generateMaskAndApplyMaskOrCreateDab(dst, coloringInformation, shape, info, subPixelX, subPixelY, softnessFactor, lightnessStrength);
}

QVector<KisGbrBrush *> KisImagePipeBrush::brushes() const
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

QString KisImagePipeBrush::parasiteSelection()
{
    return parasiteSelectionString;
}

void KisImagePipeBrush::makeMaskImage(bool preserveAlpha)
{
    KisGbrBrush::makeMaskImage(preserveAlpha);
    m_d->brushesPipe.makeMaskImage(preserveAlpha);
    setBrushType(PIPE_MASK);
}

void KisImagePipeBrush::setAdjustmentMidPoint(quint8 value)
{
    KisGbrBrush::setAdjustmentMidPoint(value);
    m_d->brushesPipe.setAdjustmentMidPoint(value);
}

void KisImagePipeBrush::setBrightnessAdjustment(qreal value)
{
    KisGbrBrush::setBrightnessAdjustment(value);
    m_d->brushesPipe.setBrightnessAdjustment(value);
}

void KisImagePipeBrush::setContrastAdjustment(qreal value)
{
    KisGbrBrush::setContrastAdjustment(value);
    m_d->brushesPipe.setContrastAdjustment(value);
}

const KisBoundary* KisImagePipeBrush::boundary() const
{
    KisGbrBrush *brush = m_d->brushesPipe.firstBrush();
    Q_ASSERT(brush);

    return brush->boundary();
}

bool KisImagePipeBrush::canPaintFor(const KisPaintInformation& info)
{
    return (!m_d->brushesPipe.parasite().needsMovement || info.drawingDistance() >= 0.5);
}

KisBrush* KisImagePipeBrush::clone() const
{
    return new KisImagePipeBrush(*this);
}

QString KisImagePipeBrush::defaultFileExtension() const
{
    return QString(".gih");
}

quint32 KisImagePipeBrush::brushIndex() const
{
    return m_d->brushesPipe.currentBrushIndex();
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

void KisImagePipeBrush::setBrushApplication(enumBrushApplication brushApplication)
{
    //Set all underlying brushes to use the same brush Application
    KisGbrBrush::setBrushApplication(brushApplication);
    m_d->brushesPipe.setBrushApplication(brushApplication);
}

void KisImagePipeBrush::setGradient(const KoAbstractGradient* gradient) {
    //Set all underlying brushes to use the same gradient
    KisGbrBrush::setGradient(gradient);
    m_d->brushesPipe.setGradient(gradient);
}

KisGbrBrush* KisImagePipeBrush::testingGetCurrentBrush(const KisPaintInformation& info) const
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
        m_d->brushesPipe.addBrush(new KisGbrBrush(devices.at(0).at(i), 0, 0, w, h));
    }
}
