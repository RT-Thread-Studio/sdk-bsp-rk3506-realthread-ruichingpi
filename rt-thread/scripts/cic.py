#-*- encoding: utf-8 -*-
#---------------------------------------------------------------------------------
# Copyright (c) 2006-2021, RT-Thread Development Team
# SPDX-License-Identifier: Apache-2.0
#---------------------------------------------------------------------------------
import os, sys
import re
import json
import fnmatch
import rtconfig
from utils import _make_path_relative
from SCons.Script import *

#---------------------------------------------------------------------------------
# 变更日志
# V1.2.0：添加导出 obj 文件路径功能
# V1.3.0：添加导出 postactions 功能
# V1.4.0: 将 postactions 解析采用 POST_ACTION 改为 POST_ACTION_IDE 字段
# V1.4.1: 将 处理部分软件包没有部分编译字段导致无法正常导出 cic 文件的问题
#---------------------------------------------------------------------------------
__VERSION__ = (1, 4, 1)

AddOption('--action', dest = 'action', type = 'string', help = 'generate cic action')
AddOption('--cfg', dest = 'cfg', type = 'string', help = 'generate cic cfg')
AddOption('--exclude', dest = 'exclude', action = 'store_true', default = False, help = 'generate cic exclude')
__CIC_FILENAME__ = 'cic.json'
__EXCLUDE_LIST_FILENAME__ = 'exclude_list.json'

invalid_parameters = ('LOCAL_CPPDEFINES', 'LOCAL_CPPPATH', 'LOCAL_ASFLAGS', 'LOCAL_CCFLAGS')
suffixes = {
    'ASM': ('.s', '.S'),
    'C': ('.c', '.cc'),
    'CPP':  ('.cpp'),
}

patterns = {
    'ASM': ['*.s', '*.S'],
    'C': ['*.c', '*.cc'],
    'CPP':  ['*.cpp'],
    'OBJ':  ['*.o'],
}

def _is_subdirectory(parent, target):
    parent = os.path.abspath(parent)
    target = os.path.abspath(target)
    if sys.platform == 'win32':
        parent = parent.lower()
        target = target.lower()
    if target.startswith(parent):
        return True
    return False

def _scan_directory(base_dir, file_patterns, exclude_dirs=[]):
    matching_files = []
    for root, dirs, files in os.walk(base_dir):
        # 排除指定的目录
        dirs[:] = [d for d in dirs if d not in exclude_dirs]
        for file_pattern in file_patterns:
            matching_files.extend(
                [os.path.join(root, file) for file in fnmatch.filter(files, file_pattern)]
            )
    return matching_files

class cic:
    def __init__(self, cfg, exclude, env, rtt_root, bsp_root):
        self.cic = {}
        self.cfg = cfg
        self.exclude = exclude
        self.env = env
        self.rtt_root = rtt_root
        self.bsp_root = bsp_root
        self.project = env['project']

        def _split_flags(flags):
            flags_list = flags.split()
            processed_flags = []
            current_flag = []
            for flag in flags_list:
                if flag.startswith('-'):
                    if current_flag:
                        processed_flags.append(' '.join(current_flag))
                    current_flag = [flag]
                else:
                    current_flag.append(flag)
            if current_flag:
                processed_flags.append(' '.join(current_flag))
            return processed_flags

        def __parse_phy_path(self, path):
            if _is_subdirectory(self.bsp_root, path):
                path_relative = _make_path_relative(self.bsp_root, path).replace('\\', '/')
                if path_relative == '.':
                    path = "${BSP_ROOT}"
                else:
                    path = "${BSP_ROOT}/%s" % path_relative
            elif _is_subdirectory(self.rtt_root, path):
                path_relative = _make_path_relative(self.rtt_root, path).replace('\\', '/')
                if path_relative == '.':
                    path = "${RTT_ROOT}"
                else:
                    path = "${RTT_ROOT}/%s" % path_relative
            else:
                path = path.replace('\\', '/')
            return path

        def __parse_vir_path(self, path):
            for mapping_item in self.cic['Linked']:
                if _is_subdirectory(mapping_item['phy_path'], path):
                    path = path.replace('\\', '/').replace(mapping_item['phy_path'],
                        '${BSP_ROOT}/%s' % mapping_item['vir_path'])
                    return path
            return path

        def __scan_file(self, file_patterns, exclude_dirs=[]):
            files = []
            files += _scan_directory(os.getcwd(), file_patterns, exclude_dirs)
            for linked in rtconfig.LINKEDRESOURCES:
                scan_path = linked['phy_path'].replace('${RTT_ROOT}/', '')
                scan_path = os.path.join(self.rtt_root, scan_path)
                scan_path = os.path.abspath(scan_path)
                files += _scan_directory(scan_path, file_patterns, exclude_dirs)

            vir_files = []
            for file in files:
                file = __parse_phy_path(self, file)
                file = __parse_vir_path(self, file)
                vir_files.append(file)

            return vir_files

        def __export_parse_linked_resources(self):
            linked_resources = []
            if rtconfig.LINKEDRESOURCES:
                for resource in rtconfig.LINKEDRESOURCES:
                    resource['phy_path'] = __parse_phy_path(self, resource['phy_path'])
                    linked_resources.append(resource)
            self.cic['Linked'] = linked_resources

        def __export_project(self):
            project = {}
            project['TARGET_EXT'] = rtconfig.TARGET_EXT
            project['TARGET_NAME'] = rtconfig.TARGET_NAME
            project['ARCH'] = rtconfig.ARCH
            project['CPU'] = rtconfig.CPU
            project['CROSS_TOOL'] = rtconfig.CROSS_TOOL
            project['BUILD'] = rtconfig.BUILD
            project['DEBUGGING_METHOD'] = []
            # rtconfig_h = os.path.join(self.bsp_root, 'kconfig.h')
            rtconfig_h = os.path.join(self.bsp_root, 'kconfig.h')
            if os.path.exists(rtconfig_h):
                with open(rtconfig_h, 'r') as file:
                    if any('#define RT_USING_AMP' in line for line in file):
                        project['DEBUGGING_METHOD'] = 'AMP'
                    else:
                        project['DEBUGGING_METHOD'] = 'SMP'
            self.cic['Project'] = project

        def __export_target(self):
            target_processor = rtconfig.TARGET_PROCESSOR.split()
            self.cic['Target'] = target_processor

        def __export_optimization(self):
            optimization = rtconfig.OPTIMIZATION.split()
            self.cic['Optimization'] = optimization

        def __export_warnings(self):
            warnings = rtconfig.WARNINGS.split()
            self.cic['Warnings'] = warnings

        def __export_debuggings(self):
            debuggings = rtconfig.DEBUGGINGS.split()
            self.cic['Debuggings'] = debuggings

        def __export_postactions(self):
            postactions = rtconfig.POST_ACTION_IDE.split("\n")
            for item in postactions:
                if item.startswith('$TOOLS_PATH'):
                    postactions.remove(item)
                    item = item.replace('$TOOLS_PATH', '${TOOLS_PATH}')
                    postactions.append(item)
            self.cic['PostActions'] = postactions

        def __export_assembly(self):
            assembler = {}
            assembler['preprocessor'] = {}
            assembler['src'] = []
            assembler['inc'] = {}
            assembler['flag'] = rtconfig.ASM_FLAGS

            # 解析预处理
            assembler['preprocessor']['defined'] = self.env.ParseFlags(rtconfig.ASM_PREPROCESSOR)['CPPDEFINES']
            assembler['preprocessor']['undefined'] = \
                str(self.env.ParseFlags(rtconfig.ASM_PREPROCESSOR)['CCFLAGS']).replace('-U', '').split()
            for group in self.project:
                if 'CPPDEFINES' in group and group['CPPDEFINES']:
                    assembler['preprocessor']['defined'].extend(group['CPPDEFINES'])

            # 解析源文件
            src_phy_temp = []
            for group in self.project:
                if 'src' in group and group['src']:
                    for src in group['src']:
                        src = src.rfile().abspath
                        src = __parse_phy_path(self, src)
                        if src.endswith(suffixes['ASM']):
                            src_phy_temp.append(src)

            # 将源文件路径映射成虚拟路径
            src_vit_temp = []
            for src in src_phy_temp:
                src_vit_temp.append(__parse_vir_path(self, src))
            assembler['src'] = src_vit_temp
            assembler['src'].sort()

            # 处理源文件排除构建信息
            if self.exclude:
                files_all = __scan_file(self, patterns['ASM'])
                assembler['exclude'] = list(set(files_all) - set(src_vit_temp))
                assembler['exclude'].sort()

            # 解析头文件路径
            assembler['inc']['include_paths'] = rtconfig.ASM_INCLUDES
            assembler['inc']['include_files'] = []

            inc_temp = []
            for group in self.project:
                if 'CPPPATH' in group and group['CPPPATH']:
                    for inc in group['CPPPATH']:
                        inc = __parse_phy_path(self, inc)
                        if not (inc in inc_temp):
                            inc_temp.append(inc)

            # 将头文件路径映射成虚拟路径
            for inc in inc_temp:
                assembler['inc']['include_paths'].append(__parse_vir_path(self, inc))
            assembler['inc']['include_paths']
            assembler['inc']['include_files']

            # 解析汇编标志位
            for group in self.project:
                if 'ASFLAGS' in group and group['ASFLAGS']:
                    assembler['flag'] = assembler['flag'] + ' ' + group['ASFLAGS']
            assembler['flag'] = assembler['flag'].split()

            self.cic['ASM'] = assembler

        def __export_c(self):
            c = {}
            c['preprocessor'] = {}
            c['src'] = []
            c['inc'] = {}
            c['flag'] = rtconfig.C_FLAGS

            # 解析预处理
            c['preprocessor']['defined'] = self.env.ParseFlags(rtconfig.C_PREPROCESSOR)['CPPDEFINES']
            c['preprocessor']['undefined'] = \
                str(self.env.ParseFlags(rtconfig.C_PREPROCESSOR)['CCFLAGS']).replace('-U', '').split()
            for group in self.project:
                if 'CPPDEFINES' in group and group['CPPDEFINES']:
                    c['preprocessor']['defined'].extend(group['CPPDEFINES'])

            # 解析源文件
            src_phy_temp = []
            for group in self.project:
                if 'src' in group and group['src']:
                    for src in group['src']:
                        src = src.rfile().abspath
                        src = __parse_phy_path(self, src)
                        if src.endswith(suffixes['C']):
                            src_phy_temp.append(src)

            # 将源文件路径映射成虚拟路径
            src_vit_temp = []
            for src in src_phy_temp:
                src_vit_temp.append(__parse_vir_path(self, src))
            c['src'] = src_vit_temp
            c['src'].sort()

            # 处理源文件排除构建信息
            if self.exclude:
                files_all = __scan_file(self, patterns['C'])
                c['exclude'] = list(set(files_all) - set(src_vit_temp))
                c['exclude'].sort()

            # 解析头文件路径
            c['inc']['include_paths'] = rtconfig.C_INCLUDES
            c['inc']['include_files'] = []

            inc_temp = []
            for group in self.project:
                if 'CPPPATH' in group and group['CPPPATH']:
                    for inc in group['CPPPATH']:
                        inc = __parse_phy_path(self, inc)
                        if not (inc in inc_temp):
                            inc_temp.append(inc)

            # 将头文件路径映射成虚拟路径
            for inc in inc_temp:
                c['inc']['include_paths'].append(__parse_vir_path(self, inc))
            c['inc']['include_paths']
            c['inc']['include_files']

            # 解析C标志位
            for group in self.project:
                if 'CCFLAGS' in group and group['CCFLAGS']:
                    c['flag'] = c['flag'] + ' ' + group['CCFLAGS']
            c['flag'] = c['flag'].split()

            self.cic['C'] = c

        def __export_cpp(self):
            cpp = {}
            cpp['preprocessor'] = {}
            cpp['src'] = []
            cpp['inc'] = {}
            cpp['flag'] = rtconfig.CXX_FLAGS

            # 解析预处理
            cpp['preprocessor']['defined'] = self.env.ParseFlags(rtconfig.CXX_PREPROCESSOR)['CPPDEFINES']
            cpp['preprocessor']['undefined'] = \
                str(self.env.ParseFlags(rtconfig.CXX_PREPROCESSOR)['CCFLAGS']).replace('-U', '').split()
            for group in self.project:
                if 'CPPDEFINES' in group and group['CPPDEFINES']:
                    cpp['preprocessor']['defined'].extend(group['CPPDEFINES'])

            # 解析源文件
            src_phy_temp = []
            for group in self.project:
                if 'src' in group and group['src']:
                    for src in group['src']:
                        src = src.rfile().abspath
                        src = __parse_phy_path(self, src)
                        if src.endswith(suffixes['CPP']):
                            src_phy_temp.append(src)

            # 将源文件路径映射成虚拟路径
            src_vit_temp = []
            for src in src_phy_temp:
                src_vit_temp.append(__parse_vir_path(self, src))
            cpp['src'] = src_vit_temp
            cpp['src'].sort()

            # 处理源文件排除构建信息
            if self.exclude:
                files_all = __scan_file(self, patterns['CPP'])
                cpp['exclude'] = list(set(files_all) - set(src_vit_temp))
                cpp['exclude'].sort()

            # 解析头文件路径
            cpp['inc']['include_paths'] = rtconfig.CXX_INCLUDES
            cpp['inc']['include_files'] = []

            inc_temp = []
            for group in self.project:
                if 'CPPPATH' in group and group['CPPPATH']:
                    for inc in group['CPPPATH']:
                        inc = __parse_phy_path(self, inc)
                        if not (inc in inc_temp):
                            inc_temp.append(inc)

            # 将头文件路径映射成虚拟路径
            for inc in inc_temp:
                cpp['inc']['include_paths'].append(__parse_vir_path(self, inc))
            cpp['inc']['include_paths']
            cpp['inc']['include_files']

            # 解析C标志位
            for group in self.project:
                if 'CCFLAGS' in group and group['CCFLAGS']:
                    cpp['flag'] = cpp['flag'] + ' ' + group['CCFLAGS']
            cpp['flag'] = cpp['flag'].split()

            self.cic['CPP'] = cpp

        def __export_link(self):
            link = {}
            link['general'] = []
            # link['libraries'] = rtconfig.LINKER_LIBRARIES
            link['libraries'] = {}
            link['miscellaneous'] = rtconfig.LINKER_MISCELLANEOUS
            link['otherobjs'] = []

            # 处理 -T
            generals = rtconfig.LINKER_GENERAL.replace("-T ", "-T").split()
            generals_temp = []
            for item in generals:
                if item.startswith("-T"):
                    item_new = item[len("-T"):]
                    item_new = __parse_phy_path(self, item_new)
                    item_new = __parse_vir_path(self, item_new)
                    generals_temp.append("-T " + item_new)
                else:
                    generals_temp.append(item)
            link['general'] = generals_temp

            link['libraries']['libraries'] = []
            link['libraries']['library_search_path'] = []

            for group in self.project:
                if 'LIBS' in group and group['LIBS']:
                    link['libraries']['libraries'].extend(group['LIBS'])

            path_temp = []
            for group in self.project:
                if 'LIBPATH' in group and group['LIBPATH']:
                    for path in group['LIBPATH']:
                        path = __parse_phy_path(self, path)
                        path_temp.append(path)

            # 将库搜索路径映射成虚拟路径
            for path in path_temp:
                link['libraries']['library_search_path'].append(__parse_vir_path(self, path))
            link['libraries']['library_search_path']

            # 解析 LINKFLAGS
            for group in self.project:
                if 'LINKFLAGS' in group and group['LINKFLAGS']:
                    link['miscellaneous'] = link['miscellaneous'] + ' ' + group['LINKFLAGS']
            link['miscellaneous'] = link['miscellaneous'].split()

            # 其它 obj 文件
            src_phy_temp = []
            for group in self.project:
                if 'src' in group and group['src']:
                    for src in group['src']:
                        src = src.rfile().abspath
                        src = __parse_phy_path(self, src)
                        if src.endswith('.o'):
                            if src not in src_phy_temp:
                                src_phy_temp.append(src)
            # 将源文件路径映射成虚拟路径
            src_vit_temp = []
            for src in src_phy_temp:
                src_vit_temp.append(__parse_vir_path(self, src))
            link['otherobjs'] = src_vit_temp
            link['otherobjs']

            self.cic['Link'] = link

        def __export_flash_image(self):
            flash_image = {}
            flash_image['general'] = []

            # 处理 -O
            image = rtconfig.FLASH_IMAGE_GENERAL.replace("-O ", "-O").split()
            image_temp = []
            for item in image:
                image_temp.append(item.replace("-O", "-O "))
            flash_image['general'] = image_temp

            self.cic['Image'] = flash_image

        def __export_print_size(self):
            print_size = {}
            print_size['general'] = rtconfig.PRINT_SIZE_GENERAL
            self.cic['Print'] = print_size

        def __export_dt(self):
            import building
            dt = {}
            dt['preprocessor'] = {}
            dt['src'] = []
            dt['inc'] = {}
            dt['flag'] = _split_flags(building.Dtc['DTCFLAGS'])

            # 解析预处理
            dt_cpp_flag = '-E -nostdinc -undef -D__DTS__ -x assembler-with-cpp'
            dt['preprocessor']['flag'] = _split_flags(dt_cpp_flag)

            # 解析源文件
            if building.Dtc['src']:
                for src in building.Dtc['src']:
                    src = src.abspath
            else:
                print("Error: building.Dtc['src'] is empty!")
            # src = building.Dtc['src'].abspath
            src = __parse_vir_path(self,  __parse_phy_path(self, src))
            dt['src'] = src

            # # 解析头文件路径
            dt['inc']['include_paths'] = []
            dt['inc']['include_files'] = []

            inc_temp = []
            for inc in building.Dtc['DTCPATH']:
                inc = __parse_phy_path(self, inc)
                if not (inc in inc_temp):
                    inc_temp.append(inc)

            # 将头文件路径映射成虚拟路径
            for inc in inc_temp:
                dt['inc']['include_paths'].append(__parse_vir_path(self, inc))
            dt['inc']['include_paths']
            dt['inc']['include_files']

            self.cic['DT'] = dt

        __export_parse_linked_resources(self)
        __export_project(self)
        __export_target(self)
        __export_optimization(self)
        __export_warnings(self)
        __export_debuggings(self)
        __export_postactions(self)
        __export_assembly(self)
        __export_c(self)
        __export_cpp(self)
        __export_link(self)
        __export_flash_image(self)
        __export_print_size(self)
        __export_dt(self)

    def __str__(self):
        return str(self.cic)

    def check(self):
        for group in self.project:
            for para in invalid_parameters:
                if para in group and group[para]:
                    print('%s is not exported, group = %s' % (para, group['name']))

def load_cics(path = __CIC_FILENAME__):
    if not os.path.exists(path):
        return {}
    else:
        with open(path, 'r') as file:
            return json.load(file)

def gen_init_cfg(cic):
    init_cfg = {}
    for type, value in suffixes.items():
        init_cfg[type] = cic[type]['src'] + cic[type]['exclude']
    return init_cfg

def load_init_cfg(cics):
    init_cfg = {}
    if cics:
        init_cfg = cics['__init__']
    return init_cfg

def gen_diff_info(init_cfg, cfg, cic):
    diff = {}
    for type, value in suffixes.items():
        diff[type] = {}
        a = init_cfg[type]
        b = cic[type]['src'] + cic[type]['exclude']
        diff[type]['add'] = [x for x in b if x not in a]
        diff[type]['del'] = [x for x in a if x not in b]

    exclude_lists = {}
    path = __EXCLUDE_LIST_FILENAME__
    if os.path.exists(path):
        with open(path, 'r') as file:
            exclude_lists = json.load(file)

        current_exclude_list = None
        for exclude_list in exclude_lists:
            if exclude_list['config_name'] == cfg:
                current_exclude_list = exclude_list
                break

        if current_exclude_list:
            tmp = []
            for exclude_file in current_exclude_list['exclude_files']:
                tmp.append('${BSP_ROOT}/%s' % exclude_file)
            current_exclude_list['exclude_files'] = tmp

            for type, value in suffixes.items():
                type_excludes = [x for x in current_exclude_list['exclude_files']
                               if str(x).endswith(value)]

                new_exclude = [x for x in cic[type]['exclude']
                              if x in type_excludes]

                new_src = [x for x in cic[type]['src']
                         if x not in type_excludes]
                new_src.extend([x for x in diff[type]['add']
                              if x not in type_excludes])

                new_exclude.extend([x for x in cic[type]['exclude']
                                  if x in diff[type]['del'] and x in type_excludes])

                cic[type]['src'] = sorted(list(set(new_src)))
                cic[type]['exclude'] = sorted(list(set(new_exclude)))

    return cic

def save_json(cics):
    cics['__verion__'] = __VERSION__
    str = json.dumps(cics, indent=2, sort_keys=True, separators=(',', ': '))
    with open(__CIC_FILENAME__, 'w') as file:
        file.write(str)

def cic_gen_parameters(parameters = []):
    flags = ''
    for p in parameters:
        if len(p):
            flags = flags + ' ' + p
    return flags

def cic_add_linked_resource(res, phy_path, vir_path):
    resource = {}
    resource['phy_path'] = phy_path
    resource['vir_path'] = vir_path
    res.append(resource)

# scons --target=cic --action=init --cfg=CK810_0 --exclude
# scons --target=cic --action=init --cfg=CK810_1 --exclude
# scons --target=cic --action=update --cfg=CK810_0 --exclude
# scons --target=cic --action=remove --cfg=CK810_0
# scons --target=cic --action=init --cfg=Debug --exclude --cc-path=D:/RTThreadStudio-RK3568/repo/Extract/ToolChain_Support_Packages/ARM/GNU_Tools_for_ARM_Embedded_Processors/5.4.1/bin 
def GenerateCIC(Env, RTT_ROOT, BSP_ROOT):
    cfg = GetOption('cfg')
    action = GetOption('action')
    exclude = GetOption('exclude')

    if not cfg:
        print('The cfg parameter is required')
        exit(1)
    if not action:
        print('The action parameter is required')
        exit(1)

    if action == 'init':
        cics = load_cics()
        item = cic(cfg, exclude, Env, RTT_ROOT, BSP_ROOT)
        item.check()
        cics['__init__'] = gen_init_cfg(item.cic)
        cics[cfg] = item.cic
        save_json(cics)
    elif action == 'update':
        cics = load_cics()
        item = cic(cfg, exclude, Env, RTT_ROOT, BSP_ROOT)
        item.check()
        init_cfg = load_init_cfg(cics)
        item.cic = gen_diff_info(init_cfg, cfg, item.cic)
        cics[cfg] = item.cic
        save_json(cics)
    elif action == 'remove':
        cics = load_cics()
        cics.pop(cfg)
        save_json(cics)

    if os.path.exists('build') and not os.listdir('build'):
        import shutil
        shutil.rmtree('build')
