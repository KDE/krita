/*
 *  SPDX-FileCopyrightText: 2006 Boudewijn Rempt <boud@valdyas.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 *  MA 02110-1301, USA.
 */
#ifndef KIS_ADJUSTMENT_LAYER_H_
#define KIS_ADJUSTMENT_LAYER_H_

#include <QObject>
#include <kritaimage_export.h>
#include "kis_selection_based_layer.h"

class KisFilterConfiguration;

class KRITAIMAGE_EXPORT KisAdjustmentLayer : public KisSelectionBasedLayer
{
    Q_OBJECT

public:
    /**
     * creates a new adjustment layer with the given
     * configuration and selection. Note that the selection
     * will be _copied_ (with COW, though).
     * @param image the image to set this AdjustmentLayer to
     * @param name name of the adjustment layer
     * @param kfc the configuration for the adjustment layer filter
     * @param selection is a mask used by the adjustment layer to
     * know where to apply the filter.
     */
    KisAdjustmentLayer(KisImageWSP image, const QString &name, KisFilterConfigurationSP  kfc, KisSelectionSP selection);
    KisAdjustmentLayer(const KisAdjustmentLayer& rhs);
    ~KisAdjustmentLayer() override;

    bool accept(KisNodeVisitor &) override;
    void accept(KisProcessingVisitor &visitor, KisUndoAdapter *undoAdapter) override;

    /**
     * clones this AdjustmentLayer into a KisNodeSP type.
     * @return the KisNodeSP returned
     */
    KisNodeSP clone() const override {
        return KisNodeSP(new KisAdjustmentLayer(*this));
    }

    /**
     * gets the adjustmentLayer's tool filter
     * @return QIcon returns the QIcon tool filter
     */
    QIcon icon() const override;

    /**
     * gets the AdjustmentLayer properties describing whether
     * or not the node is locked, visible, and the filter
     * name is it is a filter. Overrides sectionModelProperties
     * in KisLayer, and KisLayer overrides
     * sectionModelProperties in KisBaseNode.
     * @return KisBaseNode::PropertyList returns a list
     * of the properties
     */
    KisBaseNode::PropertyList sectionModelProperties() const override;

public:

    /**
     * \see KisNodeFilterInterface::setFilter()
     */
    void setFilter(KisFilterConfigurationSP filterConfig) override;

    void setChannelFlags(const QBitArray & channelFlags) override;

protected:
    // override from KisLayer
    QRect incomingChangeRect(const QRect &rect) const override;
    // override from KisNode
    QRect needRect(const QRect &rect, PositionToFilthy pos = N_FILTHY) const override;

public Q_SLOTS:
    /**
     * gets this AdjustmentLayer. Overrides function in
     * KisIndirectPaintingSupport
     * @return this AdjustmentLayer
     */
    KisLayer* layer() {
        return this;
    }
};

#endif // KIS_ADJUSTMENT_LAYER_H_

