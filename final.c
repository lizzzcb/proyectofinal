#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define NUM_ZONAS 5
#define DIAS_HISTORICOS 30
#define NUM_CONTAMINANTES 4
#define UMBRAL_ALERTA 80.0 // 80% del límite máximo

// Límites OMS (ejemplo)
const float limites_OMS[NUM_CONTAMINANTES] = {9.0, 0.5, 0.2, 25.0}; // CO2, SO2, NO2, PM2.5

typedef struct {
    float temperatura;
    float viento;
    float humedad;
} FactoresClimaticos;

typedef struct {
    char nombre[50];
    float contaminantes_actuales[NUM_CONTAMINANTES]; // [CO2, SO2, NO2, PM2.5]
    float contaminantes_historicos[DIAS_HISTORICOS][NUM_CONTAMINANTES];
    FactoresClimaticos factores_climaticos;
} Zona;

const char* nombres_contaminantes[NUM_CONTAMINANTES] = {"CO2", "SO2", "NO2", "PM2.5"};

// Función para cargar datos históricos desde archivo
void cargarDatos(Zona *zona, const char* filename) {
    FILE *file = fopen(filename, "r");
    if (!file) return;

    for (int i = 0; i < DIAS_HISTORICOS; i++) {
        for (int j = 0; j < NUM_CONTAMINANTES; j++) {
            fscanf(file, "%f", &zona->contaminantes_historicos[i][j]);
        }
    }
    fclose(file);
}

// Función para guardar datos en archivo
void guardarDatos(Zona *zona, const char* filename) {
    FILE *file = fopen(filename, "w");
    if (!file) return;

    for (int i = 0; i < DIAS_HISTORICOS; i++) {
        for (int j = 0; j < NUM_CONTAMINANTES; j++) {
            fprintf(file, "%.2f ", zona->contaminantes_historicos[i][j]);
        }
        fprintf(file, "\n");
    }
    fclose(file);
}

// Predicción por promedio ponderado (50% último día, 30% penúltimo, 20% antepenúltimo)
void predecirContaminacion(Zona *zona, float *prediccion) {
    for (int i = 0; i < NUM_CONTAMINANTES; i++) {
        prediccion[i] = (zona->contaminantes_historicos[DIAS_HISTORICOS-1][i] * 0.5) +
                        (zona->contaminantes_historicos[DIAS_HISTORICOS-2][i] * 0.3) +
                        (zona->contaminantes_historicos[DIAS_HISTORICOS-3][i] * 0.2);
    }
}

// Cálculo de promedio histórico
void promedioHistorico(Zona *zona, float *promedio) {
    for (int i = 0; i < NUM_CONTAMINANTES; i++) {
        promedio[i] = 0.0;
        for (int j = 0; j < DIAS_HISTORICOS; j++) {
            promedio[i] += zona->contaminantes_historicos[j][i];
        }
        promedio[i] /= DIAS_HISTORICOS;
    }
}

// Generación de alertas y recomendaciones
void generarAlertas(Zona *zona, float *prediccion) {
    printf("\n--- ALERTAS Y RECOMENDACIONES [%s] ---\n", zona->nombre);
    
    for (int i = 0; i < NUM_CONTAMINANTES; i++) {
        if (prediccion[i] > limites_OMS[i] * UMBRAL_ALERTA / 100.0) {
            printf("[ALERTA] %s: %.2f (supera %.2f)\n", 
                   nombres_contaminantes[i], prediccion[i], limites_OMS[i]);
            
            if (strcmp(nombres_contaminantes[i], "PM2.5") == 0) {
                printf("  -> Suspender actividades al aire libre\n");
            } else if (strcmp(nombres_contaminantes[i], "CO2") == 0) {
                printf("  -> Reducir tráfico vehicular\n");
            }
        }
    }
}

void exportarReporte(Zona *zona, float *prediccion, float *promedio) {
    char filename[100];
    sprintf(filename, "reporte_%s.txt", zona->nombre);
    
    FILE *file = fopen(filename, "w");
    if (!file) return;

    fprintf(file, "Zona: %s\n", zona->nombre);
    fprintf(file, "Fecha: %s\n", __DATE__);
    
    fprintf(file, "\nDatos Actuales:\n");
    for (int i = 0; i < NUM_CONTAMINANTES; i++) {
        fprintf(file, "- %s: %.2f μg/m³\n", nombres_contaminantes[i], zona->contaminantes_actuales[i]);
    }

    fprintf(file, "\nPredicción 24h:\n");
    for (int i = 0; i < NUM_CONTAMINANTES; i++) {
        fprintf(file, "- %s: %.2f μg/m³\n", nombres_contaminantes[i], prediccion[i]);
    }

    fprintf(file, "\nPromedio Histórico (30 días):\n");
    for (int i = 0; i < NUM_CONTAMINANTES; i++) {
        fprintf(file, "- %s: %.2f μg/m³\n", nombres_contaminantes[i], promedio[i]);
    }

    fclose(file);
    printf("Reporte exportado: %s\n", filename);
}

int main() {
    Zona zonas[NUM_ZONAS] = {
        {"Centro"}, {"Norte"}, {"Sur"}, {"Este"}, {"Oeste"}
    };


    for (int i = 0; i < NUM_ZONAS; i++) {
        char filename[50];
        sprintf(filename, "datos_%s.txt", zonas[i].nombre);
        cargarDatos(&zonas[i], filename);
    }


    for (int i = 0; i < NUM_ZONAS; i++) {
        printf("\n--- DATOS ACTUALES [%s] ---\n", zonas[i].nombre);
        for (int j = 0; j < NUM_CONTAMINANTES; j++) {
            printf("Ingrese nivel de %s: ", nombres_contaminantes[j]);
            scanf("%f", &zonas[i].contaminantes_actuales[j]);
        }
    }


    for (int i = 0; i < NUM_ZONAS; i++) {
        float prediccion[NUM_CONTAMINANTES];
        float promedio[NUM_CONTAMINANTES];
        
        predecirContaminacion(&zonas[i], prediccion);
        promedioHistorico(&zonas[i], promedio);
        generarAlertas(&zonas[i], prediccion);
        exportarReporte(&zonas[i], prediccion, promedio);
    }

    return 0;
}