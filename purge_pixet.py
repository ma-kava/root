import xml.etree.ElementTree as ET
import argparse
import os

def parseInputParameters():
    parser = argparse.ArgumentParser(description='Removing some plugins.')
    parser.add_argument('--build-dir', action='store', default='_build/Pixet', help='path to directory containing PIXet build files')
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

def remove_plugins(list, build_dir, platform):
    extension = 'dll' if 'Windows' in platform else 'so'

    print(f"Excluding some plugins from {build_dir}/plugins...")
    for plugin in list:
        path = os.path.join(os.getcwd(), build_dir, f"{plugin}.{extension}")
        print(f"  - removing '{plugin}.{extension}'")
        os.remove(path)

def modify_pixet_ini(build_dir, plugin_dir, platform):
    ini_path = os.path.join(build_dir, 'pixet.ini')
    extension = 'dll' if 'Windows' in platform else 'so'
    
    remaining_plugins = []
    for filename in os.listdir(plugin_dir):
        if filename.endswith(extension):
            plugin_name = os.path.splitext(filename)[0]
            remaining_plugins.append(plugin_name)

    with open(ini_path, 'r') as f:
        lines = f.readlines()

    new_lines = []
    in_plugins_section = False
    for line in lines:
        stripped_line = line.strip()
        if stripped_line.lower() == '[plugins]':
            in_plugins_section = True
        elif stripped_line.startswith('[') and stripped_line.endswith(']'):
            in_plugins_section = False
        
        if not in_plugins_section:
            new_lines.append(line)

    # Add the new [Plugins] section at the end
    print(f"Updating [Plugins] section in {ini_path}...")
    new_lines.append('[Plugins]\n')
    for plugin_name in sorted(remaining_plugins):
        # The original format was Plugin=plugins\plugin.dll
        plugin_path = f"plugins\\{plugin_name}.{extension}".replace('/', '\\')
        new_lines.append(f"Plugin={plugin_path}\n")
        print(f"  + Adding '{plugin_name}' to pixet.ini")

    # Write the changes back to the file
    with open(ini_path, 'w') as f:
        f.writelines(new_lines)

if __name__ == '__main__':
    args = parseInputParameters()

    plugins_to_exclude = read_plugins_list(args.xml_config, args.version)

    plugin_dir = os.path.join(args.build_dir, 'plugins')
    remove_plugins(plugins_to_exclude, plugin_dir, args.platform)

    modify_pixet_ini(args.build_dir, plugin_dir, args.platform)
