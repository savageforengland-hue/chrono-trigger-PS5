#!/usr/bin/env python3
"""ROM-free integration of actual NitroFS sprite/font/message prompt rewriting."""
import argparse,struct,subprocess,tempfile,sys
from pathlib import Path
HERE=Path(__file__).resolve().parent
sys.path.insert(0,str(HERE.parent/'tools/recomp'))
from retarget import Rom,Font,Text,decode,encode

def font():
    head=bytearray(0x10c);head[4:8]=b'FONT';head[8:10]=bytes([10,2]);struct.pack_into('<H',head,10,140)
    struct.pack_into('<128H',head,12,*range(128))
    f=Font(bytes(head)+bytes(140*4));f.glyphs=[bytes([5,2])+b'\x55'*20 for _ in range(140)]
    return f.build()

def text(malformed=False):
    strings=['A Button / B Button / X Button / Y Button. START SELECT.',
             'A brave knight visits STARTER and SELECTOR.',
             'Appuyez sur le bouton A.',
             'Crane control codes:\nPattern 00: X A\nPattern 01: B B',
             [ord(c) for c in 'A Button ']+[183]+[ord(c) for c in ' end'],
             'Hold the L Button and R Button.', 'Press the B Button. The "B" icons show boosts.']
    t=object.__new__(Text);t.head=b'\0\0\0\0TEXT'+bytes([1,len(strings),0,0])
    t.strings=[encode(map(ord,s) if isinstance(s,str) else s) for s in strings]
    d=bytearray(t.build(0))
    if malformed:struct.pack_into('<I',d,16,0xffffffff)
    return bytes(d)

def cells():
    out=bytearray(0x28+16*8+16*6);out[:4]=b'RECN';out[0x10:0x14]=b'KBEC'
    struct.pack_into('<HHII',out,0x18,16,0,0x10,0)
    for i in range(16):
        struct.pack_into('<HHI',out,0x28+i*8,1,0,i*6)
        # One 16x16 sprite, or a 32x8 START/SELECT sprite; four tiles each.
        a0,a1=(0,1<<14) if i<12 else (1<<14,1<<14)
        struct.pack_into('<HHH',out,0x28+16*8+i*6,a0,a1,i*4)
    return bytes(out)

def rom(malformed=False,code=b'YQUE'):
    # Entirely authored synthetic files; no game dump or extracted bytes.
    files={'menu/obj/icon.NCGR':b'NCG\0'+struct.pack('<HBB',64,0,0)+b'\x11'*2048,
           'menu/obj/icon.NCER':cells(),'msg/big/msgcmn.fnt':font(),'msg/big/help.msg':text(malformed),
           'untouched.bin':b'AUTHORED UNRELATED DATA'}
    tree={}
    for path,data in files.items():
        node=tree
        parts=path.split('/')
        for p in parts[:-1]:node=node.setdefault(p,{})
        node[parts[-1]]=data
    dirs=[]
    def walk(node):
        dirs.append(node)
        for v in node.values():
            if isinstance(v,dict):walk(v)
    walk(tree);index={id(d):i for i,d in enumerate(dirs)};streams=[];ordered=[]
    for node in dirs:
        first=len(ordered);stream=bytearray()
        for name,value in node.items():
            if isinstance(value,dict):stream+=bytes([128+len(name)])+name.encode()+struct.pack('<H',0xf000+index[id(value)])
            else:stream+=bytes([len(name)])+name.encode();ordered.append(value)
        streams.append((first,bytes(stream)+b'\0'))
    fnt=bytearray(8*len(dirs))
    for i,(first,stream) in enumerate(streams):
        struct.pack_into('<IHH',fnt,i*8,len(fnt),first,len(dirs) if not i else 0xf000)
        fnt+=stream
    out=bytearray(0x200);out[12:16]=code
    fat=0x200+len(fnt);struct.pack_into('<4I',out,0x40,0x200,len(fnt),fat,8*len(ordered));out+=fnt+bytes(8*len(ordered))
    for i,data in enumerate(ordered):
        a=len(out);out+=data;struct.pack_into('<II',out,fat+i*8,a,len(out))
    return bytes(out)

HARNESS=r'''
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include "rt.h"
#include "game_prompts.h"
RtRom rt_rom;static int remap;
void rt_log(const char *format,...){(void)format;}
const char *controls_pad_label(int n){
 static const char *v[]={"\xE2\x97\x8B","\xC3\x97","\xE2\x96\xB3","\xE2\x96\xA1","L1","R1","Options","Create"};
 assert(n>=0&&n<8);return n==0&&remap?(remap==1?"\xC3\x97":"L2"):v[n];
}
int main(int argc,char **argv){
 assert(argc==4);remap=atoi(argv[3]);FILE*f=fopen(argv[1],"rb");assert(f);fseek(f,0,SEEK_END);rt_rom.size=(size_t)ftell(f);rewind(f);rt_rom.data=malloc(rt_rom.size);assert(rt_rom.data);assert(fread(rt_rom.data,1,rt_rom.size,f)==rt_rom.size);fclose(f);memcpy(rt_rom.code,rt_rom.data+12,4);
 (void)egg_ps5_game_prompts_init();f=fopen(argv[2],"wb");assert(f);assert(fwrite(rt_rom.data,1,rt_rom.size,f)==rt_rom.size);fclose(f);free(rt_rom.data);
 unsigned char guarded[32*16+16];memset(guarded,0xa5,sizeof guarded);
 egg_ps5_game_prompt_mask("Options",guarded+8,32,8,0);
 for(int i=0;i<8;i++)assert(guarded[i]==0xa5&&guarded[8+32*8+i]==0xa5);
}
'''

def main():
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('--sdl-source',type=Path,required=True);args=p.parse_args()
    with tempfile.TemporaryDirectory() as tmp:
        root=Path(tmp);source=root/'test.c';source.write_text(HARNESS);exe=root/'test'
        subprocess.run(['cc','-std=c11','-O1','-Wall','-Wextra','-Werror','-I',str(HERE),'-I',str(HERE.parent/'runtime'),'-I',str(args.sdl_source/'include'),str(source),str(HERE/'game_prompts.c'),str(HERE/'game_prompt_icons.c'),str(HERE.parent/'runtime/gfx_rom.c'),'-o',str(exe)],check=True)
        glyphs=[]
        for mode in range(3):
            original=rom();a=root/'input.nds';b=root/'output.nds';a.write_bytes(original)
            subprocess.run([str(exe),str(a),str(b),str(mode)],check=True,timeout=15)
            assert a.read_bytes()==original,'Input file modified'
            old,new=Rom(original),Rom(b.read_bytes())
            get=lambda r,path:r.file(r.paths[path])
            assert get(old,'untouched.bin')==get(new,'untouched.bin')
            assert get(old,'menu/obj/icon.NCER')==get(new,'menu/obj/icon.NCER')
            assert get(old,'menu/obj/icon.NCGR')!=get(new,'menu/obj/icon.NCGR')
            fa,fb=Font(get(old,'msg/big/msgcmn.fnt')),Font(get(new,'msg/big/msgcmn.fnt'))
            assert fa.ascii==fb.ascii and fa.count==fb.count
            assert fa.glyphs[:128]==fb.glyphs[:128], 'Existing text glyph changed'
            assert fa.glyphs[134:]==fb.glyphs[134:], 'Unrelated spare glyph changed'
            ta,tb=Text(get(old,'msg/big/help.msg'),0),Text(get(new,'msg/big/help.msg'),0)
            assert ta.strings[1]==tb.strings[1], 'Ordinary dialogue changed'
            assert [c for c in decode(ta.strings[4]) if c>=140]==[c for c in decode(tb.strings[4]) if c>=140], 'Control opcode changed'
            first=decode(tb.strings[0]);assert first[:1]==[128] and 129 in first and 130 in first and 131 in first
            assert 'Options' in ''.join(chr(c) if c<128 else '?' for c in first)
            assert 'Create' in ''.join(chr(c) if c<128 else '?' for c in first)
            assert 128 in decode(tb.strings[2]), 'French button reference not changed'
            assert decode(tb.strings[5]).count(132)==1 and decode(tb.strings[5]).count(133)==1
            assert decode(tb.strings[6]).count(129)==2, 'Quoted button icon reference not changed'
            glyphs.append(fb.glyphs[128])
        assert len(set(glyphs))==3, 'Saved remapping not reflected in game font'
        for bad,code_value in [(True,b'YQUE'),(False,b'YQUP')]:
            original=rom(bad,code_value);a.write_bytes(original)
            subprocess.run([str(exe),str(a),str(b),'0'],check=True,timeout=15)
            old,new=Rom(original),Rom(b.read_bytes())
            assert old.file(old.paths['msg/big/msgcmn.fnt'])==new.file(new.paths['msg/big/msgcmn.fnt']), 'Unsafe font mutation'
            assert old.file(old.paths['msg/big/help.msg'])==new.file(new.paths['msg/big/help.msg'])
            if code_value!=b'YQUE':assert b.read_bytes()==original,'Unsupported game changed'
    print('PASS: actual NitroFS sprite/font/message edits, all face/shoulder prompts, Options/Create, saved remaps, French references, untouched dialogue/control opcodes, bad-message and unsupported-game guards')

if __name__=='__main__':main()
