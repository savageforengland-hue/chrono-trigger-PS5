"""ROM-free regression tests for full-canvas PNG pack preflight."""
from pathlib import Path
from tempfile import TemporaryDirectory
import struct
import sys
import unittest
import zipfile
import zlib

sys.path.insert(0, str(Path(__file__).resolve().parents[2] / "ps5"))
from check_fullpng_pack import validate  # noqa: E402


def fake_png(w, h):
    # Valid PNG signature/IHDR for header preflight. No IDAT: deliberately not
    # an image-decoding test.
    data = struct.pack(">IIBBBBB", w, h, 8, 6, 0, 0, 0)
    block = b"IHDR" + data
    return b"\x89PNG\r\n\x1a\n" + struct.pack(">I", 13) + block + struct.pack(">I", zlib.crc32(block))


class FullPngPackTests(unittest.TestCase):
    def test_named_pack_zip(self):
        with TemporaryDirectory() as d:
            archive = Path(d) / "test.zip"
            with zipfile.ZipFile(archive, "w") as out:
                out.writestr("graphics/rooms/0x1f0_bg2_x2.png", fake_png(512, 384))
            self.assertEqual(validate(archive), (1, 512 * 384 * 4))

    def test_local_mod_folder(self):
        with TemporaryDirectory() as d:
            room = Path(d) / "mods" / "graphics" / "rooms"
            room.mkdir(parents=True)
            (room / "0x1d0_bg2_x4.png").write_bytes(fake_png(1024, 768))
            self.assertEqual(validate(Path(d)), (1, 1024 * 768 * 4))

    def test_invalid_file_name(self):
        with TemporaryDirectory() as d:
            room = Path(d) / "rooms"
            room.mkdir()
            (room / "0x1f0_bg8_x2.png").write_bytes(fake_png(512, 384))
            with self.assertRaisesRegex(ValueError, "expected"):
                validate(Path(d))

    def test_dimension_not_divisible(self):
        with TemporaryDirectory() as d:
            room = Path(d) / "rooms"
            room.mkdir()
            (room / "0x1f0_bg2_x4.png").write_bytes(fake_png(513, 384))
            with self.assertRaisesRegex(ValueError, "integer"):
                validate(Path(d))

    def test_corrupt_header(self):
        with TemporaryDirectory() as d:
            room = Path(d) / "rooms"
            room.mkdir()
            data = bytearray(fake_png(512, 384))
            data[-1] ^= 0xFF
            (room / "0x1f0_bg2_x2.png").write_bytes(data)
            with self.assertRaisesRegex(ValueError, "CRC"):
                validate(Path(d))

    def test_duplicate_target(self):
        with TemporaryDirectory() as d:
            room = Path(d) / "rooms"
            room.mkdir()
            (room / "0x1f0_bg2_x2.png").write_bytes(fake_png(512, 384))
            (room / "0x1f0_bg2_x4.png").write_bytes(fake_png(1024, 768))
            with self.assertRaisesRegex(ValueError, "another PNG"):
                validate(Path(d))

    def test_excessive_decoded_memory(self):
        with TemporaryDirectory() as d:
            room = Path(d) / "rooms"
            room.mkdir()
            (room / "0x1f0_bg2_x8.png").write_bytes(fake_png(8000, 8000))
            with self.assertRaisesRegex(ValueError, "128 MiB"):
                validate(Path(d))


if __name__ == "__main__":
    unittest.main()
