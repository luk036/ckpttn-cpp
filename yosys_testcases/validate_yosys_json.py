# flake8: noqa: E402
import json
import os
import sys

from jsonschema import validate

# Add the src directory to the path to import Netlist
sys.path.insert(0, os.path.join(os.path.dirname(__file__), "..", "src"))

from netlistx.netlist import read_yosys_json

# Load schema and data
with open("yosys_schema.json") as f:
    schema = json.load(f)
with open("sphere_netlist.json") as f:
    data = json.load(f)

# Validate
validate(instance=data, schema=schema)

# Example usage of read_yosys_json()
if __name__ == "__main__":
    print("Testing read_yosys_json() function...")
    try:
        netlist = read_yosys_json("sphere_netlist.json")
        print("Successfully loaded netlist from sphere_netlist.json")
        print(f"  Modules: {netlist.number_of_modules()}")
        print(f"  Nets: {netlist.number_of_nets()}")
        print(f"  Total nodes: {netlist.number_of_nodes()}")
        print(f"  Pins (edges): {netlist.number_of_pins()}")
        print(f"  Pads (I/O ports): {netlist.num_pads}")
    except Exception as e:
        print(f"Error: {e}")
