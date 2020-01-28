class ManagedColor : QObject
{
%TypeHeaderCode
#include "ManagedColor.h"
%End
    ManagedColor(const ManagedColor & __0);
public:
    ManagedColor(const QString &colorModel, const QString &colorDepth, const QString &colorProfile, QObject *parent = 0);
    bool operator==(const ManagedColor &other) const;
    QColor colorForCanvas(Canvas *canvas) const;
    static ManagedColor *fromQColor(const QColor &qcolor, Canvas *canvas = 0);
    QString colorDepth() const;
    QString colorModel() const;
    QString colorProfile() const;
    bool setColorProfile(const QString &colorProfile);
    bool setColorSpace(const QString &colorModel, const QString &colorDepth, const QString &colorProfile);
    QVector<float> components() const;
    QVector<float> componentsOrdered() const;
    void setComponents(const QVector<float> &values);
    QString toXML() const;
    void fromXML(const QString &xml);
    QString toQString();

private:
};
