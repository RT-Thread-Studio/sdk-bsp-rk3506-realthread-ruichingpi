import os
import subprocess
import shutil
from shutil import ignore_patterns
from SCons.Script import *

def do_copy_file(src, dst):
    # check source file
    if not os.path.exists(src):
        return

    path = os.path.dirname(dst)
    # mkdir if path not exist
    if not os.path.exists(path):
        os.makedirs(path)

    shutil.copy2(src, dst)

def do_copy_folder(src_dir, dst_dir, ignore=None):
    # print(src_dir, dst_dir)
    import shutil
    # check source directory
    if not os.path.exists(src_dir):
        return

    try:
        if os.path.exists(dst_dir):
            shutil.rmtree(dst_dir)
    except:
        print('Deletes folder: %s failed.' % dst_dir)
        return

    shutil.copytree(src_dir, dst_dir, ignore = ignore)

source_ext = ['c', 'h', 's', 'S', 'cpp', 'cxx', 'cc', 'xpm']
source_list = []

def walk_children(child):
    global source_list
    global source_ext

    # print child
    full_path = child.rfile().abspath
    file_type  = full_path.rsplit('.',1)[1]
    #print file_type
    if file_type in source_ext:
        if full_path not in source_list:
            source_list.append(full_path)

    children = child.all_children()
    if children != []:
        for item in children:
            walk_children(item)

def walk_kconfig(RTT_ROOT, source_list):
    for parent, dirnames, filenames in os.walk(RTT_ROOT):
        if 'bsp' in parent:
            continue
        if '.git' in parent:
            continue
        if 'tools' in parent:
            continue

        if 'Kconfig' in filenames:
            pathfile = os.path.join(parent, 'Kconfig')
            source_list.append(pathfile)
        if 'KConfig' in filenames:
            pathfile = os.path.join(parent, 'KConfig')
            source_list.append(pathfile)

def bsp_update_kconfig(dist_dir):
    # change RTT_ROOT in Kconfig
    if not os.path.isfile(os.path.join(dist_dir, 'Kconfig')):
        return

    with open(os.path.join(dist_dir, 'Kconfig'), 'r') as f:
        data = f.readlines()
    with open(os.path.join(dist_dir, 'Kconfig'), 'w') as f:
        found = 0
        for line in data:
            if line.find('ROOT_SDK') != -1:
                line = "orsource 'rt-thread/Kconfig'\n"
            f.write(line)

def bsp_copy_files(bsp_root, dist_dir):
    # copy BSP files
    do_copy_folder(os.path.join(bsp_root), dist_dir,
        ignore_patterns('build', 'dist', '*.pyc', '*.old', '*.map', 'rtthread.bin', '.sconsign.dblite', '*.elf', '*.axf', 'cconfig.h', '.pycache', 'cic.json'))

def MkDist(program, BSP_ROOT, RTT_ROOT, Env, project_name, project_path):
    print('make app distribution....')

    print(project_path)
    if project_path == None:
        dist_dir = os.path.join(BSP_ROOT, 'dist', project_name)
    else:
        dist_dir = project_path
        print(dist_dir)

    rtt_dir_path = os.path.join(dist_dir, '..', 'rt-thread')


    # copy BSP files
    print('=> %s' % os.path.basename(BSP_ROOT))
    bsp_copy_files(BSP_ROOT, dist_dir)

    # copy .settings .cproject .project directory
    print('=> .settings')

    if Env['chip'] == 'rk3506':
        if "amp" in os.path.basename(BSP_ROOT):
            do_copy_folder(os.path.join(BSP_ROOT, '..', '..', '.ide', '.settings-amp'), os.path.join(dist_dir, '.settings'))
        else:
            do_copy_folder(os.path.join(BSP_ROOT, '..', '..', '.ide', '.settings-smp'), os.path.join(dist_dir, '.settings'))
    else:
        do_copy_folder(os.path.join(BSP_ROOT, '..', '..', '.ide', '.settings-amp'), os.path.join(dist_dir, '.settings'))

    do_copy_file(os.path.join(BSP_ROOT, '..', '..', '.ide', '.cproject'), os.path.join(dist_dir, '.cproject'))
    do_copy_file(os.path.join(BSP_ROOT, '..', '..', '.ide', '.project'), os.path.join(dist_dir, '.project'))

    # do bsp special dist handle
    if 'dist_handle' in Env:
        print("=> start dist handle")
        dist_handle = Env['dist_handle']
        dist_handle(BSP_ROOT, dist_dir)

    if not os.path.exists(rtt_dir_path):
        print('=> platform')
        do_copy_folder(os.path.join(BSP_ROOT, '..', '..', 'platform'), os.path.join(dist_dir, '..', 'platform'))

        # copy tools directory
        print('=> components')
        do_copy_folder(os.path.join(RTT_ROOT, 'components'), os.path.join(rtt_dir_path, 'components'))

        # copy include directory
        print('=> drivers')
        do_copy_folder(os.path.join(RTT_ROOT, 'drivers'), os.path.join(rtt_dir_path, 'drivers'))

        # skip documentation directory
        # skip examples

        # copy include directory
        print('=> include')
        do_copy_folder(os.path.join(RTT_ROOT, 'include'), os.path.join(rtt_dir_path, 'include'))

        # copy packages directory
        # print('=> packages')
        # do_copy_folder(os.path.join(RTT_ROOT, 'packages'), os.path.join(rtt_dir_path, 'packages'))

        # copy libs directory
        print('=> libs')
        do_copy_folder(os.path.join(RTT_ROOT, 'libs'), os.path.join(rtt_dir_path, 'libs'))

        # copy scripts directory
        print('=> scripts')
        do_copy_folder(os.path.join(RTT_ROOT, 'scripts'), os.path.join(rtt_dir_path, 'scripts'), ignore_patterns('*.pyc', '.pycache', 'cic.json'))

        # copy src directory
        print('=> src')
        do_copy_folder(os.path.join(RTT_ROOT, 'src'), os.path.join(rtt_dir_path, 'src'))

        do_copy_file(os.path.join(RTT_ROOT, 'Kconfig'), os.path.join(rtt_dir_path, 'Kconfig'))
        do_copy_file(os.path.join(RTT_ROOT, 'SConscript'), os.path.join(rtt_dir_path, 'SConscript'))

    bsp_update_kconfig(dist_dir)

    print('dist project successfully!')
