#include <iostream>
#include <vector>
#include <thread>
#include <random>
#include <cmath>
#include <atomic>
#include <chrono>   // para medición de tiempos
#include <algorithm> // para is_sorted (verificación opcional)
// comando para ejecutar programa:  g++ -std=c++11 -pthread  fork_join_sorting_app.cpp -o a

using namespace std;
long threshold;
atomic<long> counter_quick(0);
atomic<long> counter_merge(0);
random_device rd;  
mt19937 gen(rd());  //semilla no determinista

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

int randomized_partition(vector<int> &lista, int p, int r){
    uniform_int_distribution<> dist(p,r); //intervalo para pivote aleatorizado
    long i = dist(gen);
    swap(lista[r], lista[i]);
    return partition(lista,p,r);
}

//funcion del quicksort
void quicksort(vector<int> &lista, int p, int r){
    if((r-p)<=threshold){ //diferencia de indices que me da 1 menos que la cantidad de elementos. si la diferencia de indices es menor o igual al threshold, entonces no se siguen creando mas hilos, y se ordena con insertion sort   //(r - p + 1)
        insertion_sort(lista,p,r);
    }
    else{
        int q = randomized_partition(lista,p,r); //buscar partición de manera aleatoria
        thread left (quicksort,ref(lista),p,q-1); //este hilo ejecutara quicksort para la parte izquierda del arreglo
        thread right (quicksort,ref(lista),q+1,r); //este hilo ejecutara quicksort para la parte derecha del arreglo
        left.join();
        right.join();
    }
}

//divide el segmento lista[p..r] en dos partes de tamaño aproximadamente igual, ordena cada parte en apralelo usando hilos, y mezcla o hace merge de las dos mitades ya ordenadas para tener un segmento completo ordenado.
void merge_sort(vector<int> &lista, int p,int r){ //entra el vector a ordenar.
    if((r-p)<=threshold){ //si la diferencia de indices es menor o igual al threshhold, entonces no se siguen creando hilos y se ordena el segmento con insertion sort. (r - p + 1)??
        insertion_sort(lista, p, r);
    }
    else{
        int q = (p+r)/2; //calcula el punto medio, por ejemplo p=0, r=7, entonces q=3, entonces el segmento se divide en lista[0..3] y lista[4..7]
        thread left (merge_sort,ref(lista),p,q); //crea dos hilos para ordenar cada mitad del segmento en paralelo. el primer hilo ordenara la parte izquierda del segmento, y el segundo hilo ordenara la parte derecha del segmento.
        thread right (merge_sort,ref(lista),q+1,r);  //ref es mi referencia al vector lista, para que los hilos puedan modificar el mismo vector. el primero modifica la parte izquierda y el segundo hilo ordenara la parte derecha del segmento.
        left.join(); //espera a que ambos hilos terminen.
        right.join();
        merge(lista,p,q,r); //une ambas listas ordenadas.
    }
}

// función verificadora, se encarga de ver que ambas listas sean iguales y que estén en orden.
void verificacion(vector<int> lista, vector<int> lista_2){
    if(!is_sorted(lista.begin(),lista.end()) && is_sorted(lista_2.begin(),lista_2.end())) cout << "lista A no está ordenada.\n";
    else if(!is_sorted(lista_2.begin(),lista_2.end()) && is_sorted(lista.begin(),lista.end())) cout << "lista B no está ordenada\n";
    else if(!is_sorted(lista_2.begin(),lista_2.end()) && !is_sorted(lista.begin(),lista.end())) cout << "listas A y B no están ordenada\n";
    else cout << "listas A y B están ordenadas.";
}

int main(){
    uniform_int_distribution<> dist(1,100000); //define distribucion entre 1 a 100000
    //para valores muy grandes en potencias de dos, la diferencia entre el tamaño del treshold y el size del arreglo debe ser 2^11 ejemplo: size = 2^22 threshold = 2^11.
    int size = (int) pow(2.0,20.0); // 64, cantida de elementos del arreglo.
    threshold = (long) pow(2.0,10.0); // 8, umbral global

    vector<int> lista; //crea dos vectores para probar quicksort primero y merge sort despues.
    vector<int> lista_2;
    for (int i = 0; i < size; ++i) {
        lista.push_back(dist(gen)); //llena la lista con 64 numeros aleatorios.
    }

    lista_2 = lista; //el mismo contenido de la lista 1 lo pone en la lista 2.

    cout << "Verificación #1 ";
    verificacion(lista,lista_2);

    cout<< "size = " << size << "\n"; //imprime el tamaño del arreglo.
    cout<< "threshold = " << threshold << "\n"; // imprime el tamaño del threshold

    cout << "quicksort: \n"; //imprime la lista antes de ordenar.
    // print lista para testeo
    // for(int i: lista) {
    //     cout << i << " ";
    // }
    // cout << "\n";
    auto start = chrono::high_resolution_clock::now();
    quicksort(ref(lista),0,size-1); //llama a quicksort pasando el vector como referencia. Indices 0 y size-1 para ordenar todo el vector.
    auto end = chrono::high_resolution_clock::now();
    auto t_qs = chrono::duration_cast<chrono::microseconds>(end - start).count();
    cout << "Tiempo Quicksort : " << t_qs << " us"<< endl;
    // for(int i: lista) {
    //     cout << i << " ";   
    // }
    // cout << "\n";

    //con mergesort hace lo mismo que con quicksort.
    cout << "mergesort: \n"; 
    cout << "Verificación #2 ";
    verificacion(lista,lista_2);
    // for(int i: lista_2) {
    //     cout << i << " ";   
    // }
    // cout << "\n";
    start = chrono::high_resolution_clock::now();
    merge_sort(ref(lista_2),0,size-1);
    end = chrono::high_resolution_clock::now();
    auto t_ms = chrono::duration_cast<chrono::microseconds>(end - start).count();
    cout << "Tiempo Mergesort : " << t_ms << " us"<< endl;
    cout << "Verificación #3 ";
    verificacion(lista,lista_2);
    // for(int i: lista_2) {
    //     cout << i << " ";   
    // }
    cout << "\n";
    
    return 0;
}