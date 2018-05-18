#
# Rules to build libarchive_xmlrpc_c.a and export it as a SCons tool
#
import os

tools = Split('''
    boost_serialization
    xmlrpc_client++
''')
env = Environment(tools=['default'] + tools)

tooldir = env.Dir('.').srcnode().abspath    # this directory

# The library and header files will live in this directory.
libDir = tooldir
includeDir = tooldir

sources = Split('''
    Archive_xmlrpc_c.cpp
''')

lib = env.Library('archive_xmlrpc_c', sources)
Default(lib)
    
def archive_xmlrpc_c(env):
    env.Require(tools)
    env.AppendUnique(CPPPATH = [includeDir])
    env.AppendUnique(LIBS = [lib])

Export('archive_xmlrpc_c')
