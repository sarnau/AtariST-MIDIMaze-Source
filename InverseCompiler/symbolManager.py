#!/usr/bin/env python
# -*- coding: utf-8 -*-

import re
import sys
import copy
import unittest

globalTypes = {}
globalVars = {}
globalDefines = {}
globalTypeList = []
globalSymbolDefintions = {}
globalSubVars = set()

class TypeDecl:
    def __init__(self, varName, varType, arraySizes=None, isPointer=False, isRegister=False, isConst=False, isStatic=False):
        self.varName = varName
        if type(varType) == str:
            varType = globalTypes[varType]
        self.varType = varType
        self.assignmentValue = None
        self.hasBeenUsed = False
        self.isPointer = isPointer
        self.isRegister = isRegister
        self.isConst = isConst
        self.isStatic = isStatic
        if isRegister and arraySizes != None:
            arraySizes = None
        self.arraySizes = arraySizes
        self.isFunction = False
        self.isBaseType = False
        self.structVars = { 0:{} }  # 0: struct variables, 8: function parameters, -1: function stack variables, -2: function register variables

    def elementSize(self, namespaceIndex=0):
        elementSize = self.size(namespaceIndex)
        if elementSize != 0:
            if self.arraySizes != None:
                elementSize /= reduce(lambda x, y: x*y, self.arraySizes)
        return elementSize

    def multFactorList(self):
        factor = self.elementSize()
        arrMults = [factor]
        if self.arraySizes:
            for element in reversed(self.arraySizes):
                factor *= element
                arrMults.append(factor)
            arrMults = list(reversed(arrMults[:-1]))
        return arrMults

    # find a variable in an array
    def subArrayOffset(self, elementOffset, namespaceIndex):
        elementSize = self.size(namespaceIndex)
        str = self.varName
        if elementSize != 0:
            if self.arraySizes != None:
                elementSize /= reduce(lambda x, y: x*y, self.arraySizes)
                arrIndex = int(elementOffset / elementSize)
                arrList = []
                for arr in reversed(self.arraySizes):
                    elemIndex = int(arrIndex % arr)
                    arrList.append(elemIndex)
                    arrIndex = (arrIndex - elemIndex) / arr
                if arrIndex != 0:
                    return None
                for arrVal in reversed(arrList):
                    str += '[%d]' % arrVal
            if not self.varType.isBaseType:
                if self.isPointer:
                    return str
                else:
                    s = self.varType.varNameFromOffset(elementOffset % elementSize)
                    if not s:
                        return None
                    str += '.%s' % s
            else:
                if (elementOffset % elementSize) != 0:
                    str += '+%d' % (elementOffset % elementSize)
        return str

    # find an offset within a struct, which can be also located on the stack as parameters or variables
    def subStructOffset(self, offset, namespaceIndex):
        if namespaceIndex in self.structVars:
            for s in self.structVars[namespaceIndex]:
                soffset = s.offset
                if soffset <= offset and offset < soffset + s.size(namespaceIndex):
                    return s.subArrayOffset(offset - soffset, namespaceIndex)
        return None # offset not found in struct

    # public: return the name of a variable when providing an offset and namespace
    def varNameFromOffset(self, offset, namespaceIndex=0):
        if self.isFunction:
            if isinstance(offset, basestring): # An, Dn = register
                regIndex = None
                if offset[0] == 'A':
                    regIndex = 8+int(offset[1])
                elif offset[0] == 'D':
                    regIndex = int(offset[1])
                for s in self.structVars[-2]:   # iterate over the local registers
                    if s.offset == regIndex:
                        return s.varName
                return None
            elif offset < 0:      # searching for a local variable
                return self.subStructOffset(offset, -1)
            elif offset >= 8:   # searching for a parameter
                return self.subStructOffset(offset, 8)
            # fall-through: function name
        if self.varType == None:    # struct
            return self.subStructOffset(offset, namespaceIndex)
        else:
            return self.subArrayOffset(offset, namespaceIndex)

    # public: return the address of a variable when providing an offset and namespace
    def varAddressFromOffset(self, offset, namespaceIndex=0):

        # pointer to registers are not possible
        if self.isFunction and isinstance(offset, basestring):
            return None

        # get the variable name
        str = self.varNameFromOffset(offset, namespaceIndex)
        if str == None:
            return str

        # for a pointer we can strip the array at the end
        if str.endswith('[0]'):
            return str[:-3]

        # the rest is the name with the '&' prefix
        return '&' + str


    # public: size of a variable within a namespace
    def size(self, namespaceIndex=0):
        if self.varType == None:    # struct
            size = 0
            for s in self.structVars[namespaceIndex]:
                size += s.size()
            return size
        else:
            elements = 1
            if self.arraySizes != None:
                elements = reduce(lambda x, y: x*y, self.arraySizes)
            if self.isPointer:
                return 4 * elements
            size = self.varType
            if type(size) != int:
                size = size.size()
            return size * elements

    def Cstring(self, explodeStructs=True, functionHeader=False):
        str = ''
        if self.isRegister:
            str += 'register '
        if self.isConst:
            str += 'const '
        if self.varType == None:    # struct
            str += 'struct '
        elif not isinstance(self.varType, (int,long)):
            if not explodeStructs:
                str += '%s ' % self.varType.varName
            else:
                str += '%s ' % self.varType.Cstring(explodeStructs,functionHeader)
        if self.isPointer:
            str += '*'
        str += self.varName
        if self.isFunction:

            # first round: function parameters
            if 8 in self.structVars:
                params = []
                soffset = 8
                for s in self.structVars[8]:
                    if explodeStructs and not functionHeader:
                        params.append('%s /* %d */' % (s.Cstring(explodeStructs), soffset))
                    else:
                        params.append('%s' % s.Cstring(explodeStructs and not functionHeader))
                    soffset += s.size(8)
                ppStr  = ','.join(params)
                if len(ppStr) == 0:
                    ppStr = 'void'
                str += '(%s)' % ppStr
            else:
                str += '(...)'

            if explodeStructs:
                params = []
                # second round: print register variables
                if -2 in self.structVars:
                    for f in self.structVars[-2]:
                        if f.offset >= 8:
                            reg = 'A%d' % (f.offset - 8)
                        else:
                            reg = 'D%d' % f.offset
                        if functionHeader:
                            params.append("%s" % f)
                        else:
                            params.append("%s /* %s */" % (f, reg))

                # third round: print stack variables
                if -1 in self.structVars and len(self.structVars[-1]):
                    soffset = self.structVars[-1][0].offset
                    for s in self.structVars[-1]:
                        if functionHeader:
                            params.append("%s" % s.Cstring(False))
                        else:
                            params.append("%s /* %s(A6) */" % (s, soffset))
                        soffset += s.size()

                if len(params) != 0:
                    if not functionHeader:
                        str += ' {%s; }' % '; '.join(params)
                    else:
                        str += ' {\n%s;\n' % ';\n'.join(params)
                elif functionHeader:
                    str += ' {'
            return str

        if self.arraySizes != None:
            for arr in self.arraySizes:
                str += "[%d]" % arr
        if len(self.structVars[0]) != 0 and explodeStructs and not functionHeader:
            sstr = '{'
            for s in self.structVars[0]:
                sName = s.Cstring(explodeStructs and not functionHeader)
                if s.offset == None:
                    sstr += ' %s;' % sName
                else:
                    sstr += ' %s /* %d */;' % (sName, s.offset)
            sstr += ' }'
            str += " %s" % sstr
        return str

    def __str__(self):
        return self.Cstring()

def declareBaseType(type,size):
    decl = TypeDecl(type, size)
    decl.isBaseType = True
    globalTypes[type] = decl

declareBaseType('void', 0)
declareBaseType('char', 1)
declareBaseType('unsigned char', 1)
declareBaseType('int', 2)
declareBaseType('unsigned int', 2)
declareBaseType('short', 2)
declareBaseType('unsigned short', 2)
declareBaseType('long', 4)
declareBaseType('unsigned long', 4)

def findVariableByName(function,varName):
    # 1st round: register variables
    if -2 in function.structVars:
        for regVar in function.structVars[-2]:
            if regVar.varName == varName:
                return regVar
    # 2nd round: stack variables
    if -1 in function.structVars and len(function.structVars[-1]):
        soffset = function.structVars[-1][0].offset
        for stackVar in function.structVars[-1]:
            if stackVar.varName == varName:
                return stackVar
    # 3rd round: function parameters
    if 8 in function.structVars:
        for parameter in function.structVars[8]:
            if parameter.varName == varName:
                return parameter
    # 4th round: global variables
    for g in globalVars:
        var = globalVars[g]
        if var.varName == varName:
            return var
    # not found
    return None

def readHeaderFile(filename):
    readSymbolDefinitions('MIDIMAZE.S')
    if 0:
        for label in globalSymbolDefintions:
            dd = globalSymbolDefintions[label]
            print label,dd['type'],dd['data']
            print

    def splitComment(line):
        decl = line
        comment = ''
        if '/*' in line:
            (decl,comment) = line.split('/*')
        comment = comment.rstrip('*/').strip()
        decl = decl.strip().rstrip(';')
        return (comment,decl)

    def parseLine(line):
        (comment,decl) = splitComment(line)

        # check if this is a function declaration
        funcDecl = re.match('(.*?\W+[^\w]*)(\w+)\((.*)\)', decl)
        if funcDecl:
            (retType,funcName,params) = funcDecl.group(1,2,3)
            decl = TypeDecl(funcName,retType.strip())
            decl.isFunction = True
            if params != '...':
                paramList = []
                soffset = 8
                if len(params) != 0:
                    for parameter in params.split(','):
                        (comment2,pdecl) = parseLine(parameter)
                        pdecl.offset = soffset
                        paramList.append(pdecl)
                        soffset += pdecl.size()
                decl.structVars[8] = paramList
            del decl.structVars[0]  # not used for functions
            return (comment,decl)
        (var,nameArray) = decl.rsplit(' ',1)
        isPointer = False
        if nameArray.startswith('*'):
            nameArray = nameArray[1:]
            isPointer = True
        arr = re.match('(.*?)\[(.+)\]', nameArray)
        varName = nameArray
        arrayCount = None
        if arr:
            (varName,arrayCount) = arr.group(1,2)
            if '][' in arrayCount:
                arrayCount = list(map(int, arrayCount.split('][')))
            else:
                arrayCount = [int(arrayCount)]
        varType = var.strip()
        isRegister = False
        if varType.startswith('register '):
            varType = varType[9:]
            isRegister = True
        isStatic = False
        if varType.startswith('static '):
            varType = varType[7:]
            isStatic = True
        isConst = False
        if varType.startswith('const '):
            varType = varType[6:]
            isConst = True
        return (comment,TypeDecl(varName,varType,arrayCount,isPointer,isRegister,isConst,isStatic))

    typedefVars = None
    lastFunction = None
    funcVars = None
    registerVarOffset = 100000
    destinationFilename = None
    for line in open(filename):
        line = line.replace('\t',' ').strip()
        line = ' '.join(line.split())
        if len(line) == 0:      # ignore empty lines
            continue
        if line == '@':
            break
        if line.startswith('//'):
            if '@FILE:' in line:
                (_,name) = line.split('@FILE:')
                name = name.strip()
                if name:
                    destinationFilename = name.strip()
                else:
                    destinationFilename = None
            continue
        if line.startswith('#'):
            filename = destinationFilename
            if not filename:
                filename = 'GLOBALS.H'
            if filename not in globalDefines:
                globalDefines[filename] = []
            globalDefines[filename].append(line)
            continue
        if line == 'typedef struct':
            typedefVars = []
            lastFunction = None
            funcVars = None
            continue
        if line == '{':
            if lastFunction != None:
                funcVars = {}
            continue

        if line == '}' and lastFunction != None:
            if lastFunction != None:
                # register variables
                regVars = []
                for f in [i for i in funcVars if i > registerVarOffset]:
                    funcVars[f].offset = f - registerVarOffset
                    regVars.append(funcVars[f])
                lastFunction.structVars[-2] = regVars

                # stack variables
                localVars = []
                for f in sorted([i for i in funcVars if i < registerVarOffset]):
                    funcVars[f].offset = f
                    localVars.append(funcVars[f])
                lastFunction.structVars[-1] = localVars
            funcVars = None
            continue

        if line.startswith('}') and typedefVars != None:
            (comment,decl) = splitComment(line)
            typeName = decl[1:].strip()
            struct = TypeDecl(typeName,None)
            struct.structVars[0] = typedefVars
            struct.destinationFilename = destinationFilename
            globalTypes[typeName] = struct
            globalTypeList.append(typeName)
            typedefVars = None
            lastFunction = None
            funcVars = None
            continue
        (comment,typeDec) = parseLine(line)
        if typedefVars != None:
            typeDec.offset = int(comment)
            typedefVars.append(typeDec)
        elif funcVars != None:
            varOffset = 0
            if comment.endswith('(A6)'):
                varOffset = int(comment[:-4])
            elif comment[0] == 'A':
                varOffset = (registerVarOffset+8) + int(comment[1])
            elif comment[0] == 'D':
                varOffset = registerVarOffset + int(comment[1])
            funcVars[varOffset] = typeDec
        else:
            typeDec.destinationFilename = destinationFilename
            if typeDec.isFunction:
                lastFunction = typeDec
            # the comment can contain several labels with offsets.
            # This allows pointers into an array, which is what the compiler
            # does generate
            formatToken = None
            if '@' in comment:
                (comment,formatToken) = comment.split('@')
                comment = comment.strip()
                formatToken = formatToken.strip()
            for cc in comment.split(';'):
                subDecl = copy.copy(typeDec)
                varName = cc
                if '+' in cc:
                    (varName,offsetStr) = cc.split('+')
                    subDecl.varName = subDecl.varNameFromOffset(int(offsetStr))
                    globalSubVars.add(varName)
                if varName in globalSymbolDefintions:
                    dd = globalSymbolDefintions[varName]
                    if formatToken:
                        dd['format'] = formatToken
                        globalSymbolDefintions[varName] = dd
                    if dd['type'] == 'S':
                        subDecl.assignmentValue = dd['data']
                    elif dd['type'] == 'L' and subDecl.Cstring(False).startswith('const char **'):
                        indStringLabel = dd['data'][0]
                        if indStringLabel in globalSymbolDefintions:
                            istr = globalSymbolDefintions[indStringLabel]['data']
                            subDecl.assignmentValue = istr
                globalVars[varName] = subDecl

    buildGlobalVarCode()

def writeHeaderFile(filename):
    file = open(filename,'w')
    if len(globalDefines):
        file.write("// Defines\n")
        file.write("\n")
        for d in globalDefines:
            file.write("%s\n" % d)
        file.write("\n")

    file.write("// Global Types\n")
    file.write("\n")
    for g in globalTypeList:
        if globalTypes[g].isBaseType:
            continue
        t = globalTypes[g]
        file.write('typedef struct\n')
        file.write('{\n')
        offset = 0
        for v in t.structVars[0]:
            file.write('    %s; /* %s */\n' % (v.Cstring(False), offset))
            offset += v.size()
        file.write('} %s; /* %s */\n' % (t.varName, t.size()))
        file.write('\n')

    file.write("// Global Variables\n")
    file.write('\n')
    for g in sorted(globalVars):
        v = globalVars[g]
        file.write('%s; /* %s */\n' % (v.Cstring(False), g))
        if v.isFunction:
            file.write('{\n')
            # second round: print register variables
            if -2 in v.structVars:
                for f in v.structVars[-2]:
                    if f.offset >= 8:
                        reg = 'A%d' % (f.offset - 8)
                    else:
                        reg = 'D%d' % f.offset
                    file.write("%s; /* %s */\n" % (f, reg))

            # third round: print stack variables
            if -1 in v.structVars:
                if -2 in v.structVars and len(v.structVars[-2]):
                    file.write('\n')
                for s in v.structVars[-1][::-1]:
                    file.write("%s; /* %s(A6) */\n" % (s.Cstring(False), s.offset))
            file.write('}\n')
            file.write('\n')

def readSymbolDefinitions(filename):
    def updateData(lastLabel,type,data):
        if not data:
            return
        if type == 'B':
            newStr = ''
            slist = data
            if data[-1] == 0:
                slist = data[:-1]
            if data[-2] == 0:
                slist = data[:-2]
            isAllAscii = False
            if len(slist) > 0:
                isAllAscii = True
            for character in slist:
                if character < 32 or character >= 127:
                    isAllAscii = False
                    break
                if character == 34:
                    newStr += '\\"'
                else:
                    newStr += chr(character)
            if isAllAscii:
                data = '"%s"' % newStr
                type = 'S'
        globalSymbolDefintions[lastLabel] = { 'type':type, 'data':data }

    ignore = True
    lastLabel = None
    data = None
    type = None
    for line in open(filename):
        line = line.replace('\t',' ').strip()
        if len(line) == 0 or line[0] == '#':      # ignore empty lines
            continue
        if line == 'DATA':
            ignore = False
            continue
        elif line == 'BSS':
            updateData(lastLabel,type,data)
            ignore = True
            continue
        if not ignore:
            if line[0]=='L' and line[5]==':':
                if line[:5] not in ['L0A7A','L0A7B','L0B2C','L0A7D','L0A83','L0A84','L0B13','L0B2E','L0B2F'] and line[:5] not in globalSubVars:
                    updateData(lastLabel,type,data)
                    lastLabel = line[:5]
                    data = []
                    type = None
                line = line[6:]
            if line[:4]=='DC.B':
                type = 'B'
                lineStr = line[4:].strip()
                while len(lineStr):
                    if lineStr[0]=='$':
                        data.append(int(lineStr[1:3],16))
                        lineStr = lineStr[4:]
                    elif lineStr[0] == "0":
                        data.append(0)
                        lineStr = lineStr[1:]
                    elif lineStr[0] == "'":
                        if lineStr[2] == "'":   # one character only
                            data.append(ord(lineStr[1]))
                            lineStr = lineStr[4:]
                        else:
                            pos = lineStr[1:].find("'")
                            for character in lineStr[1:pos+1]:
                                data.append(ord(character))
                            lineStr = lineStr[pos+3:]
                    else:
                        print 'DC.B {%s}' % lineStr
                        sys.exit(0)
            elif line[:4]=='DC.W':
                type = 'B'
                lineStr = line[4:].strip()
                while len(lineStr):
                    if lineStr[0]=='$':
                        val = int(lineStr[1:5],16)
                        data.append((val >>  8) & 0xff)
                        data.append((val >>  0) & 0xff)
                        lineStr = lineStr[6:]
                    else:
                        type = 'W'
                        for val in lineStr.split(','):
                            data.append(int(val))
                        lineStr  = ''
            elif line[:4]=='DC.L':
                type = 'L'
                lineStr = line[4:].strip()
                while len(lineStr):
                    if lineStr[0]=='0':
                        data.append(0)
                        lineStr = lineStr[1:]
                    elif lineStr[0]=='$':
                        data.append(int(lineStr[1:9],16))
                        lineStr = lineStr[10:]
                    elif lineStr[0] == "'" and lineStr[2] == "'": # one character only
                        data.append(ord(lineStr[1]))
                        lineStr = lineStr[4:]
                    elif lineStr[0]=='L':
                        data.append(lineStr[0:5])
                        lineStr = lineStr[6:]
                    else:
                        print 'DC.L {%s}' % lineStr
                        sys.exit(0)
            elif line[:5]=='DCB.L':
                type = 'L'
                lineStr = line[5:].strip()
                (num,val) = line[6:].split(',')
                val = int(val)
                for i in range(0,int(num)):
                    data.append(val)
            elif line[:5]=='DCB.W':
                type = 'B'
                lineStr = line[5:].strip()
                (num,val) = line[6:].split(',')
                val = int(val)
                for i in range(0,int(num)):
                    data.append((val >>  8) & 0xff)
                    data.append((val >>  0) & 0xff)
            else:
                print lastLabel,'???',line

globalVarCode = {}
def buildGlobalVarCode():
    for s in sorted(globalSymbolDefintions):
        dd = globalSymbolDefintions[s]
        var = globalVars[s]
        data = dd['data']
        formatB = '%#2.2x'
        formatW = '%#4.4x'
        formatL = '%#8.8lx'
        if 'format' in dd:
            formatB = '%'+dd['format']
            formatW = formatB
            formatL = formatW
        requiredLabels = set()
        dataStr = '%s = ' % var.Cstring(False)
        offset = 0
        elementSize = var.elementSize()
        found = False
#        if var.varType.isBaseType and var.varType.size() == 0:
#            continue
        if var.arraySizes:
            dataStr += '{ '
            arr = []
            arraySize = var.arraySizes[0]
            indexCount = 0
            treecount = 0
            for element in range(0,arraySize):
                varTypeStr = ('%s' % var.varType)
                if varTypeStr.startswith('struct DIR_STRUCT') or varTypeStr.startswith('struct POS_STRUCT'):
                    ss = ' ' * 8 + '{'
                    buf = data[element*elementSize:(element+1)*elementSize]
                    for i in range(0,elementSize,2):
                        val = ((buf[i]<<8)|buf[i+1])
                        if val >= 32767:
                            val = val - 65536
                        valStr = '%2d' % val
                        if i == 4 and varTypeStr.startswith('struct POS_STRUCT'):
                            if val == 32:
                                valStr = ' PLAYER_DIR_NORTHEAST'
                            elif val == 96:
                                valStr = ' PLAYER_DIR_SOUTHEAST'
                            elif val == 160:
                                valStr = ' PLAYER_DIR_SOUTHWEST'
                            elif val == 224:
                                valStr = ' PLAYER_DIR_NORTHWEST'
                        ss += valStr + ','
                    ss = ss[:-1] + '}'
                    arr.append(ss)
                    found = True
                elif varTypeStr.startswith('struct TEDINFO'):
                    buf = data[element*elementSize:(element+1)*elementSize]
                    ss = ' ' * 8 + '{'
                    for i in range(0,12,4):
                        val = ((buf[i]<<24)|(buf[i+1]<<16)|(buf[i+2]<<8)|buf[i+3])
                        ss += '(char*)%ld,' % val
                    for i in range(12,elementSize,2):
                        val = ((buf[i]<<8)|buf[i+1])
                        if val >= 32767:
                            val = val - 65536
                        ss += '%d,' % val
                    ss = ss[:-1] + '}'
                    arr.append(ss)
                    found = True
                elif varTypeStr.startswith('struct OBJECT'):
                    buf = data[element*elementSize:(element+1)*elementSize]
                    sdata = []
                    for i in range(0,elementSize,2):
                        val = ((buf[i]<<8)|buf[i+1])
                        if i<12 or i>=16:
                            if val >= 32767:
                                val = val - 65536
                        sdata.append(val)
                    obType = {
                    20:'G_BOX',
                    21:'G_TEXT',
                    22:'G_BOXTEXT',
                    23:'G_IMAGE',
                    24:'G_USERDEF',
                    25:'G_IBOX',
                    26:'G_BUTTON',
                    27:'G_BOXCHAR',
                    28:'G_STRING',
                    29:'G_FTEXT',
                    30:'G_FBOXTEXT',
                    31:'G_ICON',
                    32:'G_TITLE',
                    }
                    obFlags = {
                    0:'NONE',
                    1:'SELECTABLE',
                    2:'DEFAULT',
                    4:'EXIT',
                    8:'EDITABLE',
                    16:'RBUTTON',
                    32:'LASTOB',
                    64:'TOUCHEXIT',
                    128:'HIDETREE',
                    256:'INDIRECT',
                    }
                    obState = {
                    0:'NORMAL',
                    1:'SELECTED',
                    2:'CROSSED',
                    4:'CHECKED',
                    8:'DISABLED',
                    16:'OUTLINED',
                    32:'SHADOWED',
                    }
                    def bMask(dd,val):
                        arr = []
                        for bit in range(0,16):
                            if val & (1<<bit):
                                arr.append(dd[(1<<bit)])
                        if len(arr)==0:
                            arr.append(dd[0])
                        return '|'.join(arr)
                    if element in globalSymbolDefintions['L0A7E']['data']:
                        arr.append(' ' * 8 + '/* TREE %ld (%ld) */' % (treecount,element))
                        treecount += 1
                        indexCount = 0
                    ss = ' ' * 8 + '/* %3d [%d] */ {' % (element,indexCount)
                    indexCount += 1
                    ss += '%4d, ' % sdata[0]
                    ss += '%4d, ' % sdata[1]
                    ss += '%4d, ' % sdata[2]
                    ss += '%11s, ' % obType[sdata[3]]
                    ss += '%35s, ' % bMask(obFlags,sdata[4])
                    ss += '%8s, ' % bMask(obState,sdata[5])
                    spec = ((sdata[6]<<16)|sdata[7])
                    comment = ''
                    if obType[sdata[3]] in ['G_STRING','G_BUTTON','G_TITLE']:
                        ss += '%10ld, ' % spec
                        comment = ' /* ' + globalSymbolDefintions[globalSymbolDefintions['L0A78']['data'][spec]]['data'] + ' */'
                    else:
                        ss += '%#8.8lx, ' % spec
                    ss += '%#4.4x, ' % sdata[8]
                    ss += '%#4.4x, ' % sdata[9]
                    ss += '%#4.4x, ' % sdata[10]
                    ss += '%#4.4x' % sdata[11]
                    ss = ss + ' }' + comment
                    arr.append(ss)
                    found = True
                elif elementSize == 1:
                    arr.append(formatB % data[element])
                    found = True
                elif elementSize == 2:
                    if dd['type'] == 'W':
                        dataStr += '\n'
                        for i in range(var.arraySizes[0]):
                            dataElements = []
                            for j in range(var.arraySizes[1]):
                                dataElements.append(str(data[j + i * var.arraySizes[1]]))
                            dataStr += ' ' * 8 + '{' + ','.join(dataElements) + '},\n'
                        dataStr += ' ' * 8 + '};'
                        arr = []
                        found = True
                        break
                    else:
                        val = (data[element*2]<<8)|data[element*2+1]
                        if s in ['L0A58','L0A59','L0A5A','L0B12']:
                            colorStr = [ 'COLOR_BLACK_INDEX', 'COLOR_SILVER_INDEX', 'COLOR_MAGNESIUM_INDEX', 'COLOR_BLUE_INDEX', 'COLOR_GREY_INDEX', 'COLOR_DKGREEN_INDEX', 'COLOR_ALUMINIUM_INDEX', 'COLOR_STEEL_INDEX', 'COLOR_YELLOW_INDEX', 'COLOR_ORANGE_INDEX', 'COLOR_PURPLE_INDEX', 'COLOR_MAGENTA_INDEX', 'COLOR_LTBLUE_INDEX', 'COLOR_GREEN_INDEX', 'COLOR_RED_INDEX', 'COLOR_WHITE_INDEX' ]
                            arr.append(' '+colorStr[val])
                        else:
                            arr.append(formatW % (val))
                    found = True
                elif elementSize == 4:
                    if type(data[element]) is str:
                        ss = data[element]
                        if ss in globalVars:
                            requiredLabels.add(ss)
                            ss = '&'+globalVars[ss].varName
                        arr.append('%s' % (ss))
                    else:
                        if dd['type'] == 'L':
                            arr.append(formatL % (data[element]))
                        else:
                            arr.append(formatL % ((data[element*4+0]<<24)|(data[element*4+1]<<16)|(data[element*4+2]<<8)|data[element*4+3]))
                    found = True
                    pass
            if len(arr)>0:
                if arr[0][-1] == '}' or arr[0].endswith('*/'):
                    dataStr += '\n'
                    for line in arr:
                        dataStr += line
                        if '}' in line:
                            dataStr += ','
                        dataStr += '\n'
                    dataStr += ' ' * (8-1)
                else:
                    dataStr += ','.join(arr)
                dataStr += ' };'
        else:
            if dd['type'] == 'S':
                dataStr += var.assignmentValue
                found = True
            elif elementSize == 1:
                dataStr += formatB % data[0]
                found = True
            elif elementSize == 2:
                dataStr += formatW % ((data[0]<<8)|data[1])
                found = True
            elif elementSize == 4:
                if type(data[0]) is str:
                    ss = data[0]
                    if ss in globalVars:
                        requiredLabels.add(ss)
                        ss = '&'+globalVars[ss].varName
                    dataStr += '%s' % (ss)
                else:
                    dataStr += formatL % ((data[0]<<24)|(data[1]<<16)|(data[2]<<8)|data[3])
                found = True
            dataStr += ';'
        if found:
            if len(requiredLabels):
                dataStr = '@REQ:' + ';'.join(sorted(requiredLabels)) + '\n' + dataStr
            globalVarCode[s] = dataStr

    for label in sorted(globalVarCode):
        firstLine = globalVarCode[label].split('\n',1)[0]
        if firstLine.startswith('@REQ:'):
            replaceSymbols = {}
            newStr = ''
            for reqLabel in firstLine[5:].split(';'):
                if reqLabel in globalSymbolDefintions:
                    sd = globalSymbolDefintions[reqLabel]
                    if sd['type'] == 'S':   # for now: strings only
                        replaceSymbols[globalVars[reqLabel].varName] = sd['data']
                    else:
                        newStr += globalVarCode[reqLabel] + '\n'
                else:
                    if reqLabel in globalVars:
                        newStr += 'extern ' + globalVars[reqLabel].Cstring(False) + ';\n'
            globalVarCode[label] = newStr+'\n'.join(globalVarCode[label].split('\n')[1:])
            for ll in reversed(sorted(replaceSymbols)):
                globalVarCode[label] = globalVarCode[label].replace('&'+ll, replaceSymbols[ll])

    for label in sorted(globalVars):
        var = globalVars[label]
        if var.isFunction or label in globalSymbolDefintions:
            continue
        if label in globalSubVars:
            continue
        globalVarCode[label] = '%s;' % var.Cstring(False)



class TestStringMethods(unittest.TestCase):

    def test_draw_maze_viewmatrix(self):
        var = globalVars['L0B37']
        self.assertEqual(var.Cstring(False), 'XY_SPEED_TABLE draw_maze_viewmatrix[9][17]')
        self.assertEqual(var.varNameFromOffset(0), 'draw_maze_viewmatrix[0][0].deltaY')
        self.assertEqual(var.varNameFromOffset(10), 'draw_maze_viewmatrix[0][2].deltaX')
        self.assertEqual(var.varNameFromOffset(12), 'draw_maze_viewmatrix[0][3].deltaY')
        self.assertEqual(var.varNameFromOffset(66), 'draw_maze_viewmatrix[0][16].deltaX')
        self.assertEqual(var.varNameFromOffset(68), 'draw_maze_viewmatrix[1][0].deltaY')
        self.assertEqual(var.varNameFromOffset(70), 'draw_maze_viewmatrix[1][0].deltaX')
        self.assertEqual(var.varNameFromOffset(72), 'draw_maze_viewmatrix[1][1].deltaY')
        self.assertEqual(var.varNameFromOffset(544), 'draw_maze_viewmatrix[8][0].deltaY')
        self.assertEqual(var.varNameFromOffset(546), 'draw_maze_viewmatrix[8][0].deltaX')
        self.assertEqual(var.varNameFromOffset(548), 'draw_maze_viewmatrix[8][1].deltaY')

    def test_maze_datas(self):
        var = globalVars['L0BC9']
        self.assertEqual(var.size(), 4096)
        self.assertEqual(var.Cstring(True), 'char maze_datas[4096]')

    def test_main(self):
        var = globalVars['L0009']
        self.assertEqual(var.size(), 0) # return value = void
        self.assertEqual(var.Cstring(True), 'void main(void) {int i /* -170(A6) */; int savedColors[16] /* -168(A6) */; int intout[57] /* -136(A6) */; int intin[11] /* -22(A6) */; }')
        self.assertEqual(var.varNameFromOffset(-2), 'intin[10]')
        self.assertEqual(var.varNameFromOffset(-4), 'intin[9]')
        self.assertEqual(var.varNameFromOffset(-6), 'intin[8]')
        self.assertEqual(var.varNameFromOffset(-18), 'intin[2]')
        self.assertEqual(var.varNameFromOffset(-20), 'intin[1]')
        self.assertEqual(var.varNameFromOffset(-22), 'intin[0]')
        self.assertEqual(var.varNameFromOffset(-24), 'intout[56]')
        self.assertEqual(var.varNameFromOffset(-26), 'intout[55]')
        self.assertEqual(var.varNameFromOffset(-134), 'intout[1]')
        self.assertEqual(var.varNameFromOffset(-136), 'intout[0]')
        self.assertEqual(var.varNameFromOffset(-138), 'savedColors[15]')
        self.assertEqual(var.varNameFromOffset(-140), 'savedColors[14]')
        self.assertEqual(var.varNameFromOffset(-166), 'savedColors[1]')
        self.assertEqual(var.varNameFromOffset(-168), 'savedColors[0]')
        self.assertEqual(var.varNameFromOffset(-170), 'i')
        self.assertEqual(var.varNameFromOffset(-172), None)

    def test_LoadMaze(self):
        var = globalVars['L07EC']
        self.assertEqual(var.size(), 2) # return value = int
        self.assertEqual(var.Cstring(True), 'int LoadMaze(const char *pathname /* 8 */,const char *filename /* 12 */) {char alertBuf[64] /* -140(A6) */; char tmp /* -76(A6) */; char lineBuf[66] /* -75(A6) */; int error /* -9(A6) */; int fhandle /* -7(A6) */; int fieldX /* -5(A6) */; int fieldY /* -3(A6) */; }')
        self.assertEqual(var.varNameFromOffset(8), 'pathname')
        self.assertEqual(var.varNameFromOffset(12), 'filename')
        self.assertEqual(var.varNameFromOffset(16), None)
        self.assertEqual(var.varNameFromOffset(-2), 'fieldY')
        self.assertEqual(var.varNameFromOffset(-4), 'fieldX')
        self.assertEqual(var.varNameFromOffset(-6), 'fhandle')
        self.assertEqual(var.varNameFromOffset(-8), 'error')
        self.assertEqual(var.varNameFromOffset(-10), 'lineBuf[64]')
        self.assertEqual(var.varNameFromOffset(-74), 'lineBuf[0]')
        self.assertEqual(var.varNameFromOffset(-76), 'tmp')
        self.assertEqual(var.varNameFromOffset(-77), 'alertBuf[63]')
        self.assertEqual(var.varNameFromOffset(-140), 'alertBuf[0]')
        self.assertEqual(var.varAddressFromOffset(-140), 'alertBuf')

    def test_draw_maze_generate_renderlist(self):
        var = globalVars['L081D']
        self.assertEqual(var.size(), 0) # return value = void
        self.assertEqual(var.Cstring(True), 'void draw_maze_generate_renderlist(int y /* 8 */,int x /* 10 */,int fieldOffsetY /* 12 */,int fieldOffsetX /* 14 */,int flip /* 16 */,int leftRightFlag /* 18 */) {register int viewingWidth /* D4 */; register int viewingDistance /* D5 */; register int fieldFX /* D6 */; register int fieldFY /* D7 */; int fieldX /* -8(A6) */; int fieldY /* -6(A6) */; int _fieldX /* -4(A6) */; int _fieldY /* -2(A6) */; }')
        self.assertEqual(var.varNameFromOffset(8), 'y')
        self.assertEqual(var.varNameFromOffset(10), 'x')
        self.assertEqual(var.varNameFromOffset(12), 'fieldOffsetY')
        self.assertEqual(var.varNameFromOffset(14), 'fieldOffsetX')
        self.assertEqual(var.varNameFromOffset(16), 'flip')
        self.assertEqual(var.varNameFromOffset(18), 'leftRightFlag')
        self.assertEqual(var.varNameFromOffset(20), None)
        self.assertEqual(var.varNameFromOffset(-2), '_fieldY')
        self.assertEqual(var.varNameFromOffset(-4), '_fieldX')
        self.assertEqual(var.varNameFromOffset(-6), 'fieldY')
        self.assertEqual(var.varNameFromOffset(-8), 'fieldX')
        self.assertEqual(var.varNameFromOffset('D4'), 'viewingWidth')
        self.assertEqual(var.varNameFromOffset('D5'), 'viewingDistance')
        self.assertEqual(var.varNameFromOffset('D6'), 'fieldFX')
        self.assertEqual(var.varNameFromOffset('D7'), 'fieldFY')

    def test_player_data(self):
        var = globalVars['L0B71']
        self.assertEqual(var.size(), 132*16)
        self.assertEqual(var.Cstring(False), 'PLAYER_DATA player_data[16]')
        for playerIndex in range(0,16):
            self.assertEqual(var.varNameFromOffset(0+playerIndex*132), 'player_data[%d].ply_y' % (playerIndex))
            self.assertEqual(var.varNameFromOffset(2+playerIndex*132), 'player_data[%d].ply_x' % (playerIndex))
            self.assertEqual(var.varNameFromOffset(4+playerIndex*132), 'player_data[%d].ply_dir' % (playerIndex))
            self.assertEqual(var.varNameFromOffset(26+playerIndex*132), 'player_data[%d].dr_type' % (playerIndex))
            self.assertEqual(var.varNameFromOffset(34+playerIndex*132), 'player_data[%d].dr_isInactive' % (playerIndex))
            for droneIndex in range(0,5):
                self.assertEqual(var.varNameFromOffset(36+droneIndex*2+playerIndex*132), 'player_data[%d].dr_dir[%d]' % (playerIndex,droneIndex))
            for droneIndex in range(0,5):
                self.assertEqual(var.varNameFromOffset(48+droneIndex*4+playerIndex*132), 'player_data[%d].dr_field[%d].y' % (playerIndex,droneIndex))
                self.assertEqual(var.varNameFromOffset(48+2+droneIndex*4+playerIndex*132), 'player_data[%d].dr_field[%d].x' % (playerIndex,droneIndex))
            self.assertEqual(var.varNameFromOffset(72+playerIndex*132), 'player_data[%d].dr_fieldIndex' % (playerIndex))
            self.assertEqual(var.varNameFromOffset(76+playerIndex*132), 'player_data[%d].dr_targetLocked' % (playerIndex))
            for droneIndex in range(0,18):
                self.assertEqual(var.varNameFromOffset(80+droneIndex*2+playerIndex*132), 'player_data[%d].dr_humanEnemies[%d]' % (playerIndex,droneIndex))
            self.assertEqual(var.varNameFromOffset(116+playerIndex*132), 'player_data[%d].dr_currentTarget' % (playerIndex))
            self.assertEqual(var.varNameFromOffset(130+playerIndex*132), 'player_data[%d].ply_slist' % (playerIndex))

    def test_xy_speed_table(self):
        var = globalVars['L0BE2']
        self.assertEqual(var.size(), 1024)
        self.assertEqual(var.Cstring(False), 'XY_SPEED_TABLE xy_speed_table[256]')
        for xyTable in range(0,256):
            self.assertEqual(var.varNameFromOffset(0+xyTable*4), 'xy_speed_table[%d].deltaY' % (xyTable))
            self.assertEqual(var.varNameFromOffset(2+xyTable*4), 'xy_speed_table[%d].deltaX' % (xyTable))

    def test_rsrc_object_array(self):
        var = globalVars['L0BE6']
        self.assertEqual(var.size(), 56)
        self.assertEqual(var.Cstring(False), 'OBJECT *rsrc_object_array[14]')
        for index in range(0,14):
            self.assertEqual(var.varNameFromOffset(index*4), 'rsrc_object_array[%d]' % (index))
        for index in range(15,20):
            self.assertEqual(var.varNameFromOffset(index*4), None)

    def test_OBJECT(self):
        type = globalTypes['OBJECT']
        self.assertEqual(type.size(), 24)
        self.assertEqual(type.Cstring(False), 'struct OBJECT')
        self.assertEqual(type.varNameFromOffset(0), 'ob_next')
        self.assertEqual(type.varNameFromOffset(2), 'ob_head')
        self.assertEqual(type.varNameFromOffset(4), 'ob_tail')
        self.assertEqual(type.varNameFromOffset(6), 'ob_type')
        self.assertEqual(type.varNameFromOffset(8), 'ob_flags')
        self.assertEqual(type.varNameFromOffset(10), 'ob_state')
        self.assertEqual(type.varNameFromOffset(12), 'ob_spec')
        self.assertEqual(type.varNameFromOffset(16), 'ob_x')
        self.assertEqual(type.varNameFromOffset(18), 'ob_y')
        self.assertEqual(type.varNameFromOffset(20), 'ob_width')
        self.assertEqual(type.varNameFromOffset(22), 'ob_height')
        for index in range(24,30):
            self.assertEqual(type.varNameFromOffset(index*2), None)

    def test_KBDVECS(self):
        type = globalTypes['KBDVBASE']
        self.assertEqual(type.size(), 36)
        self.assertEqual(type.Cstring(False), 'struct KBDVBASE')
        self.assertEqual(type.varNameFromOffset(0), 'kb_midivec')
        self.assertEqual(type.varNameFromOffset(4), 'kb_vkbderr')
        self.assertEqual(type.varNameFromOffset(8), 'kb_vmiderr')
        self.assertEqual(type.varNameFromOffset(12), 'kb_statvec')
        self.assertEqual(type.varNameFromOffset(16), 'kb_mousevec')
        self.assertEqual(type.varNameFromOffset(20), 'kb_clockvec')
        self.assertEqual(type.varNameFromOffset(24), 'kb_joyvec')
        self.assertEqual(type.varNameFromOffset(28), 'kb_midisys')
        self.assertEqual(type.varNameFromOffset(32), 'kb_kbdsys')
        for index in range(34,40):
            self.assertEqual(type.varNameFromOffset(index*2), None)

    def test_drone_setup(self):
        var = globalVars['L0229']
        self.assertEqual(var.size(), 0) # return value = void
        self.assertEqual(var.Cstring(False), 'void drone_setup(int humanPlayers)')
        self.assertEqual(var.varNameFromOffset(8), 'humanPlayers')
        self.assertEqual(var.varNameFromOffset(10), None)
        self.assertEqual(var.varNameFromOffset(-6), 'humanSoloPlayerList[17]')
        self.assertEqual(var.varNameFromOffset(-8), 'humanSoloPlayerList[16]')
        self.assertEqual(var.varNameFromOffset(-38), 'humanSoloPlayerList[1]')
        self.assertEqual(var.varNameFromOffset(-40), 'humanSoloPlayerList[0]')
        self.assertEqual(var.varNameFromOffset(-42), 'currentHumanSoloPlayer')
        self.assertEqual(var.varNameFromOffset(-56), 'team3HasMembers')
        self.assertEqual(var.varNameFromOffset(-58), 'team0[17]')
        self.assertEqual(var.varNameFromOffset(-92), 'team0[0]')
        self.assertEqual(var.varNameFromOffset(-94), 'team1[17]')
        self.assertEqual(var.varNameFromOffset(-128), 'team1[0]')
        self.assertEqual(var.varNameFromOffset(-130), 'team2[17]')
        self.assertEqual(var.varNameFromOffset(-164), 'team2[0]')
        self.assertEqual(var.varNameFromOffset(-166), 'team3[17]')
        self.assertEqual(var.varNameFromOffset(-200), 'team3[0]')
        self.assertEqual(var.varNameFromOffset(-202), 'team1Index')
        self.assertEqual(var.varNameFromOffset(-216), 'team3Attackable')
        self.assertEqual(var.varNameFromOffset(-218), None)

    def test_KBDVECS_ptr(self):
        var = globalVars['L0BAE']
        self.assertEqual(var.size(), 4)
        self.assertEqual(var.Cstring(False), 'KBDVBASE *KBDVECS_ptr')
        self.assertEqual(var.Cstring(True), 'struct KBDVBASE { void *kb_midivec /* 0 */; void *kb_vkbderr /* 4 */; void *kb_vmiderr /* 8 */; void *kb_statvec /* 12 */; void *kb_mousevec /* 16 */; void *kb_clockvec /* 20 */; void *kb_joyvec /* 24 */; void *kb_midisys /* 28 */; void *kb_kbdsys /* 32 */; } *KBDVECS_ptr')
#        self.assertEqual(var.varNameFromOffset(0), 'KBDVECS_ptr->midivec')
#        self.assertEqual(var.varNameFromOffset(4), 'KBDVECS_ptr->vkbderr')
#        self.assertEqual(var.varNameFromOffset(8), 'KBDVECS_ptr->vmiderr')
#        self.assertEqual(var.varNameFromOffset(32), 'KBDVECS_ptr->ikbdsys')
#        self.assertEqual(var.varNameFromOffset(36), None)



if __name__ == '__main__':
    readHeaderFile('MIDIMAZE_SYMBOLS.h')

    var = globalSymbolDefintions['L0A59']
    print var
    print globalVarCode['L0A59']

#    writeHeaderFile('MIDIMAZE_SYMBOLS2.h')
#    for label in sorted(globalVarCode):
#        print label,globalVarCode[label]
#    var = globalVars['L0B83']
#    var = globalTypes['PLAYER_DATA']
#    print var.Cstring(True)
#    print var.isPointer,var.size(),var.arraySizes,var.elementSize(),var.multFactorList()
#    print var.varType
#    print
#    print
#    var = globalVars['L0B37']
#    print var.Cstring(True)
#    print var.isPointer,var.size(),var.arraySizes,var.elementSize(),var.multFactorList()
#    sys.exit(0)
#    print var.varType.isBaseType
#    print var.varType
#    print var
#    print 'varName',var.varName
#    print 'varType',var.varType
#    print 'isPointer',var.isPointer
#    print 'isRegister',var.isRegister
#    print 'isConst',var.isConst
#    print 'isStatic',var.isStatic
#    print 'arraySizes',var.arraySizes
#    print 'isFunction',var.isFunction
#    print 'isBaseType',var.isBaseType
#    print var.varType
#    print var.varNameFromOffset(8)
#    print var.varNameFromOffset(12)
#    print var.varNameFromOffset(14)
#    if 8 in var.structVars:
#        soffset = 8
#        for parameter in var.structVars[8]:
#            print parameter.Cstring(False)
#            soffset += parameter.size(8)
#    sys.exit(0)

    unittest.main(verbosity=2)
