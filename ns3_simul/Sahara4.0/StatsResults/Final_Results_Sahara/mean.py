import re
from tkinter import Tk
from tkinter.filedialog import askopenfilename

def extract_values(file_path):
    """
    Extracts the P and B values from the file and returns them as two lists.
    Assumes that each P and B is defined as '#P = <value>' and '#B = <value>'.
    """
    P_values = []
    B_values = []
    
    with open(file_path, 'r') as file:
        for line in file:
            # Extracting values using regular expressions
            p_match = re.search(r'#P\s*=\s*(\d+)', line)
            b_match = re.search(r'#B\s*=\s*(\d+)', line)
            
            if p_match:
                P_values.append(int(p_match.group(1)))
            if b_match:
                B_values.append(int(b_match.group(1)))
    
    return P_values, B_values

def calculate_average(values):
    """
    Calculates the average of a list of numerical values.
    """
    return sum(values) / len(values) if values else 0

def main():
    # Hide the root Tk window
    Tk().withdraw()
    
    # Ask the user to select a file
    file_path = askopenfilename(title="Select your file")
    
    if file_path:
        # Extract P and B values from the file
        P_values, B_values = extract_values(file_path)
        
        # Calculate the averages
        avg_P = calculate_average(P_values)
        avg_B = calculate_average(B_values)
        
        # Output the results
        print(f"Average P: {avg_P}")
        print(f"Average B: {avg_B}")
    else:
        print("No file selected.")

if __name__ == '__main__':
    main()
