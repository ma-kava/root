import xml.etree.ElementTree as ET
import argparse
import os

EXTENSIONS = {
    'Windows_x64': 'dll',
    'Linux_x64': 'so',
    'Darwin_x64_ARM64': 'so'
}

def parseInputParameters():
    parser = argparse.ArgumentParser(description='Removing some plugins.')
    parser.add_argument('--build-dir', action='store', default='_build/Pixet', help='path to directory containing PIXet build files')
    requiredNamed = parser.add_argument_group('required named arguments')
    requiredNamed.add_argument('--xml-config', help='configuration xml file', required=True)        
    requiredNamed.add_argument('--distrib-version', help='version of PIXet to package', required=True)        
    requiredNamed.add_argument('--platform', choices=['Linux_x64', 'Windows_x64', 'Darwin_x64_ARM64'], required=True)                        

    return parser.parse_args()

def read_xml(file_name, pixet_version):
    tree = ET.parse(file_name)
    root = tree.getroot()

    ver = root.find(f'.//{pixet_version}')
    plugin_list = ver.find('plugins_to_exclude')
    hwlib_list = ver.find('hwlibs_to_exclude')

    ret = {
        'plugins': [],
        'hwlibs': []
    }
    for plugin in plugin_list.iter('plugin'):
        ret['plugins'].append(plugin.attrib['name'])
    for hwlib in hwlib_list.iter('hwlib'):
        ret['hwlibs'].append(hwlib.attrib['name'])
    return ret

def remove_files(exclude_dict, dir_dict, platform):
    extension = EXTENSIONS[platform]
    for key in exclude_dict:
        print(f"Excluding some {key} from {dir_dict[key]}...")
        for name in exclude_dict[key]:
            path = os.path.join(os.getcwd(), dir_dict[key], f"{name}.{extension}")
            print(f"  - removing {key.removesuffix('s')} '{name}.{extension}'")
            try:
                os.remove(path)
            except FileNotFoundError:
                print(f"    File not found: {path}")

def get_remaining_files(dir_dict, extension):
    remaining_files = {
        'plugins': [],
        'hwlibs': []
    }
    # read all remaining hwlibs and plugins in package
    for key in dir_dict: # key is either 'hwlibs' or 'plugins'
        for filename in os.listdir(dir_dict[key]):
            file_name = os.path.splitext(filename)[0]
            remaining_files[key].append(file_name)
    return remaining_files

def read_ini_file(ini_path):
    try:
        with open(ini_path, 'r') as f:
            print(f"Successfully opened {ini_path} for reading.")
            return f.readlines()
    except Exception as e:
        print(f"Failed to open {ini_path} for reading: {e}")
        return None

def generate_new_ini_lines(lines, remaining_files, extension, ini_path):
    new_lines = []
    in_section = False
    for line in lines:
        stripped_line = line.strip()
        if stripped_line.lower() == '[hwlibs]':
            in_section = True
        if not in_section:
            new_lines.append(line)

    for section, key, label in [("Hwlibs", "hwlibs", "Hwlib"), ("Plugins", "plugins", "Plugin")]:
        print(f"Updating [{section}] section in {ini_path}...")
        new_lines.append(f'[{section}]\n')
        for name in sorted(remaining_files[key]):
            path = os.path.join(section.lower(), f"{name}.{extension}")
            new_lines.append(f"{label}={path}\n")
            print(f"  + Adding {key.removesuffix('s')} '{name}.{extension}' to pixet.ini")
        new_lines.append('\n')
    return new_lines

def write_ini_file(ini_path, new_lines):
    try:
        with open(ini_path, 'w') as f:
            print(f"Successfully opened {ini_path} for writing.")
            f.writelines(new_lines)
    except Exception as e:
        print(f"Failed to open {ini_path} for writing: {e}")
        return

def modify_pixet_ini(build_dir, dir_dict, platform):
    ini_path = os.path.join(build_dir, 'pixet.ini')
    extension = EXTENSIONS[platform]
    remaining_files = get_remaining_files(dir_dict, extension)
    lines = read_ini_file(ini_path)
    if lines is None:
        return
    new_lines = generate_new_ini_lines(lines, remaining_files, extension, ini_path)
    write_ini_file(ini_path, new_lines)

if __name__ == '__main__':
    args = parseInputParameters()
    exclude_dict = read_xml(args.xml_config, args.distrib_version)
    dir_dict = {
        'plugins': os.path.join(args.build_dir, 'plugins'),
        'hwlibs': os.path.join(args.build_dir, 'hwlibs')
    }
    remove_files(exclude_dict, dir_dict, args.platform)
    modify_pixet_ini(args.build_dir, dir_dict, args.platform)
