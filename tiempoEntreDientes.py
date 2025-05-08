import re
import matplotlib.pyplot as plt

def extraer_valores_debug(texto):
    valores = []
    
    for linea in texto.strip().splitlines():
        # Buscar un número entero (puede ser negativo o cero)
        match = re.search(r'\[\d+\]\s+(-?\d+)', linea)
        if match:
            valor = int(match.group(1))
            if valor == 0:
                break  # Detenerse en el primer 0
            valores.append(valor)
    return valores

def graficar_valores(valores):
        # Separar en ciclos (segmentos entre -1)
    ciclos = []
    actual = []

    for valor in valores:
        if valor == -1:
            if actual:
                ciclos.append(actual)
                actual = []
        else:
            actual.append(valor)

    # Si termina sin un -1 final
    if actual:
        ciclos.append(actual)

    # Visualizar
    plt.figure(figsize=(12, 6))
    for i, ciclo in enumerate(ciclos):
        plt.plot(ciclo, label=f'Ciclo {i+1}')

    plt.xlabel('Diente')
    plt.ylabel('Tiempo entre dientes (ms)')
    plt.title('Tiempos entre dientes por ciclo de movimiento')
    plt.legend()
    plt.grid(True)
    plt.tight_layout()
    plt.show()

texto_debug = """

"""

valores_extraidos = extraer_valores_debug(texto_debug)
print(len(valores_extraidos))  # Imprime la cantidad de valores extraídos
