#!/usr/bin/env python
# -*- coding: utf-8 -*-

import sys
import lexer

debugPrint = False

class TermParser:
    def nextToken(self):
        try:
            self.cc = self.lx.token()
        except:
            print '@@@ Unknown token in \"%s\"' % self.str
            print self.stack
            sys.exit(0)

    def dumpStack(self):
        a = []
        index = 0
        for stk in self.stack:
#            a.append('%d:%s' % (stk[0],stk[2]))
            a.append('%d:%d:#%d:\"%s\"' % (index,stk[0],stk[1],stk[2]))
            index += 1
#            a.append(stk[2])
        return ' --- '.join(a)

    def addStack(self,pcount,level,data):
        self.stack.append([pcount,level,data])

    def errorMsg(self,msg):
        print '@@@ '+msg+' => %s in \"%s\"' % (self.cc, self.str)
        print self.stack
        sys.exit(0)

    def brackets(self):
        castStackStart = len(self.stack)
        castStart = self.cc.pos
        castEnd = None
        if self.cc and self.cc.type=='(':
            hasCast = True
            self.nextToken()
            self.term()
            if self.cc.type != ')':
                self.errorMsg('Term closing bracket missing')
            self.nextToken()
            if self.cc:
                castEnd = self.cc.pos
        # () before a NUM or IDENT is a cast, otherwise they are just brackets
        if self.cc and (self.cc.type=='NUM' or self.cc.type=='IDENT' or self.cc.type=='STR' or self.cc.type=='['):
            castStr = ''
            if castEnd:
                self.stack = self.stack[:castStackStart]
                castStr = self.str[castStart:castEnd]
            oldStackSize = len(self.stack)
            a = castStr
            if self.cc.type!='[':
                a += self.cc.val
                self.nextToken()
            while True:
                found = False
                if self.cc and self.cc.type=='(': # function
                    found = True
                    a += '('
                    while True:
                        self.nextToken()
                        saveStack = self.stack
                        self.stack = []
                        self.term()
                        # optimize the term inside the [], because it is treated as a string later
                        b = self.generaterString()
                        a += b
                        self.stack = saveStack
                        if self.cc.type!=',':
                            break
                        a += ', '
                    if self.cc.type != ')':
                        self.errorMsg('Function closing bracket missing')
                    self.nextToken()
                    a += ')'
                if self.cc and self.cc.type=='[':
                    found = True
                    self.nextToken()
                    saveStack = self.stack
                    self.stack = []
                    b = self.term()
                    if True:    # optimize the term inside the [], because it is treated as a string later
                        b = self.generaterString()
                    self.stack = saveStack
                    if self.cc.type != ']':
                        self.errorMsg('Array closing bracket missing')
                    self.nextToken()
                    a += '[%s]' % b
                if self.cc and (self.cc.type=='.' or self.cc.type=='->'):
                    op = self.cc.type
                    self.nextToken()
                    found = True
                    b = self.cc.val
                    self.nextToken()
                    a += op + b
                if self.cc and (self.cc.type=='++' or self.cc.type=='--'):
                    op = self.cc.type
                    self.nextToken()
                    found = True
                    a += op
                if not found:
                    break
            self.stack = self.stack[:oldStackSize]
            self.addStack(0,100,a)

    def sign(self):
        ops = []
        # there can be more than one prefix
        while self.cc and self.cc.type in ['!','-','+','*','&','--','++','~']:
            ops.append(self.cc.type)
            self.nextToken()
        self.brackets()
        for op in reversed(ops):
            self.addStack(1,50,op)

    def muldiv(self):
        self.sign()
        while self.cc:
            op = self.cc.type
            if op not in ['*','/','%']:
                break
            self.nextToken()
            self.sign()
            self.addStack(2,40,op)

    def addsub(self):
        self.muldiv()
        while self.cc:
            op = self.cc.type
            if op not in ['+','-']:
                break
            self.nextToken()
            self.muldiv()
            self.addStack(2,30,op)

    def andorxor(self):
        self.addsub()
        while self.cc:
            op = self.cc.type
            if op not in ['&','|','^']:
                break
            self.nextToken()
            self.addsub()
            self.addStack(2,9,op)   # lower than the compare operations, because of K&R-C

    def shift(self):
        self.andorxor()
        while self.cc:
            comp = self.cc.type
            if comp not in ['<<','>>']:
                break
            self.nextToken()
            self.andorxor()
            self.addStack(2,8,comp)   # lower than the compare operations, because of K&R-C

    def compare(self):
        self.shift()
        while self.cc:
            comp = self.cc.type
            if comp not in ['!=','==','<=','>=','<','>']:
                break
            self.nextToken()
            self.shift()
            self.addStack(2,10,comp)

    def booleanANDcomp(self):
        self.compare()
        stackPos = len(self.stack)
        countElements = 1
        while self.cc:
            comp = self.cc.type
            if comp != '&&':
                break
            self.nextToken()
            self.compare()
            countElements += 1
        if countElements > 1:
            # chain all possible OR or AND operations together. This makes potential optimizations easier.
            self.addStack(countElements,6,'&&')

    def booleanORcomp(self):
        self.booleanANDcomp()
        stackPos = len(self.stack)
        countElements = 1
        while self.cc:
            comp = self.cc.type
            if comp != '||':
                break
            self.nextToken()
            self.booleanANDcomp()
            countElements += 1
        if countElements > 1:
            # chain all possible OR or AND operations together. This makes potential optimizations easier.
            self.addStack(countElements,5,'||')

    def tenerary(self):
        self.booleanORcomp()
        while self.cc:
            comp = self.cc.type
            if comp not in ['?']:
                break
            self.nextToken()
            self.booleanORcomp()
            comp = self.cc.type
            if comp not in [':']:
                self.errorMsg(': of the tenerary operator is missing')
            self.nextToken()
            self.booleanORcomp()
            self.addStack(3,4,'?')

    def assign(self):
        self.tenerary()
        while self.cc:
            comp = self.cc.type
            if comp not in ['=','+=','-=','*=','/=','%=','<<=','>>=','&=','^=','|=']:
                break
            self.nextToken()
            self.tenerary()
            self.addStack(2,2,comp)

    def term(self):
        self.assign()

    def parse(self):
        self.term()
        if self.cc:
            self.errorMsg('Unknown character')

    # Special De Morgan optimizations
    def DeMorgan(self,stack):
        condInvert = { '==':'!=','!=':'==','<':'>=','<=':'>','>':'<=','>=':'<' }
        if len(stack) >= 4:
            # the last two elements always have to be || !
            currentOp = stack[-2][2]
            if (currentOp == '||' or currentOp == '&&') and stack[-1][2] == '!':
                stackTop = len(stack) - 3
                newStack = []
                while stackTop >= 0:
                    lastStackTop = stackTop
                    if stack[stackTop][2] in condInvert: # compare
                        stackTop -= 3
                        operationData = stack[stackTop+1:lastStackTop+1]
                        if operationData[0][0] != 0 or operationData[1][0] != 0:
                            return stack
                        # invert the condition
                        newStack.append([operationData[0], operationData[1], [operationData[2][0],operationData[2][1],condInvert[operationData[2][2]]]])
                        pass
                    elif stack[stackTop][2] == '!': # invert
                        stackTop -= 2
                        operationData = stack[stackTop+1:lastStackTop+1]
                        if operationData[0][0] == 1:
                            return stack
                        elif operationData[0][0] == 2:
                            # 2 parameter, ok, but are the parameters trivial? We could optimize that
                            if stack[stackTop][0] != 0 or stack[stackTop-1][0] != 0:
                                return stack
                            operationData = [stack[stackTop-1],stack[stackTop]] + operationData
                            stackTop -= 2
                        # remove invert at the very end
                        newStack.append(operationData[:-1])
                        pass
                    elif stack[stackTop][0] == 0: # constant/variable
                        stackTop -= 1
                        operationData = stack[stackTop+1:lastStackTop+1]
                        # add invert
                        newStack.append([operationData[0], [1,50,'!']])
                        pass
                    else:
                        return stack
                if stack[-2][0] != len(newStack):
                    return stack
                if currentOp == '||':
                    currentOp = '&&'
                elif currentOp == '&&':
                    currentOp = '||'
                newCompareOperation = [len(newStack),6,currentOp]
                stack = []
                for ops in reversed(newStack):
                    for op in ops:
                        stack.append(op)
                stack.append(newCompareOperation)
        return stack

    def cleanupStack(self):
        pos = 1
        # merge a chain of && or || operations
        while pos<len(self.stack):
            if self.stack[pos-1][2] == '&&' and self.stack[pos][2] == '&&':
                self.stack[pos-1][0] += 1
                del self.stack[pos]
                continue
            elif self.stack[pos-1][2] == '||' and self.stack[pos][2] == '||':
                self.stack[pos-1][0] += 1
                del self.stack[pos]
                continue
            pos += 1

        pos = 0
        while pos<len(self.stack):
            # remove any '+' before a term
            if self.stack[pos][0] == 1 and self.stack[pos][2] == '+':
                del self.stack[pos]
                pos -= 1
            # remove double negations
            elif pos > 1 and self.stack[pos][0] == 1 and self.stack[pos][2] == '-' and self.stack[pos-1][0] == 1 and self.stack[pos-1][2] == '-':
                del self.stack[pos]
                del self.stack[pos-1]
                pos -= 2
            # remove double inversions
            elif pos > 1 and self.stack[pos][0] == 1 and self.stack[pos][2] == '!' and self.stack[pos-1][0] == 1 and self.stack[pos-1][2] == '!':
                del self.stack[pos]
                del self.stack[pos-1]
                pos -= 2
            # merge inverted compares
            elif pos > 1 and self.stack[pos][2] == '!' and self.stack[pos-1][0] == 2:
                newComp = None
                if self.stack[pos-1][2] == '>=':
                    newComp = '<'
                elif self.stack[pos-1][2] == '<=':
                    newComp = '>'
                elif self.stack[pos-1][2] == '==':
                    newComp = '!='
                elif self.stack[pos-1][2] == '!=':
                    newComp = '=='
                elif self.stack[pos-1][2] == '<':
                    newComp = '>='
                elif self.stack[pos-1][2] == '>':
                    newComp = '<='
                if newComp:
                    del self.stack[pos]
                    self.stack[pos-1][2] = newComp
                    pos -= 1
            # merge a term like (a+-16) into (a-16)
            if pos > 1 and self.stack[pos-1][0] == 1 and self.stack[pos-1][2] == '-' and self.stack[pos][0] == 2 and self.stack[pos][2] == '+':
                self.stack[pos][2] = '-'
                del self.stack[pos-1]
                pos -= 1
            pos += 1

        if len(self.stack)>2:
            # !!cond ? a : b with a and b being trivial terms can also be converted
            if self.stack[-1][2] == '?' and self.stack[-2][0] == 0 and self.stack[-3][0] == 0:
                if self.stack[-4][2] == '!' and self.stack[-5][2] == '!':
                    del self.stack[-4]
                    del self.stack[-4]

        # Special De Morgan optimizations
        self.stack = self.DeMorgan(self.stack)

    def bracketsIfNecessary(self, valObject, level):
        if valObject[1] < level:
            valObject[2] = '(' + valObject[2] + ')'
        return valObject

    def generaterString(self):
        if len(self.stack) == 0:
            return ''
        global debugPrint
        if debugPrint:
            print 'Str ',self.str
            print 'ST->',self.dumpStack()
        self.cleanupStack()
        if debugPrint:
            print 'ST<-',self.dumpStack()
            print
        str = ''
        spos = 0
        while spos < len(self.stack):
            (pcount,level,pdata) = self.stack[spos]
            #print pcount,level,pdata,'-->',spos,self.dumpStack()
            # chain all possible OR or AND operations together. This makes potential optimizations easier.
            if pdata == '||' or pdata == '&&':
                newVal = None
                while pcount > 0:
                    newLevel = self.stack[spos-1][1]
                    if newLevel != 10:
                        newLevel = 50
                    valObj = self.bracketsIfNecessary(self.stack[spos-1], newLevel)
                    spos -= 1
                    del self.stack[spos]
                    if newVal:
                        newVal = '%s %s %s' % (valObj[2],pdata,newVal)
                    else:
                        newVal = valObj[2]
                    pcount -= 1
                self.stack[spos] = [0,level,newVal]
                spos += 1
                    
            elif pcount == 0:
                spos += 1
            elif pcount == 1:
                a = self.stack[spos-1]
                if a[1] <= level:   # <= level, so that the same level gets brackets
                    a[2] = '(' + a[2] + ')'
                newVal = '%s%s' % (pdata,a[2])
                self.stack[spos] = [0,level,newVal]
                del self.stack[spos-1]
            elif pcount == 2:
                a = self.bracketsIfNecessary(self.stack[spos-1], level)
                b = self.bracketsIfNecessary(self.stack[spos-2], level)
                # certain operands do not have spaces around them.
                if pdata in ['+','-','*','/','%','&','|','^','<<','>>']:
                    pass
                else:
                    pdata = ' %s ' % pdata
                newVal = b[2]+pdata+a[2]
                self.stack[spos] = [0,level,newVal]
                del self.stack[spos-1]
                del self.stack[spos-2]
                spos -= 1
            elif pcount == 3:
                a = self.stack[spos-1]
                b = self.stack[spos-2]
                c = self.stack[spos-3]
                if a[1] < level:
                    a[2] = '(' + a[2] + ')'
                if b[1] < level:
                    b[2] = '(' + b[2] + ')'
                if c[1] == 4: # 4 = ? : operator, which is associated from right to left!
                    newVal = c[2]+' ? '+b[2]+' : '+a[2]
                else:
                    if c[1] < 50:
                        c[2] = '(' + c[2] + ')'
                    newVal = c[2]+' ? '+b[2]+' : '+a[2]
                self.stack[spos] = [0,level,newVal]
                del self.stack[spos-1]
                del self.stack[spos-2]
                del self.stack[spos-3]
                spos -= 2
            else:
                self.errorMsg('Large parameter number')
        return self.stack[0][2]

    def __init__(self, str):
        rules = [
            (r'("(?:\\.|[^"\\])*")', 'STR'),
            (r'(\'.\')',         'NUM'),
            (r'0x[0-9A-Fa-f]+',  'NUM'),
            (r'\d+',             'NUM'),
            (r'[a-zA-Z_0-9]+',   'IDENT'),
            (r'<<=',             '<<='),
            (r'>>=',             '>>='),
            (r'->',              '->'),
            (r'--',              '--'),
            (r'\+\+',            '++'),
            (r'&&',              '&&'),
            (r'\|\|',            '||'),
            (r'<<',              '<<'),
            (r'<=',              '<='),
            (r'>=',              '>='),
            (r'!=',              '!='),
            (r'\+=',             '+='),
            (r'-=',              '-='),
            (r'\*=',             '*='),
            (r'/=',              '/='),
            (r'%=',              '%='),
            (r'==',              '=='),
            (r'&=',              '&='),
            (r'\|=',             '|='),
            (r'\^=',             '^='),
            (r'=',               '='),
            (r'<<',              '<<'),
            (r'<',               '<'),
            (r'>>',              '>>'),
            (r'>',               '>'),
            (r'\+',              '+'),
            (r'\-',              '-'),
            (r'\*',              '*'),
            (r'%',               '%'),
            (r'/',               '/'),
            (r'&',               '&'),
            (r'\|',              '|'),
            (r'\^',              '^'),
            (r'\(',              '('),
            (r'\)',              ')'),
            (r'\[',              '['),
            (r'\]',              ']'),
            (r'=',               '='),
            (r'!',               '!'),
            (r',',               ','),
            (r'\.',              '.'),
            (r'\?',              '?'),
            (r'\:',              ':'),
            (r'\~',              '~'),
        ]
        lbracket = str.count('(')
        rbracket = str.count(')')
        if lbracket != rbracket:
            print 'Brackets () not in-sync',str
        lsbracket = str.count('[')
        rsbracket = str.count(']')
        if lsbracket != rsbracket:
            print 'Brackets [] not in-sync',str

        self.lx = lexer.Lexer(rules, skip_whitespace=True)
        self.lx.input(str)
        if False:
            for tok in self.lx.tokens():
                print tok
            sys.exit(0)
        self.str = str
        self.stack = []
        self.nextToken()
        self.parse()

def simplifyTerm(term):
#    print '-',term
    # some additional brackets need to be optimized twice
    result = TermParser(term).generaterString()
    while True:
        result2 = TermParser(result).generaterString()
        if result2 == result:
            return result
        result = result2


if __name__ == '__main__':
    debugPrint = True
    termList = [
                r'!(ply_hitflag && ((dr_type != 107) || (dr_type != 108)))',
#                r'a || b || c || d || e',
#                r'a && b && c && d &&e',
#                r'a || b && c || d || e',
#                 r'!(!a || !(b()&1))',
#                 r"!(!(a(256)&1) || !b(c, 'w'))",
#                r'(((!((deltaX >= 0))) ? -deltaX : deltaX) > 800)',
#                r'!((!((((!((deltaY >= 0))) ? -deltaY : deltaY) > 800) || (player_data[target_player].ply_lives <= 0))))',
#                r'!(((((!((deltaX >= 0))) ? -deltaX : deltaX) > 800)) || ((!((((!((deltaY >= 0))) ? -deltaY : deltaY) > 800) || (player_data[target_player].ply_lives > 0)))))',
#                r'(y < 0 || y > 63 || x < 0) || x > 63',
#                 r'!(i < 0 || i >= b || c <= 0)',
#                 r'!((i < 0 || i >= b) || c <= 0)',
#                 r'!(!(a || b) || c() != 1)',
#                r'!(!((a && b != c) || d != e)) ? 1 : 2',
#                r'(!(!a || (b != c) || !d && (e != 1)))',
#                r'!(!a || !b)',
#                r'!(!a || b)',
#                r'!(a || !b)',
#                r'!(a || b)',
#                r'!(a != 0 || !b)',
#                r'!(a != n || b)',
#                r'!(a || b == n)',
#                r'!(!a || b == n)',
#                r'!(a != b || c != d)',
#                r'!(a == b || c >= d)',
#                r'!(!a || b == c || d == e || f == g)',
#                r'!(a<=96 || b>160 || !c)',
#                r'!(a<=96+1 || b>160 || !c)',
#                r'!(!a || b != c || d != e || f != g || h || i)',
#                r'!((((player_data[own_number].ply_lives > 0) || display_2d_map_flag) || (midicam_timer_counter <= 5)))',
#                r'!(!player_data[player].ply_hitflag || player_data[player].ply_gunman == player_data[player].dr_currentTarget || player_data[player_data[player].ply_gunman].dr_type == DRONE_NINJA || player_data[player_data[player].ply_gunman].dr_type == DRONE_STANDARD)',
#                r'(*destPtr++ = *srcPtr++) != 0',
#                r'lvar_2 = (!(x1 > x3)) ? !(!((x1 > x2) || (x2 > x3))) : !(!((x3 > x2) || (x2 > x1)))',
#                r'lvar_2 = (!((*px1 < 0))) ? !((*py1 < -*px1)) : !((*py1 < *px1))',
#                r'rsrc_object_array[8]->ob_type = G_IBOX',
#                r'rsrc_object_array[12][5].ob_state &= ~(SELECTED|DISABLED)',
#                r'read_vbclock_ret = *(int*)0x464',
#                r'((char*)param_8[45].ob_spec)[2] = 0',
#                r'form_alert(1, "[3][ |Please run in|LOW resolution|or HIGH res.][OK]")',
#                r'bconout_string("\"Have a nice day!\"")',
#                r'active_drones_by_type[0] = (active_drones_by_type[1] = (active_drones_by_type[2] = 0))',
#                r'!(!a || !b)',
#                r'!a || b!=c || !d && e!=1',
#                r'!(!a || !b)',
#                r'!(a>=0 || !b)',
#                r'!((!((mouse_record[((2)+3)] == -5) || (mouse_record[1] == -7))))',
#                r'(!((player_data[i].ply_lives > 0)))',
#                r'!((((i < 0) || (i >= all_players)) || (player_data[i].ply_lives <= 0)))',
#                r'!((!((player_data[targetPlayer].ply_lives <= 0) || team_flag && (targetPlayer < 0))))',
                ]
#    file2 = open('2_new.c', 'w')
    for tt in termList:#open('1_original.c','r'):
        tt = tt.strip()
        if tt[0] == ' ':    # ignore these lines
            continue
        print simplifyTerm(tt)
        print '#' * 60
#        file2.write(simplifyTerm(tt) + '\n')
