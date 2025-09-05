import xml.etree.ElementTree as ET
import argparse
import os

def parseInputParameters():
    parser = argparse.ArgumentParser(description='Removing some plugins.')
    parser.add_argument('--plugin-dir', action='store', default='_build/Pixet/plugins', help='path to directory containing PIXet plugin files')
    requiredNamed = parser.add_argument_group('required named arguments')
    requiredNamed.add_argument('--xml-config', help='configuration xml file', required=True)        
    requiredNamed.add_argument('--version', help='version of PIXet to package', required=True)        
    requiredNamed.add_argument('--platform', choices=['Linux_x64', 'Linux_ARM32','Linux_ARM64', 'Windows_x64', 'Darwin_x64_ARM64'], required=True)                        

    return parser.parse_args()

def read_plugins_list(file_name, pixet_version):
    tree = ET.parse(file_name)
    root = tree.getroot()

    ver = root.find(f'.//{pixet_version}')
    plugin_list = ver.find('plugins_to_exclude')

    ret = []
    for plugin in plugin_list.iter('plugin'):
        ret.append(plugin.attrib['name'])

    return ret

def remove_plugins(list, plugin_dir, platform):
    extension = ''
    if platform == 'Windows_x64':
        extension = 'dll'
    else:
        extension = 'so'

    for plugin in list:
        path = os.path.join(os.getcwd(), plugin_dir, f"{plugin}.{extension}")
        print(f"deleting plugin {plugin}.{extension}")
        os.remove(path)

if __name__ == '__main__':
    args = parseInputParameters()

    list = read_plugins_list('plugin_cookbook.xml', args.version)
    remove_plugins(list, args.plugin_dir, args.platform)
