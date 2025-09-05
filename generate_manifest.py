# this in the end won't be used as it was created when I understand whole concept wrong. However I will keep it because I created it without AI :)

import xml.etree.ElementTree as ET

def add_plugins(parent, platform):
    if platform == 'windows':
        extension = 'dll'
    else:
        extension = 'so'

    plugins_list = open('advacam-PIXet_packaging/lists_of_plugins.txt', 'r').readlines()

    for plugin_name in plugins_list:
        plugin_name = plugin_name.strip()

        plugin = ET.Element('plugin')
        plugin.set('name', f'{plugin_name}.{extension}')

        # files_node = parent.find('files')
        parent.append(plugin)

def find_tag(root, tag): # DFS
    for child in root:
        if child.tag == tag:
            return child
        
        ret = find_tag(child, tag)
        if ret is not None and ret.tag == tag:
            return ret

def remove_plugin_dir(root):
    for parent in root.iter():
        for child in list(parent):
            if (
                child.tag == "dir"
                and child.attrib.get("source") == "#BUILD_DIR#/plugins"
                and child.attrib.get("destination") == "plugins"
                and child.attrib.get("platform") == "Linux_x64,Darwin_x64_ARM64"
            ):
                parent.remove(child)

if __name__ == '__main__':
    tree = ET.parse('advacam-PIXet_packaging/plugin_cookbook.xml')
    root = tree.getroot()    
    target = find_tag(root, 'plugins')

    # remove_plugin_dir(target)
    add_plugins(target, 'linux')
    tree.write('advacam-PIXet_packaging/plugin_cookbook.xml')
