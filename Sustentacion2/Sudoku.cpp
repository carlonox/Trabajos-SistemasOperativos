// sudoku.cpp
// Compilar: g++ -std=c++11 -pthread sudoku.cpp -o sudoku
//Ejecutar Valido: ./sudoku 5 3 4 6 7 8 9 1 2 6 7 2 1 9 5 3 4 8 1 9 8 3 4 2 5 6 7 8 5 9 7 6 1 4 2 3 4 2 6 8 5 3 7 9 1 7 1 3 9 2 4 8 5 6 9 6 1 5 3 7 2 8 4 2 8 7 4 1 9 6 3 5 3 4 5 2 8 6 1 7 9


// Ejecutar Invalido: ./sudoku 1 3 4 6 7 8 9 1 2 6 7 2 1 9 5 3 4 8 1 9 8 3 4 2 5 6 7 8 5 9 7 6 1 4 2 3 4 2 6 8 5 3 7 9 1 7 1 3 9 2 4 8 5 6 9 6 1 5 3 7 2 8 4 2 8 7 4 1 9 6 3 5 3 4 5 2 8 6 1 7 9

#include <iostream>
#include <vector>
#include <thread>
#include <cstdlib> //solo la uso para atoi

// Struct para describir la región a validar, usada tambien en el libro
struct Region {
    int tipo;       // 0 = fila, 1 = columna, 2 = subcuadrícula 3x3
    int idx;        // índice de fila/columna (0-8) cuando tipo 0 o 1
    int inicioFila; // fila inicial de la subcuadrícula (0,3,6)
    int inicioCol;  // columna inicial de la subcuadrícula (0,3,6)
};

// Función que valida una región específica del tablero
bool validarRegion(const std::vector<std::vector<int>>& tablero, const Region& r) { //En los parametros defino mi matriz de 2 dimensiones(vector de vectores), y asigno r como identificador de mi struct Region, estos dos seran los parametros de entrada
    bool visto[10] = {false}; // marcador de dígitos 1..9, no uso visto[0]
    if (r.tipo == 0) { // validar fila
        for (int col = 0; col < 9; ++col) { //Escanea desde la columna 0 a la 9
            int val = tablero[r.idx][col]; //escanea cada valor de la fila mientras se mueve en columnas.
            if (val < 1 || val > 9 || visto[val]) //Verifica si ya se vio ese numero
                return false; 
            visto[val] = true; //si se vio ya ese numero asigna el valor de true a visto.
        }
    } else if (r.tipo == 1) { // validar columna
        for (int fila = 0; fila < 9; ++fila) {
            int val = tablero[fila][r.idx]; //Lo mismo que el de filas pero escanea desde la fila 0 a la 9.
            if (val < 1 || val > 9 || visto[val])
                return false;
            visto[val] = true;
        }
    } else { // validar subcuadrícula 3x3 
        for (int i = 0; i < 3; ++i) { //Uso coordenadas i,j para escanear las posiciones de la cuadricula, despues de mirar todos los i en el j, incrementa el j en 1, y asi hasta escanear toda la cuadricula.
            for (int j = 0; j < 3; ++j) {
                int val = tablero[r.inicioFila + i][r.inicioCol + j];
                if (val < 1 || val > 9 || visto[val])
                    return false;
                visto[val] = true;
            }
        }
    }
    return true;
}



int main(int argc, char* argv[]) {
    // Argument count para verificar que se hayan pasado exactamente 81 números en la entrada del usuario
    if (argc != 82) { // 1 (programa) + 81 números, argc es el contador de argumentos, sumo 1 porque argv cuenta como argumento.
        std::cerr << "Uso: " << argv[0] << " [81 números del 1 al 9]\n"; //si no pasa 81 hay error
        return 1;
    }

    // Construir el tablero 9x9 desde los argumentos
    std::vector<std::vector<int>> tablero(9, std::vector<int>(9)); //Creo matriz 9x9, con un vector de vectores para que sea bidimensional, crea un vector de tamaño 9, en el que cada uno de sus valores es un vector tamaño 9
    int idxArg = 1; //Indice para recorrer argumentos, comienzo en 1 porque argv[0] es el nombre del programa
    for (int i = 0; i < 9; ++i) { //Se recorren las celdas en orden fila , columna
        for (int j = 0; j < 9; ++j) {
            int val = std::atoi(argv[idxArg++]); //convierte char a int
            if (val < 1 || val > 9) {
                std::cerr << "Error: todos los valores deben estar entre 1 y 9.\n";
                return 1;
            }
            tablero[i][j] = val;
        }
    }

    // Total de hilos: 1 (filas) + 1 (columnas) + 9 (subcuadrículas) = 11
    const int NUM_HILOS = 11;
    std::vector<bool> resultados(NUM_HILOS, false); //creo un vector de resultados, con tamaño como el numero de hilos, inicializado con false.
    std::vector<std::thread> hilos; //Creo un vector para almacenar mis hilos.

    // Hilo 0: valida todas las filas (recorre 0..8, si alguna falla, inválido)
    Region rFilas;
    rFilas.tipo = 0;   // indica que es una fila
    // No necesitamos idx (Porque fila e idx serian lo mismo), recorreremos todas dentro del hilo
    hilos.emplace_back([&]() {
        bool ok = true;
        for (int f = 0; f < 9; ++f) {
            Region r;
            r.tipo = 0;
            r.idx = f;
            if (!validarRegion(tablero, r)) {
                ok = false;
                break;
            }
        }
        resultados[0] = ok;
    });

    // Hilo 1: valida todas las columnas
    hilos.emplace_back([&]() { //Lambda es el codigo que se ejecutara en el hilo, 
        bool ok = true;
        for (int c = 0; c < 9; ++c) {
            Region r;
            r.tipo = 1;
            r.idx = c;
            if (!validarRegion(tablero, r)) {
                ok = false;
                break;
            }
        }
        resultados[1] = ok;
    });

    // Hilos 2 al 10: cada uno valida una subcuadrícula 3x3
    int indiceSub = 2;
    for (int sr = 0; sr < 3; ++sr) {          // sr: bloque de filas 0,3,6
        for (int sc = 0; sc < 3; ++sc) {      // sc: bloque de columnas 0,3,6
            Region r;
            r.tipo = 2;
            r.inicioFila = sr * 3;
            r.inicioCol = sc * 3;
            // Capturamos por valor las variables que necesitamos
            hilos.emplace_back([&, r, indiceSub]() {
                resultados[indiceSub] = validarRegion(tablero, r);
            });
            ++indiceSub;
        }
    }

    //Esperar a que terminen todos los hilos
    for (auto& t : hilos) {
        t.join();
    }
    //No hago mutex porque cada hilo trabaja en variables diferentes de mi vector validar region
    // Verificar los resultados
    bool valido = true;
    for (bool res : resultados) {
        if (!res) {
            valido = false;
            break;
        }
    }

    std::cout << (valido ? "La solución es VÁLIDA" : "La solución es INVÁLIDA") << std::endl;

    return 0;
}