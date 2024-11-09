import matplotlib.pyplot as plt
import tkinter as tk
from tkinter import filedialog
from collections import defaultdict

def read_data(file_path):
    """Read data from a given file and return a dictionary mapping node IDs to tuples of (#P, #B) values."""
    data = {}
    with open(file_path, 'r') as file:
        lines = file.readlines()
        node_id = None
        p_value = None
        b_value = None
        for line in lines:
            line = line.strip()
            # Each node information starts with "Node"
            if line.startswith("Node"):
                node_id = int(line.split()[1])
            elif line.startswith("#P"):
                p_value = int(line.split('=')[1].strip())
            elif line.startswith("#B"):
                b_value = int(line.split('=')[1].strip())
            # Store the tuple (#P, #B) in the dictionary
            data[node_id] = (p_value, b_value)
    return data

def plot_histograms(file_paths):
    """Plot separate histograms for #P and #B values using data from the given file paths."""
    # Create a dictionary to hold data for each node ID from all files
    node_data = defaultdict(lambda: defaultdict(list))
    # Read data from each file and store it in node_data
    for file_index, file_path in enumerate(file_paths):
        data = read_data(file_path)
        for node_id, values in data.items():
            node_data[node_id]['P'].append(values[0])
            node_data[node_id]['B'].append(values[1])

    # Define colors for each file's data
    colors = ['blue', 'orange', 'green', 'red', 'pink']
    legends = ["Flooding", "Sahara SR", "Sahara SR Dynamic", "OLSR", "DSDV"]  # Legend labels

    # Plot separate histograms for #P values
    plt.figure(figsize=(12, 6))
    bar_width = 0.2  # Adjust the width of the bars
    num_files = len(file_paths)
    x_positions = [i + bar_width * (j - (num_files - 1) / 2) for i in range(len(node_data)) for j in range(num_files)]
    for file_index, file_path in enumerate(file_paths):
        p_values = [node_data[node_id]['P'][file_index] for node_id in node_data]
        plt.bar(x_positions[file_index::num_files], p_values, width=bar_width, color=colors[file_index],
                label=legends[file_index], edgecolor='black', align='edge')

    plt.xlabel('Node ID')
    plt.ylabel('Number of packets')
    plt.title('Number of Packets processed by Nodes')
    plt.xticks(range(len(node_data)), node_data.keys())
    plt.legend()
    plt.show()

    # Plot separate histograms for #B values
    plt.figure(figsize=(12, 6))
    x_positions = [i + bar_width * (j - (num_files - 1) / 2) for i in range(len(node_data)) for j in range(num_files)]
    for file_index, file_path in enumerate(file_paths):
        b_values = [node_data[node_id]['B'][file_index] for node_id in node_data]
        plt.bar(x_positions[file_index::num_files], b_values, width=bar_width, color=colors[file_index],
                label=legends[file_index], edgecolor='black', align='edge')

    plt.xlabel('Node ID')
    plt.ylabel('Number of Bytes')
    plt.title('Number of Bytes processed by nodes')
    plt.xticks(range(len(node_data)), node_data.keys())
    plt.legend()
    plt.show()

# Initialize tkinter and hide the main window
root_tk = tk.Tk()
root_tk.withdraw()

# Prompt user to select files
file_paths = []
for i in range(5):
    file_path = filedialog.askopenfilename(title=f'Select file {i + 1}')
    file_paths.append(file_path)

# Call the function to plot histograms
plot_histograms(file_paths)