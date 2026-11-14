#!/usr/bin/env python3
"""Independent regression: map linked sections through native PT_LOAD headers.

A previous converter used the GOT file offset as the base of an earlier RELRO
region. This checks executable geometry and every retained section's actual
mapped bytes rather than relying on a valid SELF digest or symbol resolution.
"""
import argparse
from pathlib import Path
import struct
from verify_native import Elf,audit_elf_layout,audit_linked_mapping


def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--llvm-input',required=True,type=Path)
    parser.add_argument('--native-elf',required=True,type=Path)
    parser.add_argument('--known-broken-elf',type=Path)
    parser.add_argument('--working-reference-elf',type=Path)
    args=parser.parse_args()
    native=Elf(args.native_elf.read_bytes())
    audit_elf_layout(native)
    result=audit_linked_mapping(args.llvm_input,native)
    print('PASS: fixed native load geometry and mapped section bytes',result['allocated_sections_verified'])
    # Recreate the exact incorrect segment-base assumption without relying on
    # a previous release file: shift the RELRO file start to the linked GOT.
    # The native converter advertises its GOT even for a no-PLT LLVM input.
    got=native.tags[3]
    require_region=next(p for p in native.ph if p[0]==1 and p[1]&2 and p[3]<=got<p[3]+p[6])
    delta=got-require_region[3]
    assert delta>0,'Regression fixture needs data preceding the GOT'
    data=bytearray(native.data)
    phoff=struct.unpack_from('<Q',data,32)[0]
    phsize=struct.unpack_from('<H',data,54)[0]
    index=native.ph.index(require_region)
    struct.pack_into('<Q',data,phoff+index*phsize+8,require_region[2]+delta)
    broken=Elf(bytes(data))
    try:
        audit_elf_layout(broken)
        audit_linked_mapping(args.llvm_input,broken)
    except ValueError as error:
        print('PASS: recreated old GOT-base mapping rejected:',error)
    else:raise AssertionError('Old converter mapping was accepted')
    # Geometry alone also cannot establish correct contents. Alter a linked
    # RELRO byte while keeping every program header valid.
    corrupt=bytearray(native.data)
    corrupt[require_region[2]] ^= 1
    altered=Elf(bytes(corrupt))
    audit_elf_layout(altered)
    try:audit_linked_mapping(args.llvm_input,altered)
    except ValueError as error:print('PASS: valid geometry with altered RELRO bytes rejected:',error)
    else:raise AssertionError('Changed mapped data was accepted')
    if args.known_broken_elf:
        try:audit_elf_layout(Elf(args.known_broken_elf.read_bytes()))
        except ValueError as error:print('PASS: actual previous R4 ELF rejected:',error)
        else:raise AssertionError('Known broken R4 passed the layout check')
    if args.working_reference_elf:
        audit_elf_layout(Elf(args.working_reference_elf.read_bytes()))
        print('PASS: user-reported working reference geometry accepted')
    print('Offline structural regression complete; PS5 boot remains unverified.')


if __name__=='__main__':main()
