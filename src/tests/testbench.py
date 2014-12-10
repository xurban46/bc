#!/usr/bin/env python
# -*- coding: utf-8 -*-
#
# Colearning in Coevolutionary Algorithms
# Bc. Michal Wiglasz <xwigla00@stud.fit.vutbr.cz>
#
# Master Thesis
# 2014/2015
#
# Supervisor: Ing. Michaela Šikulová <isikulova@fit.vutbr.cz>
#
# Faculty of Information Technologies
# Brno University of Technology
# http://www.fit.vutbr.cz/
#
# Started on 28/07/2014.
#      _       _
#   __(.)=   =(.)__
#   \___)     (___/


from __future__ import print_function


import subprocess
import argparse
import tempfile
import operator
import difflib
import string
import sys
import os
import re


RE_COMMENT = r'\s*/\*\*(.+?)\*/'
RE_FLAG = r'Compile with'


def green(s):
    return '\033[42m' + s + '\033[0m'


def yellow(s):
    return '\033[43m' + s + '\033[0m'


def red(s):
    return '\033[41m\033[37m' + s + '\033[0m'


def split_and_filter_cflags(line):
    return (
        f
        for f in re.split('[\s*,]', line.strip('\\' + string.whitespace))
        if f != '-DAVX2'
    )


def split_and_filter_libs(line):
    return re.split('[\s*,]', line.strip('\\' + string.whitespace))


def split_and_filter_sources(line):
    return (
        '../' + f
        for f in re.split('[\s*,]', line.strip('\\' + string.whitespace))
        if f != 'main.c'
    )


def parse_makefile(mkfile):
    compiler = 'gcc'
    flags = []
    libs = []
    sources = ['./testbench_mock.c']

    with open(mkfile, 'rt') as fp:
        for line in fp:
            line = line.strip()

            if not line:
                continue

            elif line.startswith('CC='):
                compiler = line[3:].strip()

            elif line.startswith('CFLAGS='):
                flags.extend(split_and_filter_cflags(line[7:]))
                while line[-1] == '\\':
                    line = fp.next()
                    flags.extend(split_and_filter_cflags(line))

            elif line.startswith('SOURCES='):
                sources.extend(split_and_filter_sources(line[8:]))
                while line[-1] == '\\':
                    line = fp.next()
                    sources.extend(split_and_filter_sources(line))

            elif line.startswith('LIBS='):
                libs.extend(split_and_filter_libs(line[5:]))
                while line[-1] == '\\':
                    line = fp.next()
                    libs.extend(split_and_filter_libs(line))

    return compiler, flags, sources, libs


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('testfiles', metavar='TEST', type=str, nargs='+',
                        help='Test source (*.c) file')
    parser.add_argument('--novalgrind', action='store_false',
                        help='Do not use valgrind')
    parser.add_argument('--verbose', '-v', action='store_true',
                        help='Do not use valgrind')
    parser.add_argument('--ignore-gcc-warnings', '-w', action='store_true',
                        help='Ignore GCC warnings')
    parser.add_argument('--stop', '-s', action='store_true',
                        help='Stop on first failure')
    args = parser.parse_args()
    verbose = args.verbose
    stop_on_failure = args.stop
    ignore_gcc_warnings = args.ignore_gcc_warnings

    gcc, gcc_flags, gcc_sources, gcc_libs = parse_makefile('../Makefile')

    for testfile in args.testfiles:

        if verbose:
            print(yellow('Test %s' % (testfile)))

        if not os.path.isfile(testfile):
            print(red("Skipping non-existing file %s" % (testfile,)))
            continue

        with open(testfile, 'rt') as fp:
            content = fp.read()

            # grab first comment
            m = re.match(RE_COMMENT, content, re.I + re.U + re.S)
            if not m:
                print(red("File %s does not contain header" % (testfile,)))
                if stop_on_failure:
                    return
                else:
                    continue

            init_comment = m.group(1)
            lines = map(str.strip, re.split(r'\s*\n\s*\*\s', init_comment))

            description = []
            flags = []
            use_diff = True

            for line in lines:
                if line.startswith('Compile with'):
                    flags.extend(re.split('[\s*,]', line[13:]))

                if line.startswith('Source files'):
                    gcc_sources = list(split_and_filter_sources(line[13:]))

                elif line.startswith('"Stand-alone" test executable'):
                    use_diff = False

                else:
                    description.append(line)

        # compile

        try:
            binary = tempfile.NamedTemporaryFile(prefix='cocotest_',
                                                 delete=False)
            binary.close()

            args = gcc_flags + flags + gcc_sources + [testfile, '-o', binary.name] + gcc_libs

            if verbose:
                print('Compiling %s to %s' % (testfile, binary.name))
                print('GCC command:')
                print('%s %s' % (gcc, ' '.join(args)))

            p = subprocess.Popen([gcc] + args,
                                 stdout=subprocess.PIPE,
                                 stderr=subprocess.PIPE)
            out, err = p.communicate()
            gcc_failed = False
            if p.returncode != 0:
                print(red('Test %s failed to compile (retval = %u)' % (testfile, p.returncode)))
                gcc_failed = True

            if err != 0:
                if p.returncode == 0 and ignore_gcc_warnings:
                    print(yellow('Test %s compiled with warnings' % (testfile)))
                else:
                    print(red('Test %s failed to compile (stderr non-empty)' % (testfile)))
                    gcc_failed = True
                print(err, file=sys.stderr)

            if gcc_failed:
                if stop_on_failure:
                    return
                else:
                    continue

            # run

            if verbose:
                print('Running %s' % (binary.name,))
            p = subprocess.Popen([binary.name],
                                 stdout=subprocess.PIPE,
                                 stderr=subprocess.PIPE)
            out, err = p.communicate()

            if err:
                print(red('Test %s failed (stderr non-empty)' % (testfile)))

            if p.returncode != 0:
                print(red('Test %s failed (retval = %u)' % (testfile, p.returncode)))

            if err or p.returncode != 0:
                print(err, file=sys.stderr)
                if stop_on_failure:
                    return
                else:
                    continue

            if use_diff:
                with open(re.sub('.c$', '.out', testfile), 'rt') as fp:
                    reference_out = fp.read()

                result = list(difflib.unified_diff(
                    reference_out.splitlines(1), out.splitlines(1)
                ))
                if len(result):
                    print(red('Test %s failed (diff mismatch)' % (testfile)))
                    if verbose:
                        print(''.join(result), file=sys.stderr)

                    if stop_on_failure:
                        return
                    else:
                        continue

            print(green('Test %s OK' % (testfile)))

        finally:
            try:
                os.unlink(binary.name)
            except OSError as e:
                if e.errno != 2:
                    raise


if __name__ == '__main__':
    main()
