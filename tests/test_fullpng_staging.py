"""Host-only tests for isolated home-screen test title staging."""
import importlib.util
import json
import tempfile
from pathlib import Path

path = Path(__file__).resolve().parents[1] / "ps5" / "stage_fullpng_test.py"
spec = importlib.util.spec_from_file_location("stage_fullpng_test", path)
module = importlib.util.module_from_spec(spec)
spec.loader.exec_module(module)

with tempfile.TemporaryDirectory() as td:
    root = Path(td)
    source = root / "build" / "PPSA90435"
    for rel in ("sce_sys/param.json", "sce_sys/icon0.png", "sce_module/libc.prx"):
        (source / rel).parent.mkdir(parents=True, exist_ok=True)
    (source / "sce_sys/param.json").write_text(json.dumps({
        "titleId": "PPSA90435",
        "conceptId": "90435",
        "contentId": "UP9000-PPSA90435_00-EGGOFTIME0000000",
        "localizedParameters": {"en-US": {"titleName": "CTPS5"}}
    }))
    (source / "sce_sys/icon0.png").write_bytes(b"icon")
    (source / "sce_module/libc.prx").write_bytes(b"runtime")
    (source / "eboot.bin").write_bytes(bytes.fromhex("5414f5ee") + b"mock SELF")
    test_title = root / "PPSA90440"
    result = module.stage(source, test_title)
    assert (source / "sce_sys/param.json").exists()
    assert b"mock SELF" in (test_title / "eboot.bin").read_bytes()
    staged = json.loads((test_title / "sce_sys/param.json").read_text())
    original = json.loads((source / "sce_sys/param.json").read_text())
    assert staged["titleId"] == "PPSA90440"
    assert original["titleId"] == "PPSA90435"
    assert staged["localizedParameters"]["en-US"]["titleName"] == "CTPS5 Full PNG Test"
    assert result["ps5_console_tested"] is False
    assert len(staged["contentId"].split("-")[-1]) == 16
    try:
        module.stage(source, test_title)
        assert False, "Should refuse overwrite"
    except FileExistsError:
        pass
    (source / "eboot.bin").write_bytes(b"not a SELF")
    try:
        module.stage(source, root / "bad-test")
        assert False, "Should reject non-SELF"
    except ValueError:
        pass
print("PASS: isolated title staging, metadata, preservation, integrity checks")
