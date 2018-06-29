#include "KisSwatch.h"

KisSwatch &KisSwatch::operator =(const KisSwatch &source)
{
    if (&source == this)
        return *this;
    m_color = source.m_color;
    m_id = source.m_id;
    m_name = source.m_name;
    m_spotColor = source.m_spotColor;
    return *this;
}
