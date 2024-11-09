import csv
import math

# Define the number of nodes
num_nodes = 30
# Define the central node position
central_node = (400, 400)

# Define the radius of the circle for placing nodes
radius = 150

# Calculate the angle step (in radians) for the spoke nodes
angle_step = 2 * math.pi / (num_nodes - 1)

# Define the file path
csv_file_path = 'mobility.csv'

# Open the CSV file for writing
with open(csv_file_path, mode='w', newline='') as file:
    writer = csv.writer(file)
    # Write the header
   
    
    # Write the central node (nodeID 1)
    writer.writerow([0, central_node[0], central_node[1]])
    
    # Calculate positions for each spoke node
    for i in range(0, (int(num_nodes/2)-1)):
        # Calculate the angle for the current spoke node
        angle = i * angle_step*2
        
        # Calculate the position for the spoke node
        posX = central_node[0] + (radius/2 + radius/4) * math.cos(angle)
        posY = central_node[1] + (radius/2 + radius/4) * math.sin(angle)
        
        # Write the spoke node (nodeID i+1) to the CSV file
        omega = 0.00
        writer.writerow([i + 1, posX, posY, (radius/2 + radius/4), omega])

    for i in range(int(num_nodes/2),( int(num_nodes/2) + int(num_nodes/4) )):
        # Calculate the angle for the current spoke node
        angle = i * angle_step*4
        
        # Calculate the position for the spoke node
        posX = central_node[0] + radius/2 * math.cos(angle)
        posY = central_node[1] + radius/2 * math.sin(angle)
        
        # Write the spoke node (nodeID i+1) to the CSV file
        omega = -0.00
        writer.writerow([i, posX, posY, radius/2, omega])

    for i in range(int(num_nodes/2) + int(num_nodes/4), num_nodes):
        # Calculate the angle for the current spoke node
        angle = i * angle_step*4
        
        # Calculate the position for the spoke node
        posX = central_node[0] + radius/4 * math.cos(angle)
        posY = central_node[1] + radius/4 * math.sin(angle)
        
        # Write the spoke node (nodeID i+1) to the CSV file
        omega = 0.00
        writer.writerow([i, posX, posY, radius/4, omega])
csv_file_path