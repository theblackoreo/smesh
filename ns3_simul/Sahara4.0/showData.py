import xml.etree.ElementTree as ET
import matplotlib.pyplot as plt
import tkinter as tk
from tkinter import filedialog

# Initialize tkinter and hide the main window
root_tk = tk.Tk()
root_tk.withdraw() 

# Prompt user to select the first file
file_path1 = filedialog.askopenfilename()  # Open file dialog

if file_path1:
    print("Selected file:", file_path1)
else:
    print("No file selected.")

# Read content from the first file
with open(file_path1, 'r') as file:
    content1 = file.read()

# Prompt user to select the second file
file_path2 = filedialog.askopenfilename()  # Open file dialog

if file_path2:
    print("Selected file:", file_path2)
else:
    print("No file selected.")

# Read content from the second file
with open(file_path2, 'r') as file:
    content2 = file.read()

# Parse the XML data from the files
root1 = ET.fromstring(content1)
root2 = ET.fromstring(content2)

# Extract flow data from the first file
flow_data1 = []
for flow in root1.find('FlowStats'):
    flow_id = int(flow.attrib['flowId'])
    delay_sum = float(flow.attrib['delaySum'][:-2]) / 1_000_000_000  # Convert to seconds
    jitter_sum = float(flow.attrib['jitterSum'][:-2]) / 1_000_000_000  # Convert to seconds
    lost_packets = int(flow.attrib['lostPackets'])
    flow_data1.append((flow_id, delay_sum, jitter_sum, lost_packets))

# Extract flow data from the second file
flow_data2 = []
for flow in root2.find('FlowStats'):
    flow_id = int(flow.attrib['flowId'])
    delay_sum = float(flow.attrib['delaySum'][:-2]) / 1_000_000_000  # Convert to seconds
    jitter_sum = float(flow.attrib['jitterSum'][:-2]) / 1_000_000_000  # Convert to seconds
    lost_packets = int(flow.attrib['lostPackets'])
    flow_data2.append((flow_id, delay_sum, jitter_sum, lost_packets))

# Unpack flow data from both files
flow_ids1, delays1, jitters1, lost_packets1 = zip(*flow_data1)
flow_ids2, delays2, jitters2, lost_packets2 = zip(*flow_data2)

# Create a figure and axis for plotting
fig, ax1 = plt.subplots()

# Plot delays from both files
ax1.set_xlabel('Flow ID')
ax1.set_ylabel('Delay (s)')
ax1.plot(flow_ids1, delays1, color='red', label='Sahara Delays')
ax1.plot(flow_ids2, delays2, color='orange', label='OLSR Delays')

fig2, ax2 = plt.subplots()
# Create a second y-axis for plotting jitters
ax2.set_ylabel('Jitter (s)')
ax2.plot(flow_ids1, jitters1, color='blue', label='Sahara Jitters')
ax2.plot(flow_ids2, jitters2, color='green', label='OLSR Jitters ')

# Add legends for both y-axes
ax1.legend(loc='upper right')

ax2.legend(loc='upper right')

# Adjust the layout and show the plot
fig.tight_layout()
plt.show()
