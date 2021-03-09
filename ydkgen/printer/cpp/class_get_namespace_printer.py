# Copyright 2021 Siklu

from ydkgen.api_model import Package


class GetNamespacePrinter(object):
    def __init__(self, ctx):
        self.ctx = ctx

    def print_output(self, clazz):
        namespace = None
        if clazz.owner is not None and isinstance(clazz.owner, Package):
            package = clazz.owner
            namespace_stmt = package.stmt.search_one('namespace')
            if namespace_stmt:
                namespace = namespace_stmt.arg
        self.ctx.writeln('const std::string %s::get_namespace() const' % clazz.qualified_cpp_name())
        self.ctx.writeln('{')
        self.ctx.lvl_inc()
        if namespace is not None:
            self.ctx.writeln('return "%s";' % namespace)
        else:
            self.ctx.writeln('return "";')
        self.ctx.lvl_dec()
        self.ctx.writeln('}')
        self.ctx.bline()