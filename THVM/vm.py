#!/usr/bin/env python

import copy
import sys

def fileLineReader(filename):
    fd = open(filename)
    try:
        while True:
            line = fd.readline()
            if not line:
                break
            yield line
    finally:
        fd.close()

class _Registers(object):
    def __init__(self):
        self.pc = 0

    def value(self, rname):
        if not hasattr(self, rname):
            return self._parseValue(rname)
        return getattr(self, rname)

    def setValue(self, rname, value):
        setattr(self, rname, self._parseValue(value))

    def clone(self):
        return copy.copy(self)

    def _parseValue(self, value):
        if isinstance(value, basestring):
            if '.' in value:
                try:
                    return float(value)
                except:
                    pass

            try:
                return int(value)
            except:
                pass

        return value

class Instruction(object):
    def __init__(self, opcode, parms):
        self.opcode = opcode
        self.parms = parms

    def __repr__(self):
        return '%s %r' % (self.opcode, self.parms)

class VirtualMachine(object):
    def __init__(self):
        self.stack = [_Registers()]
        self.asm = None

    def execute(self, asm):
        self.asm = asm
        while True:
            registers = self.stack[-1]

            # Fetch Instruction and Increment PC
            instruction = asm.code[registers.pc]
            registers.pc += 1

            opfunc = getattr(self, 'opcode_%s' % instruction.opcode)
            if not opfunc:
                raise IOError('Invalid OPCODE %s' % instruction.opcode)

            opfunc(instruction)

            if registers.pc == len(self.asm.code):
                break

    # Stack
    def opcode_push(self, instruction):
        registers = self.stack[-1].clone()
        self.stack.append(registers)

    def opcode_pop(self, instruction):
        if len(self.stack) <= 1:
            raise IOError("Stack underflow (stack %d)" % len(self.stack))

        if len(instruction.parms) > 0:
            current_stack = self.stack[-1]
            down_stack = self.stack[-2]
            for r in instruction.parms:
                down_stack.setValue(r, current_stack.value(r))
        self.stack.pop()

    # Memory
    def opcode_mov(self, instruction):
        r = instruction.parms[0]
        v = instruction.parms[1]
        registers = self.stack[-1]
        registers.setValue(r, v)

    # Math
    def opcode_add(self, instruction):
        registers = self.stack[-1]
        r = instruction.parms[0]
        r0 = registers.value(r)
        r1 = registers.value(instruction.parms[1])
        registers.setValue(r, r0 + r1)

    def opcode_sub(self, instruction):
        registers = self.stack[-1]
        r = instruction.parms[0]
        r0 = registers.value(r)
        r1 = registers.value(instruction.parms[1])
        registers.setValue(r, r0 - r1)

    def opcode_inc(self, instruction):
        registers = self.stack[-1]
        r = instruction.parms[0]
        registers.setValue(r, registers.value(r) + 1)

    def opcode_dec(self, instruction):
        registers = self.stack[-1]
        r = instruction.parms[0]
        registers.setValue(r, registers.value(r) - 1)

    def opcode_mul(self, instruction):
        registers = self.stack[-1]
        r = instruction.parms[0]
        r0 = registers.value(r)
        r1 = registers.value(instruction.parms[1])
        registers.setValue(r, r0 * r1)

    def opcode_div(self, instruction):
        registers = self.stack[-1]
        r = instruction.parms[0]
        r0 = registers.value(r)
        r1 = registers.value(instruction.parms[1])
        registers.setValue(r, r0 / r1)

    def opcode_mod(self, instruction):
        registers = self.stack[-1]
        r = instruction.parms[0]
        r0 = registers.value(r)
        r1 = registers.value(instruction.parms[1])
        registers.setValue(r, r0 % r1)

    def opcode_shl(self, instruction):
        registers = self.stack[-1]
        r = instruction.parms[0]
        r0 = registers.value(r)
        r1 = registers.value(instruction.parms[1])
        registers.setValue(r, r0 << r1)

    def opcode_shr(self, instruction):
        registers = self.stack[-1]
        r = instruction.parms[0]
        r0 = registers.value(r)
        r1 = registers.value(instruction.parms[1])
        registers.setValue(r, r0 >> r1)

    # Logic
    def opcode_or(self, instruction):
        registers = self.stack[-1]
        r = instruction.parms[0]
        r0 = registers.value(r)
        r1 = registers.value(instruction.parms[1])
        registers.setValue(r, r0 | r1)

    def opcode_xor(self, instruction):
        registers = self.stack[-1]
        r = instruction.parms[0]
        r0 = registers.value(r)
        r1 = registers.value(instruction.parms[1])
        registers.setValue(r, r0 ^ r1)

    def opcode_and(self, instruction):
        registers = self.stack[-1]
        r = instruction.parms[0]
        r0 = registers.value(r)
        r1 = registers.value(instruction.parms[1])
        registers.setValue(r, r0 & r1)

    def opcode_not(self, instruction):
        registers = self.stack[-1]
        r = instruction.parms[0]
        registers.setValue(r, not registers.value(r))

    # Jump
    def opcode_jz(self, instruction):
        r = instruction.parms[0]

        registers = self.stack[-1]
        if not registers.value(r):
            label = instruction.parms[1]
            registers.pc = self.asm.labels[label]

    def opcode_jnz(self, instruction):
        r = instruction.parms[0]

        registers = self.stack[-1]
        if registers.value(r):
            label = instruction.parms[1]
            registers.pc = self.asm.labels[label]

    def opcode_je(self, instruction):
        self._opcode_jmp_cmp(instruction, '==')

    def opcode_jne(self, instruction):
        self._opcode_jmp_cmp(instruction, '!=')

    def opcode_jlt(self, instruction):
        self._opcode_jmp_cmp(instruction, '<')

    def opcode_jle(self, instruction):
        self._opcode_jmp_cmp(instruction, '<=')

    def opcode_jgt(self, instruction):
        self._opcode_jmp_cmp(instruction, '>')

    def opcode_jge(self, instruction):
        self._opcode_jmp_cmp(instruction, '>=')

    def opcode_jmp(self, instruction):
        label = instruction.parms[0]
        registers = self.stack[-1]
        registers.pc = self.asm.labels[label]

    def _opcode_jmp_cmp(self, instruction, op_cmp):
        r0 = instruction.parms[0]
        r1 = instruction.parms[1]

        registers = self.stack[-1]
        r0 = registers.value(r0)
        r1 = registers.value(r1)

        if op_cmp == '==':
            do_jmp = (r0 == r1)
        elif op_cmp == '!=':
            do_jmp = (r0 != r1)
        elif op_cmp == '<':
            do_jmp = (r0 < r1)
        elif op_cmp == '<=':
            do_jmp = (r0 <= r1)
        elif op_cmp == '>':
            do_jmp = (r0 > r1)
        elif op_cmp == '>=':
            do_jmp = (r0 >= r1)
        else:
            raise NotImplementedError

        if do_jmp:
            label = instruction.parms[2]
            registers.pc = self.asm.labels[label]

    # I/O
    def opcode_print(self, instruction):
        registers = self.stack[-1]

        data = []
        for x in instruction.parms:
            if len(x) >= 2 and x[0] == '$' and x[1] != '$':
                data.append(str(registers.value(x[1:])))
            else:
                data.append(x)

        print ' '.join(data)

class Assembly(object):
    def __init__(self):
        self.code = []
        self.labels = {}

    def load(self, filename):
        for line in fileLineReader(filename):
            line = line.strip()
            if not line or line.startswith(';'):
                continue

            line_comment = line.find(';')
            if line_comment > 0:
                line = line[:line_comment].strip()

            parms = [x.strip().replace(',', '') for x in line.split()]
            opcode = parms[0]
            if opcode.endswith(':'):
                self._addLabel(opcode[:-1])
            else:
                self._addInstruction(opcode, parms[1:])

    def _addLabel(self, label):
        if label in self.labels:
            raise IOError("Label '%s' already assigned" % (label))
        self.labels[label] = len(self.code)

    def _addInstruction(self, opcode, parms):
        instruction = Instruction(opcode, parms)
        self.code.append(instruction)

if __name__ == '__main__':
    if len(sys.argv) < 2:
        print 'Usage: vm <filename>'
    else:
        asm = Assembly()
        asm.load(sys.argv[1])

        vm = VirtualMachine()
        vm.execute(asm)
