import mtkahypar
init = mtkahypar.initialize(1)
print("Graph class:", mtkahypar.Graph)
# Check if Graph has a constructor
try:
    g = mtkahypar.Graph()
    print("Graph created:", g)
except Exception as e:
    print(f"Graph error: {e}")

# Check help on Context
help(mtkahypar.Context)
