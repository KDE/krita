#
#  SPDX-License-Identifier: GPL-3.0-or-later
#

from .channels2layers import ChannelsToLayers

# And add the extension to Krita's list of extensions:
app = Krita.instance()
# Instantiate your class:
extension = ChannelsToLayers(parent=app)
app.addExtension(extension)
