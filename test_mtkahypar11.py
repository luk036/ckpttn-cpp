import mtkahypar
import sys

init = mtkahypar.initialize(1)

# Try file-based constructor
try:
    h = mtkahypar.Hypergraph("/mnt/d/github/cpp/ckpttn-cpp/testcases/test.hgr")
    print("File only:", h, file=sys.stderr)
    sys.stderr.flush()
except Exception as e:
    print(f"File only: {type(e).__name__}: {e}", file=sys.stderr)
    sys.stderr.flush()

try:
    h = mtkahypar.Hypergraph("/mnt/d/github/cpp/ckpttn-cpp/testcases/test.hgr", mtkahypar.FileFormat.HMETIS)
    print("File+format:", h, file=sys.stderr)
    sys.stderr.flush()
except Exception as e:
    print(f"File+format: {type(e).__name__}: {e}", file=sys.stderr)
    sys.stderr.flush()
