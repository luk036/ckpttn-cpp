import mtkahypar

init = mtkahypar.initialize(1)

try:
    h = mtkahypar.Hypergraph(
        5,
        3,
        [0, 3, 6, 9],
        [0, 1, 2, 0, 3, 4, 1, 2, 4],
        vertex_weights=[10000000, 1, 1, 1, 1]
    )
    print("Hypergraph created:", h)
    
    context = mtkahypar.Context()
    context.set_partitioning_parameters(
        number_of_blocks=2,
        imbalance=0.0000001,
    )
    
    print("Partitioning...")
    partitioned_hg = mtkahypar.partition(h, context)
    print("Blocks:", [partitioned_hg.block(v) for v in range(5)])
except Exception as e:
    print(f"Error: {type(e).__name__}: {e}")
