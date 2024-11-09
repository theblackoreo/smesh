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

# Prompt user to select the third file
file_path3 = filedialog.askopenfilename()  # Open file dialog

if file_path3:
    print("Selected file:", file_path3)
else:
    print("No file selected.")

# Read content from the third file
with open(file_path3, 'r') as file:
    content3 = file.read()

# Prompt user to select the third file
file_path4 = filedialog.askopenfilename()  # Open file dialog

if file_path4:
    print("Selected file:", file_path4)
else:
    print("No file selected.")

# Read content from the third file
with open(file_path4, 'r') as file:
    content4 = file.read()

# Parse the XML data from the files
root1 = ET.fromstring(content1)
root2 = ET.fromstring(content2)
root3 = ET.fromstring(content3)
root4 = ET.fromstring(content4)

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

# Extract flow data from the second file
flow_data3 = []
for flow in root3.find('FlowStats'):
    flow_id = int(flow.attrib['flowId'])
    delay_sum = float(flow.attrib['delaySum'][:-2]) / 1_000_000_000  # Convert to seconds
    jitter_sum = float(flow.attrib['jitterSum'][:-2]) / 1_000_000_000  # Convert to seconds
    lost_packets = int(flow.attrib['lostPackets'])
    flow_data3.append((flow_id, delay_sum, jitter_sum, lost_packets))

# Extract flow data from the second file
flow_data4 = []
for flow in root4.find('FlowStats'):
    flow_id = int(flow.attrib['flowId'])
    delay_sum = float(flow.attrib['delaySum'][:-2]) / 1_000_000_000  # Convert to seconds
    jitter_sum = float(flow.attrib['jitterSum'][:-2]) / 1_000_000_000  # Convert to seconds
    lost_packets = int(flow.attrib['lostPackets'])
    flow_data4.append((flow_id, delay_sum, jitter_sum, lost_packets))

# Unpack flow data from both files
flow_ids1, delays1, jitters1, lost_packets1 = zip(*flow_data1)
flow_ids2, delays2, jitters2, lost_packets2 = zip(*flow_data2)
flow_ids3, delays3, jitters3, lost_packets3 = zip(*flow_data3)
flow_ids4, delays4, jitters4, lost_packets4 = zip(*flow_data4)


# Create a figure and axis for plotting
fig, ax1 = plt.subplots()

# Define the width of the bars and the position offset for each protocol
bar_width = 0.25
position_offset = {
    'saharaFL': -1.5 * bar_width,
    'saharaSR': -0.5 * bar_width,
    'olsr': 0.5 * bar_width,
    'dsdv': 1.5 * bar_width  # Replace 'new_category' with your fourth category's name
}

# Plot delays from all files
ax1.bar([flow_id + position_offset['saharaFL'] for flow_id in flow_ids1], delays1, width=bar_width, color='red', label='Sahara FL')
ax1.bar([flow_id + position_offset['saharaSR'] for flow_id in flow_ids2], delays2, width=bar_width, color='orange', label='Sahara SR')
ax1.bar([flow_id + position_offset['olsr'] for flow_id in flow_ids3], delays3, width=bar_width, color='blue', label='OLSR')
ax1.bar([flow_id + position_offset['dsdv'] for flow_id in flow_ids4], delays4, width=bar_width, color='green', label='DSDV')


# Set labels and title for the plot
ax1.set_xlabel('Flow ID')
ax1.set_ylabel('Delay (s)')
ax1.set_title('Delay Comparison by Flow ID')

# Add legends
ax1.legend(loc='upper left')

# Create a figure and axis for the jitters
fig2, ax2 = plt.subplots()

# Plot jitters from all files
ax2.bar([flow_id + position_offset['saharaFL'] for flow_id in flow_ids1], jitters1, width=bar_width, color='red', label='Sahara FL')
ax2.bar([flow_id + position_offset['saharaSR'] for flow_id in flow_ids2], jitters2, width=bar_width, color='orange', label='Sahara SR')
ax2.bar([flow_id + position_offset['olsr'] for flow_id in flow_ids3], jitters3, width=bar_width, color='blue', label='OLSR')
ax2.bar([flow_id + position_offset['dsdv'] for flow_id in flow_ids4], jitters4, width=bar_width, color='green', label='DSDV')


# Set labels and title for the plot
ax2.set_xlabel('Flow ID')
ax2.set_ylabel('Jitter (s)')
ax2.set_title('Jitter Comparison by Flow ID')

# Add legends
ax2.legend(loc='upper left')

# Adjust layout and show the plots
fig.tight_layout()
fig2.tight_layout()
plt.show()
