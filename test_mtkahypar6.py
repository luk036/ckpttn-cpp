import mtkahypar
init = mtkahypar.initialize(1)

# Check if we can create via file
import tempfile, os

# Create hMetis format file (1-indexed)
hgr_content = "3 5 10\n1 2 3\n1 4 5\n2 3 5\n10000000 1 1 1 1\n"
tmpfile = "/tmp/test_balance.hgr"
with open(tmpfile, "w") as f:
    f.write(hgr_content)

# Try loading from file
try:
    h = mtkahypar.Hypergraph(tmpfile, mtkahypar.FileFormat.HMETIS)
    print("Loaded from file:", h)
    print("Nodes:", h.num_nodes(), "Edges:", h.num_edges())
except Exception as e:
    print(f"File load error: {e}")

# Check if partition works directly on dataset
try:
    h = mtkahypar.Hypergraph(
        num_nodes=5,
        num_edges=3,
        edge_indices=[0, 3, 6, 9],
        edges=[0, 1, 2, 0, 3, 4, 1, 2, 4],
        vertex_weights=[10000000, 1, 1, 1, 1]
    )
    print("Direct construction:", h)
except Exception as e:
    print(f"Direct construction error: {e}")
