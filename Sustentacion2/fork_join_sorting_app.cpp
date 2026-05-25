#include <iostream>
#include <vector>
#include <thread>
// #include <cstdlib>  //para exit()
#include <random>
#include <cmath>
// #include <atomic>
// comando para ejecutar programa:  g++ -std=c++11 -pthread  fork_join_sorting_app.cpp -o a

using namespace std;
long threshold;
// atomic<long> counter = 0;
int partition(vector<int> &lista, int p, int r ){
    int x = lista[r];
    int i = p - 1;
    for(int j=p;j<r;j++){
        if(lista[j]<=x){
            i+=1;
            int temp = lista[j];
            lista[j] = lista[i];
            lista[i] = temp;
        }
    }
    int temp = lista[r];
    lista[r] = lista[i+1];
    lista[i+1] = temp;
    return i + 1;
}


void merge(vector<int> &lista, int p, int q, int r){
    int nl = q - p;
    int nr = r - q - 1;
    vector<int> l_array;
    vector<int> r_array;
    for(int i=0;i<=nl;i++){l_array.push_back(lista[i+p]);}
    for(int i=0;i<=nl;i++){r_array.push_back(lista[q+i+1]);}
    int i = 0;
    int j = 0;
    int k = p;
    while(i<=nl && j<=nr){
        if(l_array[i]<=r_array[j]){
            lista[k] = l_array[i];
            i++;
        }else{
            lista[k] = r_array[j];
            j++;
        }
        k++;
    }
    while(i<=nl){
        lista[k] = l_array[i];
        i++;
        k++;
    }
    while(j<=nr){
        lista[k] = r_array[j];
        j++;
        k++;
    }
}

void insertion_sort(vector<int> &lista,int p,int r){
    for(int j=p+1;j<=r;j++){
        int key = lista[j];
        int i = j-1;
        while(i>=0 && lista[i]>key){
            lista[i+1] = lista[i];
            i-=1;
            lista[i+1] = key;
        }
    }
}
void quicksort(vector<int> &lista, int p, int r){
    if((r-p)<=threshold){
        insertion_sort(lista,p,r);
    }
    else{
        int q = partition(lista,p,r);
        thread left (quicksort,ref(lista),p,q-1);
        thread right (quicksort,ref(lista),q+1,r);
        left.join();
        right.join();
    }
}

void merge_sort(vector<int> &lista, int p,int r){

    if((r-p)<=threshold){
        insertion_sort(lista, p, r);
    }
    else{
        int q = (p+r)/2;
        thread left (merge_sort,ref(lista),p,q);
        thread right (merge_sort,ref(lista),q+1,r);
        left.join();
        right.join();
        merge(lista,p,q,r);
    }
}

int main(){
    random_device rd;  
    mt19937 gen(rd());  
    uniform_int_distribution<> dist(1,100000);
    //para valores muy grandes en potencias de dos, la diferencia entre el tamaño del treshold y el size del arreglo debe ser 2^11 ejemplo: size = 2^22 threshold = 2^11.
    int size = (int) pow(2.0,6.0);
    threshold = (long) pow(2.0,3.0);
    vector<int> lista;
    vector<int> lista_2;
    for (int i = 0; i < size; ++i) {
        lista.push_back(dist(gen));
    }
    lista_2 = lista;
    cout<< "size = " << size << "\n";
    cout << "quicksort: \n";
    for(int i: lista) {
        cout << i << " ";
    }
    cout << "\n";
    quicksort(ref(lista),0,size-1);
    for(int i: lista) {
        cout << i << " ";
    }
    cout << "\n";
    cout << "mergesort: \n";
    for(int i: lista_2) {
        cout << i << " ";
    }
    cout << "\n";
    merge_sort(ref(lista_2),0,size-1);
    for(int i: lista_2) {
        cout << i << " ";
    }
    cout << "\n";
    return 0;
}