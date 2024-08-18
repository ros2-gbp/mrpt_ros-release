#!/usr/bin/env python3
#
#  This file is a modification of the `mkdoc.py` script in pybind11:
#      - File: https://github.com/pybind/pybind11/blob/master/tools/mkdoc.py
#      - License: https://github.com/pybind/pybind11/blob/master/LICENSE
#
#  Where it differs:
#  - process_comment: added substitutions.  cheap hack: turn all rst into markdown ```
#        - Not necessarily ideal, but at least doesn't break the tables
#
#        s = re.sub(r'\\class\s+%s.*' % cpp_group, '', s)   # removes \class Thing file.h nanogui/file.h
#        s = re.sub(r'\\struct\s+%s.*' % cpp_group, '', s)  # removes \struct Thing file.h nanogui/file.h
#        s = re.sub(r'\\rst', r'```\n\n', s)
#        s = re.sub(r'\\endrst', r'```\n\n', s)
#        s = re.sub(r'.. note::', r'Note:', s)
#        s = re.sub(r'.. warning::', r'Warning:', s)
#        s = re.sub(r'.. danger::', r'Danger:', s)
#        s = re.sub(r'.. code-block::\s*\w*', r'', s)
#
#  - process_comment: near the end, have to treat .. code-block:: specially
#
#        for line in x.splitlines():
#            if len(line) < 4:
#                result += line.strip()
#            else:
#                # this is a .. code-block:: indentation (three spaces)
#                if line.startswith('   ') and line[3] != ' ':
#                    result += line[3:].strip() + '\n'
#                else:
#                    result += line.strip() + '\n'
#
#  Syntax: mkdoc.py [-I<path> ..] [.. a list of header files ..]
#
#  Extract documentation from C++ header files to use it in Python bindings
#

import os
import sys
import platform
import re
import textwrap

# CMake generated target is explicitly calling `python3`, this sanity check is
# to support the fact that some `pip3` installable `clang` libraries are
# packaged for python3, but only actually work in python2.
if sys.version[0] != "3":
    raise RuntimeError("`mkdoc_rst.py` must be run with Python3.")


# If user has `clang` already installed, try and use that.  There are a couple
# of unofficial options for python **3**, two that seem to work are `clang-5`
# and `libclang-py3`.  It likely depends on what you have installed on your
# system, so your mileage will very likely vary.
try:
    from clang import cindex
    from clang.cindex import CursorKind
except:
    # Fallback on pybind11
    # This file is tailored to the NanoGUI documentation build system, the clang
    # module required is present in a full recursive clone here.
    base_path = os.path.dirname(os.path.dirname(os.path.realpath(__file__)))
    clang_parent_folder = os.path.join(base_path, "ext/pybind11/tools/")
    if not os.path.isdir(clang_parent_folder):
        raise RuntimeError(
             "The NanoGUI dependencies repository (pybind11, etc.) appear to be missing!\n"
             "You probably did not clone the project with --recursive. It is possible to recover\n"
             "by calling 'git submodule update --init --recursive'"
        )
    else:
        sys.path.insert(0, clang_parent_folder)

    # Now we can import clang
    from clang import cindex
    from clang.cindex import CursorKind

from collections import OrderedDict
from threading import Thread, Semaphore
from multiprocessing import cpu_count

RECURSE_LIST = [
    CursorKind.TRANSLATION_UNIT,
    CursorKind.NAMESPACE,
    CursorKind.CLASS_DECL,
    CursorKind.STRUCT_DECL,
    CursorKind.ENUM_DECL,
    CursorKind.CLASS_TEMPLATE
]

PRINT_LIST = [
    CursorKind.CLASS_DECL,
    CursorKind.STRUCT_DECL,
    CursorKind.ENUM_DECL,
    CursorKind.ENUM_CONSTANT_DECL,
    CursorKind.CLASS_TEMPLATE,
    CursorKind.FUNCTION_DECL,
    CursorKind.FUNCTION_TEMPLATE,
    CursorKind.CONVERSION_FUNCTION,
    CursorKind.CXX_METHOD,
    CursorKind.CONSTRUCTOR,
    CursorKind.FIELD_DECL
]

CPP_OPERATORS = {
    '<=': 'le', '>=': 'ge', '==': 'eq', '!=': 'ne', '[]': 'array',
    '+=': 'iadd', '-=': 'isub', '*=': 'imul', '/=': 'idiv', '%=':
    'imod', '&=': 'iand', '|=': 'ior', '^=': 'ixor', '<<=': 'ilshift',
    '>>=': 'irshift', '++': 'inc', '--': 'dec', '<<': 'lshift', '>>':
    'rshift', '&&': 'land', '||': 'lor', '!': 'lnot', '~': 'bnot',
    '&': 'band', '|': 'bor', '+': 'add', '-': 'sub', '*': 'mul', '/':
    'div', '%': 'mod', '<': 'lt', '>': 'gt', '=': 'assign', '()': 'call'
}

CPP_OPERATORS = OrderedDict(
    sorted(CPP_OPERATORS.items(), key=lambda t: -len(t[0])))

job_count = cpu_count()
job_semaphore = Semaphore(job_count)

registered_names = dict()


def d(s):
    if type(s) is bytes:
        return s.decode('utf8')
    elif type(s) is str:
        return s
    else:
        raise RuntimeError(
            "d(s): `s` cannot be decoded, it's type is: {0}".format(type(s))
        )


def sanitize_name(name):
    global registered_names
    name = re.sub(r'type-parameter-0-([0-9]+)', r'T\1', name)
    for k, v in CPP_OPERATORS.items():
        name = name.replace('operator%s' % k, 'operator_%s' % v)
    name = re.sub('<.*>', '', name)
    name = ''.join([ch if ch.isalnum() else '_' for ch in name])
    name = re.sub('_$', '', re.sub('_+', '_', name))
    if name in registered_names:
        registered_names[name] += 1
        name += '_' + str(registered_names[name])
    else:
        registered_names[name] = 1
    return '__doc_' + name


def process_comment(comment):
    result = ''

    # Remove C++ comment syntax
    leading_spaces = float('inf')
    for s in comment.expandtabs(tabsize=4).splitlines():
        s = s.strip()
        if s.startswith('/*'):
            s = s[2:].lstrip('*')
        elif s.endswith('*/'):
            s = s[:-2].rstrip('*')
        elif s.startswith('///<'):
            s = s[4:]
        elif s.startswith('///'):
            s = s[3:]
        if s.startswith('*'):
            s = s[1:]
        if len(s) > 0:
            leading_spaces = min(leading_spaces, len(s) - len(s.lstrip()))
        result += s + '\n'

    if leading_spaces != float('inf'):
        result2 = ""
        for s in result.splitlines():
            result2 += s[leading_spaces:] + '\n'
        result = result2

    # Doxygen tags
    cpp_group = '([\w:]+)'
    param_group = '([\[\w:\]]+)'

    s = result

    s = re.sub(r'\\class\s+%s.*' % cpp_group, '', s)   # removes \class Thing file.h nanogui/file.h
    s = re.sub(r'\\struct\s+%s.*' % cpp_group, '', s)  # removes \struct Thing file.h nanogui/file.h
    s = re.sub(r'\\rst', r'```\n\n', s)
    s = re.sub(r'\\endrst', r'```\n\n', s)
    s = re.sub(r'.. note::', r'Note:', s)
    s = re.sub(r'.. warning::', r'Warning:', s)
    s = re.sub(r'.. danger::', r'Danger:', s)
    s = re.sub(r'.. code-block::\s*\w*', r'', s)

    s = re.sub(r'\\c\s+%s' % cpp_group, r'``\1``', s)
    s = re.sub(r'\\a\s+%s' % cpp_group, r'*\1*', s)
    s = re.sub(r'\\e\s+%s' % cpp_group, r'*\1*', s)
    s = re.sub(r'\\em\s+%s' % cpp_group, r'*\1*', s)
    s = re.sub(r'\\b\s+%s' % cpp_group, r'**\1**', s)
    s = re.sub(r'\\ingroup\s+%s' % cpp_group, r'', s)
    s = re.sub(r'\\param%s?\s+%s' % (param_group, cpp_group),
               r'\n\n$Parameter ``\2``:\n\n', s)
    s = re.sub(r'\\tparam%s?\s+%s' % (param_group, cpp_group),
               r'\n\n$Template parameter ``\2``:\n\n', s)

    for in_, out_ in {
        'return': 'Returns',
        'author': 'Author',
        'authors': 'Authors',
        'copyright': 'Copyright',
        'date': 'Date',
        'remark': 'Remark',
        'sa': 'See also',
        'see': 'See also',
        'extends': 'Extends',
        'throws': 'Throws',
        'throw': 'Throws'
    }.items():
        s = re.sub(r'\\%s\s*' % in_, r'\n\n$%s:\n\n' % out_, s)

    s = re.sub(r'\\details\s*', r'\n\n', s)
    s = re.sub(r'\\brief\s*', r'', s)
    s = re.sub(r'\\short\s*', r'', s)
    s = re.sub(r'\\ref\s*', r'', s)

    s = re.sub(r'\\code\s?(.*?)\s?\\endcode',
               r"```\n\1\n```\n", s, flags=re.DOTALL)

    # HTML/TeX tags
    s = re.sub(r'<tt>(.*?)</tt>', r'``\1``', s, flags=re.DOTALL)
    s = re.sub(r'<pre>(.*?)</pre>', r"```\n\1\n```\n", s, flags=re.DOTALL)
    s = re.sub(r'<em>(.*?)</em>', r'*\1*', s, flags=re.DOTALL)
    s = re.sub(r'<b>(.*?)</b>', r'**\1**', s, flags=re.DOTALL)
    s = re.sub(r'\\f\$(.*?)\\f\$', r'$\1$', s, flags=re.DOTALL)
    s = re.sub(r'<li>', r'\n\n* ', s)
    s = re.sub(r'</?ul>', r'', s)
    s = re.sub(r'</li>', r'\n\n', s)

    s = s.replace('``true``', '``True``')
    s = s.replace('``false``', '``False``')

    # Re-flow text
    wrapper = textwrap.TextWrapper()
    wrapper.expand_tabs = True
    wrapper.replace_whitespace = True
    wrapper.drop_whitespace = True
    wrapper.width = 70
    wrapper.initial_indent = wrapper.subsequent_indent = ''

    result = ''
    in_code_segment = False
    for x in re.split(r'(```)', s):
        if x == '```':
            if not in_code_segment:
                result += '```\n'
            else:
                result += '\n```\n\n'
            in_code_segment = not in_code_segment
        elif in_code_segment:
            for line in x.splitlines():
                if len(line) < 4:
                    result += line.strip()
                else:
                    # this is a .. code-block:: indentation (three spaces),
                    # strip leading three spaces, preserve remaining indentation
                    if line.startswith('   '):
                        result += line[3:].rstrip() + '\n'
                    else:
                        result += line.strip() + '\n'
        else:
            for y in re.split(r'(?: *\n *){2,}', x):
                wrapped = wrapper.fill(re.sub(r'\s+', ' ', y).strip())
                if len(wrapped) > 0 and wrapped[0] == '$':
                    result += wrapped[1:] + '\n'
                    wrapper.initial_indent = \
                        wrapper.subsequent_indent = ' ' * 4
                else:
                    if len(wrapped) > 0:
                        result += wrapped + '\n\n'
                    wrapper.initial_indent = wrapper.subsequent_indent = ''
    return result.rstrip().lstrip('\n')


def extract(filename, node, prefix, output):
    num_extracted = 0
    if not (node.location.file is None or
            os.path.samefile(d(node.location.file.name), filename)):
        return 0
    if node.kind in RECURSE_LIST:
        sub_prefix = prefix
        if node.kind != CursorKind.TRANSLATION_UNIT:
            if len(sub_prefix) > 0:
                sub_prefix += '_'
            sub_prefix += d(node.spelling)
        for i in node.get_children():
            num_extracted += extract(filename, i, sub_prefix, output)
        if num_extracted == 0:
            return 0
    if node.kind in PRINT_LIST:
        comment = d(node.raw_comment) if node.raw_comment is not None else ''
        comment = process_comment(comment)
        sub_prefix = prefix
        if len(sub_prefix) > 0:
            sub_prefix += '_'
        if len(node.spelling) > 0:
            name = sanitize_name(sub_prefix + d(node.spelling))
            output.append('\nstatic const char *%s =%sR"doc(%s)doc";' %
                (name, '\n' if '\n' in comment else ' ', comment))
            num_extracted += 1
    return num_extracted


class ExtractionThread(Thread):
    def __init__(self, filename, parameters, output):
        Thread.__init__(self)
        self.filename = filename
        self.parameters = parameters
        self.output = output
        job_semaphore.acquire()

    def run(self):
        print('Processing "%s" ..' % self.filename, file=sys.stderr)
        try:
            index = cindex.Index(
                cindex.conf.lib.clang_createIndex(False, True))
            tu = index.parse(self.filename, self.parameters)
            extract(self.filename, tu.cursor, '', self.output)
        finally:
            job_semaphore.release()

if __name__ == '__main__':
    parameters = ['-x', 'c++', '-std=c++11']
    filenames = []

    if platform.system() == 'Darwin':
        dev_path = '/Applications/Xcode.app/Contents/Developer/'
        lib_dir = dev_path + 'Toolchains/XcodeDefault.xctoolchain/usr/lib/'
        sdk_dir = dev_path + 'Platforms/MacOSX.platform/Developer/SDKs'
        libclang = lib_dir + 'libclang.dylib'

        if os.path.exists(libclang):
            cindex.Config.set_library_path(os.path.dirname(libclang))

        if os.path.exists(sdk_dir):
            sysroot_dir = os.path.join(sdk_dir, next(os.walk(sdk_dir))[1][0])
            parameters.append('-isysroot')
            parameters.append(sysroot_dir)

    for item in sys.argv[1:]:
        if item.startswith('-'):
            parameters.append(item)
        else:
            filenames.append(item)

    if len(filenames) == 0:
        print('Syntax: %s [.. a list of header files ..]' % sys.argv[0])
        exit(-1)

    print('''/*
  This file contains docstrings for the Python bindings.
  Do not edit! These were automatically extracted by mkdoc.py
 */

#define __EXPAND(x)                                      x
#define __COUNT(_1, _2, _3, _4, _5, _6, _7, COUNT, ...)  COUNT
#define __VA_SIZE(...)                                   __EXPAND(__COUNT(__VA_ARGS__, 7, 6, 5, 4, 3, 2, 1))
#define __CAT1(a, b)                                     a ## b
#define __CAT2(a, b)                                     __CAT1(a, b)
#define __DOC1(n1)                                       __doc_##n1
#define __DOC2(n1, n2)                                   __doc_##n1##_##n2
#define __DOC3(n1, n2, n3)                               __doc_##n1##_##n2##_##n3
#define __DOC4(n1, n2, n3, n4)                           __doc_##n1##_##n2##_##n3##_##n4
#define __DOC5(n1, n2, n3, n4, n5)                       __doc_##n1##_##n2##_##n3##_##n4##_##n5
#define __DOC6(n1, n2, n3, n4, n5, n6)                   __doc_##n1##_##n2##_##n3##_##n4##_##n5##_##n6
#define __DOC7(n1, n2, n3, n4, n5, n6, n7)               __doc_##n1##_##n2##_##n3##_##n4##_##n5##_##n6##_##n7
#define DOC(...)                                         __EXPAND(__EXPAND(__CAT2(__DOC, __VA_SIZE(__VA_ARGS__)))(__VA_ARGS__))

#if defined(__GNUG__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-variable"
#endif
''')

    output = []
    for filename in filenames:
        thr = ExtractionThread(filename, parameters, output)
        thr.start()

    print('Waiting for jobs to finish ..', file=sys.stderr)
    for i in range(job_count):
        job_semaphore.acquire()

    output.sort()
    for l in output:
        print(l)

    print('''
#if defined(__GNUG__)
#pragma GCC diagnostic pop
#endif
''')
