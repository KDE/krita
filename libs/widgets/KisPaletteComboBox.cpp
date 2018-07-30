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
    connect(this, SIGNAL(currentIndexChanged(int)), SLOT(slotColorSelected(int)));
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
    connect(m_model, SIGNAL(modelReset()), SLOT(slotPaletteChanged()));
    connect(m_model, SIGNAL(dataChanged(const QModelIndex &, const QModelIndex&, const QVector<int> &)),
            SLOT(slotPaletteChanged()));
}

void KisPaletteComboBox::slotPaletteChanged()
{
    if (QPointer<KoColorSet>(m_model->colorSet()).isNull()) { return; }

    clear();
    m_posIdxMap.clear();
    m_idxSwatchMap.clear();

    QVector<SwatchInfoType> infoList;
    for (const QString &groupName : m_model->colorSet()->getGroupNames()) {
        const KisSwatchGroup *group = m_model->colorSet()->getGroup(groupName);
        for (const SwatchInfoType &info : group->infoList()) {
            infoList.append(info);
        }
    }
    std::sort(infoList.begin(), infoList.end(), swatchInfoLess);
    for (int i = 0; i != infoList.size(); i++) {
        const SwatchInfoType &info = infoList[i];
        const KisSwatch &swatch = info.swatch;
        QPixmap colorSquare = createColorSquare(swatch);
        QString name = swatch.name();
        if (!swatch.id().isEmpty()){
            name = swatch.id() + " - " + swatch.name();
        }
        addItem(QIcon(colorSquare), name);
        m_posIdxMap[SwatchPosType(info.column, info.row)] = i;
        m_idxSwatchMap.push_back(swatch);
    }
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
    setCurrentIndex(m_posIdxMap[SwatchPosType(index.column(), index.row())]);
}

void KisPaletteComboBox::slotColorSelected(int idx)
{
    if (idx >= 0 && idx < m_idxSwatchMap.size()) {
        emit sigColorSelected(m_idxSwatchMap[idx].color());
    }
}
