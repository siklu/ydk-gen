# Copyright 2021 Siklu

from ydkgen.common import get_class_namespace


class GetNamespacePrinter(object):
    def __init__(self, ctx):
        self.ctx = ctx

    def print_output(self, clazz):
        namespace = get_class_namespace(clazz)
        self.ctx.writeln('std::string %s::get_namespace() const' %
                         clazz.qualified_cpp_name())
        self.ctx.writeln('{')
        self.ctx.lvl_inc()
        if namespace is not None:
            self.ctx.writeln('return "%s";' % namespace)
        else:
            self.ctx.writeln('return "";')
        self.ctx.lvl_dec()
        self.ctx.writeln('}')
        self.ctx.bline()
