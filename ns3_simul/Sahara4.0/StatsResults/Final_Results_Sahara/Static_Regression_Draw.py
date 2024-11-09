import numpy as np
import matplotlib.pyplot as plt
from scipy import stats, optimize

# Dati forniti
x = np.array([30, 50, 100, 150, 200])
P = np.array([14.83, 14.96, 14.83, 15.69, 15.805])
B = np.array([11.67, 16.04, 18.26, 34.60, 56.95])

# Regressione lineare per P
slope_P, intercept_P, r_value_P, p_value_P, std_err_P = stats.linregress(x, P)

# Regressione polinomiale per B
coeffs_B = np.polyfit(x, B, 2)
poly_B = np.poly1d(coeffs_B)

# Regressione esponenziale
def exp_func(x, a, b, c):
    return a * np.exp(b * x) + c

# Fit esponenziale per B
popt_B, _ = optimize.curve_fit(exp_func, x, B, p0=[1, 0.01, 1])

# Funzioni di previsione
def predict_P(x):
    return slope_P * x + intercept_P

def predict_B_poly(x):
    return poly_B(x)

def predict_B_exp(x):
    return exp_func(x, *popt_B)

# Previsioni per x = 500 e x = 1000
x_pred = np.array([500, 1000])
P_pred = predict_P(x_pred)
B_pred_poly = predict_B_poly(x_pred)
B_pred_exp = predict_B_exp(x_pred)

print(f"Previsioni per P: {P_pred}")
print(f"Previsioni per B (polinomiale): {B_pred_poly}")
print(f"Previsioni per B (esponenziale): {B_pred_exp}")

# Visualizzazione
plt.figure(figsize=(18, 6))

# Grafico per P
plt.subplot(1, 3, 1)
plt.scatter(x, P, color='blue', label='Dati P')
plt.plot(x, predict_P(x), color='red', label='Regressione lineare P')
plt.xlabel('x')
plt.ylabel('P')
plt.title('Regressione lineare per P')
plt.legend()

# Grafico per B (polinomiale)
plt.subplot(1, 3, 2)
plt.scatter(x, B, color='green', label='Dati B')
plt.plot(x, predict_B_poly(x), color='red', label='Regressione polinomiale B')
plt.xlabel('x')
plt.ylabel('B')
plt.title('Regressione polinomiale per B')
plt.legend()

# Grafico per B (esponenziale)
plt.subplot(1, 3, 3)
plt.scatter(x, B, color='green', label='Dati B')
plt.plot(x, predict_B_exp(x), color='purple', label='Regressione esponenziale B')
plt.xlabel('x')
plt.ylabel('B')
plt.title('Regressione esponenziale per B')
plt.legend()

plt.tight_layout()
plt.show()

# Calcolo R-squared
r_squared_P = r_value_P**2
r_squared_B_poly = 1 - (sum((B - poly_B(x))**2) / ((len(B) - 1) * np.var(B, ddof=1)))
r_squared_B_exp = 1 - (sum((B - predict_B_exp(x))**2) / ((len(B) - 1) * np.var(B, ddof=1)))

print(f"R-squared per P: {r_squared_P}")
print(f"R-squared per B (polinomiale): {r_squared_B_poly}")
print(f"R-squared per B (esponenziale): {r_squared_B_exp}")