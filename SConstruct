# -*- coding: utf-8 -*-
#-------------------------------------------------------------------------#
#   Copyright (C) 2018 by Christoph Thelen                                #
#   doc_bacardi@users.sourceforge.net                                     #
#                                                                         #
#   This program is free software; you can redistribute it and/or modify  #
#   it under the terms of the GNU General Public License as published by  #
#   the Free Software Foundation; either version 2 of the License, or     #
#   (at your option) any later version.                                   #
#                                                                         #
#   This program is distributed in the hope that it will be useful,       #
#   but WITHOUT ANY WARRANTY; without even the implied warranty of        #
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         #
#   GNU General Public License for more details.                          #
#                                                                         #
#   You should have received a copy of the GNU General Public License     #
#   along with this program; if not, write to the                         #
#   Free Software Foundation, Inc.,                                       #
#   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             #
#-------------------------------------------------------------------------#

import os.path


#----------------------------------------------------------------------------
#
# Set up the Muhkuh Build System.
#
SConscript('mbs/SConscript')
Import('atEnv')


#----------------------------------------------------------------------------
#
# Create the compiler environments.
#

# Create a build environment for the Cortex-M4 based netX chips.
env_cortexM4 = atEnv.DEFAULT.CreateEnvironment(['gcc-arm-none-eabi-4.9', 'asciidoc'])
env_cortexM4.CreateCompilerEnv('NETX90', ['arch=armv7', 'thumb'], ['arch=armv7e-m', 'thumb'])

env_xpic_default = atEnv.DEFAULT.Clone()
tLlvmXpic = atEnv.DEFAULT.GetTool('xpic-toolchain-1.0.2_1')
tLlvmXpic.ApplyToEnv(env_xpic_default)
env_xpic_default.Append(CPPDEFINES = [['ASIC_TYP', 'ASIC_TYP_NETX90']])
env_xpic_default.Append(CCFLAGS = [['-ffreestanding']])
env_xpic_default.Replace(LIBS = [])


#----------------------------------------------------------------------------
#
# Build the platform library.
#
SConscript('platform/SConscript')


# ----------------------------------------------------------------------------
#
# Get the source code version from the VCS.
#
atEnv.DEFAULT.Version('targets/version/version.h', 'templates/version.h')


# ----------------------------------------------------------------------------
#
# Build the XPIC binary.
#

sources_xpic = """
    src/xpic_main.c
"""

tEnvXpic = env_xpic_default.Clone()
tEnvXpic.Replace(LDFILE = 'src/netx90/xpic.ld')
tEnvXpic.Append(CPPPATH = ['#platform/src'])
tSrcXpic = tEnvXpic.SetBuildPath('targets/netx90/xpic/', 'src', sources_xpic)
tElfXpic = tEnvXpic.Elf('targets/netx90/xpic/xpic.elf', tSrcXpic)
tTxtXpic = tEnvXpic.ObjDump('targets/netx90/xpic/xpic.txt', tElfXpic, OBJDUMP_FLAGS=['--disassemble-all', '--disassemble', '--source', '--all-headers', '--wide'])
tBinXpicPram = tEnvXpic.ObjCopy('targets/netx90/xpic/xpic_pram.bin', tElfXpic, OBJCOPYFLAGS=['-j', '.pram', '-O', 'binary'])
tBinXpicDram = tEnvXpic.ObjCopy('targets/netx90/xpic/xpic_dram.bin', tElfXpic, OBJCOPYFLAGS=['-j', '.dram', '-O', 'binary'])


# ----------------------------------------------------------------------------
#
# Build the netX binary.
#

sources_common = """
    src/header.c
    src/init.S
    src/main_test.c
    src/xpic_loader.c
"""

aCppPath = ['src', '#platform/src', '#platform/src/lib', '#targets/version']

tEnvNetx90 = atEnv.NETX90.Clone()
tEnvNetx90.Replace(LDFILE = 'src/netx90/netx90.ld')
tEnvNetx90.Append(CPPPATH = aCppPath)
tSrcNetx90 = tEnvNetx90.SetBuildPath('targets/netx90', 'src', sources_common)
tObjXpicPram = tEnvNetx90.ObjImport(os.path.join('targets', 'netx90', 'xpic_pram.o'), tBinXpicPram, OBJIMPORT_SECTIONNAME='.xpic_pram')
tObjXpicDram = tEnvNetx90.ObjImport(os.path.join('targets', 'netx90', 'xpic_dram.o'), tBinXpicDram, OBJIMPORT_SECTIONNAME='.xpic_dram')
tElfNetx90 = tEnvNetx90.Elf('targets/netx90/rotary_netx90.elf', tSrcNetx90 + tEnvNetx90['PLATFORM_LIBRARY'] + tObjXpicPram + tObjXpicDram)
tBinNetx90 = tEnvNetx90.ObjCopy('targets/netx90/rotary_netx90.bin', tElfNetx90)
tTxtNetx90 = tEnvNetx90.ObjDump('targets/netx90/rotary_netx90.txt', tElfNetx90, OBJDUMP_FLAGS=['--disassemble', '--source', '--all-headers', '--wide'])



"""
# ----------------------------------------------------------------------------
#
# Build the documentation.
#

# Get the default attributes.
aAttribs = atEnv.DEFAULT['ASCIIDOC_ATTRIBUTES']
# Add some custom attributes.
aAttribs.update(dict({
    # Use ASCIIMath formulas.
    'asciimath': True,

    # Embed images into the HTML file as data URIs.
    'data-uri': True,

    # Use icons instead of text for markers and callouts.
    'icons': True,

    # Use numbers in the table of contents.
    'numbered': True,

    # Generate a scrollable table of contents on the left of the text.
    'toc2': True,

    # Use 4 levels in the table of contents.
    'toclevels': 4
}))
tDoc = atEnv.DEFAULT.Asciidoc(
    'targets/doc/org.muhkuh.tests-iomatrix.html',
    'doc/org.muhkuh.tests-iomatrix.asciidoc',
    ASCIIDOC_BACKEND='html5',
    ASCIIDOC_ATTRIBUTES=aAttribs
)
"""

"""
#----------------------------------------------------------------------------
#
# Build the artifacts.
#
strGroup = 'org.muhkuh.tests'
strModule = 'eth'

# Split the group by dots.
aGroup = strGroup.split('.')
# Build the path for all artifacts.
strModulePath = 'targets/jonchki/repository/%s/%s/%s' % ('/'.join(aGroup), strModule, PROJECT_VERSION)

# Set the name of the artifact.
strArtifact0 = 'lua5.1-eth'

tArcList0 = atEnv.DEFAULT.ArchiveList('zip')
tArcList0.AddFiles('netx/',
    ETH_NETX90_MPW,
    ETH_NETX90,
    ETH_NETX4000)
tArcList0.AddFiles('lua/',
    'eth/lua/test_class_eth.lua')
tArcList0.AddFiles('templates',
    'eth/lua/test.lua')
#tArcList0.AddFiles('doc/',
#    tDoc)
tArcList0.AddFiles('',
    'installer/jonchki/lua5.1/install.lua',
    'installer/jonchki/lua5.1/install_testcase.lua')

tArtifact0 = atEnv.DEFAULT.Archive(os.path.join(strModulePath, '%s-%s.zip' % (strArtifact0, PROJECT_VERSION)), None, ARCHIVE_CONTENTS = tArcList0)
tArtifact0Hash = atEnv.DEFAULT.Hash('%s.hash' % tArtifact0[0].get_path(), tArtifact0[0].get_path(), HASH_ALGORITHM='md5,sha1,sha224,sha256,sha384,sha512', HASH_TEMPLATE='${ID_UC}:${HASH}\n')
tConfiguration0 = atEnv.DEFAULT.Version(os.path.join(strModulePath, '%s-%s.xml' % (strArtifact0, PROJECT_VERSION)), 'installer/jonchki/lua5.1/%s.xml' % strModule)
tConfiguration0Hash = atEnv.DEFAULT.Hash('%s.hash' % tConfiguration0[0].get_path(), tConfiguration0[0].get_path(), HASH_ALGORITHM='md5,sha1,sha224,sha256,sha384,sha512', HASH_TEMPLATE='${ID_UC}:${HASH}\n')
tArtifact0Pom = atEnv.DEFAULT.ArtifactVersion(os.path.join(strModulePath, '%s-%s.pom' % (strArtifact0, PROJECT_VERSION)), 'installer/jonchki/lua5.1/pom.xml')
"""

"""
#----------------------------------------------------------------------------
#
# Make a local demo installation.
#
# Copy all binary binaries.
atFiles = {
    'targets/testbench/netx/eth_netx90_mpw.bin':  ETH_NETX90_MPW,
    'targets/testbench/netx/eth_netx90.bin':      ETH_NETX90,
    'targets/testbench/netx/eth_netx4000.bin':    ETH_NETX4000

    # Copy all LUA scripts.
#    'targets/testbench/lua/io_matrix.lua':             'iomatrix/templates/io_matrix.lua',
#    'targets/testbench/lua/io_matrix/ftdi_2232h.lua':  'iomatrix/templates/io_matrix/ftdi_2232h.lua',
#    'targets/testbench/lua/io_matrix/ftdi.lua':        'iomatrix/templates/io_matrix/ftdi.lua',
#    'targets/testbench/lua/io_matrix/netx90_mpw.lua':  LUA_NETX90_MPW,
#    'targets/testbench/lua/io_matrix/netx_base.lua':   LUA_NETX_BASE,
#    'targets/testbench/lua/io_matrix/netx.lua':        'iomatrix/templates/io_matrix/netx.lua'
}
for tDst, tSrc in atFiles.iteritems():
    Command(tDst, tSrc, Copy("$TARGET", "$SOURCE"))
"""
