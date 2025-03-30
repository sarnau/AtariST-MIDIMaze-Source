#!/usr/bin/env python
# -*- coding: utf-8 -*-

import sys
from findFunctions import *
from termParser import *
import collections;
import shutil,os

generateTempCode = False


def removeUnusedLabels(lineBuffer):
    # search for all label declarations in the function
    foundLabels = set()
    switchLabels = set()
    for line in lineBuffer:
        if len(line) and line.startswith('switch(') and line[-1] != '{':
            switchGroup = re.match('^switch\((.+?)\) (.+)$', line)
            if switchGroup:
                (condition,cases) = switchGroup.group(1,2)
                for ll in cases.split(' '):
                    (case,label) = ll.split(':')
                    switchLabels.add(label)
        labelGroup = re.match('^(\w+):$', line)
        if labelGroup:
            (label) = labelGroup.group(1)
            if label != 'default':  # from switch()
                foundLabels.add(label)

    foundLabels -= switchLabels

    # search for all references to this label:
    for line in lineBuffer:
        if not line.endswith(':'):  # ignore lines which declare a label
            for label in foundLabels:
                if label in line:
                    foundLabels.remove(label)
                    break

    # remove all lines with unused label declarations
    lno = 0
    while lno < len(lineBuffer):
        labelGroup = re.match('^(\w+):$', lineBuffer[lno])
        if labelGroup:
            (label) = labelGroup.group(1)
            if label in foundLabels:
                del lineBuffer[lno]
                continue
        lno += 1

    return lineBuffer

def mergeSimpleQuestionmarkTerm(lineBuffer):
    lno = 0
    while lno < len(lineBuffer):
        ifCondRe = re.match('^if\((.*?)\) goto (.*)$', lineBuffer[lno])
        if ifCondRe and lno+5<=len(lineBuffer):
            (ifCond,falseLabel) = ifCondRe.group(1,2)
            if lineBuffer[lno+2].startswith('goto ') and lineBuffer[lno+3] == falseLabel+':' and lineBuffer[lno+5].endswith(':') and lineBuffer[lno+5][:-1] == lineBuffer[lno+2][5:]:
                if ' = ' in lineBuffer[lno+1] and ' = ' in lineBuffer[lno+4]:
                    (var1,term1) = lineBuffer[lno+1].split(' = ')
                    (var2,term2) = lineBuffer[lno+4].split(' = ')
                    if var1 == var2:
                        if term1 == '0' and term2 == '1':
                            lineBuffer[lno] = '%s = %s' % (var1,ifCond)
                        else:
                            # invert condition
                            if ifCond[0] == '!':
                                ifCond = ifCond[1:]
                            else:
                                ifCond = '!(%s)' % ifCond
                            lineBuffer[lno] = '%s = (%s) ? %s : %s' % (var1,ifCond,term1,term2)
                        del lineBuffer[lno+1:lno+5]
        lno += 1
    lineBuffer = removeUnusedLabels(lineBuffer)           # remove now unused labels
    return lineBuffer

def buildFunctionCalls(lineBuffer):
    stack = [None]
    stackSize = [0]
    stackWithTemp = False
    lno = 0
    while lno < len(lineBuffer):
        line = lineBuffer[lno]
        if line.startswith('(A7) ') and line[6]=='=':
            # math operation with the top of the stack
            stack[0] = '(%s)%s%s' % (stack[0],line[5],line[8:])
            del lineBuffer[lno]
            continue
        elif line.startswith('(A7).'):
            stack = [line[9:]]
            stackSize = [int(line[5])]
            stackWithTemp = False
            del lineBuffer[lno]
            continue
        elif line.startswith('-(A7).'):
            if stack == [None]:     # No leading (A7)? This happens if a temp-result is pushed onto the stack and _then_ a function is called and it's return value uses the temp-result
                stackWithTemp = True
            stack.insert(0, line[10:])
            stackSize.insert(0, int(line[6]))
            del lineBuffer[lno]
            continue
        elif line.endswith('()') and not line.startswith(' *** '):
            correctStack = 0
            if len(lineBuffer)-1 != lno and lineBuffer[lno+1].startswith('A7 += '):
                correctStack = int(lineBuffer[lno+1][6:])
                del lineBuffer[lno+1]   # this line is no longer needed
            funcParams = []
            while correctStack > 0:
                funcParams.append(stack[0])
                correctStack -= stackSize[0]
                del stack[0]
                del stackSize[0]
            if not stackWithTemp and stack[0] != None:
                funcParams.append(stack[0])
                stack = [None]
                stackSize = [0]
            lineBuffer[lno] = line[:-1] + ', '.join(funcParams) + ')'
            if len(lineBuffer)-1 > lno and lineBuffer[lno].startswith('temp_D0 = ') and lineBuffer[lno+1].endswith('temp_D0'):
                lineBuffer[lno+1] = lineBuffer[lno+1][:-7] + lineBuffer[lno][10:]
                del lineBuffer[lno]
                continue
        elif '(A7)+' in line:
            lineBuffer[lno] = line.replace('(A7)+', stack[0])
            del stack[0]
            del stackSize[0]
        lno += 1
    return lineBuffer

def mergeTempMinimal(lineBuffer,regNo):
    lno = 0
    while lno < len(lineBuffer)-1:
        tempD0Re = re.match('^temp_%s = (.*)$' % regNo, lineBuffer[lno])
        if tempD0Re:
            (termD0) = tempD0Re.group(1)
            if not termD0.endswith('()') and lineBuffer[lno+1].endswith(' temp_%s' % regNo):
                del lineBuffer[lno]
                prefix = lineBuffer[lno][:-7]
                if prefix.endswith(' = '):
                    lineBuffer[lno] = prefix + termD0
                else:
                    lineBuffer[lno] = prefix + '(%s)' % termD0
        lno += 1
    return lineBuffer

def mergeTempRegs(lineBuffer,regNo,retLabel,retType):
    lno = 0
    while lno < len(lineBuffer):
        tempD0Re = re.match('^temp_%s = (.*)$' % regNo, lineBuffer[lno])
        if tempD0Re:
            (termD0) = tempD0Re.group(1)
            if len(lineBuffer)-1 == lno:    # last line => temp_D0 = is useless
                lineBuffer[lno] = lineBuffer[lno][10:]  # remove the temp_D0, it is not used
            elif 'temp_%s' % regNo in lineBuffer[lno+1][1:]:
                del lineBuffer[lno]
                while 'temp_%s' % regNo in lineBuffer[lno][1:]:
                    lineBuffer[lno] = lineBuffer[lno][0] + lineBuffer[lno][1:].replace('temp_%s' % regNo, '(%s)' % termD0)
                    lno += 1
                lno -= 1    # one back, because the last temp_Dx followed by a "goto" could be converted into a return
                continue
            elif lineBuffer[lno+1].endswith(':') or lineBuffer[lno+1].startswith('(A7)'):   # next line is a label or a push on stack (= next function call)
                lineBuffer[lno] = lineBuffer[lno][10:]  # remove the temp_D0, it is not used
            elif retLabel != None and lineBuffer[lno+1] == 'goto ' + retLabel:
                if retType == globalTypes['void']:
                    lineBuffer[lno+1] = 'return'
                else:
                    lineBuffer[lno] = 'return %s' % simplifyTerm(lineBuffer[lno][10:])
                    del lineBuffer[lno+1]
            elif lineBuffer[lno+1].endswith('()'):
                lineBuffer[lno] = lineBuffer[lno][10:]  # remove the temp_D0, it is not used
        lno += 1
    lineBuffer = removeUnusedLabels(lineBuffer)
    return lineBuffer

def mergeAssignments(lineBuffer):
    lno = 0
    while lno < len(lineBuffer)-1:
        if lineBuffer[lno][:-2].endswith('= temp_') and lineBuffer[lno+1][:-2].endswith('= temp_'):
            lineBuffer[lno] = lineBuffer[lno][:-7] + lineBuffer[lno+1]
            del lineBuffer[lno+1]
        lno += 1
    return lineBuffer

def mergeIncrDecrWithPrevline(lineBuffer):
    lno = 0
    while lno < len(lineBuffer)-1:
        lineStr = lineBuffer[lno]
        if (lineStr.endswith('++') or lineStr.endswith('--')) and not ' = ' in lineStr:
            symbol = '['+lineStr[:-2]+']'
            if lineBuffer[lno-1].count(symbol) == 1: # only _ONE_ match
                lineBuffer[lno-1] = lineBuffer[lno-1].replace(symbol,'['+lineStr+']')
                del lineBuffer[lno]
                continue
        lno += 1
    return lineBuffer

def removeUnusedTempD0(lineBuffer):
    lno = len(lineBuffer)-1
    while lno >= 0:
        if 'temp_D0' in lineBuffer[lno][1:]:    # temp_D0 is still used? => done
            break
        if lineBuffer[lno].startswith('temp_D0 = '):    # assignment to D0, but not yet used: remove
            lineBuffer[lno] = lineBuffer[lno][10:]
        lno -= 1
    return lineBuffer

def cleanupIfConditions(lineBuffer):
    lno = 0
    while lno < len(lineBuffer)-1:
        lineStr = lineBuffer[lno]
        if lineStr.startswith('if('):
            rpos = lineStr.rfind(')')
            condition = simplifyTerm(lineStr[3:rpos])
            lineBuffer[lno] = 'if(' + condition + lineStr[rpos:]
        lno += 1
    return lineBuffer

def createReturn(lineBuffer,retLabel):
    if retLabel != None:
        lno = 0
        while lno < len(lineBuffer):
            if lineBuffer[lno].endswith('goto ' + retLabel):
                lineBuffer[lno] = lineBuffer[lno][:-len('goto ' + retLabel)] + 'return'
            lno += 1
    lineBuffer = removeUnusedLabels(lineBuffer)
    return lineBuffer

def createIfElse(lineBuffer):
    lno = 0
    knownLabels = set()
    lastLinenumber = None
    while lno < len(lineBuffer):
        lineStr = lineBuffer[lno]
        lno += 1
        if lineStr.endswith(':'):
            knownLabels.add(lineStr[:-1])
            continue
        if lineStr.startswith('if(') and lineStr[:-4].endswith(') goto L'):
            ifStartLine = lno - 1
            ifLabel = lineStr[-5:]

            # goto out of an if() condition detection
            if lno>=lastLinenumber:
                lastLinenumber = None
            lineNo = lno
            while lineNo < len(lineBuffer)-1:
                lineNo += 1
                if lineBuffer[lineNo] == ifLabel + ':':
                    if lastLinenumber and lastLinenumber < lineNo:
                        knownLabels.add(ifLabel)
                    break
            if ifLabel in knownLabels:   # a jump back, which at this point is an actual goto
                continue
            while lno < len(lineBuffer)-1:
                lno += 1
                if lineBuffer[lno] == ifLabel + ':':
                    elseStartLine = None    # default: no else case
                    endifLine = lno
                    lastLinenumber = endifLine
                    if lineBuffer[lno-1].startswith('goto '):   # else case?
                        elseLabel = lineBuffer[lno-1][5:]
                        if elseLabel != 'L00F2':
                            elseStartLine = lno
                            lastLinenumber = elseStartLine
                            forwardLabel = elseLabel
                            while lno < len(lineBuffer):
                                lno += 1
                                if lineBuffer[lno] == elseLabel + ':':
                                    endifLine = lno
                                    break
                    lineBuffer[ifStartLine] = 'if(!(' + lineBuffer[ifStartLine][3:-12] + ')) {'
                    if elseStartLine:
                        lineBuffer[elseStartLine-1] = '} else {'
                    if lineBuffer[endifLine][:-1] == ifLabel and ifLabel == 'L00F2':
                        endifLine -= 1
                    lineBuffer.insert(endifLine+1, '}')
                    break
            lno = ifStartLine + 1
    lineBuffer = removeUnusedLabels(lineBuffer)
    return lineBuffer

def foundLostLabels(lineBuffer):
    # search for all label declarations in the function
    foundLabels = set()
    for line in lineBuffer:
        labelGroup = re.match('^(\w+):$', line)
        if labelGroup:
            (label) = labelGroup.group(1)
            foundLabels.add(label)
    lostLabels = set()
    for line in lineBuffer:
        labelReg = re.match('goto (L....)$', line)
        if labelReg:
            label = labelReg.group(1)
            if label not in foundLabels:
                lostLabels.add(label)
    if len(lostLabels):
        print '@3@',lostLabels,lineBuffer

def cleanupCondition(str, doFlip=False):
    if ' || ' in str or ' && ' in str:
        if doFlip:
            return '!(' + str + ')'
        return str
    else:
        arrGroup = re.match(r"(.*) ([=><!]=?) (.*)", str)
        if not arrGroup:
            return str
        (term1,comp,term2) = arrGroup.group(1,2,3)

        # compare for 0 is special and looks nicer
        if term2 == '0':
            if comp == '==':
                if doFlip:
                    return term1
                return '!' + term1
            elif comp == '!=':
                if doFlip:
                    return '!' + term1
                return term1
        if term1 == '0':
            if comp == '==':
                if doFlip:
                    return term2
                return '!' + term2
            elif comp == '!=':
                if doFlip:
                    return '!' + term2
                return term2

        # invert the condition
        if doFlip:
            if comp == '==':
                comp = '!='
            elif comp == '!=':
                comp = '=='
            elif comp == '<=':
                comp = '>'
            elif comp == '>=':
                comp = '<'
            elif comp == '<':
                comp = '>='
            elif comp == '>':
                comp = '<='
            else:
                print "UNKNOWN",comp
        return "(%s %s %s)" % (term1,comp,term2)

def mergeCompares(lineBuffer):
    lno = 0
    while lno < len(lineBuffer)-1:
        if lineBuffer[lno].startswith('if(') and lineBuffer[lno+1].startswith('if('):
            slno = lno
            while lineBuffer[slno].startswith('if('):
                falseSymbol = lineBuffer[slno][-5:]
                slno += 1
            trueSymbol = None
            if lineBuffer[slno].endswith(':'):
                trueSymbol = lineBuffer[slno][:-1]
                slno -= 1
            endLno = slno
            condStr = ''
            while slno >= lno:
                cStr = lineBuffer[slno][3:-12]  # the condition without the brackets
                if lineBuffer[slno].endswith('goto '+falseSymbol):
                    cStr = cleanupCondition(cStr,True)
                    if condStr != '':
                        condStr = '(' + cStr + ' && ' + condStr + ')'
                    else:
                        condStr = cStr
                elif trueSymbol != None and lineBuffer[slno].endswith('goto '+trueSymbol):
                    cStr = cleanupCondition(cStr)
                    if condStr != '':
                        condStr = '(' + cStr + ' || ' + condStr + ')'
                    else:
                        condStr = cStr
                else:
                    break
                slno -= 1
            if condStr != '':
                lineBuffer[slno+1] = 'if(!%s) goto %s' % (condStr,falseSymbol)
                if endLno-slno > 1:
                    del lineBuffer[slno+2:endLno+1]
                lno = slno+1
        lno += 1
    lineBuffer = removeUnusedLabels(lineBuffer)           # remove now unused labels
    return lineBuffer

def mergeCompares2(lineBuffer):
    lno = 0
    while lno < len(lineBuffer)-1:
        if lineBuffer[lno].startswith('if(') and lineBuffer[lno+1].startswith('if(') and lineBuffer[lno+1].endswith(lineBuffer[lno][-10:]):
            lineBuffer[lno] = 'if(' + cleanupCondition(lineBuffer[lno][3:-12]) + ' || ' + cleanupCondition(lineBuffer[lno+1][3:-12]) + lineBuffer[lno+1][-12:]
            del lineBuffer[lno+1]
            continue
        lno += 1
    return lineBuffer

def cleanupIf(lineBuffer):
    lno = 0
    while lno < len(lineBuffer)-1:
        if lineBuffer[lno].startswith('if('):
            lineBuffer[lno] = 'if(' + cleanupCondition(lineBuffer[lno][3:-12]) + lineBuffer[lno][-12:]
        lno += 1
    return lineBuffer

def generateIncrDecr(lineBuffer):
    # convert simple -1/+1 into decr/incr
    lno = 0
    while lno < len(lineBuffer):
        if lineBuffer[lno].endswith(' -= 1'):
            lineBuffer[lno] = lineBuffer[lno][:-5] + '--'
        elif lineBuffer[lno].endswith(' += 1'):
            lineBuffer[lno] = lineBuffer[lno][:-5] + '++'
        lno += 1

    # merge following ++ inside a compare (the if(?PREVLINE...) into one line, otherwise the if would not work!
    lno = 2
    while lno < len(lineBuffer):
        if lineBuffer[lno].startswith('if(?PREVLINE'):
            variableNames = []
            operations = []
            ll = lno-1
            while True:
                match = re.match('([\w]+)(\+\+|\-\-)', lineBuffer[ll])
                if match:
                    (variableName,operation) = match.groups()
                    variableNames.append(variableName)
                    operations.append(operation)
                    ll -= 1
                else:
                    break
            replLine = lineBuffer[ll]
            numberOfVars = len(variableNames)
            foundAllVariables = True
            while len(variableNames) > 0:
                variableName = variableNames.pop(0)
                operation = operations.pop(0)
                match2 = re.search('\W('+variableName+')(\W|$)', replLine)
                if match2:
                    replLine = match2.string[:match2.start(1)]+variableName+operation+match2.string[match2.end(1):]
                else:
                    foundAllVariables = False
                    break
            if foundAllVariables and replLine != lineBuffer[ll]:
                lineBuffer[ll] = replLine
                lno -= numberOfVars
                while numberOfVars > 0:
                    del lineBuffer[lno]
                    numberOfVars -= 1
        lno += 1
    return lineBuffer

def mergeFUNCTIONPREVLINE(lineBuffer):
    lno = 0
    while lno < len(lineBuffer):
        lineStr = lineBuffer[lno]
        if lineStr.endswith('?PREVLINE') and (lineStr.startswith('-(A7)') or lineStr.startswith('(A7)')):
            lineBuffer[lno-1] = lineStr.replace('?PREVLINE', '(%s)' % lineBuffer[lno-1])
            del lineBuffer[lno]
            continue
        lno += 1
    return lineBuffer

def mergePREVLINE(lineBuffer):
    lno = 0
    while lno < len(lineBuffer):
        if '?PREVLINE' in lineBuffer[lno]:
            lStr = lineBuffer[lno-1]
            if not lStr.endswith('++') and not lStr.endswith('--') or ' = ' in lStr:
                lStr = '(' + lStr + ')'
            else:
                lStr = lStr[-2:] + lStr[:-2]    # convert post-incr/decr to pre-incr/decr, because we merge with the _NEXT_ line
            lineBuffer[lno] = lineBuffer[lno].replace('?PREVLINE', lStr)
            del lineBuffer[lno-1]
            continue
        lno += 1
    lno = 1
    while lno < len(lineBuffer):
        lStr = lineBuffer[lno-1]
        if (lStr.endswith('++') or lStr.endswith('--')) and lineBuffer[lno].startswith('if('):
            found = lineBuffer[lno].count(lStr[:-2])
            if found > 0:
                if found != 1:
                    print '@@@ Multiple matches of \"%s\"' % lStr[:-2]
                else:
                    lineBuffer[lno] = lineBuffer[lno].replace(lStr[:-2], lStr[-2:] + lStr[:-2])
                    del lineBuffer[lno-1]
                    continue
        lno += 1
    return lineBuffer

# {A6+-170} = 0
# goto L000B
# L000A:
# {(({A6+-170}*2)+A6)+-22} = 1
# ++{A6+-170}
# L000B:
# if({A6+-170} < 10) goto L000A
def createForLoops(lineBuffer):
    lno = 0
    knownLabels = set()
    while lno < len(lineBuffer):
        if lineBuffer[lno].endswith(':'):
            knownLabels.add(lineBuffer[lno][:-1])
        elif lineBuffer[lno].startswith('goto ') and lineBuffer[lno][5:] not in knownLabels and ' = ' in lineBuffer[lno-1] and not lineBuffer[lno-1].startswith('temp_') and lineBuffer[lno+1].endswith(':'):
            forStartLine = lno
            forCompareLabel = lineBuffer[forStartLine][5:]
            bail = False
            lno += 1
            while lineBuffer[lno] != forCompareLabel+':':
                if lineBuffer[lno].endswith('goto ' + forCompareLabel):
                    bail = True
                    break
                lno += 1
            if not bail and lno < len(lineBuffer)-1 and lineBuffer[lno+1].startswith('if(') and lineBuffer[lno+1].endswith('goto '+lineBuffer[forStartLine+1][:-1]):
                forIncr = ''
                forIncVariableName = lineBuffer[forStartLine-1].split(' ')[0]
                forIncrCandididate = lineBuffer[lno-1]
                if (forIncrCandididate.endswith('++') or forIncrCandididate.endswith('--')) and forIncVariableName == forIncrCandididate[:-2]:
                    forIncr = forIncrCandididate
                elif forIncVariableName == forIncrCandididate.split(' ')[0]:
                    forIncr = forIncrCandididate
                if len(forIncr):    # we don't generate a for() without an increment. A while() typically looks better
                    breakLabel = None
                    if lno < len(lineBuffer)-2 and lineBuffer[lno+2].endswith(':'):
                        breakLabel = lineBuffer[lno+2][:-1]
                    lineBuffer[forStartLine-1] = 'for(%s; %s; %s)' % (simplifyTerm(lineBuffer[forStartLine-1]), simplifyTerm(lineBuffer[lno+1][3:-12]), simplifyTerm(forIncr))
                    lineBuffer[forStartLine] = '{'  # replace "goto" with the bracket
                    lineBuffer[lno+1] = '}'         # replace condition with the bracket
                    if forIncr != '':
                        del lineBuffer[lno-1]       # delete the increment instruction
                    if breakLabel:
                        while lno > forStartLine:
                            lno -= 1
                            if lineBuffer[lno].endswith('goto '+breakLabel):
                                lineBuffer[lno] = lineBuffer[lno][:-10] + 'break'
            lno = forStartLine
        lno += 1
    lineBuffer = removeUnusedLabels(lineBuffer)
    return lineBuffer

def createWhileLoops(lineBuffer):
    lno = 0
    knownLabels = set()
    while lno < len(lineBuffer):
        if lineBuffer[lno].endswith(':'):
            knownLabels.add(lineBuffer[lno][:-1])
        elif lineBuffer[lno].startswith('goto ') and lineBuffer[lno][5:] not in knownLabels and lineBuffer[lno+1].endswith(':'):
            whileStartLine = lno
            loopLabel = lineBuffer[whileStartLine+1][:-1]
            while lineBuffer[lno] != lineBuffer[whileStartLine][5:]+':':
                lno += 1
            if lno < len(lineBuffer)-1 and ((lineBuffer[lno+1].startswith('if(') and lineBuffer[lno+1].endswith('goto '+loopLabel)) or (lineBuffer[lno+1] == 'goto '+loopLabel)):
                breakLabel = None
                if lno < len(lineBuffer)-2 and lineBuffer[lno+2].endswith(':'):
                    breakLabel = lineBuffer[lno+2][:-1]
                if lineBuffer[lno+1].startswith('goto '):
                    lineBuffer[whileStartLine] = 'while(1)'
                else:
                    lineBuffer[whileStartLine] = 'while(%s)' % simplifyTerm(lineBuffer[lno+1][3:-12])
                lineBuffer[lno+1] = '}'         # replace condition with the bracket
                lineBuffer.insert(whileStartLine+1, '{')
                if breakLabel:
                    while lno > whileStartLine:
                        if lineBuffer[lno].endswith('goto '+breakLabel):
                            lineBuffer[lno] = lineBuffer[lno][:-10] + 'break'
                        lno -= 1
            lno = whileStartLine
        lno += 1
    lineBuffer = removeUnusedLabels(lineBuffer)
    return lineBuffer

def createDoWhileLoops(lineBuffer):
    lno = 1
    while lno < len(lineBuffer):
        if lineBuffer[lno].endswith(':'):
            doWhileStartLine = lno
            loopLabel = lineBuffer[doWhileStartLine][:-1]
            ifLabelBefore = None
            # if the line before the label has a condition, we keep that label to try to detect goto's into this if. A do {} while() would be wrong for this
            beforeline = lineBuffer[lno-1]
            if beforeline.startswith('if(') and ') goto L' in beforeline:
                ifLabelBefore = beforeline[-5:] + ':'
            # find potential break label
            found = False
            while lno < len(lineBuffer)-2:
                lno += 1
                if lineBuffer[lno] == ifLabelBefore:
                    break
                if (lineBuffer[lno+1].startswith('if(') and lineBuffer[lno+1].endswith('goto '+loopLabel)) or (lineBuffer[lno+1] == 'goto '+loopLabel):
                    found = True
                    break
            if found:
                breakLabel = None
                if lno < len(lineBuffer)-2 and lineBuffer[lno+2].endswith(':'):
                    breakLabel = lineBuffer[lno+2][:-1]
                if lineBuffer[lno+1].startswith('goto '):
                    lineBuffer[lno+1] = '} while(1)'
                else:
                    lineBuffer[lno+1] = '} while(%s)' % simplifyTerm(lineBuffer[lno+1][3:-12])
                lineBuffer.insert(doWhileStartLine+1, 'do {')
                if breakLabel:
                    while lno > doWhileStartLine:
                        if lineBuffer[lno].endswith('goto '+breakLabel):
                            lineBuffer[lno] = lineBuffer[lno][:-10] + 'break'
                        lno -= 1
            lno = doWhileStartLine # reset to behind the label
        lno += 1
    lineBuffer = removeUnusedLabels(lineBuffer)
    return lineBuffer

def createSwitch(lineBuffer):
    lno = 0
    while lno < len(lineBuffer):
        lineStr = lineBuffer[lno]
        if lineStr.startswith('switch('):
            pos = lineStr.find(') #')
            labels = {}
            defaultLabel = None
            breakLabel = None
            allCasesAreAscii = True
            for obj in lineStr[pos+2:].split(' '):
                (item,label) = obj.split(':')
                if item[0] == '#':
                    asc = int(item[1:])
                    if asc < 32 or asc >= 127:
                        allCasesAreAscii = False
                    if label + ':' in labels:
                        labels[label + ':'].append(item[1:])
                    else:
                        labels[label + ':'] = [item[1:]]
                elif item == 'default':
                    defaultLabel = label
                elif item == 'break':
                    breakLabel = label
                else:
                    print '@A@ SWITCH UNKNOWN',item
            lineBuffer[lno] = 'switch(' + simplifyTerm(lineStr[7:pos]) + ') {'
            startLine = lno
            while lno < len(lineBuffer)-1:
                lno += 1
                lineStr = lineBuffer[lno]
                for label in labels:
                    if lineStr == label:
                        # there can be more than one case for the same code
                        for caseStr in sorted(labels[label]):
                            if allCasesAreAscii:
                                caseStr = "'%c'" % int(caseStr)
                            lineBuffer.insert(lno+1, 'case %s:' % caseStr)
                        break
                if lineStr == defaultLabel + ':' and defaultLabel != breakLabel:
                    lineBuffer.insert(lno+1, 'default:')
                elif lineStr == 'goto ' + breakLabel:
                    lineBuffer[lno] = 'break'
                elif lineStr == breakLabel + ':':
                   lineBuffer.insert(lno, '}')
                   break
            lno = startLine
        lno += 1
    lineBuffer = removeUnusedLabels(lineBuffer)
    return lineBuffer

def deleteUnreachableGotos(lineBuffer):
    while True:
        foundLines = False
        lno = 0
        while lno < len(lineBuffer)-1:
            if lineBuffer[lno].startswith('goto ') and lineBuffer[lno+1].startswith('goto '):
                del lineBuffer[lno+1]
                foundLines = True
                continue
            lno += 1
        lineBuffer = removeUnusedLabels(lineBuffer)
        if not foundLines:
            break
    return lineBuffer

def findBackJumps(lineBuffer,f):
    lno = 0
    knownLabels = set()
    foundLabels = False
    while lno < len(lineBuffer):
        if lineBuffer[lno].endswith(':'):
            knownLabels.add(lineBuffer[lno][:-1])
        elif lineBuffer[lno][-5:] in knownLabels:
            label = lineBuffer[lno][-5:]
            # these 2 are known goto-jumps in MainStage
            if label != 'L06FE' and label != 'L0707':
                print '@GOTO-BACKWARDS@ %s: %s' % (f.varName,lineBuffer[lno])
                foundLabels = True
            pass
        lno += 1
    if foundLabels:
        print
        pass


def rreplace(s, old, new, occurrence):
    li = s.rsplit(old, occurrence)
    return new.join(li)


_term1 = r'(?P<term1>[^ {}]*?)'
_multFactor1 = r'(?P<multFactor1>[\d]+)'
_shiftFactor1 = r'(?P<shiftFactor1>[\d]+)'
_term2 = r'(?P<term2>[^ {}]*?)'
_multFactor2 = r'(?P<multFactor2>[\d]+)'
_shiftFactor2 = r'(?P<shiftFactor2>[\d]+)'
_addressLabel = r'(?P<label>A6|(&?[a-zA-Z0-9_\[\]]+?))' # an address label is either the A6 register or a an address of a label (&SYMBOL)
_absoluteLabel = r'(?P<label>[a-zA-Z0-9_\[\]]+?)' # a label is just a string with no special characters
_structOffset = r'(?P<structOffset>[\d-]+)'

regExList = [
    # {((TERM1*NUM)+A6 or &LABEL)+OFFSET} - also: the brackets which end behind LABEL are optional   (OFFSET is optional)
    r'\{\(?\('+_term1+'\*'+_multFactor1+'\)\+'+_addressLabel+'\)?(\+'+_structOffset+')?\}',

    # {((TERM1*NUM)+OFFSET+A6 or &LABEL)} - also: the brackets which end behind LABEL are optional   (OFFSET is optional)
    r'\{\(?\('+_term1+'\*'+_multFactor1+'\)(\+'+_structOffset+')?\+'+_addressLabel+'\)?\}',

    # {(A6 or &LABEL+OFFSET+(TERM1*NUM))} - also: the brackets which end behind LABEL are optional   (OFFSET is optional)
    r'\{\(?'+_addressLabel+'(\+'+_structOffset+')?\+\('+_term1+'\*'+_multFactor1+'\)\)?\}',

    # {(((TERM1*NUM)+(TERM2<<SHIFT))+OFFSET+A6 or &LABEL)}
    r'\{\(?\(\('+_term1+'\*'+_multFactor1+'\)\+\('+_term2+'<<'+_shiftFactor2+'\)\)\+'+_structOffset+'\+'+_addressLabel+'\)?\}',

    # {(((TERM1<<SHIFT)+(TERM2<<SHIFT))+A6 or &LABEL)+OFFSET}   (OFFSET is optional)
    r'\{\(?\(\('+_term1+'<<'+_shiftFactor1+'\)\+\('+_term2+'<<'+_shiftFactor2+'\)\)\+'+_addressLabel+'\)?(\+'+_structOffset+')?\}',

    # {(((TERM1*NUM)+(TERM2<<SHIFT))+A6 or &LABEL)+OFFSET}   (OFFSET is optional)
    r'\{\(?\(\('+_term1+'\*'+_multFactor1+'\)\+\('+_term2+'<<'+_shiftFactor2+'\)\)\+'+_addressLabel+'\)?(\+'+_structOffset+')?\}',

    # {(TERM1+A6)+OFFSET} - stack relative arrays
    r'\{\('+_term1+'\+(?P<label>A6)\)\+'+_structOffset+'\}',

    # {LABEL+OFFSET} - a struct pointer  (OFFSET is optional)
    r'\{'+_absoluteLabel+'(\+'+_structOffset+')?\}',

    # {(TERM1+LABEL)} - we switch term/label to position a pointer in the label
    r'\{\(?'+_term1+'\+'+_addressLabel+'\)?\}',

    # {LABEL+(TERM1<<SHIFT)}
    r'\{'+_absoluteLabel+'\+\('+_term1+'<<'+_shiftFactor1+'\)\}',

    # {(LABEL+OFFSET+(TERM1*NUM)}
    r'\{'+_addressLabel+'\+'+_structOffset+'\+\('+_term1+'\*'+_multFactor1+'\)\}',

    # {TERM1} - already being a pointer, this will be just term1[0]
    r'\{'+_term1+'\}',
    ]

def structExpression(function,lineStr):
    if '(A7)' in lineStr:   # not fully resolved addressing, there is a stack-relative one left
        return lineStr

    for regExpr in regExList:
        match = re.search(regExpr, lineStr)
        if match:
            expDict = match.groupdict('0')

            # convert keys to integers
            for element in ['structOffset','multFactor1','multFactor2']:
                if element in expDict:
                    expDict[element] = int(expDict[element])
            # convert shifted keys into multiplications
            for element in ['shiftFactor1','shiftFactor2']:
                if element in expDict:
                    expDict['multFactor'+element[-1:]] = 1 << int(expDict[element])
                    del expDict[element]
            # fix brackets around terms and remove excess brackets
            for element in ['term1','term2']:
                if element in expDict:
                    str = expDict[element]
                    # The regex can remove an optional bracket, which results in a missing open one. We fix that here
                    if str.count('(') < str.count(')'):
                        str = '(' + str
                    expDict[element] = str

            # a special case of {term1} will be converted into {label+0}, which will end up as term1[0]
            if 'label' not in expDict and 'term1' in expDict:
                expDict['label'] = expDict['term1']
                expDict['structOffset'] = 0
                del expDict['term1']

            # it is possible to have label and term1 be both variables.
            # we make sure that a pointer is in 'label' and switch, if necessary
            label = expDict['label']
            if label[0] != '&' and label != 'A6' and 'term1' in expDict:
                try:
                    varTerm1 = findVariableByName(function,expDict['term1'])
                    varLabel = findVariableByName(function,label)
                    # if term1 is a pointer, but label is not, then switch
                    if varTerm1 and varLabel and varTerm1.isPointer and not varLabel.isPointer:
                        expDict['label'] = expDict['term1']
                        expDict['term1'] = label
                    # if label is an integer value, then switch with the label as well
                    elif '%d' % int(label) == label:
                        expDict['label'] = expDict['term1']
                        expDict['term1'] = label
                except:
                    pass

            # If the match has an integer in the term1 (and no mult factor), but no struct offset
            # we simply convert the term into an integer and put it into the struct offset
            # This keeps the code behind simpler and matches reality better
            if 'term1' in expDict and not 'multFactor1' in expDict and not 'structOffset' in expDict:
                val = expDict['term1']
                try:
                    if '%d' % int(val) == val:
                        expDict['structOffset'] = int(val)
                        del expDict['term1']
                except:
                    pass

            newString = '@@@' # placeholder to find missed ones

            # first: absolute labels
            label = expDict['label']
            if label[0] == '&':
                var = findVariableByName(function,label[1:])
                if var:
                    # check for unknown condiditions
                    if 'multFactor2' in expDict:
                        if len(var.arraySizes) == 2:
                            if var.arraySizes[-1] * var.elementSize() != expDict['multFactor1']:
                                print '@9@A',var,var.elementSize(), expDict['multFactor1']
                            if var.elementSize() != expDict['multFactor2']:
                                print '@9@B',var,var.elementSize(), expDict['multFactor2']
                        else:
                            if var.elementSize() != expDict['multFactor1']:
                                print '@9@C',var,var.elementSize(), expDict['multFactor1']
                            if 2 != expDict['multFactor2'] and 4 != expDict['multFactor2']:
                                print '@9@D',var,'2 or 4', expDict['multFactor2']
                        pass
                    elif 'multFactor1' in expDict and expDict['multFactor1'] != 0:
                        if var.elementSize() != expDict['multFactor1']:
                            print '@9@E',var,var.elementSize(), expDict['multFactor1']

                    if 'structOffset' in expDict:
                        varStr = var.varNameFromOffset(expDict['structOffset'])
                    else:
                        varStr = var.varNameFromOffset(0)
                    if not 'multFactor1' in expDict and not 'multFactor2' in expDict and 'term1' in expDict and 'structOffset' not in expDict:
                        if var.isPointer:
                            varStr = varStr + '[' + expDict['term1'] + ']'
                        else:
                            varStr = varStr.replace('[0]', '[' + expDict['term1'] + ']', 1)
                    else:
                        multFactors = var.multFactorList()
                        firstArrayElement = None
                        lastArrayElement = None
                        if 'multFactor1' in expDict:
                            if expDict['multFactor1'] == multFactors[0]:
                                firstArrayElement = '1'
                            if expDict['multFactor1'] < multFactors[0]:
                                lastArrayElement = '1'
                        if 'multFactor2' in expDict:
                            if expDict['multFactor2'] == multFactors[0]:
                                firstArrayElement = '2'
                            if expDict['multFactor2'] < multFactors[0]:
                                lastArrayElement = '2'
                        varStr = varStr.replace('[0]', '[@]')
                        if lastArrayElement:
                            varStr = rreplace(varStr, '[@]', '[@@%s]' % expDict['term'+lastArrayElement], 1)
                        if firstArrayElement:
                            varStr = varStr.replace('[@]', '[@@%s]' % expDict['term'+firstArrayElement], 1)
                        varStr = varStr.replace('[@]', '[0]').replace('[@@', '[')
                    newString = varStr
                else:
                    print '& >',label
                    if 'structOffset' in expDict:
                        if 0 != expDict['structOffset']:
                            print '@9@F', expDict['structOffset']
                    if 'multFactor2' in expDict:
                        if 8 != expDict['multFactor1'] and 2 != expDict['multFactor2']:
                            print '@9@G', expDict['multFactor1'], expDict['multFactor2']
                        newString = '%s[%s][%s]' % (label[1:], expDict['term1'], expDict['term2'])
                    elif 'multFactor1' in expDict:
                        if 2 != expDict['multFactor1'] and 4 != expDict['multFactor1']:
                            print '@9@H', expDict['multFactor1']
                        newString = '%s[%s]' % (label[1:], expDict['term1'])
                pass
            
            # second: A6 relative addressing
            elif label == 'A6':
                if 'multFactor1' in expDict:
                    if 2 != expDict['multFactor1'] and 4 != expDict['multFactor1'] and 6 != expDict['multFactor1']:
                        print '@9@I', expDict['multFactor1']

                varStr = function.varNameFromOffset(expDict['structOffset'])
                varStr = varStr.replace('[0]', '[%s]' % expDict['term1'], 1)
                if 'term2' in expDict:
                    varStr = varStr.replace('[0]', '[%s]' % expDict['term2'], 1)
                newString = varStr
                pass

            else:
                # an indirect pointer to a datatype
                var = findVariableByName(function,label)
                if var:
                    varIsBaseType = False
                    for iii in globalTypes:
                        if not globalTypes[iii].isBaseType:
                            continue
                        if var.varType.varName == globalTypes[iii].varName:
                            varIsBaseType = True
                            break
                    if not varIsBaseType:
                        varSize = var.varType.size()
                        if 'multFactor1' in expDict:
                            if varSize != expDict['multFactor1']:
                                print '@9@K', varSize, expDict['multFactor1']
                        soffset = expDict['structOffset']
                        varStr = expDict['label']
                        structVar = var.varType.varNameFromOffset(soffset % varSize)
                        if 'multFactor1' in expDict:
                            varStr += '[%s]' % (expDict['term1'])
                            varStr += '.' + structVar
                        elif (soffset / varSize) != 0:
                            varStr += '[%s]' % (soffset/varSize)
                            varStr += '.' + structVar
                        else:
                            varStr += '->' + structVar
                        newString = varStr
                        pass
                    elif varIsBaseType:
                        if not 'multFactor1' in expDict and not 'multFactor2' in expDict and 'term1' in expDict and 'structOffset' not in expDict:
                            varStr = expDict['label'] + '[' + expDict['term1'] + ']'
                        else:
                            # integer etc. pointers as variable type
                            if 'structOffset' in expDict and 0 != expDict['structOffset']:
                                # a specific absolute character inside a string
                                if 'char' == var.varType.varName and 'multFactor1' not in expDict:
                                    varStr = '%s[%d]' % (expDict['label'], expDict['structOffset'])
                                # unknown offset/type
                                elif 0 != expDict['structOffset']:
                                    print '@9@J', expDict['structOffset']
                                    print '   ',label,var.Cstring(False),varIsBaseType
                                    print '   ',function.varName,lineStr
                                    print '   ',expDict
                                    print
                            elif 'multFactor1' in expDict:
                                mf = expDict['multFactor1']
                                if var.varType.isBaseType and var.varType.size() == mf:
                                    pass
                                else:
                                    print '@9@L',mf,var.varType.varName
                                    print '   ',label,var.Cstring(False),varIsBaseType
                                    print '   ',function.varName,lineStr
                                    print '   ',expDict
                                    print
                                varStr = '%s[%s]' % (expDict['label'], expDict['term1'])
                            else:
                                varStr = '*' + expDict['label']
                        newString = varStr
                        pass
                elif 1:
                    #print '-3>',function.varName,lineStr
                    #print '   ',match.string[match.start():match.end()]
                    #print '   ',label
                    #print '   ',expDict
                    #print
                    newString = '((char*)%s)[%d]' % (expDict['label'],expDict['structOffset'])
                pass

            return match.string[:match.start()]+newString+match.string[match.end():]
    return lineStr


def expressionsToVariables(lineBuffer,f):
    while True:
        replaced = False
        lno = 0
        while lno < len(lineBuffer):
            lineStr = lineBuffer[lno]
            while True:
                replaced2 = False
                lineStr = structExpression(f,lineStr)
                if lineBuffer[lno] != lineStr:
                    lineBuffer[lno] = lineStr
                    replaced = True
                    replaced2 = True
                if not replaced2:
                    break
            lno += 1
        if not replaced:
            break

    if 0:
        lno = 0
        while lno < len(lineBuffer):
            lineStr = lineBuffer[lno]
            if '{' in lineStr:
                print '%s: %s' % (f.varName,lineStr)
                pass
            lno += 1

    return lineBuffer

def fixABS(lineBuffer):
    lno = 0
    regExpr = r'\(\((?P<var>.+?) < 0\) \? -\1 : \1\)'
    while lno < len(lineBuffer):
        lineStr = lineBuffer[lno]
        match = re.search(regExpr, lineStr)
        if match:
            dd = match.groupdict()
            lineBuffer[lno] = match.string[:match.start()]+'abs('+dd['var']+')'+match.string[match.end():]
            continue
        lno += 1

    lno = 0
    regExpr = r'if\((?P<var>.+?) < 0\) {'
    while lno < len(lineBuffer):
        lineStr = lineBuffer[lno]
        match = re.search(regExpr, lineStr)
        if match:
            dd = match.groupdict()
            if lineBuffer[lno+1] == '%s = -%s' % (dd['var'],dd['var']) and lineBuffer[lno+2] == '}':
                lineBuffer[lno] = '%s = abs(%s)' % (dd['var'],dd['var'])
                del lineBuffer[lno+2]
                del lineBuffer[lno+1]
                continue
        lno += 1
    return lineBuffer

def fixBITTST(lineBuffer):
    lno = 0
    regExpr = r'if\(!\(!BITTST\((?P<term>.*?),(?P<bit>\d+)\)\)\) {'
    regExpr2 = r'\(BITTST\((?P<term>.*?),(?P<bit>\d+)\)\)'
    regExpr3 = r'\!BITTST\((?P<term>.*?),(?P<bit>\d+)\)'
    simpleCases = ['midiCamRotateIndex','joystickDirection','joystickData','currently_displayed_notes_score[player]','fieldX','fieldY']
    while lno < len(lineBuffer):
        lineStr = lineBuffer[lno]
        match = re.search(regExpr, lineStr)
        ifCondition = True
        negate = False
        if not match:
            ifCondition = False
            match = re.search(regExpr2, lineStr)
            if not match:
                match = re.search(regExpr3, lineStr)
                negate = True
        if match:
            dd = match.groupdict()
            newTest = None
            for simpleCase in simpleCases:
                if dd['term'] == simpleCase+'+1':
                    bitMask = 1 << int(dd['bit'])
                    newTest = '%s & %d' % (simpleCase,bitMask)
                    break
            if not newTest:
                if dd['term'] == 'which_events+1':
                    newTest = '(which_events&MU_KEYBD)==MU_KEYBD'
                elif dd['term'] == 'joystick[1]':
                    newTest = 'joystick[1] & (1<<%s)' % (dd['bit'])
                elif dd['term'].endswith('.ob_state+1'):
                    newTest = '('+dd['term'][:-2] + '&SELECTED)==SELECTED'
                elif dd['term'] == 'orgBitMask':
                    bitMask = 1 << (24 + int(dd['bit']))  # 32 bit variable
                    newTest = '%s & %#lx' % (dd['term'],bitMask)
                else:
                    print '@9@ UNKNOWN BITTST',dd
            if newTest:
                if ifCondition:
                    ss = 'if('+newTest+') {'
                else:
                    ss = '('+newTest+')'
                    if negate:
                        ss = ss+'==0'
                lineBuffer[lno] = match.string[:match.start()]+ss+match.string[match.end():]
                continue    # try again in the same line!
            else:
                print dd
                print newTest
                print match.string[:match.start()]+'@@@'+match.string[match.end():]
            pass
        lno += 1
    return lineBuffer

def fillStaticPointers(lineBuffer):
    assignedVars = {}
    for g in globalVars:
        var = globalVars[g]
        if var.assignmentValue:
            assignedVars[var.varName] = var.assignmentValue
            var.hasBeenUsed = True
    lno = 0
    while lno < len(lineBuffer):
        lineStr = lineBuffer[lno]
        newLineStr = lineStr
        for varName in assignedVars:
            if '&'+varName in lineStr:
                newLineStr = newLineStr.replace('&'+varName, assignedVars[varName])
            elif ' '+varName in lineStr:
                newLineStr = newLineStr.replace(' '+varName, ' '+assignedVars[varName])
            elif varName[0] == '*' and varName in lineStr:
                newLineStr = newLineStr.replace(varName, assignedVars[varName])
        if newLineStr != lineStr:
            lineBuffer[lno] = newLineStr
            continue
        lno += 1
    return lineBuffer

def fixAtariOSFunctions(lineBuffer):
    regExpr = r'(?P<osfunction>bios|xbios|gemdos)\((?P<functionid>[\d]+)[, ]*'
    osMapping = {
        'bios' : { 1:'Bconstat', 2:'Bconin', 3:'Bconout' },
        'xbios' : { 0:'Initmouse', 2:'Physbase', 4:'Getrez', 5:'Setscreen', 6:'Setpalette', 7:'Setcolor', 17:'Random', 28:'Giaccess', 32:'Dosound', 34:'Kbdvbase', 37:'Vsync', 38:'Supexec' },
        'gemdos' : { 25:'Dgetdrv', 61:'Fopen', 62:'Fclose', 63:'Fread', 71:'Dgetpath' },
    }

    regExList = {
        r'(?P<function>Bconstat|Bconin|Bconout)\((?P<parameter>[\d]+)':{ 0:'PRT', 1:'AUX', 2:'CON', 3:'MIDI', 4:'IKBD' },
        r'(?P<function>graf_mouse)\((?P<parameter>[\d]+)':{ 0:'ARROW', 2:'HOURGLASS', 256:'M_OFF', 257:'M_ON' },
        r'(?P<function>form_dial)\((?P<parameter>[\d]+)':{ 0:'FMD_START', 1:'FMD_GROW', 2:'FMD_SHRINK', 3:'FMD_FINISH' },
        r'(?P<function>wind_update)\((?P<parameter>[\d]+)':{ 0:'END_UPDATE', 1:'BEG_UPDATE', 2:'END_MCTRL', 3:'BEG_MCTRL' },
    }

    regExOBState = r'.ob_state (?P<andorflag>&|\|)= (?P<bitmask>[\d-]+)'
    obStates = { 1:'SELECTED', 2:'CROSSED', 4:'CHECKED', 8:'DISABLED', 16:'OUTLINED', 32:'SHADOWED', 256:'HIGHLIGHTED', 512:'UNHIGHLIGHTED' }

    regExprDrone = r'[ #](?P<droneid>107|108|114)'
    droneTable = { 107:'DRONE_NINJA', 108:'DRONE_STANDARD', 114:'DRONE_TARGET' }

    replaceStr = { "case 'k':":"case DRONE_NINJA:",
                   "case 'l':":"case DRONE_STANDARD:",
                   "case 'r':":"case DRONE_TARGET:",
                   '.ob_type == 32':'.ob_type == G_TITLE',
                   '.ob_type == 28':'.ob_type == G_STRING',
                   '.ob_type == 26':'.ob_type == G_BUTTON',
                   '.ob_type == 30':'.ob_type == G_FBOXTEXT',
                   '->ob_type = 25':'->ob_type = G_IBOX',
                   'case 10:':'case MN_SELECTED:',
                   'case 29:':'case WM_NEWTOP:',
                   'evnt_multi(17,':'evnt_multi(MU_KEYBD|MU_MESAG,',
                   'wi_gfield = 11':'wi_gfield = WF_FIRSTXYWH',
                   'wi_gfield = 12':'wi_gfield = WF_NEXTXYWH',
                   '.ob_flags |= 2':'.ob_flags |= DEFAULT',
                   '.ob_flags &= -3':'.ob_flags &= ~DEFAULT',
                   'vro_cpyfm(wi_ghandle, 3,':'vro_cpyfm(wi_ghandle, S_ONLY,',
                   '-2147483648':'0x80000000',
                   '2147483647':'0x7FFFFFFF',
                   'L06FE':'MIDIRING_BOOBOO',
                   'L0703':'PLAY_GAME',
                   'L0707':'MIDIRING_BOOBOO2',
                   'L0790':'SLAVE_LOOP_EXIT',
                   'midiByte == 128':'midiByte == MIDI_COUNT_PLAYERS',
                   'Bconout(MIDI, 128)':'Bconout(MIDI, MIDI_COUNT_PLAYERS)',
                   'midiByte == 129':'midiByte == MIDI_RESET_SCORE',
                   'Bconout(MIDI, 129)':'Bconout(MIDI, MIDI_RESET_SCORE)',
                   'player_joy_table[i] = 130':'player_joy_table[i] = MIDI_TERMINATE_GAME',
                   'player_joy_table[0] == 130':'player_joy_table[0] == MIDI_TERMINATE_GAME',
                   '? 132 : 130':'? MIDI_START_GAME : MIDI_TERMINATE_GAME',
                   'Bconout(MIDI, 131)':'Bconout(MIDI, MIDI_SEND_DATA)',
                   'midiByte != 131':'midiByte != MIDI_SEND_DATA',
                   'i != 132':'i != MIDI_START_GAME',
                   'midiByte == 132':'midiByte == MIDI_START_GAME',
                   'Bconout(MIDI, 132)':'Bconout(MIDI, MIDI_START_GAME)',
                   'midiByte == 133':'midiByte == MIDI_ABOUT',
                   'Bconout(MIDI, 133)':'Bconout(MIDI, MIDI_ABOUT)',
                   'midiByte != 134':'midiByte != MIDI_NAME_DIALOG',
                   'Bconout(MIDI, 134)':'Bconout(MIDI, MIDI_NAME_DIALOG)',
                   'get_midi(20)':'get_midi(MIDI_DEFAULT_TIMEOUT)',
                   '== 40':"== '('",
                   '== 45':"== '-'",
                   '+48':"+'0'",
                   '-48':"-'0'",
                   '== 46':"== '.'",
                   '== 88':"== 'X'",
                   '? 32 : 115':"? ' ' : 's'",
                   '? 32 :':"? ' ' :",
                   'tmp < 48 || tmp > 57':"tmp < '0' || tmp > '9'",
                   '] = 32':"] = ' '",
                   '] = 48':"] = '0'",
                   ', 45)':", '-')",
                   ') : 32':") : ' '",
                   ': 115':": 's'",
                   '&= -9':'&= ~8',
                   '+65':"+'A'",
                   '[1] = 58':"[1] = ':'",
                   '[i++] = 42':"[i++] = '.'",
                   '[i++] = 46':"[i++] = '*'",
                   '[i++] = 77':"[i++] = 'M'",
                   '[i++] = 65':"[i++] = 'A'",
                   '[i++] = 90':"[i++] = 'Z'",
                   'Bconout(CON, 58)':"Bconout(CON, ':')",
                   '*playerNameStrPtr ==':'playerNameStrPtr[0] ==',
                   'rsc_draw_buttonless_dialog(1)':'rsc_draw_buttonless_dialog(RSCTREE_SLAVE)',
                   'rsc_draw_buttonless_dialog(2)':'rsc_draw_buttonless_dialog(RSCTREE_MIDIRING_TIMEOUT)',
                   'rsc_draw_buttonless_dialog(3)':'rsc_draw_buttonless_dialog(RSCTREE_GAME_TERMINATED)',
                   'rsc_draw_buttonless_dialog(4)':'rsc_draw_buttonless_dialog(RSCTREE_MAZE_ERROR)',
                   'rsc_draw_buttonless_dialog(5)':'rsc_draw_buttonless_dialog(RSCTREE_MOUSE_CTRL)',
                   'rsc_draw_buttonless_dialog(6)':'rsc_draw_buttonless_dialog(RSCTREE_JOYSTICK_CTRL)',
                   'rsc_draw_buttonless_dialog(10)':'rsc_draw_buttonless_dialog(RSCTREE_SUSPENDED)',
                   'rsc_draw_buttonless_dialog(12)':'rsc_draw_buttonless_dialog(RSCTREE_ABOUT)',
                   'rsc_draw_buttonless_dialog(13)':'rsc_draw_buttonless_dialog(RSCTREE_MIDICAM)',
                   'do_about_dialog(12)':'do_about_dialog(RSCTREE_ABOUT)',
                   'rsrc_object_array[0]':'rsrc_object_array[RSCTREE_MENU]',
                   'rsrc_object_array[1]':'rsrc_object_array[RSCTREE_SLAVE]',
                   'rsrc_object_array[2]':'rsrc_object_array[RSCTREE_MIDIRING_TIMEOUT]',
                   'rsrc_object_array[3]':'rsrc_object_array[RSCTREE_GAME_TERMINATED]',
                   'rsrc_object_array[4]':'rsrc_object_array[RSCTREE_MAZE_ERROR]',
                   'rsrc_object_array[5]':'rsrc_object_array[RSCTREE_MOUSE_CTRL]',
                   'rsrc_object_array[6]':'rsrc_object_array[RSCTREE_JOYSTICK_CTRL]',
                   'rsrc_object_array[7]':'rsrc_object_array[RSCTREE_NUMBER_XX]',
                   'rsrc_object_array[8]':'rsrc_object_array[RSCTREE_PLAY_DIALOG]',
                   'rsrc_object_array[9]':'rsrc_object_array[RSCTREE_TEAM_DIALOG]',
                   'rsrc_object_array[10]':'rsrc_object_array[RSCTREE_SUSPENDED]',
                   'rsrc_object_array[11]':'rsrc_object_array[RSCTREE_SET_NAME]',
                   'rsrc_object_array[12]':'rsrc_object_array[RSCTREE_ABOUT]',
                   'rsrc_object_array[13]':'rsrc_object_array[RSCTREE_MIDICAM]',
                   # VT52 ESC codes
                   'Bconout(CON, 89)':'Bconout(CON, \'Y\')',
                   'Bconout(CON, 98)':'Bconout(CON, \'b\')',
                   'Bconout(CON, 99)':'Bconout(CON, \'c\')',
                   # convert the compass direction into ASCII characters. These 2 groups will catch all cases in MM
                   'viewCompassDirChar == 110':'viewCompassDirChar == \'n\'',
                   'viewCompassDirChar == 115':'viewCompassDirChar == \'s\'',
                   'viewCompassDirChar == 119':'viewCompassDirChar == \'w\'',
                   'viewCompassDirChar == 101':'viewCompassDirChar == \'e\'',
                   ', 110)':', \'n\')',
                   ', 115)':', \'s\')',
                   ', 119)':', \'w\')',
                   ', 101)':', \'e\')',
                   # Colors
                   'i+3':'i+COLOR_BLUE_INDEX',
                   'color_cnv_frame[i] = 0':'color_cnv_frame[i] = COLOR_BLACK_INDEX',
                   '2 : 7':'COLOR_MAGNESIUM_INDEX : COLOR_STEEL_INDEX',
                   'wand_farb_tab[0] = 2':'wand_farb_tab[0] = COLOR_MAGNESIUM_INDEX',
                   'wand_farb_tab[1] = 2':'wand_farb_tab[1] = COLOR_MAGNESIUM_INDEX',
                   'Bconout(CON, 0)':'Bconout(CON, COLOR_BLACK_INDEX)',
                   '(screen_rez) ? 1 : 7':'screen_rez ? COLOR_SILVER_INDEX : COLOR_STEEL_INDEX',
                   '(screen_rez) ? 1 : 15':'screen_rez ? COLOR_SILVER_INDEX : COLOR_WHITE_INDEX',
                   '? 0 : color_cnv_frame':'? COLOR_BLACK_INDEX : color_cnv_frame',
                   '? 0 : color_ply_frame':'? COLOR_BLACK_INDEX : color_ply_frame',
                   'bw_fillpattern_table[0]':'bw_fillpattern_table[COLOR_BLACK_INDEX]',
                   'bw_fillpattern_table[2]':'bw_fillpattern_table[COLOR_MAGNESIUM_INDEX]',
                   'col_setcolor_jumptable[14]':'col_setcolor_jumptable[COLOR_RED_INDEX]',
                   'col_setcolor_jumptable[5])':'col_setcolor_jumptable[COLOR_DKGREEN_INDEX])',
                   '219':'NUM_OBS',
                   'obj < 14':'obj < NUM_TREE',
                   '((screen_rez) ? 6 : 3)':'MAP_BORDER',
                   '((screen_rez) ? 307 : 153)':'MAP_WIDTH',
                   '(screen_rez) ? 307 : 153':'MAP_WIDTH',
                   '((screen_rez) ? 188 : 94)':'MAP_HEIGHT',
                   '(screen_rez) ? 188 : 94':'MAP_HEIGHT',
                   'ply_dir = 0':'ply_dir = PLAYER_DIR_NORTH',
                   'ply_dir > 0':'ply_dir > PLAYER_DIR_NORTH',
                   'ply_dir <= 32':'ply_dir <= PLAYER_DIR_NORTHEAST',
                   'ply_dir > 32':'ply_dir > PLAYER_DIR_NORTHEAST',
                   'ply_dir <= 96':'ply_dir <= PLAYER_DIR_SOUTHEAST',
                   'ply_dir > 96':'ply_dir > PLAYER_DIR_SOUTHEAST',
                   'ply_dir <= 160':'ply_dir <= PLAYER_DIR_SOUTHWEST',
                   'ply_dir > 160':'ply_dir > PLAYER_DIR_SOUTHWEST',
                   'ply_dir <= 224':'ply_dir <= PLAYER_DIR_NORTHWEST',
                   'ply_dir > 224':'ply_dir > PLAYER_DIR_NORTHWEST',
                   'dir = 0':'dir = PLAYER_DIR_NORTH',
                   '.ply_dir != 128':'.ply_dir != PLAYER_DIR_SOUTH',
                   'dir >= 128':'dir >= PLAYER_DIR_SOUTH',
                   '< 64':'< PLAYER_DIR_EAST',
                   '= 64':'= PLAYER_DIR_EAST',
                   '? 64 ':'? PLAYER_DIR_EAST ',
                   '? 128':'? PLAYER_DIR_SOUTH',
                   '< 128':'< PLAYER_DIR_SOUTH',
                   ' = 128':' = PLAYER_DIR_SOUTH',
                   '== 128':'== PLAYER_DIR_SOUTH',
                   ': 192':': PLAYER_DIR_WEST',
                   '< 192':'< PLAYER_DIR_WEST',
                   '= 192':'= PLAYER_DIR_WEST',
                   '] = 256':'] = (PLAYER_DIR_NORTH+256)',
                   '] == 256':'] == (PLAYER_DIR_NORTH+256)',
                   'ply_dir < 256':'ply_dir < (PLAYER_DIR_NORTH+256)',
                   'mouse_record[0] == -5':'mouse_record[0] == 0xfb',
                   'mouse_record[0] == -6':'mouse_record[0] == 0xfa',
                   'mouse_record[0] == -7':'mouse_record[0] == 0xf9',
                   'return -1':'return FAILURE',
                 }

    while True:
        found = False
        lno = 0
        while lno < len(lineBuffer):
            line = lineBuffer[lno]
            if line.startswith('blit_draw_vline_') or line.startswith('blit_draw_hline_') or line.startswith('blit_fill_box_'):
                elements = line.split(', ')
                try:
                    val = int(elements[-1][:-1])
                    colorStr = [ 'COLOR_BLACK_INDEX', 'COLOR_SILVER_INDEX', 'COLOR_MAGNESIUM_INDEX', 'COLOR_BLUE_INDEX', 'COLOR_GREY_INDEX', 'COLOR_DKGREEN_INDEX', 'COLOR_ALUMINIUM_INDEX', 'COLOR_STEEL_INDEX', 'COLOR_YELLOW_INDEX', 'COLOR_ORANGE_INDEX', 'COLOR_PURPLE_INDEX', 'COLOR_MAGENTA_INDEX', 'COLOR_LTBLUE_INDEX', 'COLOR_GREEN_INDEX', 'COLOR_RED_INDEX', 'COLOR_WHITE_INDEX' ]
                    lineBuffer[lno] = ', '.join(elements[:-1] + [colorStr[val]]) + ')'
                except:
                    pass
            match = re.search(regExOBState, line)
            if match:
                dd = match.groupdict()
                bitMask = int(dd['bitmask'])
                if dd['andorflag'] == '&':
                    bitMask = ~bitMask
                bits = []
                for bit in range(0,31):
                    if (1<<bit) & bitMask:
                        bits.append(obStates[1<<bit])
                bitStr = '|'.join(bits)
                if dd['andorflag'] == '&':
                    if len(bits) > 1:
                        bitStr = '~(%s)' % bitStr
                    else:
                        bitStr = '~%s' % bitStr
                lineBuffer[lno] = match.string[:match.start()]+'.ob_state '+dd['andorflag']+'= '+bitStr+match.string[match.end():]
                found = True

            match = re.search(regExpr, lineBuffer[lno])
            if match:
                osDict = match.groupdict('0')
                dd = osMapping[osDict['osfunction']]
                osFunctionId = int(osDict['functionid'])
                if osFunctionId in dd:
                    lineBuffer[lno] = match.string[:match.start()]+dd[osFunctionId]+'('+match.string[match.end():]
                else:
                    print '@@@', osFunctionId,dd
                found = True

            for regEx in regExList:
                dict = regExList[regEx]
                match = re.search(regEx, lineBuffer[lno])
                if match:
                    resultDict = match.groupdict('0')
                    lineBuffer[lno] = match.string[:match.start()]+resultDict['function']+'('+dict[int(resultDict['parameter'])]+match.string[match.end():]
                    found = True

            match = re.search(regExprDrone, lineBuffer[lno])
            if match:
                dd = match.groupdict()
                lineBuffer[lno] = match.string[:match.start()]+' '+droneTable[int(dd['droneid'])]+match.string[match.end():]
                found = True

            for replStr in replaceStr:
                lstr = lineBuffer[lno].replace(replStr, replaceStr[replStr])
                if lstr != lineBuffer[lno]:
                    lineBuffer[lno] = lstr
                    found = True

            lno += 1
        if not found:
            break
    return lineBuffer

def addWhileForBrackets(lineBuffer):
    lno = 1
    while lno < len(lineBuffer):
        lineStr = lineBuffer[lno]
        if lineStr == '{':
            lineBuffer[lno-1] = lineBuffer[lno-1] + ' {'
            del lineBuffer[lno]
            continue
        lno += 1
    return lineBuffer

def addSemicolons(lineBuffer):
    lno = 0
    while lno < len(lineBuffer):
        lineStr = lineBuffer[lno]
        if (not lineStr.endswith('{') and not lineStr.startswith('}') and not lineStr.endswith(':')) or lineStr.startswith('} while'):
            if lineStr.startswith('return ') or lineStr.startswith('goto ') or lineStr.startswith('if(') or lineStr.startswith('}'):
                pass
            else:
                lineStr = simplifyTerm(lineStr)
            lineBuffer[lno] = lineStr + ';'
        lno += 1
    return lineBuffer

def indentSource(lineBuffer):
    lno = 0
    indent = 4
    while lno < len(lineBuffer):
        lineStr = lineBuffer[lno]
        if lineStr.startswith('}'):
            indent -= 4
            lineBuffer[lno] = ' ' * indent + lineStr
        if lineStr == '{' or ' {\n' in lineStr or lineStr.endswith(' {'):
            lineBuffer[lno] = ' ' * indent + lineStr
            indent += 4
        else:
            relativeIndent = 0
            if lineStr.startswith('case ') or lineStr == 'default:':
                relativeIndent = -4
            lineBuffer[lno] = ' ' * (indent + relativeIndent) + lineStr
        lno += 1
    return lineBuffer

if generateTempCode:
    tempCodeDirectory = './TEMPCODE/'
    try:
        shutil.rmtree(tempCodeDirectory)
    except:
        pass
    os.mkdir(tempCodeDirectory)       

def generateCode(lineBuffer, function, label):
    if not generateTempCode:
        return
    file = open(tempCodeDirectory + label, 'a+')
    file.write('/' + '*' * 60 + '\n')
    file.write(' *** ' + function.Cstring(False) + '\n')
    file.write(' ' + '*' * 60 + '/\n')
#    file.write(','.join(function.globaleLabels) + '\n')
    file.write(function.Cstring(True,True) + '\n')
    for l in outputLines:
        file.write(l.replace('/*','/ *') + '\n')    # division by a dereferenced pointer is not a comment...
    file.write('}\n')
    file.write('\n')


if not generateTempCode:
    outputDirectory = './Midi-Maze-1/SOURCES/'
    try:
        shutil.rmtree(outputDirectory)
    except:
        pass
    os.mkdir(outputDirectory)       

fileGlobalSymbols = {}
fileLineCache = {}
fileFunctionCache = {} # which functions are defined in a file
fileFunctionCallCache = {} # which functions are used in a file

flist = findFunctions(AsmSourceCode(cleanSourceLines(originalSourceLines)))
for f in flist:
#    if f.varName != 'play_game':
#        continue
    outputLines = parseFunction(f)
    generateCode(outputLines, f, "010_parse.c")
    outputLines = deleteUnreachableGotos(outputLines)                 # C generates sometimes BRA aaa followed by BRA bbb, which will never be reached
    generateCode(outputLines, f, "020_double.c")
    outputLines = expressionsToVariables(outputLines,f)
    generateCode(outputLines, f, "030_expressions.c")
    if 1:                        
        outputLines = mergeFUNCTIONPREVLINE(outputLines)        # merge the previous line into a function parameter, like function(a=2, b=3)
        generateCode(outputLines, f, "050_function_prev.c")
        outputLines = generateIncrDecr(outputLines)             # += 1 into a pre-increment and -= 1 into a pre-decrement
        generateCode(outputLines, f, "060_incr_decr.c")
        outputLines = mergeCompares(outputLines)                # merge several conditions into one if()
        generateCode(outputLines, f, "070_mergeCompares.c")
        outputLines = mergeCompares2(outputLines)               # merge if conditions to the same destination
        generateCode(outputLines, f, "080_mergeCompares2.c")
        outputLines = cleanupIf(outputLines)                    # cleanup compares with 0
        generateCode(outputLines, f, "090_cleanup.c")
        outputLines = mergeSimpleQuestionmarkTerm(outputLines)  # merge simple ? : terms into one line (1. pass)
        generateCode(outputLines, f, "100_questionmark.c")
        outputLines = mergeSimpleQuestionmarkTerm(outputLines)  # merge simple ? : terms into one line (1. pass)
        generateCode(outputLines, f, "101_questionmark.c")
        outputLines = mergeAssignments(outputLines)             # merge assigns in the style: a = temp_xx; b = temp_xx into a = b = temp_x
        generateCode(outputLines, f, "110_mergeAssignments.c")
        for reg in mathRegs:                                    # merge assigns temp_xx = a; dest ?= temp_xx into one line. xx always seems to be D0
            outputLines = mergeTempMinimal(outputLines, reg)
        generateCode(outputLines, f, "120_mergeTemp.c")
        outputLines = buildFunctionCalls(outputLines)
        generateCode(outputLines, f, "130_buildFunctionCalls.c")
        outputLines = expressionsToVariables(outputLines,f)     # 2nd round to find expressions created by the function folding
        for tries in range(0,2):
            for reg in mathRegs:
                outputLines = mergeTempRegs(outputLines,reg,f.returnLabel,f.varType)  # merge temp regs several times, till it's stable
        generateCode(outputLines, f, "150_mergeTemp.c")
        outputLines = mergeCompares2(outputLines)               # merge if conditions to the same destination
        generateCode(outputLines, f, "160_mergeCompares2.c")
        outputLines = mergePREVLINE(outputLines)                # merge the previous line into a condition, this is typically used for if((a = func()) < 0) or similar. Needed before for() and while()
        generateCode(outputLines, f, "170_mergePREVLINEB.c")
        if 1:
            # now we start generating { } pairs! This makes it harder to find {} pairs for indirection
            outputLines = createForLoops(outputLines)               # for loops are a special while case, we create them first
            generateCode(outputLines, f, "200_for.c")
            outputLines = createWhileLoops(outputLines)             # now create all while loops
            generateCode(outputLines, f, "210_while.c")
            outputLines = createDoWhileLoops(outputLines)           # now create all do while loops
            generateCode(outputLines, f, "220_do_while.c")
            outputLines = createSwitch(outputLines)               # now create all switch()
            generateCode(outputLines, f, "230_switch.c")
            outputLines = createReturn(outputLines, f.returnLabel)  # a goto the end of the function is a 'return'
            generateCode(outputLines, f, "240_return.c")
            outputLines = mergeCompares(outputLines)              # merge if() lines
            generateCode(outputLines, f, "245_mergeCompares.c")
            outputLines = createIfElse(outputLines)              # now create the if/else cases
            generateCode(outputLines, f, "250_if_else.c")
        outputLines = mergeIncrDecrWithPrevline(outputLines)    # merge ++ or -- with the previous line, if possible
        generateCode(outputLines, f, "300_mergeIncrDecrWithPrevline.c")
        outputLines = removeUnusedTempD0(outputLines)           # remove all assignments to D0, which are not used
        generateCode(outputLines, f, "310_unused_D0.c")
        outputLines = fixBITTST(outputLines)
        generateCode(outputLines, f, "320_BITTST.c")
        outputLines = fillStaticPointers(outputLines)
        generateCode(outputLines, f, "330_staticPointers.c")
        outputLines = cleanupIfConditions(outputLines)          # Cleanup if conditions
        generateCode(outputLines, f, "340_cleanupIF.c")
        outputLines = fixABS(outputLines)
        generateCode(outputLines, f, "350_abs.c")
        outputLines = fixAtariOSFunctions(outputLines)          # Fix Atari BIOS/XBIOS/GEMDOS functions
        generateCode(outputLines, f, "400_atari.c")
        foundLostLabels(outputLines)                            # DEBUG: find all label targets, which no longer exist => BUG!
        findBackJumps(outputLines,f)
        outputLines = addWhileForBrackets(outputLines)          # pull { after for() and while() into the line of the statement
        generateCode(outputLines, f, "900_while_and_for_bracket.c")
        outputLines = addSemicolons(outputLines)                # add semicolons
        generateCode(outputLines, f, "901_semicolons.c")
        outputLines = indentSource(outputLines)                 # indent the code between { ... }
        generateCode(outputLines, f, "999_indent.c")
    if not generateTempCode:
        # which functions are used in this file?
        label = None
        for ll in globalVars:
            var = globalVars[ll]
            if var == f:
                label = ll
                break
        if f.destinationFilename in fileFunctionCache:
            fileFunctionCache[f.destinationFilename].append(label)
        else:
            fileFunctionCache[f.destinationFilename] = [label]

        # which functions are called from this file?
        if f.destinationFilename in fileFunctionCallCache:
            fileFunctionCallCache[f.destinationFilename] = fileFunctionCallCache[f.destinationFilename] | f.referencedFunctionLabels
        else:
            fileFunctionCallCache[f.destinationFilename] = f.referencedFunctionLabels

        lines = []
        if f.destinationFilename in fileLineCache:
            lines = fileLineCache[f.destinationFilename]

        lines.append('/' + '*' * 60 + '\n')
        lines.append(' *** ' + f.Cstring(False) + '\n')
        lines.append(' ' + '*' * 60 + '/\n')
        lines.append(f.Cstring(True,True) + '\n')
        for l in outputLines:
            lines.append(l.replace('/*','/ *') + '\n')    # division by a dereferenced pointer is not a comment...
        lines.append('}\n')
        lines.append('\n')

        fileLineCache[f.destinationFilename] = lines

        if f.destinationFilename in fileGlobalSymbols:
            fileGlobalSymbols[f.destinationFilename] = fileGlobalSymbols[f.destinationFilename] | f.globalLabels
        else:
            fileGlobalSymbols[f.destinationFilename] = f.globalLabels

if generateTempCode:
    sys.exit(0)

# which functions are shared globally?
globalFunctionLabels = set()
for filename in fileFunctionCache:
    globalFunctionLabels |= fileFunctionCallCache[filename]-set(fileFunctionCache[filename])

# clean up
for filename in fileGlobalSymbols:
    gl = set()
    for label in fileGlobalSymbols[filename]:
        if label+'_BASE' in globalVars:
            label = label + '_BASE'
        if label in globalVars and label not in globalSubVars and not globalVars[label].isFunction:
            gl.add(label)
    fileGlobalSymbols[filename] = gl

# count usage of labels in their sources
labelCounter = collections.Counter()
for retainLabel in ['L0B46','L0A7F','L0A89']:
    labelCounter[retainLabel] = 1
for filename in fileGlobalSymbols:
    for label in fileGlobalSymbols[filename]:
        labelCounter[label] += 1

globVarSet = set()

for filename in sorted(fileGlobalSymbols):
    gvarList = []
    sv = set()
    for label in sorted(fileGlobalSymbols[filename]):
        if labelCounter[label] > 1:
            globVarSet.add(label)
        else:
            sv.add(label)
    for label in sorted(sv):
        var = globalVars[label]
        if var.hasBeenUsed: # if the variable has been "used-up" we don't need it anymore
            continue
        for v in globalVarCode[label].split('\n'):
            if v[0] == ' ':
                gvarList.append(v+'\n')
            else:
                gvarList.append('static '+v+'\n')
    if len(gvarList) > 0:
        gvarList.append('\n')
        fileLineCache[filename] = gvarList + fileLineCache[filename]

def globalDefinesIntoFile(file,filename):
    if filename in globalDefines:
        for line in globalDefines[filename]:
            file.write(line + '\n')
        file.write('\n')

def globalTypesIntoFile(file,filename):
    foundTypes = False
    for g in globalTypeList:
        if globalTypes[g].isBaseType:
            continue
        t = globalTypes[g]
        if not t.destinationFilename:
            continue
        if t.destinationFilename != filename:
            continue
        file.write('typedef struct {\n')
        for v in t.structVars[0]:
            file.write('    %s;\n' % (v.Cstring(False)))
        file.write('} %s;\n' % (t.varName))
        file.write('\n')
        foundTypes = True
    if foundTypes:
        file.write('\n')


finalPatchesStr = {
                   # merge these two lines include a cast
                   'scrPtr = saveAreas[areaIndex].offset;':'scrPtr = (unsigned short*)(saveAreas[areaIndex].offset + (char*)screen_ptr[0]);',
                   'scrPtr += screen_ptr[0];':None,
                   # main is a special case. On the Atari it is a void function, but in ANSI-C it has to return a value.
                   'void main();':None,
                   'void main()':'int main()',
#                   'imageMaskPtr = &crossedsmil_img':'imageMaskPtr = (unsigned long*)crossedsmil_img',
#                   'flip_crossedsmil_img();':'//flip_crossedsmil_img();',
                   # We need to patch the code to do correct byte swap on Intel machines
                   'unsigned long *imageMaskPtr':'unsigned short *imageMaskPtr',
                   'orgBitMask = imageMaskPtr[i]':'orgBitMask = (imageMaskPtr[i*2]<<16)|imageMaskPtr[i*2+1]',
                   'imageMaskPtr[i] = newBitMask':'imageMaskPtr[i*2] = newBitMask>>16; imageMaskPtr[i*2+1] = newBitMask',
                   # casts are necessary for non-matching types
                   'const void *col_setcolor_jumptable[16]':'void (*col_setcolor_jumptable[16])(unsigned short *scrPtr, int xoffs, int orMask, int andMask)',
                   'const void *colorFuncPtr':'void (*colFuncPtr)(unsigned short *,int,int,int)',
                   ', KBDVECS_ptr->kb_mousevec':', (void (*)())KBDVECS_ptr->kb_mousevec',
                   'kb_mousevec = own_mousevec':'kb_mousevec = (void *)own_mousevec',
                   'screen_ptr[1] = Physbase()':'screen_ptr[1] = (unsigned short*)Physbase()',
                   'Setpalette(':'Setpalette((void*)',
                   '&rs_tedinfo':'(long)&rs_tedinfo',
                   'rs_strings[rs_tedinfo':'rs_strings[(long)rs_tedinfo',
                   '.ob_spec = rs_strings':'.ob_spec = (long)rs_strings',
                   'name = rsrc_object_array':'name = (char *)rsrc_object_array',
                   ', rsrc_object_array':', (char*)rsrc_object_array',
                   'playerNameStrPtr = rsrc_object_array':'playerNameStrPtr = (char*)rsrc_object_array',
                   'decompress_image_to_screen(&load_buffer,':'decompress_image_to_screen((unsigned short *)load_buffer,',
                   'Setscreen(screen_ptr[screen_flag], -1, -1);':'Setscreen(screen_ptr[screen_flag], (void*)-1, -1);',
                   'Dosound(':'Dosound((void*)',
                   'sine_table = &load_buffer':'sine_table = (short *)load_buffer',
                   'shape_face_ptr = &load_buffer[130]':'shape_face_ptr = (unsigned short *)(&load_buffer[130])',
                   'shape_ptr = &load_buffer[3302]':'shape_ptr = (unsigned short *)(&load_buffer[3302])',
                   # the '&' is simply wrong, but it allows easier parsing. We remove the compiler errors by patching.
                   'v_opnvwk(&intin, &vdi_handle, &intout)':'v_opnvwk(intin, &vdi_handle, intout)',
                   '&sound_shot':'sound_shot',
                   '&sound_hit':'sound_hit',
                   '&own_mousevec':'own_mousevec',
                   '&live_shape_img':'live_shape_img',
                   '&mapsmily_img':'mapsmily_img',
                   '&mapsmily2_img':'mapsmily2_img',
                   '&loosershape_img':'loosershape_img',
                   '&blinzshape_img':'blinzshape_img',
                   '&draw_shadow_img':'draw_shadow_img',
                   'crossedsmil_ptr = &crossedsmil_img':'crossedsmil_ptr = crossedsmil_img',
                   'imageMaskPtr = &crossedsmil_img':'imageMaskPtr = crossedsmil_img',
                   '&own_joyvec':'(void*)own_joyvec',
                   '&mouse_defaults':'mouse_defaults',
                   '&rs_object':'rs_object',
                   '&rsc_drones':'rsc_drones',
                   '&pxy':'pxy',
                   '&colortable':'colortable',
                   'form_alert(1, &alertBuf)':'form_alert(1, alertBuf)',
                   '&active_drones_by_type':'active_drones_by_type',
                   '&buffer':'buffer',
                   'fsel_input(&filepath, &filename':'fsel_input(filepath, filename',
                   'LoadMaze(&mazePathAndName, &filename)':'LoadMaze(mazePathAndName, filename)',
                   '    Bconin(MIDI)&0xff;':'    Bconin(MIDI);',
                   # silence clang unused warning
                   'unused_14;':'unused_14 __attribute__((unused));',
                   'unused_18;':'unused_18 __attribute__((unused));',
                   'unused_22;':'unused_22 __attribute__((unused));',
                   'unused_24;':'unused_24 __attribute__((unused));',
                   'unused_26;':'unused_26 __attribute__((unused));',
                   # silence clang warning around an assignment in a while() or if() condition
                   'while(mazePathAndName[i++] = filepath[j++])':'while((mazePathAndName[i++] = filepath[j++]))',
                   'while(mazePathAndName[i++] = filename[j++])':'while((mazePathAndName[i++] = filename[j++]))',
                   'while(alertBuf[fieldY++]':'while((alertBuf[fieldY++]',
                   '[fieldX++]) {':'[fieldX++])) {',
                   '(user_is_midicam = isMidicamFlag)':'((user_is_midicam = isMidicamFlag))',
                   # we need to put brackets for the <</>> operator, otherwise clang will issue a warning
                   '5-tries/20<<8':'(5-tries/20)<<8',
                   'saveCellY+shotYField>>1':'(saveCellY+shotYField)>>1',
                   'saveCellX+shotXField>>1':'(saveCellX+shotXField)>>1',
                   '(shapeScale-16<<1)+16':'((shapeScale-16)<<1)+16',
                   '8-(size-15>>1)':'8-((size-15)>>1)',
                   '(size-1>>3)+1':'((size-1)>>3)+1',
                   '3<<bitPos+bitPos':'3<<(bitPos+bitPos)',
                   'imgPtrReg[wordsReg]>>bitPos+8)':'imgPtrReg[wordsReg]>>(bitPos+8))',
                   'deltaY = draw_maze_viewmatrix':'deltaY = (draw_maze_viewmatrix',
                   '.deltaY>>1;':'.deltaY)>>1;',
                   'deltaX = draw_maze_viewmatrix':'deltaX = (draw_maze_viewmatrix',
                   '.deltaX>>1;':'.deltaX)>>1;',
                   # type is wrong.
                   'Bconout(MIDI, Random())':'Bconout(MIDI, (int)Random())',
                   # variable not initialized warning by Clang. Probably theoretical errors.
                   'int returnCode;':'int returnCode = -1;',
                   'int midiByte;':'int midiByte = 0;',
                   'int joystickDirection;':'int joystickDirection = 0;',
                   'int joystickButton;':'int joystickButton = 0;',
                   # wrong detecting of the type. Patching is the easiest.
                   '(&draw_maze_viewmatrix, ':'(&draw_maze_viewmatrix[0][0].deltaY, ',
                   # accidentally the author added another 0 as a parameter
                   'set_object(player, player_data[player].ply_y, player_data[player].ply_x, 0);':'set_object(player, player_data[player].ply_y, player_data[player].ply_x);',
                   # accidentally the author returned a value inside functions without a return value
                   '    0;':None, # Remove unused return value
                   '   -1;':None, # Remove unused return value
                   # cast the screen address
                   '&screenBuffer[255]&-256':'(unsigned short *)(((long)&screenBuffer[255]) &-256)',
                   # short ptr addition
                   'screen_ptr[screen_flag]+screen_offset':'(unsigned short *)((char*)screen_ptr[screen_flag]+screen_offset)', # fix that the offset actually is added to an unsigned short *
                   # short ptr addition
                   '(screen_rez ? 40 : 80)<<1':'screen_rez ? 40 : 80',
                   # OBJECT * pointer addition
                   'lmul(rs_trindex[obj], 24)':'rs_trindex[obj]',
                   # typically we don't add a ';' behind a label. This is the exception
                   'SLAVE_LOOP_EXIT:':'SLAVE_LOOP_EXIT: ;',
                   # The Atari ST ignores a division by zero. If the wall has no slope, other platforms would crash
                   'slope = slope/(h2-h1);':'if(h2==h1) /* PATCH DIV0 */ return; slope = slope/(h2-h1);',
                   # we don't correctly detect a break out of two while loops. This only occurs once, so we patch it
                   'obj_index == 48) break':'obj_index == 48) goto done', # double-while loop, we break out of both

                   # adding comments to the keycodes and convert them to hex
                   'key_code == 2064':'key_code == 0x810 /* ALT-Q */',
                   'key_code == 2067':'key_code == 0x813 /* ALT-R */',
                   'key_code == 2079':'key_code == 0x81F /* ALT-S */',
                   'key_code == 2084':'key_code == 0x824 /* ALT-J */',
                   'key_code == 2094':'key_code == 0x82E /* ALT-C */',
                   'key_code == 2098':'key_code == 0x832 /* ALT-M */',
             }
def processLine(line):
    for replStr in finalPatchesStr:
        if finalPatchesStr[replStr] is not None:
            line = line.replace(replStr, finalPatchesStr[replStr])
        elif replStr in line:
            return None
    return line

# Generate GLOBAL.H
globalHeaderFile = open(outputDirectory + 'GLOBALS.H', 'a+')
globalHeaderFile.write('/' + '*' * 60 + '\n')
globalHeaderFile.write(' *** Header for global variables and functions\n')
globalHeaderFile.write(' ' + '*' * 60 + '/\n')
globalHeaderFile.write('#include "PORTAB.H"\n')
globalHeaderFile.write('\n')
globalDefinesIntoFile(globalHeaderFile, 'GLOBALS.H')
globalTypesIntoFile(globalHeaderFile, 'GLOBALS.H')
for label in sorted(globVarSet):
    var = globalVars[label]
    line = 'extern ' + var.Cstring(False) + ';\n'
    line = processLine(line)
    if line:
        globalHeaderFile.write(line)
globalHeaderFile.write('\n')
for label in sorted(globalFunctionLabels):
    var = globalVars[label]
    line = 'extern ' + var.Cstring(False) + ';\n'
    if '(...)' in line:
        continue
    line = processLine(line)
    if line:
        globalHeaderFile.write(line)

# Generate GLOBALS.C
globalVarsFile = open(outputDirectory + 'GLOBALS.C', 'a+')
globalVarsFile.write('/' + '*' * 60 + '\n')
globalVarsFile.write(' *** Global variables\n')
globalVarsFile.write(' ' + '*' * 60 + '/\n')
globalVarsFile.write('#include "GLOBALS.H"\n')
globalVarsFile.write('\n')
globalDefinesIntoFile(globalVarsFile, 'GLOBALS.C')
globalTypesIntoFile(globalHeaderFile, 'GLOBALS.C')
for label in sorted(globVarSet):
    for v in globalVarCode[label].split('\n'):
        line = processLine(v.replace('&','') + '\n')
        if line:
            globalVarsFile.write(line)

# Generate all other source files
foundPatch = False
for filename in fileLineCache:
    file = open(outputDirectory + filename, 'a+')
    file.write('/' + '*' * 60 + '\n')
    file.write(' *** ' + filename + '\n')
    file.write(' ' + '*' * 60 + '/\n')
    file.write('#include "GLOBALS.H"\n')
    file.write('\n')
    globalDefinesIntoFile(file, filename)
    globalTypesIntoFile(file, filename)
    for label in fileFunctionCache[filename]:
        if label in globalFunctionLabels:
            continue
        var = globalVars[label]
        line = processLine(var.Cstring(False) + ';\n')
        if line:
            file.write(line)
    file.write('\n')
    for line in fileLineCache[filename]:
        line = processLine(line)
        if line:
            if filename == 'PREFDIALOG.C':
                if line.startswith('    form_dial(FMD_FINISH, fo_cx, fo_cy, fo_cw, fo_ch, fo_cx, fo_cy, fo_cw, fo_ch);'):
                    file.write('done:\n')
                    foundPatch = True
            file.write(line)

if not foundPatch:
    print '### PATCH MISSING!'
