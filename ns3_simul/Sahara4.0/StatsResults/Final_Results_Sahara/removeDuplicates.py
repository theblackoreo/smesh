import os
import tkinter as tk
from tkinter import filedialog

def elimina_duplicati(file_input, file_output):
    # Legge il file .rtf rimuovendo i duplicati
    try:
        with open(file_input, 'r') as file:
            # Legge il file e rimuove duplicati
            lines = file.readlines()
            unique_lines = list(set(lines))  # Usa un set per rimuovere duplicati

        # Ordina le righe in modo che abbiano lo stesso ordine
        unique_lines.sort()

        # Scrive le righe uniche nel file di output
        with open(file_output, 'w') as output_file:
            for line in unique_lines:
                output_file.write(line)
        
        print(f"Duplicati rimossi con successo. File salvato come: {file_output}")
    
    except FileNotFoundError:
        print(f"Errore: il file {file_input} non è stato trovato.")
    except Exception as e:
        print(f"Si è verificato un errore: {e}")

def seleziona_file_rtf():
    # Apre una finestra di dialogo per selezionare un file .rtf
    root = tk.Tk()
    root.withdraw()  # Nasconde la finestra principale
    file_input = filedialog.askopenfilename(title="Seleziona un file RTF", filetypes=[("RTF files", "*.rtf")])
    return file_input

if __name__ == "__main__":
    # Seleziona il file tramite finestra di dialogo
    file_input = seleziona_file_rtf()
    
    if not file_input:
        print("Nessun file selezionato.")
    else:
        file_output = input("Inserisci il nome del file di output (es. risultato.txt): ")
        
        # Verifica che il file abbia estensione .rtf
        if not file_input.endswith(".rtf"):
            print("Errore: il file di input deve essere un file .rtf")
        else:
            elimina_duplicati(file_input, file_output)
