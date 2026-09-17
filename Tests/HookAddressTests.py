"""Exercise the binary verifier using real output and isolated byte mutations.

No mutated DLL is executed. The original user-crashing build can additionally
be supplied with --old-dll to prove the original defect is caught.
"""
import argparse
import contextlib
import io
from pathlib import Path
import struct
import sys
import tempfile

p = argparse.ArgumentParser(description=__doc__)
p.add_argument('--dll', required=True)
p.add_argument('--kenshilib', required=True)
p.add_argument('--tools-path')
p.add_argument('--old-dll')
a = p.parse_args()
if a.tools_path:
    sys.path.insert(0, a.tools_path)
sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
import pefile
from VerifyHookAddresses import verify


def quiet_verify(path):
    with contextlib.redirect_stdout(io.StringIO()):
        return verify(path, a.kenshilib)


def rejected(path, label):
    try:
        quiet_verify(path)
    except ValueError as error:
        print('PASS: rejected ' + label + ': ' + str(error))
        return
    raise AssertionError('Incorrectly accepted ' + label)


calls = quiet_verify(a.dll)
print('PASS: accepted the complete rebuilt DLL')
original = Path(a.dll).read_bytes()
pe = pefile.PE(data=original)
base = pe.OPTIONAL_HEADER.ImageBase
with tempfile.TemporaryDirectory(prefix='lootall-binary-tests-') as directory:
    mutant = Path(directory) / 'never-execute.dll'
    for _, load, name in calls:
        offset = pe.get_offset_from_rva(load)
        assert original[offset:offset+3] == b'\x48\x8b\x0d'
        data = bytearray(original)
        data[offset+1] = 0x8D  # LEA points into LootAll instead of loading the imported pointer.
        mutant.write_bytes(data)
        rejected(mutant, 'local-image LEA for ' + name)

    call, load, _ = calls[0]
    getter = next(e.address for d in pe.DIRECTORY_ENTRY_IMPORT for e in d.imports
                  if e.name == b'?GetRealAddress@KenshiLib@@YA_JPEAX@Z')
    data = bytearray(original)
    struct.pack_into('<i', data, pe.get_offset_from_rva(load)+3, getter-(base+load+7))
    mutant.write_bytes(data)
    rejected(mutant, 'wrong imported symbol as game-stub argument')

    data = bytearray(original)
    data[pe.get_offset_from_rva(call):pe.get_offset_from_rva(call)+6] = b'\x90'*6
    mutant.write_bytes(data)
    rejected(mutant, 'missing lifecycle resolver call')

if a.old_dll:
    rejected(a.old_dll, 'original crashing LootAll 1.0.0')
print('%d binary regression checks passed' % (8 if a.old_dll else 7))
