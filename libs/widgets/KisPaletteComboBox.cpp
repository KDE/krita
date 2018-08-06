// Qt
#include <QPainter>
#include <QPen>
#include <QVector>

// STL
#include <algorithm>

#include "KisPaletteComboBox.h"

KisPaletteComboBox::KisPaletteComboBox(QWidget *parent)
    : QComboBox(parent)
    , m_completer(new QCompleter(QComboBox::model(), this))
    , m_model(Q_NULLPTR)
{
    m_completer->setCompletionMode(QCompleter::PopupCompletion);
    m_completer->setCaseSensitivity(Qt::CaseInsensitive);
    m_completer->setFilterMode(Qt::MatchContains);
    setCompleter(m_completer.data());
    connect(this, SIGNAL(currentIndexChanged(int)), SLOT(slotIndexSelected(int)));
}

KisPaletteComboBox::~KisPaletteComboBox()
{ }

void KisPaletteComboBox::setPaletteModel(const KisPaletteModel *paletteModel)
{
    if (!m_model.isNull()) {
        m_model->disconnect(this);
    }
    m_model = paletteModel;
    if (m_model.isNull()) { return; }
    slotPaletteChanged();
    connect(m_model, SIGNAL(modelReset()),
            SLOT(slotPaletteChanged()));
    connect(m_model, SIGNAL(dataChanged(const QModelIndex &, const QModelIndex&, const QVector<int> &)),
            SLOT(slotPaletteChanged()));
}

void KisPaletteComboBox::slotPaletteChanged()
{
    if (QPointer<KoColorSet>(m_model->colorSet()).isNull()) { return; }

    blockSignals(true); // avoid changing fg color
    clear();
    m_groupMapMap.clear();
    m_idxSwatchMap.clear();

    for (const QString &groupName : m_model->colorSet()->getGroupNames()) {
        QVector<SwatchInfoType> infoList;
        PosIdxMapType posIdxMap;
        const KisSwatchGroup *group = m_model->colorSet()->getGroup(groupName);
        for (const SwatchInfoType &info : group->infoList()) {
            infoList.append(info);
        }
        std::sort(infoList.begin(), infoList.end(), swatchInfoLess);
        for (const SwatchInfoType &info : infoList) {
            const KisSwatch &swatch = info.swatch;
            QString name = swatch.name();
            if (!swatch.id().isEmpty()){
                name = swatch.id() + " - " + swatch.name();
            }
            addItem(QIcon(createColorSquare(swatch)), name);
            posIdxMap[SwatchPosType(info.column, info.row)] = count() - 1;
            m_idxSwatchMap.push_back(swatch);
        }
        m_groupMapMap[group->name()] = posIdxMap;
    }
    setCurrentIndex(0);
    blockSignals(false);
}

bool KisPaletteComboBox::swatchInfoLess(const SwatchInfoType &first, const SwatchInfoType &second)
{
    return first.swatch.name() < second.swatch.name();
}

QPixmap KisPaletteComboBox::createColorSquare(const KisSwatch &swatch) const
{
    QPixmap colorSquare(32, 32);
    if (swatch.spotColor()) {
        QImage img = QImage(32, 32, QImage::Format_ARGB32);
        QPainter circlePainter;
        img.fill(Qt::transparent);
        circlePainter.begin(&img);
        QBrush brush = QBrush(Qt::SolidPattern);
        brush.setColor(swatch.color().toQColor());
        circlePainter.setBrush(brush);
        QPen pen = circlePainter.pen();
        pen.setColor(Qt::transparent);
        pen.setWidth(0);
        circlePainter.setPen(pen);
        circlePainter.drawEllipse(0, 0, 32, 32);
        circlePainter.end();
        colorSquare = QPixmap::fromImage(img);
    } else {
        colorSquare.fill(swatch.color().toQColor());
    }
    return colorSquare;
}

void KisPaletteComboBox::slotSwatchSelected(const QModelIndex &index)
{
    QString gName = qvariant_cast<QString>(index.data(KisPaletteModel::GroupNameRole));
    int rowInGroup = qvariant_cast<int>(index.data(KisPaletteModel::RowInGroupRole));
    setCurrentIndex(m_groupMapMap[gName][SwatchPosType(index.column(), rowInGroup)]);
}

void KisPaletteComboBox::slotIndexSelected(int idx)
{
    if (idx >= 0 && idx < m_idxSwatchMap.size()) {
        emit sigColorSelected(m_idxSwatchMap[idx].color());
    }
}
