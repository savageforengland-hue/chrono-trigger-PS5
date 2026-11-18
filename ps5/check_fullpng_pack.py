#!/usr/bin/env python3
"""Preflight full-canvas PNG room images in a local directory or named texture-pack ZIP.

This checks filenames, basic PNG headers and expected decoded memory use.
It does not decode or authenticate image pixels, or verify room art alignment.
"""
import argparse
from pathlib import Path, PurePosixPath
import re
import struct
import sys
import zipfile
import zlib

PNG_SIGNATURE = b"\x89PNG\r\n\x1a\n"
ROOM_PATTERN = re.compile(r"0x([0-9a-fA-F]{1,4})_bg([0-3])_x([2-8])\.png$")
MAX_BYTES = 128 * 1024 * 1024
MAX_FILES = 127
MAX_PIXELS_AXIS = 4096


def read_header(buf: bytes):
    if len(buf) < 33 or not buf.startswith(PNG_SIGNATURE):
        raise ValueError("bad/truncated PNG signature or IHDR")
    length = struct.unpack_from(">I", buf, 8)[0]
    if length != 13 or buf[12:16] != b"IHDR":
        raise ValueError("invalid IHDR header")
    if zlib.crc32(buf[12:29]) != struct.unpack_from(">I", buf, 29)[0]:
        raise ValueError("IHDR CRC mismatch")
    w, h = struct.unpack_from(">II", buf, 16)
    if not w or not h:
        raise ValueError("zero-size PNG")
    return w, h


def check_entry(filename: str, header: bytes):
    name = PurePosixPath(filename).name
    match = ROOM_PATTERN.fullmatch(name)
    if not match:
        raise ValueError(f"{filename}: expected 0x<location>_bg<0-3>_x<2-8>.png")
    location, bg, scale = (int(match[1], 16), int(match[2]), int(match[3]))
    width, height = read_header(header)
    if width % scale or height % scale:
        raise ValueError(f"{filename}: {width}x{height} is not an integer {scale}x scale")
    native_w, native_h = width // scale, height // scale
    if native_w > MAX_PIXELS_AXIS or native_h > MAX_PIXELS_AXIS:
        raise ValueError(f"{filename}: native canvas exceeds {MAX_PIXELS_AXIS} pixels on an axis")
    decoded_bytes = width * height * 4
    if decoded_bytes > MAX_BYTES:
        raise ValueError(f"{filename}: image exceeds 128 MiB decoded")
    return (location, bg), decoded_bytes, (width, height), scale


def validate(source: Path):
    if not source.exists():
        raise ValueError(f"{source}: path does not exist")
    found = 0
    used = 0
    targets = set()

    def check(filename, header):
        nonlocal found, used
        key, bytes_needed, dimensions, scale = check_entry(filename, header)
        if key in targets:
            raise ValueError(f"{filename}: another PNG already targets location 0x{key[0]:x} BG{key[1]}")
        targets.add(key)
        found += 1
        used += bytes_needed
        if found > MAX_FILES or used > MAX_BYTES:
            raise ValueError("pack exceeds 127 PNGs or 128 MiB decoded image budget")
        return (key, dimensions, scale)

    if source.is_dir():
        for file in sorted(source.rglob("*")):
            if file.is_file() and file.parent.name == "rooms" and file.suffix.lower() == ".png":
                with file.open("rb") as stream:
                    check(str(file), stream.read(33))
    elif zipfile.is_zipfile(source):
        with zipfile.ZipFile(source) as zf:
            for info in sorted(zf.infolist(), key=lambda x: x.filename):
                path = PurePosixPath(info.filename)
                if path.is_absolute() or ".." in path.parts or "\\" in info.filename:
                    raise ValueError(f"unsafe archive entry: {info.filename}")
                if not info.is_dir() and path.parent.name == "rooms" and path.suffix.lower() == ".png":
                    with zf.open(info) as stream:
                        check(info.filename, stream.read(33))
    else:
        raise ValueError(f"{source}: expected a directory or ZIP")
    if not found:
        raise ValueError(f"{source}: no rooms/*.png files found")
    return found, used


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("source", type=Path, help="mod root, folder of PNGs, or texture-pack ZIP")
    args = parser.parse_args()
    try:
        images, memory = validate(args.source)
    except (OSError, ValueError, zipfile.BadZipFile) as error:
        parser.exit(1, "Full-PNG pack invalid: " + str(error) + "\n")
    print(f"PASS: {images} room PNG(s), estimated decoded memory {memory / (1024 * 1024):.2f} MiB")
    print("Note: this is header/preflight validation, not full PNG decoding or PS5 gameplay verification.")


if __name__ == "__main__":
    main()
