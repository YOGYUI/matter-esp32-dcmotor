import os
import sys

cur_path = os.path.dirname(os.path.abspath(__file__))  # ({project_path}/scripts
proj_path = os.path.dirname(cur_path)
cfgfilepath = os.path.join(proj_path, 'sdkconfig.defaults')

with open(cfgfilepath, 'r') as fp:
    sdkconfig_lines = fp.readlines()

sdkconfig_lines = [x.replace('\n', '') for x in sdkconfig_lines]
line_find = list(filter(lambda x: 'CONFIG_PARTITION_TABLE_OFFSET' in x, sdkconfig_lines))
if len(line_find) > 0:
    value = line_find[0].split('=')[-1]
    print(value)
