import pandas as pd
import matplotlib.pyplot as plt
import xml.etree.ElementTree as ET
import matplotlib.pyplot as plt
import tkinter as tk
from tkinter import filedialog

# Define file paths for the four CSV files
#file_paths = ["file1.csv", "file2.csv", "file3.csv", "file4.csv"]

# Initialize tkinter and hide the main window
root_tk = tk.Tk()
root_tk.withdraw()

# Prompt user to select files
file_paths = []
for i in range(4):
    file_path = filedialog.askopenfilename(title=f'Select file {i + 1}')
    file_paths.append(file_path)



# Define colors and labels for the plots
colors = ['blue', 'orange', 'green', 'red']
labels = ['File 1', 'File 2', 'File 3', 'File 4']

# Create a figure and axis
fig, ax = plt.subplots()

# Iterate over each file path, color, and label
for file_path, color, label in zip(file_paths, colors, labels):
    # Load the CSV file into a DataFrame
    df = pd.read_csv(file_path)
    
    # Extract the relevant columns
    seconds = df['SimulationSecond']
    throughput = df['ReceiveRate']
    packets_received = df['PacketsReceived']
    
    # Plot the line
    ax.plot(seconds, throughput, color=color, label=label)
    
    # Annotate each point with the value of PacketsReceived
    for x, y, p in zip(seconds, throughput, packets_received):
        ax.annotate(f"{p}", (x, y), textcoords="offset points", xytext=(0, 5), ha='center', color=color)

# Set labels and title
ax.set_xlabel('Simulation Second')
ax.set_ylabel('Throughput (Receive Rate)')
ax.set_title('Throughput vs Simulation Second with Packets Received Annotation')

# Add a legend
ax.legend()

# Show the plot
plt.show()
