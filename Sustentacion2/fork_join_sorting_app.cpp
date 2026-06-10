#include <iostream>
#include <vector>
#include <random>     // libreria para obtener números aleatorios
#include <cmath>      // libreria de funciones matemáticas
#include <chrono>     // para medición de tiempos
#include <algorithm>  // para is_sorted (verificación opcional)
#include <omp.h>      // IMPORTANTE para utilizar librería openMP verificar que sí esté instalada en la máquina plz.

// IMPORTANTE 2 comando para ejecutar programa:  g++ -std=c++17 -fopenmp fork_join_sorting_app.cpp -o (nombre que le quiera poner)

/*
COMANDOS LINUX:
comando para revisar el número de procesadores de máquina: nproc
comando para revisar el max de hilos posibles en el SO cat /proc/sys/kernel/threads-max
*/


using namespace std;
long threshold; //threshold indica el límite inferior en merge y quick, evita llamadas recursivas en subarreglos pequeños y  ejecuta un insertion sort.
random_device rd;   // generador no determista 
mt19937 gen(rd());  // se utiliza como semilla el mersenne twister. Un generador de número pseudoaleatorios.

//organiza todos los menores al pivote a la izquierda y los mayores a la derecha, devuelve la posicion del pivote
int partition(vector<int> &lista, int p, int r ){
    int x = lista[r]; //1. pivote
    int i = p - 1; //2. frontera izquierda 
    for(int j=p;j<r;j++){ //3. recorrer desde p hasta r-1
        if(lista[j]<=x){ //4. si el elemento es menor o igual al pivote
            i+=1;    //5. Ampliar zona izquierda
            //intercambio lista [i] con lista[j]
            int temp = lista[j];
            lista[j] = lista[i];
            lista[i] = temp;
        }
    }
    // 6. coloco el pivote en su posicion final
    int temp = lista[r];
    lista[r] = lista[i+1];
    lista[i+1] = temp;
    return i + 1; //devuelvo la posicion del pivote
}

//mezcla dos subarreglos contiguos orgenados lista[p..q] y lista[q+1..r] en un solo subarreglo ordenado lista[p..r]
void merge(vector<int>& arr, int left, int mid, int right) {
    int n1 = mid - left + 1;      // número de elementos izquierda
    int n2 = right - mid;         // número de elementos derecha
    vector<int> L(n1), R(n2);     // instanciamos dos arreglos de tamaño n1 y n2
    for (int i = 0; i < n1; ++i)
        L[i] = arr[left + i];     // llenamos arreglo L con los elementos de arr[left..n1]
    for (int j = 0; j < n2; ++j)
        R[j] = arr[mid + 1 + j];  // llenamos el arreglo R con los elementos de arr[mid + 1.. n2]
    int i = 0, j = 0, k = left;
    while (i < n1 && j < n2) {
        if (L[i] <= R[j])
            arr[k++] = L[i++];
        else
            arr[k++] = R[j++];
    }
    while (i < n1) arr[k++] = L[i++];
    while (j < n2) arr[k++] = R[j++];
}

//ordena el segmento lista[p..r] usando el algoritmo de inserción, se usa cuando el tamaño del segmento es menor o igual al umbral (threshold), evitando crear hilos para problemas muy pequeños.
void insertion_sort(vector<int> &lista,int p,int r){
    for(int j=p+1;j<=r;j++){
        int key = lista[j]; //elemento a insertar
        int i = j-1;
        while(i>=p && lista[i]>key){ //cambiar a i >= p? Rta: SÍ :v , porque el segmento a ordenar es lista[p..r], entonces el indice i no debe ser menor que p, sino que debe ser mayor o igual a p para evitar acceder a elementos fuera del segmento.
            lista[i+1] = lista[i]; //desplazo a la derecha
            i-=1;
            lista[i+1] = key; //asigno key dentro del bucle
        }
    }
}

// escoger un pivote aleatorio, para que quicksort sea un algoritmo aleatorizado y logre un O(n log(n))
int randomized_partition(vector<int> &lista, int p, int r){
    uniform_int_distribution<> dist(p,r); //intervalo para pivote aleatorizado
    long i = dist(gen);
    swap(lista[r], lista[i]);
    return partition(lista,p,r);
}

//función quick sort
void quick_sort(vector<int> &lista, int p, int r){
    if((r-p+1)<=threshold){ //diferencia de indices que me da 1 menos que la cantidad de elementos. si la diferencia de indices es menor o igual al threshold, entonces no se siguen creando mas hilos, y se ordena con insertion sort   //(r - p + 1)
        insertion_sort(lista,p,r);
    }
    else{
        int q = randomized_partition(lista,p,r);
        #pragma omp task shared(lista)
        quick_sort(ref(lista),p,q-1);  
        #pragma omp task shared(lista)
        quick_sort(ref(lista),q+1,r);
        // #pragma omp taskwait 
    }
}

//función merge sort
void merge_sort(vector<int> &lista, int p,int r){ //entra el vector a ordenar.
    if((r-p+1)<=threshold){
        insertion_sort(lista, p, r);
    }
    else{
        int q = (p+r)/2;
        // creación de las tareas a realizar por los hilos, utilizar la clausula shared para que tengan acceso las tareas
        // a la lista a ordenar, sin ello queda por default privada y los cambios no se guardarán.
        #pragma omp task shared(lista)
        merge_sort(ref(lista),p,q);
        #pragma omp task shared(lista)
        merge_sort(ref(lista),q+1,r);
        // condicional para evitar condiciones de carrera, indica que se debe esperar a que las tareas (linea 108 y 111)
        // terminen antes de poder ejecutar el algoritmo de merge.
        #pragma omp taskwait
        merge(lista,p,q,r); 
    }
}
// iniciar paralelismo de merge sort
void start_merge_parallel(vector<int> &lista, int p,int r){
    //OpenMP utiliza el modelo fork-join para ralizar tareas de paralelización por medio de multihilos.
    //se crea una región paralela sobre un hilo maestro, se designa que el máximo de hilos producidos serán 4 Fork.
    #pragma omp parallel num_threads(4)
    {
    /*
    por medio del constructor single, se escoge uno de los hilos creados y tendrá la tarea de crear la piscina de tareas
    al para ejecutar y los otros hilos quedarán esperando que finalice al final de la llave (linea 127). Los hilos irán ejecutando
    las tareas creadas de manera paralela.
    */
    #pragma omp single
    {
    merge_sort(ref(lista),p,r);
    }
    }
    //al finalizar las bifurcaciones vuelven al hilo original al terminar la tarea (join)
}
// iniciar paralelismo de quick sort
void start_quick_parallel(vector<int> &lista, int p, int r){
    #pragma omp parallel num_threads(4)
    {
    #pragma omp single
    {
    quick_sort(ref(lista),p,r);
    }
    }
}

// función verificadora, se encarga de ver que ambas listas sean iguales y que estén en orden.
void verificacion(vector<int> lista, vector<int> lista_2){
    if(!is_sorted(lista.begin(),lista.end()) && is_sorted(lista_2.begin(),lista_2.end())) cout << "lista A no está ordenadas.\n";
    else if(!is_sorted(lista_2.begin(),lista_2.end()) && is_sorted(lista.begin(),lista.end())) cout << "lista B no está ordenada\n";
    else if(!is_sorted(lista_2.begin(),lista_2.end()) && !is_sorted(lista.begin(),lista.end())) cout << "listas A y B no están ordenada\n";
    else cout << "listas A y B están ordenadas.\n";
}
//imprimir lista, pruebas para listas pequeñas.
void print_lista(vector<int> lista){
    for(int i: lista) {
        cout << i << " ";   
    }
    cout << "\n";
}

int main(){
    uniform_int_distribution<> dist(1,100000); //define distribucion entre 1 a 100000
    //para valores muy grandes en potencias de dos, si se desea un tamaño específico descomentar intercambiar por las variables de abajo.
    int size = (int) pow(2.0,18.0); // indica 2^n elementos de lista
    // threshold = (long) pow(2.0,7.0); // indica 2^n elementos de threshold
    // int size = 1000000;
    threshold = 100; // probar 0
    if(threshold<1){
        cout << "threshold debe ser mayor a 1.\n";
        return 0;
    }
    vector<int> lista; //crea dos vectores para probar quick_sort primero y merge sort despues.
    vector<int> lista_2;
    for (int i = 0; i < size; ++i) { //lista aleatoria
        lista.push_back(dist(gen)); //llena la lista con numeros aleatorios.
    }
    // for(int i=0;i<size;i++) lista.push_back(i); //lista ordenada
    //for(int i=size;i>0;i--) lista.push_back(i); // lista inversa

    lista_2 = lista; //el mismo contenido de la lista 1 lo pone en la lista 2.

    cout << "Verificación #1 "; //verificar que lista y lista_2 no están ordenadas
    verificacion(lista,lista_2);

    cout<< "size = " << size << "\n"; //imprime el tamaño del arreglo.
    cout<< "threshold = " << threshold << "\n"; // imprime el tamaño del threshold

    cout << "\nquicksort: \n";
    // print_lista(lista); // si se desea ver la lista descomentar esta línea
    auto start = chrono::high_resolution_clock::now(); // medir tiempo de ejecución
    start_quick_parallel(ref(lista),0,size-1); //llama a quicksort pasando el vector como referencia. Indices 0 y size-1 para ordenar todo el vector.
    auto end = chrono::high_resolution_clock::now();
    auto t_qs = chrono::duration_cast<chrono::microseconds>(end - start).count();
    cout << "Tiempo Quicksort: " << t_qs << " us"<< endl;
    // print_lista(lista);

    //MergeSort, se realiza lo mismo que con QuickSort
    cout << "\nVerificación #2 "; // verificar que lista está ordenada y lista_2 no
    verificacion(lista,lista_2);

    cout << "\nmergesort: \n"; 
    // print_lista(lista_2); // si se desea imprimir las listas para verificar que sí están ordenadas.
    start = chrono::high_resolution_clock::now();
    start_merge_parallel(ref(lista_2),0,size-1);
    end = chrono::high_resolution_clock::now();
    auto t_ms = chrono::duration_cast<chrono::microseconds>(end - start).count();
    cout << "Tiempo Mergesort : " << t_ms << " us"<< endl;
    // print_lista(lista_2);

    cout << "\nVerificación #3 ";//última verificación que ambas listas han sido ordenadas satisfactoriamente.
    verificacion(lista,lista_2);
    cout << "\n";

    //fin
    return 0;
}