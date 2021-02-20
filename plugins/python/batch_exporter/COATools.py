#
#  SPDX-License-Identifier: GPL-3.0-or-later
#

import json
from pathlib import Path


class COAToolsFormat:
    def __init__(self, cfg, statusBar):
        self.cfg = cfg
        self.statusBar = statusBar
        self.reset()

    def reset(self):
        self.nodes = []

    def showError(self, msg):
        msg, timeout = (self.cfg["error"]["msg"].format(msg), self.cfg["error"]["timeout"])
        self.statusBar.showMessage(msg, timeout)

    def collect(self, node):
        print("COAToolsFormat collecting %s" % (node.name))
        self.nodes.append(node)

    def remap(self, oldValue, oldMin, oldMax, newMin, newMax):
        if oldMin == newMin and oldMax == newMax:
            return oldValue
        return (((oldValue - oldMin) * (newMax - newMin)) / (oldMax - oldMin)) + newMin

    def save(self, output_dir=""):
        """
        Parses layers configured to export to COA Tools and builds the JSON data
        COA Tools need to import the files
        """
        # For each top-level node (Group Layer)
        export_dir = output_dir
        for wn in self.nodes:
            children = wn.children
            path = wn.path

            if path != "":
                export_dir = path

            print("COAToolsFormat exporting %d items from %s" % (len(children), wn.name))
            try:
                if len(children) <= 0:
                    raise ValueError(wn.name, "has no children to export")

                coa_data = {"name": wn.name, "nodes": []}
                print("COAToolsFormat exporting %s to %s" % (wn.name, export_dir))
                for idx, child in enumerate(children):
                    sheet_meta = dict()
                    if child.coa != "":
                        fn, sheet_meta = child.saveCOASpriteSheet(export_dir)
                    else:
                        fn = child.saveCOA(export_dir)
                    path = Path(fn)
                    node = child.node
                    coords = node.bounds().getCoords()
                    relative_coords = coords

                    parent_node = node.parentNode()
                    parent_coords = parent_node.bounds().getCoords()
                    relative_coords = [coords[0] - parent_coords[0], coords[1] - parent_coords[1]]

                    p_width = parent_coords[2] - parent_coords[0]
                    p_height = parent_coords[3] - parent_coords[1]

                    tiles_x, tiles_y = 1, 1
                    if len(sheet_meta) > 0:
                        tiles_x, tiles_y = sheet_meta["tiles_x"], sheet_meta["tiles_y"]

                    coa_entry = {
                        "children": [],
                        "frame_index": 0,
                        "name": child.name,
                        "node_path": child.name,
                        "offset": [-p_width / 2, p_height / 2],
                        "opacity": self.remap(node.opacity(), 0, 255, 0, 1),
                        "pivot_offset": [0.0, 0.0],
                        "position": relative_coords,
                        "resource_path": str(path.relative_to(Path(export_dir))),
                        "rotation": 0.0,
                        "scale": [1.0, 1.0],
                        "tiles_x": tiles_x,
                        "tiles_y": tiles_y,
                        "type": "SPRITE",
                        "z": idx - len(children) + 1,
                    }
                    coa_data["nodes"].append(coa_entry)
                json_data = json.dumps(coa_data, sort_keys=True, indent=4, separators=(",", ": "))
                Path(export_dir, wn.name + ".json").write_text(json_data)

            except ValueError as e:
                self.showError(e)
