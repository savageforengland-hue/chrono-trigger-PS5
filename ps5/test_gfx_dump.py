#!/usr/bin/env python3
"""Exercise indexed output/failure handling; optionally dump a player's ROM."""
from pathlib import Path
import argparse,struct,subprocess,tempfile,zlib,os,hashlib,stat

p=argparse.ArgumentParser(description=__doc__)
p.add_argument('--sdl-source',type=Path,required=True)
p.add_argument('--sdl-library',type=Path,required=True)
p.add_argument('--rom',type=Path)
p.add_argument('--output',type=Path)
a=p.parse_args();root=Path(__file__).resolve().parents[1]

def png(path):
    data=path.read_bytes();assert data[:8]==b'\x89PNG\r\n\x1a\n';at=8;chunks={}
    while at<len(data):
        n,=struct.unpack_from('>I',data,at);kind=data[at+4:at+8];body=data[at+8:at+8+n]
        crc,=struct.unpack_from('>I',data,at+8+n);assert zlib.crc32(kind+body)&0xffffffff==crc,(path,kind)
        chunks[kind]=chunks.get(kind,b'')+body;at+=n+12
    assert at==len(data) and b'IEND' in chunks
    w,h,depth,color,_,_,_=struct.unpack('>IIBBBBB',chunks[b'IHDR']);assert depth==8 and color==3
    raw=zlib.decompress(chunks[b'IDAT']);assert len(raw)==(w+1)*h
    rows=[];previous=bytes(w)
    for y in range(h):
        kind=raw[y*(w+1)];row=bytearray(raw[y*(w+1)+1:(y+1)*(w+1)]);assert kind<=4
        for x in range(w):
            left=row[x-1] if x else 0;up=previous[x];corner=previous[x-1] if x else 0
            if kind==1:predict=left
            elif kind==2:predict=up
            elif kind==3:predict=(left+up)//2
            elif kind==4:
                base=left+up-corner;dist=[abs(base-left),abs(base-up),abs(base-corner)]
                predict=[left,up,corner][dist.index(min(dist))]
            else:predict=0
            row[x]=(row[x]+predict)&255
        rows.append(bytes(row));previous=rows[-1]
    return w,h,chunks,b''.join(rows)

with tempfile.TemporaryDirectory() as tmp:
    d=Path(tmp);exe=d/'writer'
    flags=['cc','-std=c11','-O1','-Wall','-Wextra','-Werror','-I',str(root),'-I',str(a.sdl_source/'include')]
    subprocess.run([*flags,str(root/'ps5/tests/gfx_dump_io.c'),str(root/'runtime/gfx_dump_io.c'),'-Wl,--wrap=fwrite','-Wl,--wrap=fclose','-Wl,--wrap=rename',str(a.sdl_library),'-lm','-ldl','-pthread','-o',str(exe)],check=True)
    subprocess.run([str(exe),str(d)],check=True,timeout=20)
    for name,w,h,bpp8 in [('bpp8.png',513,257,True),('bpp4.png',33,17,False)]:
        pw,ph,chunks,px=png(d/'nested/a/b'/name);assert (pw,ph)==(w,h)
        original=bytes((x+3*y)&255 for y in range(257) for x in range(513));assert px==original[:w*h]
        for i in range(256):
            assert chunks[b'PLTE'][i*3:i*3+3]==bytes([i,255-i,i^0x55])
            assert chunks[b'tRNS'][i]==(0 if (i==0 if bpp8 else i%16==0) else 255)
    print('PASS: PNG CRCs, zlib streams, exact palette indexes/colours/transparency and unchanged valid file after injected failures')
    if a.rom:
        if not (a.sdl_library and a.output):p.error('--rom needs --sdl-library and --output')
        a.output.mkdir(parents=True,exist_ok=True);exe=d/'rom'
        subprocess.run([*flags,str(root/'ps5/tests/gfx_dump_rom.c'),*[str(root/'runtime'/s) for s in ['gfx_dump.c','gfx_dump_io.c','gfx_rom.c']],str(a.sdl_library),'-lm','-ldl','-pthread','-o',str(exe)],check=True)
        subprocess.run([str(exe),str(a.rom),str(a.output)],check=True,timeout=300)
        images=list((a.output/'mods/graphics/dump').rglob('*.png'));assert images
        for path in images:png(path)
        report=(a.output/'graphics-dump.log').read_text();assert f'Verified PNG files: {len(images)}\n' in report and 'State: complete\n' in report
        dump=a.output/'mods/graphics/dump'
        witness=dump/'frames/Chara/Chara_0000/199.png';digest=hashlib.sha256(witness.read_bytes()).hexdigest()
        outside=d/'outside.txt';outside.write_text('leave this alone');outside.chmod(0o600)
        link=dump/'outside-link'
        if link.is_symlink():link.unlink()
        link.symlink_to(outside)
        for image in images:image.chmod(0o600)
        directories=[dump,*[path for path in dump.rglob('*') if path.is_dir()]]
        for folder in directories:folder.chmod(0o700)
        subprocess.run([str(exe),str(a.rom),str(a.output),'repair'],check=True,timeout=300)
        assert all(stat.S_IMODE(path.stat().st_mode)==0o755 for path in directories)
        assert all(stat.S_IMODE(path.stat().st_mode)==0o644 for path in images)
        assert stat.S_IMODE(outside.stat().st_mode)==0o600 and outside.read_text()=='leave this alone'
        assert hashlib.sha256(witness.read_bytes()).hexdigest()==digest
        report=(a.output/'graphics-dump.log').read_text();assert f'Verified PNG files: {len(images)}\n' in report and 'repair existing permissions; no re-export' in report
        blocked=d/'blocked';blocked.mkdir();(blocked/'mods').write_text('blocked directory')
        subprocess.run([str(exe),str(a.rom),str(blocked),'expect-fail'],check=True,timeout=30)
        assert 'ERROR:' in (blocked/'graphics-dump.log').read_text()
        print(f'PASS: full own-ROM dump; {len(images)} files decoded/CRC-checked, on-disk count matches report; startup repair restores 0755/0644 without changing PNG bytes or following links; blocked output reports failure')
