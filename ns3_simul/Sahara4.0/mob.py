import csv
import math

# Definisci il numero totale di nodi e anelli
num_nodes = 70
num_rings = 5
central_node = (400, 400)

# Definisci il raggio base e l'incremento del raggio per ogni anello
base_radius = 33
delta_radius = 33

# Numero di nodi per anello, aumentando verso l'esterno
nodes_per_ring = [5, 10, 15, 20, 19]  # Somma = 69 (70 meno il nodo centrale)

# Verifica che il numero totale di nodi sia corretto
assert sum(nodes_per_ring) == num_nodes - 1, "Il numero totale di nodi non corrisponde"

# Definisci il percorso del file CSV
csv_file_path = 'mobility.csv'

# Apri il file CSV per scrivere
with open(csv_file_path, mode='w', newline='') as file:
    writer = csv.writer(file)
    # Scrivi l'intestazione (se necessario)
    # writer.writerow(['NodeID', 'PosX', 'PosY', 'Radius', 'Omega'])
    
    # Scrivi il nodo centrale (nodeID 0)
    writer.writerow([0, central_node[0], central_node[1], 0, 0.0])
    
    nodeID = 1
    for ring_index, nodes_in_current_ring in enumerate(nodes_per_ring):
        ring_radius = base_radius + ring_index * delta_radius
        angle_step = 2 * math.pi / nodes_in_current_ring
        for node_index in range(nodes_in_current_ring):
            angle = node_index * angle_step
            posX = central_node[0] + ring_radius * math.cos(angle)
            posY = central_node[1] + ring_radius * math.sin(angle)
            omega = 0.0  # Puoi modificare omega se necessario
            writer.writerow([nodeID, posX, posY, ring_radius, omega])
            nodeID += 1

print("File CSV creato con successo in:", csv_file_path)
