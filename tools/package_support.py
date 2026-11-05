import binascii,hashlib,json,struct
from pathlib import Path
import numpy as np
NAMES=('ex1990_051000.bi', '_tpj01_0210.bin', 'eg1990_053200.bi', 'vr1990_010200.bi', '_gst00_0017.bin', 'li1990_053200.bi')
def sha(b):
    return hashlib.sha256(b).hexdigest()

def read_material(tables_path, integrity_path):
    try:
        t = json.loads(Path(tables_path).read_text(encoding='utf-8'))
        rows = [bytes.fromhex(t[n]) for n in ('T1', 'T2', 'T3')]
        k = bytes.fromhex(json.loads(Path(integrity_path).read_text(encoding='utf-8'))['K8'])
        if any((len(r) != 256 for r in rows)) or len(k) != 8:
            raise ValueError()
    except (ValueError, TypeError, KeyError):
        raise ValueError('Private format material has an unsupported shape') from None
    return ([np.frombuffer(r, dtype=np.uint8) for r in rows], k)

def stock_modules(plain, k):
    body = plain[32:]
    if len(body) < 256 or body[:32] != b' ' * 32 or hashlib.sha1(body + k).digest() != plain[:20]:
        raise ValueError('Original container integrity check failed')
    if struct.unpack_from('>II', body, 32) != (6, 240) or body[40:48] != bytes(8) or body[-14:] != bytes(14):
        raise ValueError('Original directory shape')
    if binascii.crc_hqx(body[:-16], 0) != int.from_bytes(body[-16:-14], 'big'):
        raise ValueError('Original container CRC')
    modules = {}
    cursor = 240
    for i, expected in enumerate(NAMES):
        pos = 48 + 32 * i
        name = body[pos:pos + 16].rstrip(b'\x00').decode('ascii')
        off, n = struct.unpack_from('>II', body, pos + 16)
        if name != expected or off != cursor or n < 2 or (off + n > len(body) - 16) or (body[pos + 24:pos + 32] != bytes(8)):
            raise ValueError('Original module layout')
        m = body[off:off + n]
        if binascii.crc_hqx(m[:-2], 0) != int.from_bytes(m[-2:], 'big'):
            raise ValueError('Original module CRC')
        modules[name] = m
        cursor = off + n
    if cursor != len(body) - 16:
        raise ValueError('Original extent')
    return modules
