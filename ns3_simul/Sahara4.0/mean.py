import matplotlib.pyplot as plt
import numpy as np
import pandas as pd
import xml.etree.ElementTree as ET
import tkinter as tk
from tkinter import filedialog

def read_throughput_values(file_path):
    """Read throughput values from a file."""
    throughput_values = []
    with open(file_path, 'r') as file:
        for line in file:
            # Extract the numeric value from the line
            value = float(line.strip().split(':')[1])
            throughput_values.append(value)
    return throughput_values

def calculate_mean_average_every_100(values):
    """Calculate mean average of values every 100 data points."""
    mean_averages = []
    num_points = len(values)
    
    for i in range(0, num_points, 100):
        # Calculate the mean for the next 100 points (or remaining points)
        mean_average = np.mean(values[i:i + 100])
        mean_averages.append(mean_average)
        
    return mean_averages

def plot_data_and_mean(values, mean_averages):
    """Plot data points as 'x' markers and mean averages as a line with 'o' markers."""
    # Create an x-axis based on the number of data points
    x_axis_data = np.arange(1, len(values) + 1) / 100
    x_axis_mean = np.arange(1, len(mean_averages) + 1)
    
    # Plot all data points as "x" markers
    plt.figure()
    plt.plot(x_axis_data, values, 'x', label='Throughput Data', alpha=0.7)
    
    # Plot mean averages as a line with "o" markers
    plt.plot(x_axis_mean, mean_averages, 'o-', label='Mean Average (every 100 points)', color='red')
    
    # Add labels and legend
    plt.title('Throughput Data and Mean Average every 100 data points')
    plt.xlabel('Seconds ')
    plt.ylabel('Throughput (Mbs)')
    plt.legend()
    plt.grid(True)
    
    # Display the plot
    plt.show()


def main():
    file_path = filedialog.askopenfilename(title=f'Select file')
    
   # Read throughput values from the file
    throughput_values = read_throughput_values(file_path)
    
    # Calculate mean average every 100 data points
    mean_averages = calculate_mean_average_every_100(throughput_values)
    
    # Plot all data points and mean average
    plot_data_and_mean(throughput_values, mean_averages)

if __name__ == "__main__":
    main()
