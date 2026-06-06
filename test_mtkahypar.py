import mtkahypar

# Build hypergraph from 5 vertices, 3 nets with weighted vertices
# Mimics our test_balance.hgr: weights [10000000, 1, 1, 1, 1]
dataset = mtkahypar.Hypergraph(
    mtkahypar.HypergraphType.STATIC,
    num_vertices=5,
    num_hyperedges=3,
    edge_indices=[0, 3, 6, 9],
    edges=[0, 1, 2, 0, 3, 4, 1, 2, 4],
    vertex_weights=[10000000, 1, 1, 1, 1]
)

# Try partitioning with very strict imbalance (epsilon=0.0000001)
print("Partitioning with imbalance=0.0000001 (very strict)...")
try:
    context = mtkahypar.Context()
    context.set_partitioning_parameters(
        number_of_blocks=2,
        imbalance=0.0000001,
    )
    context.set_verbosity(mtkahypar.Verbosity.STATUS)

    partition = mtkahypar.partition(dataset, context)
    print("Partition:", partition)
    print("No error - partition succeeded")
except Exception as e:
    print("Error:", e)
