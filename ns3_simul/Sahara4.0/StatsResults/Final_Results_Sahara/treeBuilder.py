import re
import tkinter as tk
from tkinter import filedialog
import networkx as nx
import matplotlib.pyplot as plt

# Function to ask user for a file
def ask_for_file():
    root = tk.Tk()
    root.withdraw()  # Hide the root window
    file_path = filedialog.askopenfilename(filetypes=[("RTF files", "*.rtf")])
    return file_path

# Function to read the file
def read_routing_file(file_path):
    with open(file_path, 'r') as file:
        data = file.read()
    return data

# Function to extract nodes and parents and construct a tree
def build_tree(data):
    nodes = re.split(r"End_{30,}", data)
    parent_child_pairs = []
    
    for node_data in nodes:
        node_match = re.search(r"Node: (\d+)", node_data)
        parent_match = re.search(r"Parent: ([\d.]+)", node_data)
        if node_match and parent_match:
            node_id = node_match.group(1)
            parent_ip = parent_match.group(1)
            parent_child_pairs.append((parent_ip, f"Node {node_id}"))

    return parent_child_pairs

# Function to draw the tree
def draw_tree(parent_child_pairs):
    G = nx.DiGraph()
    
    for parent, child in parent_child_pairs:
        G.add_edge(parent, child)
    
    pos = nx.spring_layout(G)
    plt.figure(figsize=(10, 8))
    nx.draw(G, pos, with_labels=True, node_color='skyblue', node_size=3000, font_size=10, font_weight='bold', edge_color='gray')
    plt.title("Parent-Child Tree Structure")
    plt.show()

# Main function
def main():
    file_path = ask_for_file()
    data = read_routing_file(file_path)
    
    parent_child_pairs = build_tree(data)
    
    if parent_child_pairs:
        draw_tree(parent_child_pairs)
    else:
        print("No valid parent-child relationships found.")

# Run the program
if __name__ == "__main__":
    main()

