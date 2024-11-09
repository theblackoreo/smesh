import matplotlib.pyplot as plt
import numpy as np
import tkinter as tk
from tkinter import filedialog

def read_latency_values(file_path):
    """Read latency values from a file."""
    latency_values = []
    with open(file_path, 'r') as file:
        for line in file:
            parts = line.strip().split(':')
            if len(parts) == 2:  # Ensure there are exactly 2 parts
                try:
                    value = float(parts[1])
                    latency_values.append(value)
                except ValueError:
                    print(f"Could not convert '{parts[1]}' to float. Skipping line.")
            else:
                print(f"Skipping malformed line: {line}")
    print(f"Read {len(latency_values)} latency values from {file_path}")
    return latency_values

def calculate_mean_average_every_5(values):
    """Calculate mean average of values every 5 data points."""
    mean_averages = []
    num_points = len(values)
    
    for i in range(0, num_points, 5):
        mean_average = np.mean(values[i:i + 5])
        mean_averages.append(mean_average)
        
    return mean_averages

def plot_data_and_mean(data_dict):
    """Plot only lines for data points and mean averages for multiple protocols."""
    plt.figure()
    
    for protocol, values_tuple in data_dict.items():
        values, mean_averages = values_tuple  # Ensure tuple unpacking happens here
        # Create an x-axis for data and mean values
        x_axis_data = np.arange(len(values)) * 0.2  # assuming each value is recorded every 0.2 seconds
        x_axis_mean = np.arange(len(mean_averages)) * 0.4  # mean values every 5 data points
        
        # Plot the latency data as a line
        #plt.plot(x_axis_data, values, label=f'{protocol} Latency Data', alpha=0.7)
        
        # Plot the mean averages as a line (without markers)
        plt.plot(x_axis_mean, mean_averages, label=f'{protocol} Mean Latency', alpha=0.7)

    # Dynamically set y limit based on maximum value from all protocols
    all_values = [max(values) for values, _ in data_dict.values()]
    plt.ylim(0, max(all_values) + 5)

    # Add labels and legend
    plt.title('Latency Mean Average every 5 data points')
    plt.xlabel('Seconds')
    plt.ylabel('Latency (ms)')
    plt.legend()
    plt.grid(True)
    plt.xlim(0, 7)
    
    
    # Display the plot
    plt.show()


def main():
    # Initialize tkinter window for file dialog
    root = tk.Tk()
    root.withdraw()  # Hide the root window

    # File dialog to select four files
    file_paths = {}
    protocols = ['SAHARA', 'OLSR', 'AODV', 'DSDV']
    
    for protocol in protocols:
        file_path = filedialog.askopenfilename(title=f'Select file for {protocol}')
        file_paths[protocol] = file_path
    
    data_dict = {}
    
    # Read and process latency values for each protocol
    for protocol, file_path in file_paths.items():
        latency_values = read_latency_values(file_path)
        mean_averages = calculate_mean_average_every_5(latency_values)
        data_dict[protocol] = (latency_values, mean_averages)
    
    # Plot all data points and mean averages for each protocol
    plot_data_and_mean(data_dict)

if __name__ == "__main__":
    main()
