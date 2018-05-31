#include "KoColorSetEntry.h"

KoColorSetEntry::KoColorSetEntry()
    : m_spotColor(false)
{ }

KoColorSetEntry::KoColorSetEntry(const KoColor &color, const QString &name)
    : m_color(color)
    , m_name(name)
    , m_spotColor(false)
{ }

