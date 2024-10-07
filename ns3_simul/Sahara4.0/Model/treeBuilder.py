import tkinter as tk
from tkinter import filedialog
from anytree import Node, RenderTree

def main():
    # Hide the root Tkinter window
    root = tk.Tk()
    root.withdraw()

    # Ask the user to select a .rtf file
    file_path = filedialog.askopenfilename(filetypes=[("RTF files", "*.rtf")])
    if not file_path:
        print("No file selected.")
        return

    # Read the content of the file
    with open(file_path, 'r') as f:
        data = f.read()

    # Split the data into blocks for each node
    blocks = data.split('End______________________________________')

    node_parent_map = {}  # Maps node numbers to their parent node numbers

    for block in blocks:
        lines = block.strip().splitlines()
        node = None
        parent_ip = None
        for line in lines:
            if line.startswith('Node:'):
                node = line.split(':')[1].strip()
            elif line.startswith('Parent:'):
                parent_ip = line.split(':')[1].strip()
        if node and parent_ip:
            node_num = int(node)
            parent_node_num = int(parent_ip.split('.')[-1])  # Extract the last octet
            node_parent_map[node_num] = parent_node_num

    # Build the tree using anytree
    node_dict = {}  # Maps node numbers to anytree Nodes

    for node_num in node_parent_map:
        parent_num = node_parent_map[node_num]
        # Create node if it doesn't exist
        if node_num not in node_dict:
            node_dict[node_num] = Node(f"Node {node_num}")
        # Create parent node if it doesn't exist
        if parent_num not in node_dict:
            node_dict[parent_num] = Node(f"Node {parent_num}")
        # Set the parent
        node_dict[node_num].parent = node_dict[parent_num]

    # Find all root nodes (nodes without parents)
    roots = [node for node in node_dict.values() if node.is_root]

    # Display the tree(s)
    for root_node in roots:
        print(f"Tree rooted at {root_node.name}:")
        for pre, fill, node in RenderTree(root_node):
            print(f"{pre}{node.name}")
        print("\n")

if __name__ == "__main__":
    main()
