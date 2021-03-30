#!/usr/bin/env python2
'''
This script is for generating VID/PID table from DVB driver modules.
Prepare folder architecture like this and run me:

---- 2.6.32 --- a8293.ko    <- untar kernel 2.6.32 DVB drivers here
 |          |-- adv7170.ko
 |          |-- adv7175.ko
 |          |-- ...
 |
 |-- 3.x ------ a8293.ko    <- untar kernel 3.x DVB drivers here
 |          |-- adv7170.ko
 |          |-- adv7175.ko
 |          |-- ...
 |
 |-- usb.DVB.dep.table           <- dependency table already generated by generate_dependency_table.py
 |-- generate_VIDPID_table.py    <- put me here and run me
 |-- usb.DVB.VIDPID.table        <- this will be generated

'''
import codecs
import glob
import os
import subprocess
import sys

DEPENDENCY_TABLE_PATH = './usb.DVB.dep.table'
VIDPID_TABLE_PATH = './usb.DVB.VIDPID.table'


def parse_dependency_table():
    if not os.path.exists(DEPENDENCY_TABLE_PATH):
        print('ERROR: dependency table file {} not found!'.format(DEPENDENCY_TABLE_PATH))
        sys.exit(1)

    driver_deps = {}
    with codecs.open(DEPENDENCY_TABLE_PATH, 'r', 'ascii') as f:
        for line in f:
            if line.startswith('#'):
                continue
            driver, deps = line.strip().split(':')
            deps = deps.split(' ')
            driver_deps[driver] = deps
    return driver_deps


driver_deps = parse_dependency_table()


def dongle_is_buggy(kernel_version, vid, pid):
    # dongles known buggy and we don't want to support them
    buggy_dongle_list = [
        ('2.6.32', '0b05', '1736'),  # ASUS My Cinema - U3000 Hybrid, always call trace on 2.6.32 <DSM> #64262
        ('2.6.32', '1b24', '4001'),  # Telegent TLG2300, using poseidon driver and always call trace <Video Station> #1573
        ('3.x', '1b24', '4001'),  # Telegent TLG2300, using poseidon driver and always call trace
    ]
    return (kernel_version, vid, pid) in buggy_dongle_list


def extract_vidpid_list(filepath, kernel_version):
    assert(filepath.startswith('./'))
    assert(filepath.endswith('.ko'))
    driver = filepath[2:-3]
    table = []

    try:
        matched_lines = subprocess.check_output(['modinfo {} | grep "^alias: *usb"'.format(filepath)], shell=True, universal_newlines=True).split('\n')
    except subprocess.CalledProcessError:
        return []
    matched_lines = [line.lower() for line in matched_lines if line]

    for line in matched_lines:
        # example: 'alias:          usb:v0A6Fp0400d*dc*dsc*dp*ic*isc*ip*in*'
        #           012345678901234567890123456789
        #                                VID  PID
        vid = line[21:25]
        pid = line[26:30]
        if dongle_is_buggy(kernel_version, vid, pid):
            continue
        entry = (driver, vid, pid)
        table.append(entry)
    return sorted(table)


def fix_missing_vidpid(vidpid_table):
    missing_entries = []
    for (driver, vid, pid) in vidpid_table:
        if driver == 'smsdvb':
            return
        if driver == 'smsusb':
            missing_entries.append(('smsdvb', vid, pid))
    vidpid_table.extend(missing_entries)


def expand_vidpid_from_parent_modules(vidpid_table):
    '''
    For example, if we have a (already expanded) dependency chain like this:
    A:B C D A
    Then we will copy all VID/PID entries found in B, C, D to A
    and remove all B, C, D entries
    '''
    def driver_vidpid_list(driver):
        '''return the list of all (VID, PID) for a given driver'''
        return [(driver, vid, pid) for (d, vid, pid) in vidpid_table if d == driver]

    copied_vidpid_table = []
    table = list(vidpid_table)

    for driver, deps in driver_deps.items():
        children_vidpid_table = []
        for dep in deps[:-1]:  # the last one dependency must be itself
            children_vidpid_table.extend(driver_vidpid_list(dep))
        table = [entry for entry in table if entry not in children_vidpid_table]
        for (_, pid, vid) in children_vidpid_table:
            copied_vidpid_table.append((driver, pid, vid))

    table.extend(copied_vidpid_table)
    return sorted(table)


def write_vidpid_table(vidpid_table):
    vidpid_table.sort()
    recorded_entries = set()  # for dedup
    with codecs.open(VIDPID_TABLE_PATH, 'a', 'ascii') as f:
        for (driver, vid, pid) in vidpid_table:
            # ignore *-alsa modules
            if driver.endswith('-alsa'):
                continue
            if (driver, vid, pid) in recorded_entries:
                continue
            else:
                recorded_entries.add((driver, vid, pid))

            line = '(0x{}:0x{},{})\n'.format(vid, pid, driver)
            f.write(line)


def write_line(line):
    with codecs.open(VIDPID_TABLE_PATH, 'a', 'ascii') as f:
        f.write(line + '\n')


def main():
    if os.path.exists(VIDPID_TABLE_PATH):
        os.remove(VIDPID_TABLE_PATH)

    #for kernel_version in ('2.6.32', '3.x', '3.10.x'):
    for kernel_version in ('4.4.x',):
        folder = './' + kernel_version
        if not os.path.isdir(folder):
            print('ERROR: folder {} not exists!'.format(folder))
            sys.exit(1)

        print('Extracting kernel {} dvb VID/PID list...'.format(kernel_version))

        vidpid_table = []
        write_line('#Kernel {} - VIDPID - start'.format(kernel_version))
        os.chdir(folder)

        for filepath in sorted(glob.glob('./*.ko')):
            vidpid_table.extend(extract_vidpid_list(filepath, kernel_version))
        vidpid_table = expand_vidpid_from_parent_modules(vidpid_table)
        #fix_missing_vidpid(vidpid_table)

        os.chdir('..')
        write_vidpid_table(vidpid_table)
        write_line('#Kernel {} - VIDPID - end'.format(kernel_version))

    print('VID/PID table generated as: ' + VIDPID_TABLE_PATH)


if __name__ == '__main__':
    main()