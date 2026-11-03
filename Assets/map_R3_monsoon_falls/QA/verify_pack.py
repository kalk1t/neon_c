"""Verify this extracted pack using the standard library only."""
from pathlib import Path
import hashlib
import json
import sys
root=Path(__file__).resolve().parents[1]
manifest=json.loads((root/"manifest.json").read_text())
bad=[]
for rec in manifest["files"]:
    p=root/rec["path"]
    if not p.is_file():
        bad.append("MISSING "+rec["path"]);continue
    raw=p.read_bytes()
    if len(raw)!=rec["bytes"] or hashlib.sha256(raw).hexdigest()!=rec["sha256"]:
        bad.append("CHANGED "+rec["path"])
print(json.dumps({"ok":not bad,"checked_files":len(manifest["files"]),"issues":bad},indent=2))
sys.exit(1 if bad else 0)
