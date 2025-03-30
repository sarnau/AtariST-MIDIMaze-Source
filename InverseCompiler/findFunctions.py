#!/usr/bin/env python
# -*- coding: utf-8 -*-

import re
import copy
from symbolManager import *
from termParser import *

replaceVars = True


readHeaderFile('MIDIMAZE_SYMBOLS.h')

mathRegs = ['D0','D1','D2','A0','A1','A2']
localRegs = ['D3','D4','D5','D6','D7','A3','A4','A5']


# cleanup the opcode to simplify, add a size to make it easier
def cleanOpcode(opcode,operand1,operand2):
    if opcode.endswith('.S'):   # remove "Short" from branches
        opcode = opcode[:-2]
    elif opcode[:4] == 'CMPI' or opcode[:4] == 'ADDA' or opcode[:4] == 'SUBA' or opcode[:4] == 'ANDI' or opcode[:4] == 'EORI' or opcode[:4] == 'ADDI' or opcode[:4] == 'SUBI' or opcode[:4] == 'ADDQ' or opcode[:4] == 'SUBQ':    # remove "I", "A" or "Q"
        opcode = opcode[:3] + opcode[4:]
    elif opcode[:3] == 'ORI':    # remove "I"
        opcode = opcode[:2] + opcode[3:]
    elif opcode[:5] == 'MOVEA':    # remove "A"
        opcode = opcode[:4] + opcode[5:]
    elif opcode in ['MULU', 'MULS', 'DIVU', 'DIVS', 'MOVE', 'ASL', 'ASR','SWAP']:
        opcode += '.W'
    elif opcode in ['BTST']:
        opcode += '.B'
    if opcode == 'MOVE' and (operand1 == 'SR' or operand2 == 'CCR'):
        opcode += '.W'
    return opcode


class AsmSourceCode:
    # parse a 68000 CPU assembly line (does not work for 68020+) into label,opcode,operand1,operand2
    opcodeRegEx = re.compile(r"^(?:([\S]+):\s*|\s+)*([^ ]+)[ \t]*((?:[^,]*?\(.*?\)\+?)|(?:[^,]*))(?:,(.*))?")

    # this keeps the current state, so we can peek into the next line and restore back
    class AsmSourceCodeState:
        def __init__(self):
            self.currentLine = -1
            self.prevlabel = None
            self.foundStart = None
            self.skipNextLines = 0

    def resetLine(self):
        self.state = AsmSourceCode.AsmSourceCodeState()

    def __init__(self, sourceLines, useStartEnd=False):
        self.sourceLines = sourceLines
        self.state = AsmSourceCode.AsmSourceCodeState()
        self.useStartEnd = useStartEnd

    def currentLineNumber(self):
        return self.state.currentLine

    def peekNextLine(self, lines):
        tempState = copy.copy(self.state)
        ret = []
        while lines > 0:
            nextLine = self.nextLine()
            if nextLine == None:
                nextLine = ''
            ret.append(nextLine)
            lines -= 1
        self.state = tempState
        return ret

    def skipNextLine(self, lines):
        self.prevlabel = None
        self.state.currentLine += lines
        self.state.skipNextLines -= lines
        if self.state.skipNextLines < 0:
            self.state.skipNextLines = 0

    def nextLine(self):
        while True:
            self.state.currentLine += 1

            # end of the source code
            if len(self.sourceLines) <= self.state.currentLine:
                return None

            line = self.sourceLines[self.state.currentLine]

            # remove the comment from the line
            comment = None
            if ';' in line:
                commentPos = line.find(';')
                comment = line[commentPos + 1:].strip()
                if len(comment) == 0:
                    comment = None
                if commentPos > 0:
                    line = line[:commentPos-1].strip()
                else:
                    line = ''

            # ignore empty lines
            if len(line) == 0:
                continue

            # line with just a label in it
            if line.endswith(':'):
                self.state.prevlabel = line[:-1]
                continue

            # detect special synthetic opcodes, like ?SWITCH
            group = re.match(r"(.*?):(@.*) (.*)", line)
            if group != None:
                (label,opcode,operandS) = group.group(1,2,3)
                operand = [operandS,None]
            else:
                # take opcode apart
                group = re.match(self.opcodeRegEx, line)
                if group == None:       # regex didn't match. That should surprise us
                    print "$",line  # unknown line
                    continue
                operand = [None,None]
                (label,opcode,operand[0],operand[1]) = group.group(1,2,3,4)

            if self.state.skipNextLines:
                self.state.skipNextLines -= 1
                continue

            # only parse between START ... END
            if not self.state.foundStart and self.useStartEnd:
                if opcode == 'START':
                    self.state.foundStart = True
                    self.state.prevlabel = None
                continue
            if opcode == 'START':
                continue
            if opcode == 'STOP':
                return None
            if opcode == 'END':
                if self.useStartEnd:
                    self.state.foundStart = False
                    continue
                continue

            # move a previous label into the current line
            if label == None and self.state.prevlabel != None:
                label = self.state.prevlabel
                self.state.prevlabel = None

            opcode = cleanOpcode(opcode,operand[0],operand[1])
            # MOVEQ is a short form of MOVE.L #,<ea>
            if opcode == 'MOVEQ':
                op = operand[0][1:]
                op = op.replace('$', '')
                opv = int(op, 16)
                if opv >= 128:              # negative sign extension
                    opv = -(256 - opv)
                operand[0] = '#%d' % opv
                opcode = 'MOVE.L'

            # size of the operand, -1 is for all branches
            size = -1
            if opcode.endswith('.B'):
                size = 1
                opcode = opcode[:-2]
            elif opcode.endswith('.W'):
                size = 2
                opcode = opcode[:-2]
            elif opcode.endswith('.L'):
                size = 4
                opcode = opcode[:-2]

            # convert hex, strip word-adressing
            if operand[0] != None:
                if '$' in operand[0]:
                    operand[0] = operand[0].replace('$', '0x')
                    if operand[0].endswith('.W'):
                        operand[0] = operand[0][:-2]
            if operand[1] != None:
                if '$' in operand[1]:
                    operand[1] = operand[1].replace('$', '0x')
                    if operand[1].endswith('.W'):
                        operand[1] = operand[1][:-2]
            if operand[0] != None and len(operand[0]) == 0:
                operand[0] = None
            if operand[1] != None and len(operand[1]) == 0:
                operand[1] = None
            return (label,size,opcode,operand[0],operand[1],comment)
        pass


# clean up opcodes, like TST <ea> into a CMP #0,<ea> for simpler parsing. Also convert all generated switch() into ?SWITCH
def cleanSourceLines(sourceScan):
    sourceLines = []
    lastOpcode = None
    opcode = None
    knownLabels = set()
    keepLabel = None
    while True:
        ret = sourceScan.nextLine()
        if ret == None:
            break
        operand = [None,None]
        lastOpcode = opcode
        (label,size,opcode,operand[0],operand[1],comment) = ret
        if label != None:
            knownLabels.add(label)
        if keepLabel != None:
            if label == None:
                label = keepLabel
            keepLabel = None

        # jump switch, uses value in D0 to index into table of labels
        if opcode == 'SUB' and operand[0][0] == '#' and operand[1] == 'D0' and lastOpcode == 'BRA':
            ret = sourceScan.peekNextLine(8)
            if ret != None:
                if ret[0][2] == 'CMP' and ret[0][4] == 'D0' and ret[1][2] == 'BHI' and ret[2][2] == 'ASL' and ret[2][3] == '#2' and ret[2][4] == 'D0' and ret[3][2] == 'MOVE' and ret[3][3] == 'D0' and ret[3][4] == 'A0' and ret[4][2] == 'ADD' and ret[4][4] == 'A0' and ret[5][2] == 'MOVE' and ret[5][3] == '(A0)' and ret[5][4] == 'A0' and ret[6][2] == 'JMP' and ret[6][3] == '(A0)':
                    fromValue = int(operand[0][1:],0)
                    if fromValue > 32767:
                        fromValue = fromValue - 65536
                    switchSymbols = None
                    breakSymbol = ret[7][0]
                    switchTableLabel = ret[4][3][1:]
                    if switchTableLabel in globalSymbolDefintions:
                        switchSymbols = globalSymbolDefintions[switchTableLabel]['data']
                    else:
                        print '@B@ UNKNOWN SWITCH ADDR',ret[0][3][1:]
                    line = '?SWITCH'
                    for s in switchSymbols:
                        line += ' #%s:%s' % (fromValue,s)
                        fromValue += 1
                    line += ' default:'+ret[1][3]
                    line += ' break:'+breakSymbol
                    offset = -1
                    while 'BRA ' + label not in sourceLines[offset]:
                        offset -= 1
                    sourceLines[offset] = '      ' + line
                    sourceScan.skipNextLine(7)
                    continue

        # direct switch, searches down table of values for match, if match
        # found, branches to corresponding label in label table.
        if opcode == 'EXT' and operand[0] == 'D0' and lastOpcode == 'BRA':
            ret = sourceScan.peekNextLine(7)
            if ret != None:
                if ret[0][2] == 'MOVE' and ret[0][4] == 'A0' and ret[1][2] == 'MOVE' and ret[1][4] == 'D1' and ret[2][2] == 'CMP' and ret[2][3] == '(A0)+' and ret[2][4] == 'D0' and ret[3][2] == 'DBEQ' and ret[3][3] == 'D1' and ret[4][2] == 'MOVE' and ret[4][4] == 'A0' and ret[5][2] == 'JMP' and ret[5][3] == '(A0)':
                    switchValues = None
                    switchSymbols = None
                    breakSymbol = ret[6][0]
                    switchTableLabel = ret[0][3][1:]
                    if switchTableLabel in globalSymbolDefintions:
                        ll = globalSymbolDefintions[switchTableLabel]['data']
                        switchValues = ll[:len(ll)/2]
                        switchSymbols = ll[len(ll)/2:]
                    else:
                        print '@B@ UNKNOWN SWITCH ADDR',ret[0][3][1:]
                    index = 0
                    line = '?SWITCH'
                    while index < len(switchValues)-1:
                        line += ' #%d:%s' % (switchValues[index],switchSymbols[index])
                        index += 1
                    line += ' default:'+switchSymbols[-1]
                    line += ' break:'+breakSymbol
                    offset = -1
                    while 'BRA ' + label not in sourceLines[offset]:
                        offset -= 1
                    sourceLines[offset] = '      ' + line
                    sourceScan.skipNextLine(6)
                    continue

        # simplify/clarify some opcodes for easier parsing
        # MOVE.L A6,(A7) (marked as 'PEA0') or MOVE.L A6,-(A7) plus the following ADD or SUB can be converted into one operation
        if opcode == 'MOVE' and (operand[0] != None and operand[0] == 'A6') and (operand[1] != None and operand[1]== '-(A7)'):
            (label2,size2,opcode2,operand20,operand21,comment2) = sourceScan.nextLine()
            if opcode2 == 'ADD':
                opcode = 'PEA'
                operand[0] = '-%d(A6)' % (0x100000000 - int(operand20[1:],0))
                operand[1] = None
                size = -1   # unknown
            elif opcode2 == 'SUB':
                opcode = 'PEA'
                operand[0] = '-%d(A6)' % int(operand20[1:],0)
                operand[1] = None
                size = -1   # unknown
            else:
                print '$ UNEXPECTED OPCODE',opcode2,operand20,operand21
        elif opcode == 'MOVE' and (operand[0] != None and operand[0] == 'A6') and (operand[1] != None and operand[1]== '(A7)'):
            (label2,size2,opcode2,operand20,operand21,comment2) = sourceScan.nextLine()
            if opcode2 == 'ADD':
                opcode = 'PEA0'
                operand[0] = '-%d(A6)' % (0x100000000 - int(operand20[1:],0))
                operand[1] = None
                size = -1   # unknown
            elif opcode2 == 'SUB':
                opcode = 'PEA0'
                operand[0] = '-%d(A6)' % int(operand20[1:],0)
                operand[1] = None
                size = -1   # unknown
            else:
                print '$ UNEXPECTED OPCODE',opcode2,operand20,operand21

        # a CLR can always be treated as a MOVE
        if opcode.startswith('CLR'):
            opcode = 'MOVE'
            operand[1] = operand[0]
            operand[0] = '#0'
        # a TST can always be treated as a CMP #0,<ea>
        if opcode.startswith('TST'):
            opcode = 'CMP'
            operand[1] = operand[0]
            operand[0] = '#0'
        if opcode.startswith('SUB') and operand[0] == operand[1]:
            opcode = 'MOVE'
            operand[0] = '#0'
        # remove this opcode, we don't care about the size conversion
        if opcode == 'EXT':
            keepLabel = label
            continue
        # DIVS + SWAP is a modulo operation
        if opcode == 'DIVS':
            ret = sourceScan.peekNextLine(1)
            if ret != None:
                if ret[0][2] == 'SWAP' and ret[0][3] == operand[1]:
                    opcode = 'MODS'
                    sourceScan.skipNextLine(1)
        # converting into a 2-operand operation
        if (opcode == 'ASL' or opcode == 'ASR') and operand[1] == None:
            operand[1] = operand[0]
            operand[0] = '#1'

        # convert one or two ADD Rn,Rn into one MULU #2/4,Rn to simplify word and long addressing
        if opcode == 'ADD' and operand[0] == operand[1]:
            ret = sourceScan.peekNextLine(1)
            opcode = 'MULU'
            operand[0] = '#2'
            if ret != None:
                if ret[0][2] == 'ADD' and ret[0][3] == ret[0][4] and ret[0][3] == operand[1]:
                    operand[0] = '#4'
                    sourceScan.skipNextLine(1)

        # ASL or ASR for an int or long operation, we merge them into one opcode
        if opcode == 'MOVE':
            ret = sourceScan.peekNextLine(2)
            if ret != None:
                if ret[1][2] == opcode and ret[1][3] == operand[1] and ret[1][4] == operand[0] and ret[0][4] == operand[1] and ret[0][0] == None and ret[1][0] == None:
                    opcode = ret[0][2]
                    operand[1] = operand[0]
                    operand[0] = ret[0][3]
                    sourceScan.skipNextLine(2)

        # simple switch, do compares and branches, followed by branch to default
        # this is generated for ncases <= 4
        if opcode == 'CMP' and (operand[1] == 'D0' or (operand[0] == 'D0' and operand[1] == '#0')) and lastOpcode == 'BRA':
            ret = sourceScan.peekNextLine(9)
            if ret != None:
                if ret[0][2] == 'BEQ' and ret[0][3] in knownLabels:
                    line = '?SWITCH #%d:%s' % (int(operand[0][1:],0),ret[0][3])
                    nextoffset = 1
                    while ret[nextoffset][2] == 'CMP' and ret[nextoffset+1][2] == 'BEQ' and ret[nextoffset+1][3] in knownLabels:
                        line += ' #%d:%s' % (int(ret[nextoffset][3][1:],0), ret[nextoffset+1][3])
                        nextoffset += 2
                    if ret[nextoffset][2] == 'BRA': # default branch
                        line += ' default:' + ret[nextoffset][3]
                        nextoffset += 1
                        line += ' break:' + ret[nextoffset][0]
                        offset = -1
                        while 'BRA ' + label not in sourceLines[offset]:
                            offset -= 1
                        sourceLines[offset] = '      ' + line
                        sourceScan.skipNextLine(nextoffset)
                        continue

        # re-create the source line
        line = ''
        if label != None and len(label) != 0:
            line += label + ':'
        else:
            line = '      '
        line += opcode
        if size == 1:
            line += '.B'
        elif size == 2:
            line += '.W'
        elif size == 4:
            line += '.L'
        line += ' '
        if operand[0]:
            line += operand[0]
            if operand[1]:
                line += ',' + operand[1]

        sourceLines.append(line)
    return sourceLines


def findFunctions(originalSourceLines):
    functionList = []
    currentFunction = None
    prevLines = []
    label = None
    firstLine = None
    while True:
        ret = originalSourceLines.nextLine()
        if ret == None:
            break
        operand = [None,None]
        if label != None:
            lastLabel = label
        (label,size,opcode,operand[0],operand[1],comment) = ret
        prevLines.append((label,size,opcode,operand[0],operand[1],comment))

        # start of a function
        if opcode == 'LINK':
            if currentFunction != None:
                print '$ Function is open',function
            if label == None:
                label = '%s_UNUSED_%4.4x' % (lastLabel,originalSourceLines.currentLineNumber())
            if operand[0] != 'A6':
                print '$ INVALID LINK REGISTER!'

            if label not in globalVars:
                decl = TypeDecl(label,'void')
                decl.isFunction = True
                globalVars[label] = decl
            currentFunction = globalVars[label]
            firstLine = originalSourceLines.currentLineNumber()
            currentFunction.returnLabel = None
            currentFunction.referencedFunctionLabels = set()

            # placeholder for local variables on the stack
            localVarSize = 0
            vOffset = int(operand[1][1:], 0)
            if vOffset != 0:
                localVarSize = 0x10000 - vOffset
                localVarSize -= 4       # 4 bytes are always reserved
            continue

        elif opcode == 'MOVEM':   # in C code this opcode only happens at the very beginning or the end of a function and we can ignore it
            # only look for the MOVEM.L at the end, because at the beginning C might push an additional reg on the stack and pull it with TST.L (A7)+ without using it
            if operand[0] == '(A7)+':
                # register variables already defined?
                if -2 not in currentFunction.structVars:
                    regs = []   # convert a register list into an array of registers
                    for regList in operand[1].split('/'):
                        regRange = regList.split('-')
                        if len(regRange) == 1:
                            regs.append(regRange[0])
                        else:
                            for regIndex in range(int(regRange[0][1]), int(regRange[1][1])+1):
                                regs.append("%s%d" % (regRange[0][0],regIndex))
                    regVars = []
                    for reg in regs:
                        decl = TypeDecl('var_' + reg, 'int', None, False, True)
                        regIndex = None
                        if reg[0] == 'D':
                            regIndex = int(reg[1])
                        elif reg[0] == 'A':
                            regIndex = 8 + int(reg[1])
                        decl.offset = regIndex
                        regVars.append(decl)
                    currentFunction.structVars[-2] = regVars
            continue

        # the end of a function is either a TST.L (A7)+ (followed by a MOVEM (A7)+,<regs>) or an UNLK
        elif (opcode == 'CMP' and operand[1] == '(A7)+' and operand[0] == '#0') or opcode == 'UNLK':   # in C code this opcode only happens at the very end of a function and we can ignore it
            if currentFunction == None:
                print '$ Function is not open'

            if opcode == 'CMP':
                localVarSize += 4       # the reserve was not needed

            # branch onto the label which ends the function _and_ the line before is not a branch (which points to a switch(...))
            if label != None:
                if prevLines[-2][2] == 'BRA' and prevLines[-2][3] == label and prevLines[-3][2][0] != 'B':
                    currentFunction.varType = globalTypes['int']
                currentFunction.returnLabel = label
            continue

        elif opcode == 'RTS':
            if label != None:
                print '$ Label on an RTS',label

            currentFunction.lines = prevLines
            functionList.append(currentFunction)

            currentFunction = None
            prevLines = []
            continue

        # function calls are _always_ a JSR!
        elif opcode == 'JSR':
            # declare all unknown jump destinations
            jumpDestination = operand[0]
            currentFunction.referencedFunctionLabels.add(jumpDestination)
            if jumpDestination not in globalVars:
                decl = TypeDecl(jumpDestination,'void')
                decl.isFunction = True
                globalVars[jumpDestination] = decl

    return functionList


def parseFunction(currentFunction):
    # 1.pass: find all functions and call functions. Declare function parameters and variables. Identify if function uses "return"
    localVars = {}
    localVarSize = 0
    label = None
    mathTranslations = { 'ADD':'+', 'SUB':'-', 'AND':'&', 'OR':'|', 'EOR':'^', 'ASL':'<<', 'ASR':'>>', 'LSL':'<<', 'LSR':'>>', 'MULS':'*', 'MULU':'*', 'DIVS':'/', 'MODS':'%' }
    compareTranslations = { 'EQ':'==','NE':'!=','LS':'<=','LE':'<=','LT':'<','GE':'>=','GT':'>','HI':'>','CC':'>=','HS':'>=','CS':'<','LO':'<' }
    regVals = None
    compareTerm = None
    outputLines = []
    currentFunction.globalLabels = set()

    originalSourceLines = currentFunction.lines
    lno = 0
    while lno < len(originalSourceLines):
        operand = [None,None]
        if label != None:
            lastLabel = label
        (label,size,opcode,operand[0],operand[1],comment) = originalSourceLines[lno]
        lno += 1

###        print (label,size,opcode,operand[0],operand[1],comment),regVals,compareTerm

        # detect A6 relative parameters or variables
        if currentFunction != None:
            if label != None and lno > 1:
                outputLines.append('%s:' % label)

            offset = None
            operandStr = None
            if operand[0] != None:
                if operand[0].endswith('(A6)'):
                    offset = int(operand[0][:-4])
                    operandStr = operand[0]
            if operand[1] != None:
                if operand[1].endswith('(A6)'):
                    offset = int(operand[1][:-4])
                    operandStr = operand[1]
            if offset == None:
                if ((operand[0] != None and 'A6' in operand[0] and operand[0][0] != 'L' and operand[0][0] != '#') or (operand[1] != None and 'A6' in operand[1] and operand[1][0] != 'L' and operand[1][0] != '#')) and (opcode != 'UNLK' and opcode[0] != 'B'):
                    if operand[0] != None and (opcode == 'MOVE' or opcode == 'TST'):
                        group = re.match('(-\d+)\(A6,D[0-7].L\)', operand[0])
                        if group != None:       # regex did match.
                            offset = int(group.group(1))
                    if offset == None:
    #                    print (label,size,opcode,operand[0],operand[1],comment)
                        pass
            if offset != None:
                type = 'int'
                if size == 1:
                    type = 'char'
                elif size == 4:
                    type = 'long'
                if offset < 0:
                    decl = TypeDecl('lvar_%d' % -offset, type)
                    decl.offset = offset
                else:
                    decl = TypeDecl('param_%d' % offset, type)
                    decl.offset = offset
                localVars[offset] = decl
                pass

        # start of a function
        if opcode == 'LINK':
            regVals = {}
            compareTerm = None

            # placeholder for local variables on the stack
            localVarSize = 0
            vOffset = int(operand[1][1:], 0)
            if vOffset != 0:
                localVarSize = 0x10000 - vOffset
                localVarSize -= 4       # 4 bytes are always reserved
            continue

        elif opcode == 'MOVEM':   # in C code this opcode only happens at the very beginning or the end of a function and we can ignore it
            # only look for the MOVEM.L at the end, because at the beginning C might push an additional reg on the stack and pull it with TST.L (A7)+ without using it
            if operand[0] == '(A7)+':
                # register variables already defined?
                if -2 not in currentFunction.structVars:
                    regs = []   # convert a register list into an array of registers
                    for regList in operand[1].split('/'):
                        regRange = regList.split('-')
                        if len(regRange) == 1:
                            regs.append(regRange[0])
                        else:
                            for regIndex in range(int(regRange[0][1]), int(regRange[1][1])+1):
                                regs.append("%s%d" % (regRange[0][0],regIndex))
                    regVars = []
                    for reg in regs:
                        decl = TypeDecl('var_' + reg, 'int', None, False, True)
                        regIndex = None
                        if reg[0] == 'D':
                            regIndex = int(reg[1])
                        elif reg[0] == 'A':
                            regIndex = 8 + int(reg[1])
                        decl.offset = regIndex
                        regVars.append(decl)
                    currentFunction.structVars[-2] = regVars
            continue

        # the end of a function is either a TST.L (A7)+ (followed by a MOVEM (A7)+,<regs>) or an UNLK
        elif (opcode == 'CMP' and operand[0] == '#0' and operand[1] == '(A7)+') or opcode == 'UNLK':   # in C code this opcode only happens at the very end of a function and we can ignore it
            if currentFunction == None:
                print '$ Function is not open'

            if opcode == 'CMP':
                localVarSize += 4       # the reserve was not needed

            # branch onto the label which ends the function _and_ the line before is not a branch (which points to a switch(...))
            if label != None and originalSourceLines[lno-2][2] == 'BRA' and originalSourceLines[lno-2][3] == label and originalSourceLines[lno-3][2][0] != 'B':
                currentFunction.varType = globalTypes['int']
            continue

        elif opcode == 'RTS':
            if label != None:
                print '$ Label on an RTS',label

            # add parameters and local stack variables
            vars = sorted([int('%s' % var) for var in localVars])
            if 8 not in currentFunction.structVars:
                currentFunction.structVars[8] = [localVars[var] for var in vars if var > 0]
            if -1 not in currentFunction.structVars:
                currentFunction.structVars[-1] = [localVars[var] for var in vars if var < 0]

            return outputLines

        # special ?SWITCH case
        elif opcode == '?SWITCH':
            outputLines.append('switch(%s) %s' % (regVals['D0'],operand[0]))
            continue

        # function calls are _always_ a JSR!
        elif opcode == 'JSR':
            # declare all unknown jump destinations
            if operand[0] not in globalVars:
                decl = TypeDecl(operand[0],'void')
                decl.isFunction = True
                globalVars[operand[0]] = decl

            # for test: add the function name to the JSR
    #        operand[0] = globalVars[operand[0]].varName
            if globalVars[operand[0]].varType != globalTypes['void']:
                outputLines.append('temp_D0 = %s()' % (globalVars[operand[0]].varName))
            else:
                outputLines.append('%s()' % (globalVars[operand[0]].varName))

            regVals = {}
            if globalVars[operand[0]].varType != globalTypes['void']:
                regVals['D0'] = 'temp_D0'

            # remove the stack pointer correction after the JSR in the next line
            ret = originalSourceLines[lno]
            if ret != None:
                if ret[2] == 'ADD' and (ret[4] != None and ret[4] == 'A7'):
                    outputLines.append('A7 += %s' % int(ret[3][1:],0))
                    lno += 1
            continue

        def getImmediateValue(result, size):
            val = int(result, 0)
            # extend the sign
            if size == 1 and val >= 0x80:
                val = val - 0x100
                if val == -0x80:
                    val = 0x80
            elif size == 2 and val >= 0x8000:
                val = val - 0x10000
                if val == -0x8000:
                    val = 0x8000
            elif size == 4 and val >= 0x80000000:
                val = val - 0x100000000
            return str(val)

        def checkEA(operand, size):
            if operand in localRegs: # local register variables
                if replaceVars:
                    operand = currentFunction.varNameFromOffset(operand)
                pass
            elif operand in mathRegs:    # temp variables
                pass
            elif operand == 'A6':    # local A6 relative in a term
                pass
            elif operand == '(A7)' or operand == '-(A7)' or operand == '(A7)+': # stack
                pass
            elif operand == 'CCR' or operand == 'SR':   # special case for if (a++) {}
                return '?CCR'
            elif operand.startswith('#'):    # immediate
                if operand[1] == 'L':
                    varStr = operand[1:]
                    currentFunction.globalLabels.add(varStr)
                    if replaceVars:
                        if varStr in globalVars:
                            var = globalVars[varStr]
                            if var:
                                varStr = var.varName
                    return '&' + varStr
                else:   # symbol not found
                    return getImmediateValue(operand[1:], size)
            elif operand.startswith('0x'):   # absolute
                castType = 'int'
                if size == 1:
                    castType = 'char'
                elif size == 4:
                    castType = 'long'
                return '*(%s*)%s' % (castType,operand)
            elif operand[0] == 'L' and len(operand) == 5:   # label, absolute
                varStr = operand
                currentFunction.globalLabels.add(varStr)
                if replaceVars:
                    if varStr in globalVars:
                        var = globalVars[varStr]
                        if var:
                            varStr = var.varName
                            if var.arraySizes and ']' not in varStr:
                                varStr += '[0]' * len(var.arraySizes) # it is an array and we access the first element
                            if not var.isPointer and not var.varType.isBaseType and '.' not in varStr:
                                varStr += '.' + var.varType.varNameFromOffset(0)
                return varStr
            else:
                # (An), (An)+, d(An) and d(An,Rn.x)
                adrgroup = re.match(r"^(-?[\d]+)*\((A\d)(?:,([AD]\d)(?:\.([lLwW]))?)?\)(\+?)$", operand)
                if adrgroup != None:        # regex didn't match. That should surprise us
                    (disp,aReg,dReg,dSize,incr) = adrgroup.group(1,2,3,4,5)
                    if disp == None or disp == '':
                        disp = '0'
                    found = True
                    if replaceVars:
                        if aReg in localRegs:
                            aReg = currentFunction.varNameFromOffset(aReg)
                        if dReg != None and dReg in localRegs:
                            dReg = currentFunction.varNameFromOffset(dReg)

                        if aReg == 'A6': # local variables
                            found = False
                            if dReg == None:
                                operand = currentFunction.varNameFromOffset(int(disp))
                            else:
                                if dReg in mathRegs:
                                    str = regVals[dReg]
                                else:
                                    str = "???"
                                    print '@1@',operand,currentFunction.varNameFromOffset(int(disp)),(disp,aReg,dReg,dSize,incr),str
                                operand = '{(%s+%s)+%s}' % (str,aReg,disp)
                    if found:
                        str = aReg
                        if aReg in mathRegs:
                            str = regVals[aReg]
                        if disp != '0':
                            str += '+'+disp
                        if dReg != None:
                            str += '+'+regVals[dReg]
                            del regVals[dReg]
                        operand = '{%s}' % str  # indirect addressing marker
                        if incr:
                            operand += '++'
                else:
                    print '@2@',operand
            return operand

        if opcode in ['MOVE']:
            operand[0] = checkEA(operand[0], size)
            operand[1] = checkEA(operand[1], size)
            val = operand[0]

            if val == '?CCR':   # push condition register
                regVals[operand[1]] = compareTerm
                continue
            elif operand[1] == '?CCR':  # pop condition register
                opStr = outputLines[-1].replace(regVals[val][0], '')
                if opStr == ' += 1':
                    opStr = '++'
                elif opStr == ' -= 1':
                    opStr = '--'
                else:
                    print '??? Unknown postfix for condition [%s][%s]' % (opStr,regVals[val][0])
                del outputLines[-1]
                compareTerm = (regVals[val][0]+opStr,regVals[val][1])
                continue

            if operand[1] in mathRegs:
                origVal = val
                regValFlag = False
                if val in mathRegs:
                    regValFlag = True
                    val = regVals[val]
                    del regVals[operand[0]]
                regVals[operand[1]] = val
                # next line check, generate a storage to temp, if necessary
                (label0,size0,opcode0,operand00,operand01,comment0) = originalSourceLines[lno]
                if label0 != None and outputLines[-1].endswith(':') and outputLines[-2] == 'goto ' + label0 and outputLines[-3].startswith('temp_%s = ' % operand[1]):
                    outputLines.append('temp_%s = %s' % (operand[1],val))
                    regVals[operand[1]] = 'temp_%s' % operand[1]
                elif opcode0 == 'BRA':        # next one is a branch?
                    prevLine = outputLines[-1]
                    if prevLine.endswith(':'):
                        prevLine = outputLines[-2]
                    if prevLine.startswith('if'):   # some special case?!?
                        outputLines.append('temp_%s = %s' % (operand[1],val))
                        regVals[operand[1]] = 'temp_%s' % operand[1]
                    elif not regValFlag:            # any specific move of an absolute value
                        outputLines.append('temp_%s = %s' % (operand[1],origVal))
                        regVals[operand[1]] = 'temp_%s' % operand[1]
            else:
                if val in mathRegs:
                    val = regVals[val]
                if operand[1] == '-(A7)' or operand[1] == '(A7)':
                    outputLines.append('%s.%d = %s' % (operand[1],size,val))
                else:
                    outputLines.append('%s = %s' % (operand[1],val))
                    val = '?PREVLINE'
                    regVals[operand[0]] = '?PREVLINE'   # val = term_in_D0 could be re-used by a following CMP, in this case the expression should be (val = term_in_D0)
            compareTerm = (val,'0')
            continue

        elif opcode == 'CMP':
            operand[0] = checkEA(operand[0], size)
            operand[1] = checkEA(operand[1], size)
            if operand[0] in mathRegs:
                operand[0] = regVals[operand[0]]
            if operand[1] in mathRegs:
                operand[1] = regVals[operand[1]]
            compareTerm = (operand[1],operand[0])
            continue

        elif opcode in 'BRA':
#            if operand[0] == currentFunction.returnLabel:
#                if 'D0' in regVals and regVals['D0'] != 'temp_D0':
#                    outputLines.append('temp_D0 = %s' % regVals['D0'])
#                regVals = {}
            outputLines.append('goto %s' % operand[0])
            continue

        elif opcode in ['BEQ','BNE','BLT','BLE','BGT','BGE','BHI','BLS','BCC']:
            if compareTerm[0].startswith('BITTST'):
                if opcode != 'BEQ':
                    print '@9@ BTST requires to be followed by a BEQ'
                conditionStr = '!' + compareTerm[0]
            else:
                conditionStr = '%s %s %s' % (compareTerm[0],compareTranslations[opcode[-2:]],compareTerm[1])
            outputLines.append('if(%s) goto %s' % (conditionStr, operand[0]))
            compareTerm = None
            continue

        elif opcode == 'PEA':
            if replaceVars:
                operand[0] = currentFunction.varAddressFromOffset(int(operand[0][:-4]))
            ptrstr = operand[0]
            if ptrstr[0] != '&':
                ptrstr = '&' + ptrstr
            outputLines.append('-(A7).4 = %s' % (ptrstr))
            continue
        elif opcode == 'PEA0':
            if replaceVars:
                operand[0] = currentFunction.varAddressFromOffset(int(operand[0][:-4]))
            ptrstr = operand[0]
            if ptrstr[0] != '&':
                ptrstr = '&' + ptrstr
            outputLines.append('(A7).4 = %s' % (ptrstr))
            continue

        elif opcode == 'NEG':
            operand[0] = checkEA(operand[0], size)
            if operand[0] in mathRegs:
                regVals[operand[0]] = '-' + regVals[operand[0]]
                # next line check, generate a storage to temp, if necessary
                (label0,size0,opcode0,operand00,operand01,comment0) = originalSourceLines[lno]
                if opcode0 == 'BRA' or label0 != None:
                    outputLines.append('temp_%s = %s' % (operand[0], regVals[operand[0]]))
                    regVals[operand[0]] = 'temp_%s' % operand[0]
            else:
                # merge operations onto the stack
                if operand[0] == '(A7)' and outputLines[-1][:8] == '-(A7) = ':
                    outputLines[-1] = '-(A7).%d = -(%s)' % (size,outputLines[-1][8:])
                else:
                    outputLines.append('%s = -%s' % (operand[0],operand[0]))
            continue

        elif opcode == 'BTST':
            operand[0] = checkEA(operand[0], size)
            operand[1] = checkEA(operand[1], size)
            compareTerm = ('BITTST(%s,%s)' % (operand[1],operand[0]), '1')
            # always seems to be followed by a BEQ
            continue

        elif opcode in mathTranslations:
            operand[0] = checkEA(operand[0], size)
            operand[1] = checkEA(operand[1], size)
            mathCode = mathTranslations[opcode]
            mathStr = None
            mathReg = None

            # find pointer arithmetic with symbols
            for varLabel in globalVars:
                var = globalVars[varLabel]
                if var and (var.isPointer or var.arraySizes):
                    if var.varName.endswith(']'):   # already a valid array
                        continue
                    foundVar = None
                    code = None
                    for op in operand:
                        label = op
                        if label[0] == '&':
                            label = label[1:]
                        if var.varName == label:
                            code = 1
                            foundVar = var
                            break
                        val = op
                        if val in mathRegs:
                            val = regVals[val]
                        if var.varName == val:
                            code = 2
                            foundVar = var
                            break
                    if not foundVar:
                        if operand[1] == '(A7)' and (outputLines[-1][:10] == '-(A7).2 = ' or outputLines[-1][:10] == '-(A7).4 = '):
                            if var.varName == outputLines[-1][10:]:
                                code = 3
                                foundVar = var
                        elif operand[1] == '(A7)' and (outputLines[-1][:9] == '(A7).2 = ' or outputLines[-1][:9] == '(A7).4 = '):
                            if var.varName == outputLines[-1][9]:
                                code = 4
                                foundVar = var
                    if foundVar:
                        if foundVar.varType.isBaseType and foundVar.varType.size() == 0:
                            print '@6@ Operation on void *',operand[0],mathCode,foundVar.Cstring(False),currentFunction.Cstring(False)
                        else:
                            if foundVar.varType.isBaseType and foundVar.varType.size() == 1:
                                pass    # nothing to do, because the size of the type is 1 anyway
                            else:
                                # stack operations: just patch the operand 0, same with ADD #offset,var
                                # it seems there is never a case of ADD val,var generated by the compiler. In that case we have to check for int() - for now we'll get an exception
                                if code == 2 or code == 3 or code == 4:
                                    operand[0] = str(int(operand[0]) / foundVar.varType.size())
                                elif code == 1:
                                    found = False
                                    val = operand[1]
                                    if val in mathRegs:
                                        valStr = regVals[val]
                                        match = re.search(r'\(\((?P<term1>.+?)<<1\)\+\((?P<term2>.+?)<<1\)\)', valStr)
                                        if match:
                                            dd = match.groupdict()
                                            valStr = match.string[:match.start()]+dd['term1']+'+'+dd['term2']+match.string[match.end():]
                                            found = True
                                        elif valStr.startswith('(') and valStr.endswith('<<1)') and foundVar.varType.size() == 2:
                                            valStr = valStr[1:-4]
                                            found = True
                                        elif valStr.startswith('(') and valStr.endswith('<<2)') and foundVar.varType.size() == 4:
                                            valStr = valStr[1:-4]
                                            found = True
                                        regVals[val] = valStr
                                    if not found:
                                        if not var.arraySizes:
                                            val = operand[1]
                                            if val in mathRegs:
                                                val = regVals[val]
                                            print '@@1@C',code,operand,val,foundVar,currentFunction.Cstring(False)
                                        pass
                                else:
                                    val = operand[1]
                                    print '@@1@A',code,operand,val,foundVar,currentFunction.Cstring(False)
                                    if val in mathRegs:
                                        val = regVals[val]
                                    print '@@1@B',code,operand,val,foundVar,currentFunction.Cstring(False)

            if operand[1] in mathRegs:
                val = operand[0]
                if val in mathRegs:
                    val = regVals[val]
                if mathCode in ['&','|','^']:
                    try:
                        if int(val) > 9:
                            val = '%#x' % (int(val))
                    except:
                        pass
                regVals[operand[1]] = '(%s%s%s)'  % (regVals[operand[1]],mathCode,val)
                mathReg = operand[1]
                mathStr = regVals[operand[1]]
                if operand[0] in mathRegs:
                    del regVals[operand[0]]
                if mathCode == '&':
                    compareTerm = (mathStr,'0')
                elif mathCode == '*':
                    compareTerm = (mathStr,'0')
                else:
                    compareTerm = None
                # next line check, generate a storage to temp, if necessary
                (label0,size0,opcode0,operand00,operand01,comment0) = originalSourceLines[lno]
                if opcode0 == 'BRA' or label0 != None:
                    outputLines.append('temp_%s = %s' % (operand[1], regVals[operand[1]]))
                    regVals[operand[1]] = 'temp_%s' % operand[1]
            else:
                val = operand[0]
                if val in mathRegs:
                    val = regVals[val]
                mathReg = operand[1]
                mathStr = '%s %s= %s' % (operand[1],mathCode,val)
                # merge operations on the stack
                if operand[1] == '(A7)' and (outputLines[-1][:10] == '-(A7).2 = ' or outputLines[-1][:10] == '-(A7).4 = '):
                    outputLines[-1] = '-(A7).%d = (%s%s%s)' % (size,outputLines[-1][10:],mathCode,val)
                elif operand[1] == '(A7)' and (outputLines[-1][:9] == '(A7).2 = ' or outputLines[-1][:9] == '(A7).4 = '):
                    outputLines[-1] = '(A7).%d = (%s%s%s)' % (size,outputLines[-1][9:],mathCode,val)
                else:
                    outputLines.append(mathStr)
                if mathCode == '-':
                    compareTerm = ('?PREVLINE','0')
                elif mathCode == '+':
                    compareTerm = ('?PREVLINE','0')
                else:
                    compareTerm = None
            continue

        if operand[0] == None:
            operand[0] = ''
        if operand[1] == None:
            operand[1] = ''
        else:
            operand[1] = ',' + operand[1]
        outputLines.append("%s\t%s%s" % (opcode,operand[0],operand[1]))

class TestStringMethods(unittest.TestCase):

    def test_lines(self):
        lines = '''
L0004:LINK      A6,#$FFFC
L0005:JSR       L0002
      SUB.W     L0B32,D0
      AND.W     #$8000,D0
      BNE       L0007
      MOVEQ     #$FF,D0
      BRA       L0008
L0007:BRA.S     L0005
L0008:UNLK      A6
      RTS
'''
        originalSourceLines = AsmSourceCode([line.strip() for line in lines.split('\n')])
        sourcecode = AsmSourceCode(cleanSourceLines(originalSourceLines))
        if False:
            sourcecode.resetLine()
            while True:
                line = sourcecode.nextLine()
                if not line:
                    break
                print line

        for f in findFunctions(sourcecode):
            print f
            for line in parseFunction(f):
                print line



if __name__ == '__main__':
    unittest.main(verbosity=2)
else:
    originalSourceLines = AsmSourceCode([line.strip() for line in open('MIDIMAZE.S')], True)
