#!/usr/bin/env python3

import xml.etree.ElementTree as ET
import argparse
import os
import sys

parser = argparse.ArgumentParser(description='A script for merging libs.xml for different ABIs into one')
parser.add_argument('-p', '--xml-paths', type=str, help='Paths to the xml files as list of strings separated by comma', required=True)
parser.add_argument('--output', type=str, help='Output file path', required=True)
arguments = parser.parse_args()

def merge_xmls(xml_paths):
    merged_root = None

    # Merge arrays from all XML files
    for xml_path in xml_paths:
        if not os.path.exists(xml_path):
            print(f'merge-libs-xml: File {xml_path} does not exist')
            sys.exit(1)

        tree = ET.parse(xml_path)
        root = tree.getroot()

        if merged_root is None:
            merged_root = root
        else:
            # Merge arrays
            arrays_merged = {array.attrib['name']: array for array in merged_root.findall('array')}
            for array in root.findall('array'):
                name = array.attrib['name']
                if name == 'qt_sources':
                    continue
                if name in arrays_merged:
                    arrays_merged[name].extend(array)
                else:
                    merged_root.append(array)

    # Save the merged XML
    return ET.ElementTree(merged_root)

    return merged_xml_path

if __name__ == '__main__':
    xml_paths = arguments.xml_paths.split(',')
    merged_tree = merge_xmls(xml_paths)
    merged_tree.write(arguments.output, encoding='utf-8', xml_declaration=True)

    for xml_path in xml_paths:
        print(f'Removing original {xml_path}')
        os.remove(xml_path)
