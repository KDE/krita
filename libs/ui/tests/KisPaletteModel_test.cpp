#include <QTest>
#include <KisPaletteModel.h>

class PaletteModelTester : public QObject
{
    Q_OBJECT
private slots:
    void initTestCase()
    {}
    void addSwatchAtEmptySpot()
    {}
    void deleteSwatchFromEmptySpot()
    {}
    void deleteSwatchToSpotOutOfPositionRange()
    {}
    void deleteSwatchFromSpotOutOfPositionRange()
    {}
    void changePaletteSize()
    {}
};
