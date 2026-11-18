"""ROM-free end-to-end test of a generated PNG pack and existing installer."""
import json
from pathlib import Path
from tempfile import TemporaryDirectory
import struct
import sys
import unittest
import zipfile
import zlib

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / "ps5"))
from check_fullpng_pack import validate  # noqa: E402
from install_texture_pack import install  # noqa: E402


def chunk(kind, data):
    payload = kind + data
    return struct.pack(">I", len(data)) + payload + struct.pack(">I", zlib.crc32(payload))


def png_rgba(width, height):
    ihdr = struct.pack(">IIBBBBB", width, height, 8, 6, 0, 0, 0)
    # Opaque red pixels in RGBA, one PNG filter type byte per scanline.
    raw = b"".join(b"\x00" + b"\xff\x00\x00\xff" * width for _ in range(height))
    return (b"\x89PNG\r\n\x1a\n" + chunk(b"IHDR", ihdr) +
            chunk(b"IDAT", zlib.compress(raw)) + chunk(b"IEND", b""))


class PackIntegrationTest(unittest.TestCase):
    def test_png_pack_stages_without_changing_active_config(self):
        with TemporaryDirectory() as temporary:
            base = Path(temporary)
            name = "0x1d0_bg2_x2.png"
            data = png_rgba(64, 64)
            archive = base / "test-pack.zip"
            manifest = {
                "format_version": 1, "game_code": "YQUE",
                "id": "fullpng-smoke", "name": "Full PNG smoke", "version": "1.0"
            }
            with zipfile.ZipFile(archive, "w", compression=zipfile.ZIP_DEFLATED) as z:
                z.writestr("pack.json", json.dumps(manifest))
                z.writestr("graphics/rooms/" + name, data)
            self.assertEqual(validate(archive), (1, 64 * 64 * 4))
            folder = base / "data"
            (folder).mkdir()
            config = folder / "ps5.ini"
            config.write_text("active_pack=original-pack\n")
            original = config.read_bytes()
            destination = install(archive, folder, activate=False)
            self.assertEqual(
                (destination / "graphics" / "rooms" / name).read_bytes(), data)
            self.assertEqual(config.read_bytes(), original,
                             "Testing must not replace the working active graphics pack")
            self.assertEqual(destination.name, "fullpng-smoke")


if __name__ == "__main__":
    unittest.main()
