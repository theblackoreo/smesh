import matplotlib.pyplot as plt
import numpy as np
import tkinter as tk
from tkinter import filedialog

def read_throughput_values(file_path):
    """Read throughput values from a file."""
    throughput_values = []
    with open(file_path, 'r') as file:
        for line in file:
            parts = line.strip().split(':')
            if len(parts) == 2:  # Ensure there are exactly 2 parts
                try:
                    value = float(parts[1])
                    throughput_values.append(value)
                except ValueError:
                    print(f"Could not convert '{parts[1]}' to float. Skipping line.")
            else:
                print(f"Skipping malformed line: {line}")
    print(f"Read {len(throughput_values)} throughput values from {file_path}")
    return throughput_values

def calculate_mean_average_every_2(values):
    """Calculate mean average of values every 2 data points."""
    mean_averages = []
    num_points = len(values)
    
    for i in range(0, num_points, 5):
        mean_average = np.mean(values[i:i + 5])
        mean_averages.append(mean_average)
        
    return mean_averages

def plot_data_and_mean(data_dict):
    """Plot only lines for data points and mean averages for multiple protocols."""
    plt.figure()
    
    for protocol, (values, mean_averages) in data_dict.items():
        # Create an x-axis for data and mean values
        x_axis_data = np.arange(len(values)) * 0.2
        x_axis_mean = np.arange(len(mean_averages)) * 0.4
        
        # Plot the throughput data as a line
        #plt.plot(x_axis_data, values, label=f'{protocol} Data', alpha=0.7)
        
        # Plot the mean averages as a line (without markers)
        plt.plot(x_axis_mean, mean_averages, label=f'{protocol}', alpha=0.7)

    # Set the x-axis limits from 10 to 40 seconds
    plt.xlim(0, 7)
    plt.ylim(0, 80)
    
    # Add labels and legend
    plt.title('Throughput Mean Average every 5 data points')
    plt.xlabel('Seconds')
    plt.ylabel('Throughput (Mbs)')
    plt.legend()
    plt.grid(True)
    
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
    
    # Read and process throughput values for each protocol
    for protocol, file_path in file_paths.items():
        throughput_values = read_throughput_values(file_path)
        mean_averages = calculate_mean_average_every_2(throughput_values)
        data_dict[protocol] = (throughput_values, mean_averages)
    
    # Plot all data points and mean averages for each protocol
    plot_data_and_mean(data_dict)

if __name__ == "__main__":
    main()
