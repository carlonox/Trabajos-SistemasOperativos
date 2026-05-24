package main //Le indica a el compilador que main() es mi funcion de entrada

import (
    "fmt" //Librerias de formateo y salida
    "math/rand" //Libreria de generacion de numeros aleatorios
    "sync" //Libreria de sincronizacion,(mutex,waitgroup)
    "time" //Tiempo y semillas
)

func main() {
    totalPuntos := 1_000_000
    numGoroutines := 4
    puntosPorGo := totalPuntos / numGoroutines
    resto := totalPuntos % numGoroutines

    var contador int64 //Variable global
    var mu sync.Mutex //Funcion mutex para lock
    var wg sync.WaitGroup //Contador de goroutines activas, similar al join()

    for i := 0; i < numGoroutines; i++ {
        wg.Add(1) //Incrementa mi contador de goroutines en 1
        puntos := puntosPorGo
        if i == numGoroutines-1 { //Define que el ultimo hilo tambien genera los puntos restantes
            puntos += resto
        }
        go func(id int, cantidad int) { //funcion sin nombre, se ejecuta como goroutine al usar go al principio.
            defer wg.Done()
            r := rand.New(rand.NewSource(time.Now().UnixNano() + int64(id))) //Generador de semillas, nanosegundos actuales(generador no determinista) + la id de cada goroutine, para evitar que dos hilos hagan lo mismo.
            local := 0
            for j := 0; j < cantidad; j++ {
                //Normalizo, la funcion Float64() recibe el numero generado y devuelve un numero en [0.0, 1.0), lo multiplico por 2 y me da [0.0, 2.0) y lo resto por 1 y me da [-1.0, 1.0) que es el rango que quiero para mis coordenadas,(equivalente al uniform_real_distribution de c++)
                x := r.Float64()*2 - 1   
                y := r.Float64()*2 - 1

                if x*x+y*y <= 1.0 { //Hago la formula de la circunferencia y comparo distancia con radio maximo para ver que punto esta dentro y fuera de la circunferencia.
                    local++ //Si esta dentro añado 1 en mi variable local
                }
            }
            mu.Lock() //Mutex para prevenir que otro hilo cambie mi variable global mientras este hilo la modifica.
            contador += int64(local)
            mu.Unlock()
        }(i, puntos) //Parametros que se pasan a la funcion anterior
    }

    wg.Wait()
    pi := 4.0 * float64(contador) / float64(totalPuntos) //Estimo pi con la formula dada.
    fmt.Printf("Estimación de π = %f\n", pi)
}