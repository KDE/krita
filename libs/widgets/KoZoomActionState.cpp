/*
 *  SPDX-FileCopyrightText: 2025 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "KoZoomActionState.h"

#include <QDebug>
#include <QLocale>
#include <klocalizedstring.h>

namespace
{
QString zoomToString(qreal value)
{
    const qreal valueInPercent = value * 100;
    const int precision = (value > 10.0) ? 0 : 1;

    return i18n("%1%", QLocale().toString(valueInPercent, 'f', precision));
}

QVector<KoZoomActionState::ZoomItem> generateZoomGuiItems(const QVector<qreal> &standardZoomLevels)
{
    QVector<qreal> zoomLevels;

    // we keep only sane zoom levels in the dropdown menu
    std::copy_if(standardZoomLevels.begin(), standardZoomLevels.end(),
        std::back_inserter(zoomLevels),
        [] (qreal zoom) { return zoom >= 0.2 && zoom <= 10; });

    QVector<KoZoomActionState::ZoomItem> result;
    result.reserve(zoomLevels.size() + 3);

    result.push_back(KoZoomActionState::ZoomItem{KoZoomMode::ZOOM_PAGE, -1.0, KoZoomMode::toString(KoZoomMode::ZOOM_PAGE)});
    result.push_back(KoZoomActionState::ZoomItem{KoZoomMode::ZOOM_WIDTH, -1.0, KoZoomMode::toString(KoZoomMode::ZOOM_WIDTH)});
    result.push_back(KoZoomActionState::ZoomItem{KoZoomMode::ZOOM_HEIGHT, -1.0, KoZoomMode::toString(KoZoomMode::ZOOM_HEIGHT)});

    Q_FOREACH (qreal value, zoomLevels) {
        result.push_back(KoZoomActionState::ZoomItem{KoZoomMode::ZOOM_CONSTANT, value, zoomToString(value)});
    }

    return result;
}

} // namespace

KoZoomActionState::KoZoomActionState(const KoZoomState &state)
{
    setZoomState(state);
}

void KoZoomActionState::setZoomState(const KoZoomState &newState)
{
    bool changedGuiLevels = false;

    if (!qFuzzyCompare(newState.minZoom, zoomState.minZoom) ||
        !qFuzzyCompare(newState.maxZoom, zoomState.maxZoom)) {

        auto newStandardLevels = KoZoomMode::generateStandardZoomLevels(newState.minZoom, newState.maxZoom);
        if (newStandardLevels != standardLevels) {
            standardLevels = newStandardLevels;
            auto newGuiLevels = generateZoomGuiItems(newStandardLevels);
            if (newGuiLevels != guiLevels) {
                guiLevels = newGuiLevels;
                changedGuiLevels = true;
            }
        }
    }

    int proposedCurrentIndex = currentRealLevelIndex;

    if (changedGuiLevels || !qFuzzyCompare(newState.zoom, zoomState.zoom)) {
        realGuiLevels = guiLevels;

        const qreal eps = 1e-5;
        auto fuzzyCompareLess = [eps] (const ZoomItem &lhs, qreal rhs) {
            return std::get<qreal>(lhs) + eps < rhs;
        };

        auto it = std::lower_bound(realGuiLevels.begin(), realGuiLevels.end(),
                                   newState.zoom, fuzzyCompareLess);

        /**
         * 1) If all elements are smaller than the new zoom, then add into the end
         * 2) If the found element is too similar to the new zoom, then just keep
         *    the existing element instead
         */
        if (it == realGuiLevels.end() || !(qAbs(std::get<qreal>(*it) - newState.zoom) < eps)) {
            it = realGuiLevels.insert(it, {KoZoomMode::ZOOM_CONSTANT, newState.zoom, zoomToString(newState.zoom)});
        }

        proposedCurrentIndex = std::distance(realGuiLevels.begin(), it);
    }

    switch (newState.mode) {
    case KoZoomMode::ZOOM_PAGE:
        currentRealLevelIndex = 0;
        break;
    case KoZoomMode::ZOOM_WIDTH:
        currentRealLevelIndex = 1;
        break;
    case KoZoomMode::ZOOM_HEIGHT:
        currentRealLevelIndex = 2;
        break;

    case KoZoomMode::ZOOM_CONSTANT:
    default:
        currentRealLevelIndex = proposedCurrentIndex;
        break;
    }

    zoomState = newState;
}

int KoZoomActionState::calcNearestStandardLevel(qreal zoom) const
{
    const qreal eps = 1e-5;
    auto fuzzyCompareLess = [eps](qreal lhs, qreal rhs) {
        return lhs + eps < rhs;
    };

    auto it = std::lower_bound(standardLevels.begin(), standardLevels.end(),
                               zoom, fuzzyCompareLess);

    return qMin(standardLevels.size() - 1, std::distance(standardLevels.begin(), it));
}

int KoZoomActionState::calcNearestStandardLevel() const
{
    return calcNearestStandardLevel(zoomState.zoom);
}

QDebug operator<<(QDebug dbg, const KoZoomActionState::ZoomItem &item)
{
    dbg.nospace() << "KoZoomActionState::ZoomItem("
                  << std::get<0>(item) << ", "
                  << std::get<1>(item) << ", "
                  << std::get<2>(item) << ")";

    return dbg.space();
}

