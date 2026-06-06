import mtkahypar

init = mtkahypar.initialize(1)

# Check for factory functions
for attr in dir(mtkahypar):
    obj = getattr(mtkahypar, attr)
    if callable(obj) and attr[0].islower():
        print(f"{attr} -> {type(obj).__name__}")
