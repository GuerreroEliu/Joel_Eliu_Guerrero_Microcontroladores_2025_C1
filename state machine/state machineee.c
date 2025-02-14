#include <stdio.h>
#include <stdlib.h>

// Definición de estados
typedef enum {
    ESTADO_RESET,
    ESTADO_CERRADA,
    ESTADO_ABIERTO,
    ESTADO_PARADA,
    ESTADO_CERRANDO,
    ESTADO_ABRIENDO,
    ESTADO_BLOQUEADA_ERROR,
    ESTADO_DESCONOCIDA,
    NUM_ESTADOS
} EstadoPuerta;


typedef enum {
    EVENTO_ABRIR,
    EVENTO_CERRAR,
    EVENTO_PARAR,
    EVENTO_ERROR,
    EVENTO_RESET,
    EVENTO_FIN_CARRERA,
    EVENTO_FIN_APERTURA,
    NUM_EVENTOS
} EventoPuerta;


typedef struct {
    EstadoPuerta estadoActual;
    int tiempoEnEstado;
    int sensorObstaculo;
} MaquinaEstadosPuerta;

// Funciones de acción para cada estado
void accionReset(MaquinaEstadosPuerta* maquina) {
    printf("Ejecutando reset del sistema\n");
    maquina->tiempoEnEstado = 0;
    maquina->sensorObstaculo = 0;
}

void accionCerrada(MaquinaEstadosPuerta* maquina) {
    printf("Puerta cerrada\n");
}

void accionAbierto(MaquinaEstadosPuerta* maquina) {
    printf("Puerta abierta\n");
}

void accionParada(MaquinaEstadosPuerta* maquina) {
    printf("Puerta detenida\n");
}

void accionCerrando(MaquinaEstadosPuerta* maquina) {
    printf("Cerrando puerta\n");

}

void accionAbriendo(MaquinaEstadosPuerta* maquina) {
    printf("Abriendo puerta\n");

}

void accionBloqueadaError(MaquinaEstadosPuerta* maquina) {
    printf("¡ERROR! Puerta bloqueada\n");
}

void accionDesconocida(MaquinaEstadosPuerta* maquina) {
    printf("Estado desconocido - requiere calibración\n");
}


EstadoPuerta obtenerSiguienteEstado(MaquinaEstadosPuerta* maquina, EventoPuerta evento) {
    switch (maquina->estadoActual) {
        case ESTADO_RESET:
            if (evento == EVENTO_FIN_CARRERA) return ESTADO_CERRADA;
            break;

        case ESTADO_CERRADA:
            if (evento == EVENTO_ABRIR) return ESTADO_ABRIENDO;
            if (evento == EVENTO_ERROR) return ESTADO_BLOQUEADA_ERROR;
            break;

        case ESTADO_ABIERTO:
            if (evento == EVENTO_CERRAR) return ESTADO_CERRANDO;
            if (evento == EVENTO_ERROR) return ESTADO_BLOQUEADA_ERROR;
            break;

        case ESTADO_PARADA:
            if (evento == EVENTO_ABRIR) return ESTADO_ABRIENDO;
            if (evento == EVENTO_CERRAR) return ESTADO_CERRANDO;
            if (evento == EVENTO_ERROR) return ESTADO_BLOQUEADA_ERROR;
            break;

        case ESTADO_CERRANDO:
            if (evento == EVENTO_PARAR) return ESTADO_PARADA;
            if (evento == EVENTO_FIN_CARRERA) return ESTADO_CERRADA;
            if (evento == EVENTO_ERROR) return ESTADO_BLOQUEADA_ERROR;
            break;

        case ESTADO_ABRIENDO:
            if (evento == EVENTO_PARAR) return ESTADO_PARADA;
            if (evento == EVENTO_FIN_APERTURA) return ESTADO_ABIERTO;
            if (evento == EVENTO_ERROR) return ESTADO_BLOQUEADA_ERROR;
            break;

        case ESTADO_BLOQUEADA_ERROR:
            if (evento == EVENTO_RESET) return ESTADO_RESET;
            break;

        case ESTADO_DESCONOCIDA:
            if (evento == EVENTO_RESET) return ESTADO_RESET;
            break;
    }

    return maquina->estadoActual;
}


void inicializarMaquina(MaquinaEstadosPuerta* maquina) {
    maquina->estadoActual = ESTADO_DESCONOCIDA;
    maquina->tiempoEnEstado = 0;
    maquina->sensorObstaculo = 0;
}


void procesarEvento(MaquinaEstadosPuerta* maquina, EventoPuerta evento) {
    EstadoPuerta nuevoEstado = obtenerSiguienteEstado(maquina, evento);

    if (nuevoEstado != maquina->estadoActual) {
        maquina->estadoActual = nuevoEstado;
        maquina->tiempoEnEstado = 0;


        switch (maquina->estadoActual) {
            case ESTADO_RESET:
                accionReset(maquina);
                break;
            case ESTADO_CERRADA:
                accionCerrada(maquina);
                break;
            case ESTADO_ABIERTO:
                accionAbierto(maquina);
                break;
            case ESTADO_PARADA:
                accionParada(maquina);
                break;
            case ESTADO_CERRANDO:
                accionCerrando(maquina);
                break;
            case ESTADO_ABRIENDO:
                accionAbriendo(maquina);
                break;
            case ESTADO_BLOQUEADA_ERROR:
                accionBloqueadaError(maquina);
                break;
            case ESTADO_DESCONOCIDA:
                accionDesconocida(maquina);
                break;
        }
    }
    maquina->tiempoEnEstado++;
}

// Ejemplo de uso
int main() {
    MaquinaEstadosPuerta puerta;
    inicializarMaquina(&puerta);

    // Ejemplo de secuencia de eventos más completa
    printf("Iniciando secuencia de prueba...\n");
    procesarEvento(&puerta, EVENTO_RESET);          // -> RESET
    procesarEvento(&puerta, EVENTO_FIN_CARRERA);    // -> CERRADA
    procesarEvento(&puerta, EVENTO_ABRIR);          // -> ABRIENDO
    procesarEvento(&puerta, EVENTO_FIN_APERTURA);   // -> ABIERTO
    procesarEvento(&puerta, EVENTO_CERRAR);         // -> CERRANDO
    procesarEvento(&puerta, EVENTO_PARAR);          // -> PARADA
    procesarEvento(&puerta, EVENTO_ABRIR);          // -> ABRIENDO
    procesarEvento(&puerta, EVENTO_ERROR);          // -> BLOQUEADA_ERROR

    return 0;
}
