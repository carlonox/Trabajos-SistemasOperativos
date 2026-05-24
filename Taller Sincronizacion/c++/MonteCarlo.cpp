#include <iostream>
#include<thread>
#include<mutex>
#include<vector>
#include<random>
std::mutex mtx;
int puntos_dentro=0;

void calcular(int puntos_a_generar,int id)
{
    std::random_device rd; //generador no determinista
    std::mt19937 gen(rd()+id); //generador pseudodeterminista con semilla definida por un numero aleatorio + id del hilo para evitar repeticiones
    std::uniform_real_distribution<double>dis(-1.0,1.0); //distribucion uniforme entre -1 y 1, crea un objeto clase uniform_real_distribution llamado dis que genera números aleatorios de tipo double en el rango [-1.0, 1.0]

    int local=0;
    for (int i=0; i<puntos_a_generar; i++)
    {
        double x=dis(gen); //genera un número aleatorio para la coordenada x utilizando el generador de números aleatorios y la distribución definida anteriormente
        double y=dis(gen); //genera un número aleatorio para la coordenada y utilizando el generador de números aleatorios y la distribución definida anteriormente

        if (x*x+y*y<=1)  //verifica si el punto generado está dentro del círculo unitario, es decir, si la suma de los cuadrados de las coordenadas x e y es menor o igual a 1
        {
            local++; //incrementa el contador local si el punto está dentro del círculo
        }
    }
    std::lock_guard<std::mutex> lock(mtx); //evita que dos hilos modifiquen mi var global al tiempo haciendo que el otro hilo solo pueda acceder cuando termina el hilo

    puntos_dentro+=local; //suma el contador local al contador global de puntos dentro del círculo
}


int main()
{
    int total_puntos=1000000; //total de puntos a generar
    int num_hilos=4; //número de hilos a utilizar
    
    int por_hilo= total_puntos/num_hilos;
    int resto= total_puntos%num_hilos;

    std::vector<std::thread>hilos; //vector para almacenar los hilos

    for(int i=0; i<num_hilos; i++)
    {
        int tarea= por_hilo;
        if(i== num_hilos-1) //si es el último hilo, asigna el resto de puntos
        {
            tarea+=resto;
        }
        hilos.emplace_back(calcular,tarea,i); //crea un nuevo hilo que ejecuta la función calcular con los argumentos tarea e id del hilo
    }
    for(auto &t : hilos) t.join(); //espera a que todos los hilos terminen su ejecución antes de continuar con el programa principal
    double pi= 4.0*puntos_dentro/total_puntos;
    std::cout<<"Valor aproximado de pi: "<<pi<<std::endl; //imprime el valor aproximado de pi calculado a partir de la proporción de puntos dentro del círculo en relación con el total de puntos generados
    return 0;

}