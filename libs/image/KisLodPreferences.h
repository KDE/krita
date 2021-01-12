#ifndef KISLODPREFERENCES_H
#define KISLODPREFERENCES_H

#include <QFlags>
#include "kis_assert.h"

struct KisLodPreferences
{
    enum PreferenceFlag {
        None = 0x0,
        LodSupported = 0x1,
        LodPreferred = 0x2
    };
    Q_DECLARE_FLAGS(PreferenceFlags, PreferenceFlag)

    KisLodPreferences()
    {
    }

    KisLodPreferences(PreferenceFlags flags, int desiredLevelOfDetail)
        : m_flags(flags), m_desiredLevelOfDetail(desiredLevelOfDetail)
    {
        KIS_SAFE_ASSERT_RECOVER(m_desiredLevelOfDetail == 0 || m_flags & LodSupported) {
            m_desiredLevelOfDetail = 0;
        }
    }

    KisLodPreferences(int desiredLevelOfDetail)
        : m_flags(LodSupported | LodPreferred), m_desiredLevelOfDetail(desiredLevelOfDetail)
    {
    }

    KisLodPreferences(const KisLodPreferences &rhs) = default;

    PreferenceFlags flags() const {
        return m_flags;
    }

    bool lodPreferred() const {
        return m_flags & LodPreferred;
    }

    bool lodSupported() const {
        return m_flags & LodSupported;
    }

    int desiredLevelOfDetail() const {
        return m_desiredLevelOfDetail;
    }

private:
    PreferenceFlags m_flags = None;
    int m_desiredLevelOfDetail = 0;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(KisLodPreferences::PreferenceFlags)

#endif // KISLODPREFERENCES_H
