//
// Created by Andres on 22/6/2026.
//

#ifndef FUNCIONES_H
#define FUNCIONES_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>

#define MAX_ZONAS       10
#define MIN_ZONAS        5
#define NUM_DIAS        30
#define NOMBRE_MAX      50
#define ARCHIVO_DATOS   "historico.dat"

#define LIMITE_CO2    1000.0f
#define LIMITE_SO2      40.0f
#define LIMITE_NO2      25.0f
#define LIMITE_PM25     15.0f

typedef struct {
    int dia;
    int mes;
    int anio;
} Fecha;

typedef struct {
    float co2;
    float so2;
    float no2;
    float pm25;
    Fecha fecha;
} Lectura;

typedef struct {
    float temperatura;
    float viento;
    float humedad;
} FactoresClimaticos;

typedef struct {
    char nombre[NOMBRE_MAX];
    Lectura historico[NUM_DIAS];
    int num_lecturas;
    Lectura actual;
    int tiene_actual;
    FactoresClimaticos clima;
} Zona;

void  limpiarBuffer(void);
void  presioneContinuar(void);
Fecha obtenerFechaActual(void);
float leerFloatPositivo(const char *campo);
void  aMayusculas(char *cadena);
int   nombreExiste(Zona zonas[], int num_zonas, const char *nombre_buscar, int indice_ignorar);

void configuracionInicial(Zona zonas[], int *num_zonas);
int  cargarHistorico(Zona zonas[], int *n);
int  guardarHistorico(Zona zonas[], int n);

void gestionarZonas(Zona zonas[], int *num_zonas);
void registrarLectura(Zona zonas[], int n);
void verMonitoreoActual(Zona zonas[], int num_zonas);
void verPromediosHistoricos(Zona zonas[], int num_zonas);
void generarSemillaOculta(Zona zonas[], int num_zonas);
void predecirNiveles(Zona zonas[], int num_zonas);
void verAlertasActivas(Zona zonas[], int num_zonas);
void verRecomendaciones(Zona zonas[], int num_zonas);
void exportarHistorialCSV(Zona zonas[], int num_zonas);
void ejecutarSistema(void);

#endif