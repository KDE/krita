#ifndef KOCOLORSETENTRY_H
#define KOCOLORSETENTRY_H

#include "kritapigment_export.h"
#include <QString>
#include "KoColor.h"

class KRITAPIGMENT_EXPORT KoColorSetEntry
{
public:
    KoColorSetEntry();
    KoColorSetEntry(const KoColor &color, const QString &name);

public:
    QString name() const { return m_name; }
    void setName(const QString &name) { m_name = name; }
    QString id() const { return m_id; }
    void setId(const QString &id) { m_id = id; }
    KoColor color() const { return m_color; }
    void setColor(const KoColor &color) { m_color = color; }
    bool spotColor() const { return m_spotColor; }
    void setSpotColor(bool spotColor) { m_spotColor = spotColor; }

public:
    bool operator==(const KoColorSetEntry& rhs) const {
        return m_color == rhs.m_color && m_name == rhs.m_name;
    }

private:
    KoColor m_color;
    QString m_name;
    QString m_id;
    bool m_spotColor;
};

#endif // KOCOLORSETENTRY_H
