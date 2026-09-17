"""Fail-closed static check for the VC2010 LootAll hook-address regression.

Never loads or executes either DLL. Requires the exact installed KenshiLib DLL.
Accepts only the audited immediate MOV RCX,[KenshiLib IAT]; CALL [GetRealAddress]
code generation. A different optimizer sequence requires review, not a bypass.
The runtime helper separately validates actual relocated addresses before use.
"""
import argparse
from collections import Counter
from pathlib import Path
import sys

EXPECTED = (
    "?_clearAndDestroyGameWorldStuff@GameWorld@@QEAAXXZ",
    "?_NV_show@InventoryGUI@@QEAAX_N@Z",
    "?_DESTRUCTOR@InventoryGUI@@QEAAXXZ",
    "?_NV_update@InventoryGUI@@QEAAXXZ",
)
GET_REAL = "?GetRealAddress@KenshiLib@@YA_JPEAX@Z"


def verify(dll, library, report=None):
    import pefile
    from capstone import Cs, CS_ARCH_X86, CS_MODE_64
    from capstone.x86 import X86_OP_MEM, X86_OP_REG, X86_OP_IMM, X86_REG_RIP, X86_REG_RCX

    def require(condition, message):
        if not condition:
            raise ValueError(message)

    pe = pefile.PE(data=Path(dll).read_bytes())
    # 0.5.0 has 10,131 exports, exceeding pefile's default 8,192-name limit.
    klib = pefile.PE(data=Path(library).read_bytes(), max_symbol_exports=20000)
    for obj in (pe, klib):
        require(obj.FILE_HEADER.Machine == 0x8664 and obj.OPTIONAL_HEADER.Magic == 0x20B,
                "Expected AMD64 PE32+")
    base = pe.OPTIONAL_HEADER.ImageBase
    imports = {entry.address: (desc.dll.decode().lower(), (entry.name or b'').decode())
               for desc in pe.DIRECTORY_ENTRY_IMPORT for entry in desc.imports}
    getter_slots = [va for va, name in imports.items() if name == ('kenshilib.dll', GET_REAL)]
    require(len(getter_slots) == 1, "Expected one KenshiLib GetRealAddress IAT entry")
    getter_slot = getter_slots[0]
    exports = {e.name.decode(): e for e in klib.DIRECTORY_ENTRY_EXPORT.symbols if e.name}
    for name in ('FUNC_BEGIN', 'FUNC_END') + EXPECTED:
        require(name in exports and not exports[name].forwarder, "Missing/local export required: " + name)
    begin, end = exports['FUNC_BEGIN'].address, exports['FUNC_END'].address
    require(begin < end, "Invalid generated function table bounds")
    md = Cs(CS_ARCH_X86, CS_MODE_64)
    md.detail = True

    def rip_address(ins, op):
        if op.type == X86_OP_MEM and op.mem.base == X86_REG_RIP and not op.mem.index:
            return ins.address + ins.size + op.mem.disp
        return None

    lines = ['Static hook-address verification (neither DLL executed)',
             'LootAll SHA256: ' + __import__('hashlib').sha256(Path(dll).read_bytes()).hexdigest(),
             'KenshiLib SHA256: ' + __import__('hashlib').sha256(Path(library).read_bytes()).hexdigest(),
             'KenshiLib exported stub table: RVA 0x%X .. 0x%X' % (begin, end)]
    for name in EXPECTED:
        rva = exports[name].address
        require(begin <= rva < end, "Export outside generated table: " + name)
        stub = next(md.disasm(klib.get_data(rva, 6), klib.OPTIONAL_HEADER.ImageBase + rva), None)
        require(stub is not None and stub.mnemonic == 'jmp' and stub.size == 6
                and rip_address(stub, stub.operands[0]) is not None,
                "Export is not a generated indirect-jump stub: " + name)
        lines.append('PASS: KenshiLib RVA 0x%X: %s; %s %s' % (rva, name, stub.mnemonic, stub.op_str))

    start = next(e.address for e in pe.DIRECTORY_ENTRY_EXPORT.symbols if e.name == b'?startPlugin@@YAXXZ')
    end_start = next(e.struct.EndAddress for e in pe.DIRECTORY_ENTRY_EXCEPTION if e.struct.BeginAddress == start)
    # Scan every executable section, including non-.pdata leaf functions/thunks.
    # Any additional IAT reference or unreviewed indirect getter form is rejected.
    md.skipdata = True
    calls = []
    branches = set()
    all_instructions = []
    for section in pe.sections:
        if not section.Characteristics & 0x20000000:
            continue
        instructions = list(md.disasm(section.get_data()[:section.Misc_VirtualSize], base + section.VirtualAddress))
        all_instructions.extend(instructions)
        for index, ins in enumerate(instructions):
            if not ins.id:
                continue
            if (ins.mnemonic.startswith('j') or ins.mnemonic == 'call') and ins.operands[0].type == X86_OP_IMM:
                branches.add(ins.operands[0].imm)
            refs = [rip_address(ins, op) for op in ins.operands]
            if getter_slot not in refs:
                continue
            require(ins.mnemonic == 'call' and len(ins.operands) == 1,
                    'Unreviewed GetRealAddress reference at RVA 0x%X' % (ins.address-base))
            require(base+start <= ins.address < base+end_start, 'GetRealAddress call outside startPlugin')
            require(index > 0, 'Missing argument load')
            load = instructions[index-1]
            require(load.mnemonic == 'mov' and len(load.operands) == 2
                    and load.operands[0].type == X86_OP_REG and load.operands[0].reg == X86_REG_RCX
                    and load.operands[1].size == 8 and load.address+load.size == ins.address,
                    'GetRealAddress at RVA 0x%X does not immediately load RCX from the IAT (local LEA/thunk or unknown provenance)' % (ins.address-base))
            slot = rip_address(load, load.operands[1])
            module, name = imports.get(slot, ('', ''))
            require(module == 'kenshilib.dll' and name in EXPECTED,
                    'GetRealAddress argument is not an expected KenshiLib function IAT load')
            calls.append((ins.address, load.address, name))
            lines.append('PASS: call RVA 0x%X, argument MOV at RVA 0x%X, IAT RVA 0x%X -> KenshiLib!%s (stub RVA 0x%X)' %
                         (ins.address-base, load.address-base, slot-base, name, exports[name].address))
    require(Counter(c[2] for c in calls) == Counter(EXPECTED), 'Expected exactly one audited call per hook, with no other getter references')
    require(not any(call in branches for call, _, _ in calls), 'Control flow can bypass an argument load')
    lines.append('PASS: all 4 GetRealAddress calls use actual imported generated stubs; no local thunk arguments.')
    lines.append('Runtime relocated addresses are checked by ValidateGameStub before every call. In-game startup remains untested.')
    lines.append('\nComplete startPlugin disassembly (preferred image addresses; ASLR may relocate):')
    for ins in all_instructions:
        if base+start <= ins.address < base+end_start:
            annotations = []
            if ins.id:
                for op in ins.operands:
                    imp = imports.get(rip_address(ins, op))
                    if imp:
                        annotations.append(':'.join(imp))
            lines.append('%016X  %-24s %-8s %-48s %s' %
                         (ins.address, ins.bytes.hex(' '), ins.mnemonic, ins.op_str, '; '.join(annotations)))
    if report:
        Path(report).write_text('\n'.join(lines)+'\n', encoding='utf-8')
    print('\n'.join(lines[:12]))
    return [(call-base, load-base, name) for call, load, name in calls]


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--dll', required=True)
    parser.add_argument('--kenshilib', required=True)
    parser.add_argument('--tools-path', help='Optional isolated pip --target directory')
    parser.add_argument('--report', help='Write complete evidence and startPlugin disassembly')
    args = parser.parse_args()
    if args.tools_path:
        sys.path.insert(0, args.tools_path)
    try:
        verify(args.dll, args.kenshilib, args.report)
    except Exception as error:
        print('FAIL: ' + str(error), file=sys.stderr)
        sys.exit(1)
