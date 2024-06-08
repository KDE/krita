class GuidesConfig : QObject
{
%TypeHeaderCode
#include "GuidesConfig.h"
%End
    GuidesConfig(const GuidesConfig & __0);
public:
    bool operator==(const GuidesConfig &other) const;
    bool operator!=(const GuidesConfig &other) const;
public:
    GuidesConfig(QObject*  parent /TransferThis/ = 0);
    virtual ~GuidesConfig();

public Q_SLOTS:
    QColor color() const;
    void setColor(const QColor &color) const;

    QString lineType() const;
    void setLineType(const QString &lineType);

    bool hasGuides() const;
    bool hasSamePositionAs(const GuidesConfig &guideConfig) const;

    QList<qreal> horizontalGuides() const;
    void setHorizontalGuides(const QList<qreal> &lines);

    QList<qreal> verticalGuides() const;
    void setVerticalGuides(const QList<qreal> &lines);

    bool fromXml(const QString &xmlContent) const;
    QString toXml() const;

    void removeAllGuides();

    bool visible() const;
    void setVisible(const bool value);

    bool locked() const;
    void setLocked(const bool value);

    bool snap() const;
    void setSnap(const bool value);
private:

};
