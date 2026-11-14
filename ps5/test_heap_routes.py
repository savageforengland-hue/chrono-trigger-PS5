#!/usr/bin/env python3
"""Verify SDL's four libc allocator adapters reach the title heap in the ELF.

The link can succeed even with the --wrap flags omitted. Check the mapped
machine-code branch target rather than inferring routing from symbol presence.
"""
import argparse
from pathlib import Path
import struct
import subprocess
from verify_native import Elf

def audit(image, symbols):
    for suffix in ('malloc','calloc','realloc','free'):
        address=symbols['real_'+suffix]
        code=image.read(address,5)
        assert code[0]==0xe9, ('Expected direct adapter jump',suffix,code.hex())
        target=address+5+struct.unpack_from('<i',code,1)[0]
        assert target==symbols['__wrap_'+suffix], ('SDL bypasses application heap',suffix,hex(target))

def main():
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument('--llvm-input',type=Path,required=True)
    p.add_argument('--native-elf',type=Path,required=True)
    p.add_argument('--nm',type=Path,required=True)
    a=p.parse_args();symbols={}
    for line in subprocess.check_output([a.nm,'--defined-only',a.llvm_input],text=True).splitlines():
        fields=line.split()
        if len(fields)==3:symbols[fields[2]]=int(fields[0],16)
    image=Elf(a.native_elf.read_bytes());audit(image,symbols)
    print('PASS: all four SDL allocator adapters jump to title-heap wrappers in the signed extracted ELF')
    data=bytearray(image.data);address=symbols['real_calloc']
    load=next(h for h in image.ph if h[0]==1 and h[3]<=address<h[3]+h[5])
    data[load[2]+address-load[3]+1]^=1
    try:audit(Elf(bytes(data)),symbols)
    except AssertionError:print('PASS: incorrect allocator branch target rejected')
    else:raise AssertionError('Incorrect allocator routing accepted')

if __name__=='__main__':main()
