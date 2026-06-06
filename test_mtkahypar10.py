import mtkahypar
init = mtkahypar.initialize(1)

# Try with keyword arguments that match the native function
try:
    h = mtkahypar.Hypergraph(
        num_vertices=5,
        num_hyperedges=3,
        edge_indices=[0, 3, 6, 9],
        edges=[0, 1, 2, 0, 3, 4, 1, 2, 4],
        vertex_weights=[10000000, 1, 1, 1, 1]
    )
    print("Success!")
    print(h)
except TypeError as e:
    print(f"TypeError: {e}")
except Exception as e:
    print(f"Other error: {type(e).__name__}: {e}")
