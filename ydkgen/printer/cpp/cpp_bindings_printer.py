#  ----------------------------------------------------------------
# Copyright 2016-2019 Cisco Systems
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#    http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
# ------------------------------------------------------------------
# This file has been modified by Yan Gorelik, YDK Solutions.
# All modifications in original under CiscoDevNet domain
# introduced since October 2019 are copyrighted.
# All rights reserved under Apache License, Version 2.0.
# ------------------------------------------------------------------

"""
   YDK PY converter
"""

from __future__ import print_function

import os

from ydkgen.api_model import Class, Enum
from ydkgen.builder import MultiFileBuilder, MultiFileHeader, MultiFileSource
from ydkgen.common import get_rst_file_name
from ydkgen.printer.language_bindings_printer import LanguageBindingsPrinter, _EmitArgs

from .header_printer import HeaderPrinter
from .source_printer import SourcePrinter
from .test_case_cmake_file_printer import CMakeListsPrinter
from ..tests import TestPrinter


class CppBindingsPrinter(LanguageBindingsPrinter):

    def __init__(self, ydk_root_dir, bundle, generate_tests, one_class_per_module):
        super().__init__(ydk_root_dir, bundle, generate_tests, one_class_per_module)
        self.source_files = []
        self.header_files = []

    def print_files(self):
        only_modules = [package.stmt for package in self.packages]
        size = len(only_modules)

        for index, package in enumerate(self.packages):
            self._print_module(index, package, size)

        if self.generate_tests:
            self._print_cmake_file(self.packages, self.bundle_name, self.test_dir)
        return (self.source_files, self.header_files)

    def _print_module(self, index, package, size):
        print('Processing %d of %d %s' % (index + 1, size, package.stmt.pos.ref))
        # Skip generating module for empty modules
        if len(package.owned_elements) == 0:
            return
        builder = MultiFileBuilder(package, self.classes_per_source_file)
        self._print_header_file(package, builder.multi_file_data, self.models_dir)
        self._print_source_file(package, builder.multi_file_data, self.models_dir)
        if self.generate_tests:
            self._print_tests(package, self.test_dir)

    def _print_header_file(self, package, multi_file_data, path):
        hp = HeaderPrinter(self.ypy_ctx,
                           self.identity_subclasses, self.bundle_name)
        for multi_file_header in [x for x in multi_file_data.multi_file_list if isinstance(x, MultiFileHeader)]:
            hp.print_output(
                            package,
                            multi_file_header,
                            path
                            )
            if not multi_file_header.fragmented:
                self.header_files.append(multi_file_header.file_name)

    def _print_source_file(self, package, multi_file_data, path):
        sp = SourcePrinter(self.ypy_ctx, self.bundle_name, self.module_namespace_lookup)
        for multi_file_source in [x for x in multi_file_data.multi_file_list if isinstance(x, MultiFileSource)]:
            sp.print_output(
                            package,
                            multi_file_source,
                            path
                            )
            file_name = multi_file_source.file_name
            if multi_file_source.fragmented:
                file_name = os.path.join('fragmented', file_name)
            self.source_files.append(file_name)

    def _print_tests(self, package, path):
        empty = self.is_empty_package(package)
        if not empty:
            self.print_file(get_testcase_file_name(path, package),
                            emit_test_cases,
                            _EmitArgs(self.ypy_ctx, package, self.identity_subclasses))

    def _print_cmake_file(self, packages, bundle_name, path):
        args = {'bundle_name': bundle_name,
                'identity_subclasses': self.identity_subclasses}
        self.print_file(get_testcase_cmake_file_name(path),
                        emit_cmake_file,
                        _EmitArgs(self.ypy_ctx, packages, args))


def get_tests_dir(path):
    return os.path.join(path, 'test')


def get_testcase_file_name(path, package):
    return '%s/test_%s.cpp' % (path, package.stmt.arg.replace('-', '_'))


def get_testcase_cmake_file_name(path):
    return '%s/CMakeLists.txt' % path


def emit_header(ctx, package, extra_args):
    HeaderPrinter(ctx, extra_args[0], extra_args[1]).print_output(package)


def emit_test_cases(ctx, package, identity_subclasses):
    TestPrinter(ctx, 'cpp').print_tests(package, identity_subclasses)


def emit_cmake_file(ctx, packages, args):
    CMakeListsPrinter(ctx).print_cmakelists_file(packages, args)
