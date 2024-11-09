import re

# Function to read the file
def read_routing_file(file_path):
    with open(file_path, 'r') as file:
        data = file.read()
    return data

# Function to process and check duplicate tuples
def check_duplicate_tuples(data):
    nodes = re.split(r"End_{30,}", data)
    duplicates = {}
    
    for node_data in nodes:
        node_match = re.search(r"Node: (\d+)", node_data)
        if node_match:
            node_id = node_match.group(1)
            tuples = re.findall(r"Tuple: ([\d., ]+)", node_data)
            duplicates_in_node = [tuple_ for tuple_ in tuples if tuples.count(tuple_) > 1]
            if duplicates_in_node:
                duplicates[node_id] = duplicates_in_node
    
    return duplicates

# Main function
def main():
    file_path = input("Please enter the path to the .rtf file: ")
    data = read_routing_file(file_path)
    
    duplicates = check_duplicate_tuples(data)
    
    if duplicates:
        for node, duplicate_tuples in duplicates.items():
            print(f"Node {node} contains duplicate tuples: {set(duplicate_tuples)}")
    else:
        print("No duplicate tuples found in any node.")

# Run the program
if __name__ == "__main__":
    main()
