import numpy as np
import pandas as pd
import matplotlib.pyplot as plt
from scipy.optimize import curve_fit

# Dati forniti
data = {
    'nodes': [25, 36, 49, 64, 81, 100, 121, 144, 169, 185, 190],
    'amountOfData': [1.65, 2.37, 3.57, 4.75, 6.24, 7.95, 9.78, 12.61, 16.20, 18.21, 19.06]
}

# Creiamo un DataFrame
df = pd.DataFrame(data)

# Funzione sub-quadratica
def sub_quadratic(x, a, b, c):
    return a * x**b + c

# Variabile indipendente: nodes
X = np.array(df['nodes'])

# Dati target (amountOfData)
y = np.array(df['amountOfData'])

# Eseguiamo il fitting con la funzione sub-quadratica
popt, pcov = curve_fit(sub_quadratic, X, y, bounds=(0, [np.inf, 2, np.inf]))  # limitiamo b a valori <= 2

# Coefficienti della regressione non lineare
a, b, c = popt

print(f'Funzione trovata: amountOfData = {a:.4f} * nodes^{b:.4f} + {c:.4f}')

# Previsioni basate sui dati di input
y_pred = sub_quadratic(X, a, b, c)

# Visualizziamo i risultati
plt.scatter(X, y, color='blue', label='Real Data')
plt.plot(X, y_pred, color='red', label='Non-linear fitting (Sub-Quadratic)')
plt.xlabel('Nodes')
plt.ylabel('Amount of Data')
plt.legend()
plt.title('Non-linear regression (Polynomial on Nodes)')
plt.show()
