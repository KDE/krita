class GridConfig : QObject
{
%TypeHeaderCode
#include "GridConfig.h"
%End

%ConvertToSubClassCode
  // Need to add sub class conversion, otherwise a GridConfig passed to a Signal is provided as a QObject instead of
  // a GridConfig object
  // (not sure why, but explicit conversion here fix the problem)
  GridConfig* mc = qobject_cast<GridConfig*>(sipCpp);
  sipType = 0;
  if (mc) {
      sipType = sipType_GridConfig;
  }
%End

    GridConfig(const GridConfig & __0);
public:
    bool operator==(const GridConfig &other) const;
    bool operator!=(const GridConfig &other) const;
public:
    GridConfig(QObject*  parent /TransferThis/ = 0);
    virtual ~GridConfig();

public Q_SLOTS:
    bool visible() const;
    void setVisible(bool visible);

    bool snap() const;
    void setSnap(bool snap);

    QPoint offset() const;
    void setOffset(QPoint offset);

    QPoint spacing() const;
    void setSpacing(QPoint spacing);

    bool spacingActiveHorizontal() const;
    void setSpacingActiveHorizontal(bool active);

    bool spacingActiveVertical() const;
    void setSpacingActiveVertical(bool active);

    int subdivision() const;
    void setSubdivision(int subdivision);

    qreal angleLeft() const;
    void setAngleLeft(qreal angleLeft);

    qreal angleRight() const;
    void setAngleRight(qreal angleRight);

    bool angleLeftActive() const;
    void setAngleLeftActive(bool active);

    bool angleRightActive() const;
    void setAngleRightActive(bool active);

    int cellSpacing() const;
    void setCellSpacing(int cellSpacing);

    int cellSize() const;
    void setCellSize(int cellSize);

    QString type() const;
    void setType(const QString &gridType);

    bool offsetAspectLocked() const;
    void setOffsetAspectLocked(bool offsetAspectLocked);

    bool spacingAspectLocked() const;
    void setSpacingAspectLocked(bool spacingAspectLocked);

    bool angleAspectLocked() const;
    void setAngleAspectLocked(bool angleAspectLocked);

    QString lineTypeMain() const;
    void setLineTypeMain(const QString &lineType);

    QString lineTypeSubdivision() const;
    void setLineTypeSubdivision(const QString &lineType);

    QString lineTypeVertical() const;
    void setLineTypeVertical(const QString &lineType);

    QColor colorMain() const;
    void setColorMain(QColor colorMain);

    QColor colorSubdivision() const;
    void setColorSubdivision(QColor colorSubdivision);

    QColor colorVertical() const;
    void setColorVertical(QColor colorVertical);

    bool fromXml(const QString &xmlContent) const;
    QString toXml() const;

private:
};
