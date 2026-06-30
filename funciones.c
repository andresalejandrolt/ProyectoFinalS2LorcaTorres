//
// Created by Andres on 22/6/2026.
//

#include "funciones.h"

static int calcularSubIndice(float c, float bp_lo, float bp_hi, int i_lo, int i_hi) {
    return (int)(((float)(i_hi - i_lo) / (bp_hi - bp_lo)) * (c - bp_lo) + i_lo + 0.5f);
}

static int calcularIQCA_Global(float pm25, float pm10, float o3, float co, float so2, float no2) {
    int max_iqca = 0, iq = 0;

    if(pm25 <= 12.0f) iq = calcularSubIndice(pm25, 0.0f, 12.0f, 0, 50);
    else if(pm25 <= 35.4f) iq = calcularSubIndice(pm25, 12.1f, 35.4f, 51, 100);
    else if(pm25 <= 55.4f) iq = calcularSubIndice(pm25, 35.5f, 55.4f, 101, 200);
    else iq = calcularSubIndice(pm25, 55.5f, 150.4f, 201, 300);
    if(iq > max_iqca) max_iqca = iq;

    iq = (int)(pm10 * (100.0f / 154.0f)); if(iq > max_iqca) max_iqca = iq;
    iq = (int)(co * (100.0f / 9.4f)); if(iq > max_iqca) max_iqca = iq;
    iq = (int)(so2 * (100.0f / 75.0f)); if(iq > max_iqca) max_iqca = iq;
    iq = (int)(no2 * (100.0f / 100.0f)); if(iq > max_iqca) max_iqca = iq;
    iq = (int)(o3 * (100.0f / 70.0f)); if(iq > max_iqca) max_iqca = iq;

    return max_iqca;
}

static int determinarNivelRiesgo(int iqca) {
    if (iqca <= 50) return 1;
    if (iqca <= 100) return 2;
    if (iqca <= 200) return 3;
    if (iqca <= 300) return 4;
    if (iqca <= 400) return 5;
    return 6;
}

static const char* obtenerNombreNivel(int nivel) {
    switch(nivel) {
        case 1: return "DESEABLE (Verde)";
        case 2: return "ACEPTABLE (Amarillo)";
        case 3: return "PRECAUCION (Naranja)";
        case 4: return "ALERTA (Rojo)";
        case 5: return "ALARMA (Morado)";
        case 6: return "EMERGENCIA (Marron)";
        default: return "DESCONOCIDO";
    }
}

void alterarDatosExposicion(Zona zonas[], int num_zonas) {
    float factor;
    printf("\n==================================================\n");
    printf("    [MODO EXPOSICION] ALTERACION DE DATOS         \n");
    printf("==================================================\n");
    printf("Ingrese multiplicador (ej. 2.0 para duplicar, 0.5 para reducir): ");

    if (scanf("%f", &factor) != 1) {
        limpiarBuffer();
        return;
    }
    limpiarBuffer();

    long max_fecha_val = 0;
    for (int i = 0; i < num_zonas; i++) {
        if (zonas[i].tiene_actual) {
            long val = zonas[i].actual.fecha.anio * 10000 +
                       zonas[i].actual.fecha.mes * 100 +
                       zonas[i].actual.fecha.dia;
            if (val > max_fecha_val) {
                max_fecha_val = val;
            }
        }
    }

    int alterados = 0;
    for (int i = 0; i < num_zonas; i++) {
        if (!zonas[i].tiene_actual) continue;

        long val = zonas[i].actual.fecha.anio * 10000 +
                   zonas[i].actual.fecha.mes * 100 +
                   zonas[i].actual.fecha.dia;

        if (val == max_fecha_val) {
            zonas[i].actual.pm25 *= factor;
            zonas[i].actual.pm10 *= factor;
            zonas[i].actual.o3 *= factor;
            zonas[i].actual.co *= factor;
            zonas[i].actual.so2 *= factor;
            zonas[i].actual.no2 *= factor;

            zonas[i].actual.iqca = calcularIQCA_Global(
                zonas[i].actual.pm25, zonas[i].actual.pm10, zonas[i].actual.o3,
                zonas[i].actual.co, zonas[i].actual.so2, zonas[i].actual.no2);

            zonas[i].actual.nivel_riesgo = determinarNivelRiesgo(zonas[i].actual.iqca);

            for (int j = zonas[i].num_lecturas - 1; j >= 0; j--) {
                if (zonas[i].historico[j].fecha.dia == zonas[i].actual.fecha.dia &&
                    zonas[i].historico[j].fecha.mes == zonas[i].actual.fecha.mes &&
                    zonas[i].historico[j].fecha.anio == zonas[i].actual.fecha.anio) {

                    zonas[i].historico[j] = zonas[i].actual;
                    break;
                }
            }
            alterados++;
        }
    }
    printf("\n[INFO] %d zonas alteradas exitosamente. El publico esta en peligro/a salvo.\n", alterados);
}

void limpiarBuffer(void) {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

void presioneContinuar(void) {
    printf("\nPresione ENTER para continuar...");
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}


Fecha obtenerFechaActual(void) {
    time_t t = time(NULL);
    struct tm *tm_info = localtime(&t);
    Fecha f;
    f.dia  = tm_info->tm_mday;
    f.mes  = tm_info->tm_mon + 1;
    f.anio = tm_info->tm_year + 1900;
    return f;
}

float leerFloatPositivo(const char *campo) {
    float valor;
    int valido = 0;
    while (!valido) {
        printf("  %s: ", campo);
        if (scanf("%f", &valor) == 1 && valor >= 0.0f) {
            valido = 1;
            limpiarBuffer();
        } else {
            printf("  Valor invalido. Ingrese un numero positivo.\n");
            limpiarBuffer();
        }
    }
    return valor;
}

void aMayusculas(char *cadena) {
    for (int i = 0; cadena[i]; i++) {
        cadena[i] = toupper((unsigned char) cadena[i]);
    }
}

int nombreExiste(Zona zonas[], int num_zonas, const char *nombre_buscar, int indice_ignorar) {
    for (int i = 0; i < num_zonas; i++) {
        if (i != indice_ignorar && strcmp(zonas[i].nombre, nombre_buscar) == 0) {
            return 1;
        }
    }
    return 0;
}

void configuracionInicial(Zona zonas[], int *num_zonas) {
    *num_zonas = MIN_ZONAS;
    char temp_nombre[NOMBRE_MAX];

    printf("\n==================================================\n");
    printf("  CONFIGURACION INICIAL - SISTEMA DE CONTAMINACION\n");
    printf("==================================================\n");
    printf("Bienvenido. Para iniciar, debe registrar las %d zonas base.\n\n", MIN_ZONAS);

    for (int i = 0; i < MIN_ZONAS; i++) {
        int nombre_valido = 0;
        while (!nombre_valido) {
            printf("Ingrese nombre para la Zona %d: ", i + 1);
            fgets(temp_nombre, NOMBRE_MAX, stdin);
            temp_nombre[strcspn(temp_nombre, "\n")] = '\0';

            if (strlen(temp_nombre) == 0) continue;

            aMayusculas(temp_nombre);

            if (nombreExiste(zonas, i, temp_nombre, -1)) {
                printf("  Error: El nombre '%s' ya esta en uso. Intente con otro.\n", temp_nombre);
            } else {
                strncpy(zonas[i].nombre, temp_nombre, NOMBRE_MAX - 1);
                zonas[i].nombre[NOMBRE_MAX - 1] = '\0';
                zonas[i].num_lecturas = 0;
                zonas[i].tiene_actual = 0;
                nombre_valido = 1;
            }
        }
    }
    printf("\nZonas base configuradas exitosamente.\n");
    presioneContinuar();
}

int cargarHistorico(Zona zonas[], int *n) {
    FILE *archivo = fopen(ARCHIVO_DATOS, "rb");
    if (archivo == NULL) return 0;

    if (fread(n, sizeof(int), 1, archivo) != 1 || *n < MIN_ZONAS || *n > MAX_ZONAS) {
        fclose(archivo);
        return 0;
    }

    if (fread(zonas, sizeof(Zona), *n, archivo) != (size_t) *n) {
        fclose(archivo);
        return 0;
    }

    fclose(archivo);
    return 1;
}

int guardarHistorico(Zona zonas[], int n) {
    FILE *archivo = fopen(ARCHIVO_DATOS, "wb");
    if (archivo == NULL) return 0;

    fwrite(&n, sizeof(int), 1, archivo);
    fwrite(zonas, sizeof(Zona), n, archivo);
    fclose(archivo);
    return 1;
}

void gestionarZonas(Zona zonas[], int *num_zonas) {
    int opcion, idx;
    char temp_nombre[NOMBRE_MAX];

    do {
        printf("\n--- GESTION DE ZONAS ---\n");
        printf("  1. Agregar nueva zona (Actual: %d/%d)\n", *num_zonas, MAX_ZONAS);
        printf("  2. Editar nombre de zona\n");
        printf("  3. Eliminar zona (Minimo: %d)\n", MIN_ZONAS);
        printf("  4. Volver al menu principal\n");
        printf("  Opcion: ");

        if (scanf("%d", &opcion) != 1) opcion = -1;
        limpiarBuffer();

        switch (opcion) {
            case 1:
                if (*num_zonas >= MAX_ZONAS) {
                    printf("\nLimite maximo de zonas alcanzado (%d).\n", MAX_ZONAS);
                } else {
                    printf("\nIngrese nombre de la nueva zona: ");
                    fgets(temp_nombre, NOMBRE_MAX, stdin);
                    temp_nombre[strcspn(temp_nombre, "\n")] = '\0';
                    aMayusculas(temp_nombre);

                    if (strlen(temp_nombre) > 0 && !nombreExiste(zonas, *num_zonas, temp_nombre, -1)) {
                        strncpy(zonas[*num_zonas].nombre, temp_nombre, NOMBRE_MAX - 1);
                        zonas[*num_zonas].nombre[NOMBRE_MAX - 1] = '\0';
                        zonas[*num_zonas].num_lecturas = 0;
                        zonas[*num_zonas].tiene_actual = 0;
                        (*num_zonas)++;
                        printf("Zona '%s' agregada con exito.\n", temp_nombre);
                    } else {
                        printf("Nombre invalido o ya existe.\n");
                    }
                }
                break;
            case 2:
                for (int i = 0; i < *num_zonas; i++) printf("  %d. %s\n", i + 1, zonas[i].nombre);
                printf("\nSeleccione zona a editar (1-%d): ", *num_zonas);
                if (scanf("%d", &idx) == 1 && idx >= 1 && idx <= *num_zonas) {
                    limpiarBuffer();
                    idx--;
                    printf("Nuevo nombre para '%s': ", zonas[idx].nombre);
                    fgets(temp_nombre, NOMBRE_MAX, stdin);
                    temp_nombre[strcspn(temp_nombre, "\n")] = '\0';
                    aMayusculas(temp_nombre);

                    if (strlen(temp_nombre) > 0 && !nombreExiste(zonas, *num_zonas, temp_nombre, idx)) {
                        strncpy(zonas[idx].nombre, temp_nombre, NOMBRE_MAX - 1);
                        zonas[idx].nombre[NOMBRE_MAX - 1] = '\0';
                        printf("Nombre actualizado exitosamente.\n");
                    } else {
                        printf("Nombre invalido o ya existe.\n");
                    }
                } else {
                    printf("Seleccion invalida.\n");
                    limpiarBuffer();
                }
                break;
            case 3:
                if (*num_zonas <= MIN_ZONAS) {
                    printf("\nNo se pueden eliminar zonas. El sistema requiere un minimo de %d.\n", MIN_ZONAS);
                } else {
                    for (int i = 0; i < *num_zonas; i++) printf("  %d. %s\n", i + 1, zonas[i].nombre);
                    printf("\nSeleccione zona a ELIMINAR permanentemente (1-%d): ", *num_zonas);
                    if (scanf("%d", &idx) == 1 && idx >= 1 && idx <= *num_zonas) {
                        idx--;
                        printf("Eliminando zona '%s' y todo su historial...\n", zonas[idx].nombre);
                        for (int i = idx; i < *num_zonas - 1; i++) {
                            zonas[i] = zonas[i + 1];
                        }
                        (*num_zonas)--;
                        printf("Zona eliminada correctamente.\n");
                    }
                    limpiarBuffer();
                }
                break;
            case 4:
                break;
            default:
                printf("\nOpcion no valida.\n");
        }
    } while (opcion != 4);
}

void registrarLectura(Zona zonas[], int n) {
    int idx;
    printf("\n--- REGISTRAR LECTURA ACTUAL ---\n");
    for (int i = 0; i < n; i++) printf("  %d. %s\n", i + 1, zonas[i].nombre);

    printf("Seleccione zona (1-%d): ", n);
    if (scanf("%d", &idx) != 1 || idx < 1 || idx > n) {
        printf("Zona no valida.\n");
        limpiarBuffer();
        return;
    }
    limpiarBuffer();
    idx--;

    Zona *zona = &zonas[idx];
    Lectura nueva;
    FactoresClimaticos clima_nuevo;

    printf("\nNiveles de contaminacion para %s\n", zona->nombre);
    nueva.pm25 = leerFloatPositivo("PM2.5 (ug/m3)");
    nueva.pm10 = leerFloatPositivo("PM10  (ug/m3)");
    nueva.o3   = leerFloatPositivo("O3    (ug/m3)");
    nueva.co   = leerFloatPositivo("CO    (mg/m3)");
    nueva.so2  = leerFloatPositivo("SO2   (ug/m3)");
    nueva.no2  = leerFloatPositivo("NO2   (ug/m3)");
    nueva.fecha = obtenerFechaActual();

    printf("\nFactores climaticos en %s:\n", zona->nombre);
    clima_nuevo.temperatura = leerFloatPositivo("Temperatura (C)   ");
    clima_nuevo.viento      = leerFloatPositivo("Viento      (km/h)");
    clima_nuevo.humedad     = leerFloatPositivo("Humedad     (%)   ");

    nueva.iqca = calcularIQCA_Global(nueva.pm25, nueva.pm10, nueva.o3, nueva.co, nueva.so2, nueva.no2);
    nueva.nivel_riesgo = determinarNivelRiesgo(nueva.iqca);

    int es_mismo_dia = 0;
    if (zona->num_lecturas > 0) {
        Fecha ult = zona->historico[zona->num_lecturas - 1].fecha;
        if (ult.dia == nueva.fecha.dia && ult.mes == nueva.fecha.mes && ult.anio == nueva.fecha.anio) {
            es_mismo_dia = 1;
        }
    }

    if (es_mismo_dia) {
        int i_ult = zona->num_lecturas - 1;
        zona->historico[i_ult] = nueva;
        zona->clima = clima_nuevo;
        zona->actual = nueva;
        zona->tiene_actual = 1;
        printf("\n[INFO] Ya existia un registro de hoy. Los datos fueron SOBRESCRITOS exitosamente.\n");
    } else {
        zona->clima        = clima_nuevo;
        zona->actual       = nueva;
        zona->tiene_actual = 1;

        if (zona->num_lecturas < NUM_DIAS) {
            zona->historico[zona->num_lecturas] = nueva;
            zona->num_lecturas++;
        } else {
            for (int i = 0; i < NUM_DIAS - 1; i++) {
                zona->historico[i] = zona->historico[i + 1];
            }
            zona->historico[NUM_DIAS - 1] = nueva;
        }
        printf("\nLectura registrada: %s [%02d/%02d/%04d]\n",
               zona->nombre, nueva.fecha.dia, nueva.fecha.mes, nueva.fecha.anio);
    }
    printf("Nivel IQCA Calculado: %d [%s]\n", nueva.iqca, obtenerNombreNivel(nueva.nivel_riesgo));
}

void verMonitoreoActual(Zona zonas[], int num_zonas) {
    printf("\n==================================================\n");
    printf("         MONITOREO ACTUAL DE CONTAMINACION        \n");
    printf("==================================================\n");

    long max_fecha_val = 0;

    // 1. Encontrar la fecha mas reciente (la de "hoy" o la ultima ingresada)
    for (int i = 0; i < num_zonas; i++) {
        if (zonas[i].tiene_actual) {
            long val = zonas[i].actual.fecha.anio * 10000 +
                       zonas[i].actual.fecha.mes * 100 +
                       zonas[i].actual.fecha.dia;
            if (val > max_fecha_val) {
                max_fecha_val = val;
            }
        }
    }

    // 2. Imprimir SOLO las zonas que coincidan con esa fecha mas reciente
    for (int i = 0; i < num_zonas; i++) {
        if (zonas[i].tiene_actual) {
            long val = zonas[i].actual.fecha.anio * 10000 +
                       zonas[i].actual.fecha.mes * 100 +
                       zonas[i].actual.fecha.dia;

            if (val == max_fecha_val) {
                printf("\nZONA: %s [%02d/%02d/%04d]\n", zonas[i].nombre, zonas[i].actual.fecha.dia, zonas[i].actual.fecha.mes, zonas[i].actual.fecha.anio);
                printf("  PM2.5: %.2f | PM10: %.2f | O3: %.2f\n", zonas[i].actual.pm25, zonas[i].actual.pm10, zonas[i].actual.o3);
                printf("  CO: %.2f | SO2: %.2f | NO2: %.2f\n", zonas[i].actual.co, zonas[i].actual.so2, zonas[i].actual.no2);
                printf("  >> IQCA: %d [%s]\n", zonas[i].actual.iqca, obtenerNombreNivel(zonas[i].actual.nivel_riesgo));
            } else {
                printf("\nZONA: %s [Datos desactualizados, no son de hoy]\n", zonas[i].nombre);
            }
        } else {
            printf("\nZONA: %s [Sin datos registrados]\n", zonas[i].nombre);
        }
    }
}

void verPromediosHistoricos(Zona zonas[], int num_zonas) {
    printf("\n==================================================\n");
    printf("              PROMEDIOS HISTORICOS                \n");
    printf("==================================================\n");

    for (int i = 0; i < num_zonas; i++) {
        if (zonas[i].num_lecturas > 0) {
            float suma_iqca = 0;
            for (int j = 0; j < zonas[i].num_lecturas; j++) {
                suma_iqca += zonas[i].historico[j].iqca;
            }

            Fecha inicio = zonas[i].historico[0].fecha;
            Fecha fin = zonas[i].historico[zonas[i].num_lecturas - 1].fecha;

            printf("\nZONA: %s | Promedio IQCA: %.1f\n", zonas[i].nombre, suma_iqca / zonas[i].num_lecturas);
            int nivel_promedio = determinarNivelRiesgo((int)(suma_iqca / zonas[i].num_lecturas));
            printf("  Clasificacion del promedio historico: %s\n", obtenerNombreNivel(nivel_promedio));
            printf("  Periodo evaluado: %02d/%02d/%04d al %02d/%02d/%04d (%d lecturas)\n",
                   inicio.dia, inicio.mes, inicio.anio,
                   fin.dia, fin.mes, fin.anio,
                   zonas[i].num_lecturas);
        } else {
            printf("\nZONA: %s | Sin lecturas historicas.\n", zonas[i].nombre);
        }
    }
}

void predecirNiveles(Zona zonas[], int num_zonas) {
    printf("\n==================================================\n");
    printf("       PREDICCION METEOROLOGICA E INDICE (24H)    \n");
    printf("==================================================\n");

    Fecha hoy = obtenerFechaActual();
    struct tm tm_hoy = {0};
    tm_hoy.tm_mday = hoy.dia;
    tm_hoy.tm_mon = hoy.mes - 1;
    tm_hoy.tm_year = hoy.anio - 1900;
    tm_hoy.tm_isdst = -1;
    time_t time_hoy = mktime(&tm_hoy);

    for (int i = 0; i < num_zonas; i++) {
        printf("\nZona: %s\n", zonas[i].nombre);

        if (zonas[i].num_lecturas == 0) {
            printf("  [!] Sin datos historicos suficientes para predecir.\n");
            continue;
        }

        struct tm tm_zona = {0};
        tm_zona.tm_mday = zonas[i].actual.fecha.dia;
        tm_zona.tm_mon = zonas[i].actual.fecha.mes - 1;
        tm_zona.tm_year = zonas[i].actual.fecha.anio - 1900;
        tm_zona.tm_isdst = -1;
        time_t time_zona = mktime(&tm_zona);

        double diff_dias = difftime(time_hoy, time_zona) / 86400.0;
        if (diff_dias > 2.0) {
            printf("  [!] Advertencia: Datos no recientes (mas de 2 dias desactualizados).\n");
        }

        if (zonas[i].num_lecturas < 5) {
            printf("  [!] Atencion: pocos datos (%d registrados). La prediccion puede no ser exacta.\n", zonas[i].num_lecturas);
        }

        float suma_ponderada = 0;
        int suma_pesos = 0;
        for (int j = 0; j < zonas[i].num_lecturas; j++) {
            int peso = j + 1;
            suma_ponderada += zonas[i].historico[j].iqca * peso;
            suma_pesos += peso;
        }
        float prediccion_iqca = (suma_pesos > 0) ? (suma_ponderada / suma_pesos) : 0;

        if (zonas[i].clima.viento > 15.0f) prediccion_iqca *= 0.90f;
        if (zonas[i].clima.humedad > 80.0f) prediccion_iqca *= 1.15f;

        int prediccion_entera = (int)(prediccion_iqca + 0.5f);
        int riesgo_futuro = determinarNivelRiesgo(prediccion_entera);

        if (zonas[i].tiene_actual) {
            printf("  IQCA Actual: %d\n", zonas[i].actual.iqca);
        }
        printf("  IQCA Predicho: %d [%s]\n", prediccion_entera, obtenerNombreNivel(riesgo_futuro));
    }
}

void verAlertasActivas(Zona zonas[], int num_zonas) {
    printf("\n==================================================\n");
    printf("         PANEL DE ESTADO REMAQ (TIEMPO REAL)      \n");
    printf("==================================================\n");

    long max_fecha_val = 0;

    for (int i = 0; i < num_zonas; i++) {
        if (zonas[i].tiene_actual) {
            long val = zonas[i].actual.fecha.anio * 10000 +
                       zonas[i].actual.fecha.mes * 100 +
                       zonas[i].actual.fecha.dia;
            if (val > max_fecha_val) {
                max_fecha_val = val;
            }
        }
    }

    int mostrados = 0;

    for (int i = 0; i < num_zonas; i++) {
        if (!zonas[i].tiene_actual) continue;

        long val = zonas[i].actual.fecha.anio * 10000 +
                   zonas[i].actual.fecha.mes * 100 +
                   zonas[i].actual.fecha.dia;

        if (val == max_fecha_val) {
            mostrados++;
            int riesgo_actual = zonas[i].actual.nivel_riesgo;

            float suma_ponderada = 0;
            int suma_pesos = 0;
            for (int j = 0; j < zonas[i].num_lecturas; j++) {
                int peso = j + 1;
                suma_ponderada += zonas[i].historico[j].iqca * peso;
                suma_pesos += peso;
            }
            float prediccion_iqca = (suma_pesos > 0) ? (suma_ponderada / suma_pesos) : 0;

            if (zonas[i].clima.viento > 15.0f) prediccion_iqca *= 0.90f;
            if (zonas[i].clima.humedad > 80.0f) prediccion_iqca *= 1.15f;

            int prediccion_entera = (int)(prediccion_iqca + 0.5f);
            int riesgo_futuro = determinarNivelRiesgo(prediccion_entera);

            printf("\nZONA: %s [%02d/%02d/%04d]\n",
                   zonas[i].nombre,
                   zonas[i].actual.fecha.dia,
                   zonas[i].actual.fecha.mes,
                   zonas[i].actual.fecha.anio);

            printf("  [ALERTA DIARIA]\n");
            printf("    Indice IQCA: %d puntos\n", zonas[i].actual.iqca);
            printf("    Estado: %s\n", obtenerNombreNivel(riesgo_actual));
            if (riesgo_actual >= 3) {
                printf("    >> ALERTA DEL DIA ACTIVADA <<\n");
            } else {
                printf("    >> Nivel diario seguro.\n");
            }

            printf("  [ALERTA PREDICCION 24H]\n");
            printf("    Indice IQCA Predicho: %d puntos\n", prediccion_entera);
            printf("    Estado Futuro: %s\n", obtenerNombreNivel(riesgo_futuro));
            if (riesgo_futuro >= 3) {
                printf("    >> ALERTA DE PREDICCION ACTIVADA <<\n");
            } else {
                printf("    >> Nivel de prediccion seguro.\n");
            }
            printf("--------------------------------------------------\n");
        }
    }

    if (mostrados == 0) {
        printf("\n  [INFO] No hay datos registrados.\n");
    }
}

void verRecomendaciones(Zona zonas[], int num_zonas) {
    int sub_opcion;

    printf("\n==================================================\n");
    printf("      PROTOCOLOS OFICIALES DE ACCION (NECA)       \n");
    printf("==================================================\n");
    printf("  1. Ver recomendaciones actuales (hoy)\n");
    printf("  2. Ver recomendaciones para prediccion a 24h\n");
    printf("  Opcion: ");

    if (scanf("%d", &sub_opcion) != 1) {
        limpiarBuffer();
        return;
    }
    limpiarBuffer();

    if (sub_opcion != 1 && sub_opcion != 2) {
        printf("\n  [ERROR] Opcion no valida.\n");
        return;
    }

    long max_fecha_val = 0;

    for (int i = 0; i < num_zonas; i++) {
        if (zonas[i].tiene_actual) {
            long val = zonas[i].actual.fecha.anio * 10000 +
                       zonas[i].actual.fecha.mes * 100 +
                       zonas[i].actual.fecha.dia;
            if (val > max_fecha_val) {
                max_fecha_val = val;
            }
        }
    }

    int mostrados = 0;

    for (int i = 0; i < num_zonas; i++) {
        if (!zonas[i].tiene_actual) continue;

        long val = zonas[i].actual.fecha.anio * 10000 +
                   zonas[i].actual.fecha.mes * 100 +
                   zonas[i].actual.fecha.dia;

        if (val == max_fecha_val) {
            mostrados++;
            int riesgo = 0;

            if (sub_opcion == 1) {
                riesgo = zonas[i].actual.nivel_riesgo;
                printf("\nZONA: %s [%02d/%02d/%04d] | ESTADO ACTUAL: %s\n",
                       zonas[i].nombre,
                       zonas[i].actual.fecha.dia,
                       zonas[i].actual.fecha.mes,
                       zonas[i].actual.fecha.anio,
                       obtenerNombreNivel(riesgo));
            } else {
                float suma_ponderada = 0;
                int suma_pesos = 0;
                for (int j = 0; j < zonas[i].num_lecturas; j++) {
                    int peso = j + 1;
                    suma_ponderada += zonas[i].historico[j].iqca * peso;
                    suma_pesos += peso;
                }
                float prediccion_iqca = (suma_pesos > 0) ? (suma_ponderada / suma_pesos) : 0;

                if (zonas[i].clima.viento > 15.0f) prediccion_iqca *= 0.90f;
                if (zonas[i].clima.humedad > 80.0f) prediccion_iqca *= 1.15f;

                int prediccion_entera = (int)(prediccion_iqca + 0.5f);
                riesgo = determinarNivelRiesgo(prediccion_entera);

                printf("\nZONA: %s [%02d/%02d/%04d] | PREDICCION 24H: %s\n",
                       zonas[i].nombre,
                       zonas[i].actual.fecha.dia,
                       zonas[i].actual.fecha.mes,
                       zonas[i].actual.fecha.anio,
                       obtenerNombreNivel(riesgo));
            }

            if (riesgo <= 2) {
                printf("  >> Nivel seguro. Sin acciones de emergencia.\n");
                continue;
            }

            switch (riesgo) {
                case 3:
                    printf("  - Limitar esfuerzo al aire libre.\n");
                    printf("  - Monitoreo continuo de estaciones.\n");
                    break;
                case 4:
                    printf("  - Evitar esfuerzo al aire libre.\n");
                    printf("  - Restriccion preventiva de transito pesado.\n");
                    break;
                case 5:
                    printf("  - Mantener ventanas cerradas.\n");
                    printf("  - Suspension de obras y reduccion industrial.\n");
                    break;
                case 6:
                    printf("  - Permanecer en interiores obligatoriamente.\n");
                    printf("  - Declaratoria de Emergencia Sectorizada.\n");
                    break;
            }
        }
    }

    if (mostrados == 0) {
        printf("\n  [INFO] No hay datos registrados para mostrar recomendaciones.\n");
    }
}

void exportarHistorialCSV(Zona zonas[], int num_zonas) {
    printf("\n==================================================\n");

    FILE *archivo = fopen("reporte_quito.csv", "w");
    if (archivo == NULL) {
        printf("[ERROR] No se pudo generar el archivo.\n");
        return;
    }

    fprintf(archivo, "Zona,Fecha,PM2.5,PM10,O3,CO,SO2,NO2,IQCA,Riesgo\n");

    for (int i = 0; i < num_zonas; i++) {
        for (int j = 0; j < zonas[i].num_lecturas; j++) {
            Lectura lec = zonas[i].historico[j];
            fprintf(archivo, "%s,%02d/%02d/%04d,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%d,%s\n",
                    zonas[i].nombre, lec.fecha.dia, lec.fecha.mes, lec.fecha.anio,
                    lec.pm25, lec.pm10, lec.o3, lec.co, lec.so2, lec.no2,
                    lec.iqca, obtenerNombreNivel(lec.nivel_riesgo));
        }
    }
    fclose(archivo);

    printf("[EXITO] Exportacion finalizada: reporte_quito.csv\n");
    printf("==================================================\n");
}

void importarDatosREMAQ(Zona zonas[], int n) {
    int idx;
    printf("\n--- IMPORTAR DATOS REMAQ (CSV) ---\n");
    for (int i = 0; i < n; i++) printf("  %d. %s\n", i + 1, zonas[i].nombre);

    printf("Seleccione la zona (1-%d): ", n);
    if (scanf("%d", &idx) != 1 || idx < 1 || idx > n) {
        printf("Zona no valida.\n");
        limpiarBuffer();
        return;
    }
    limpiarBuffer();
    idx--;

    char nombre_archivo[100];
    printf("Ingrese nombre exacto del archivo: ");
    scanf("%99[^\n]", nombre_archivo);
    limpiarBuffer();

    FILE *archivo = fopen(nombre_archivo, "r");
    if (!archivo) {
        printf("Error: No se pudo abrir el archivo '%s'.\n", nombre_archivo);
        return;
    }

    char linea[256];
    int agregados = 0, sobreescritos = 0;
    Zona *zona = &zonas[idx];

    fgets(linea, sizeof(linea), archivo);

    while (fgets(linea, sizeof(linea), archivo)) {
        int usa_pyc = (strchr(linea, ';') != NULL);
        if (usa_pyc) {
            for (int i = 0; linea[i]; i++) {
                if (linea[i] == ',') linea[i] = '.';
            }
        }

        const char *delim = usa_pyc ? ";" : ",";

        char *token = strtok(linea, delim);
        if (!token) continue;
        int d = atoi(token);

        token = strtok(NULL, delim); int m = token ? atoi(token) : 1;
        token = strtok(NULL, delim); int y = token ? atoi(token) : 2026;

        token = strtok(NULL, delim); float pm25 = token ? atof(token) : 0;
        token = strtok(NULL, delim); float pm10 = token ? atof(token) : 0;
        token = strtok(NULL, delim); float o3 = token ? atof(token) : 0;
        token = strtok(NULL, delim); float co = token ? atof(token) : 0;
        token = strtok(NULL, delim); float so2 = token ? atof(token) : 0;
        token = strtok(NULL, delim); float no2 = token ? atof(token) : 0;

        Lectura nueva;
        nueva.fecha.dia = d;
        nueva.fecha.mes = m;
        nueva.fecha.anio = y;
        nueva.pm25 = pm25;
        nueva.pm10 = pm10;
        nueva.o3 = o3;
        nueva.co = co;
        nueva.so2 = so2;
        nueva.no2 = no2;

        nueva.iqca = calcularIQCA_Global(pm25, pm10, o3, co, so2, no2);
        nueva.nivel_riesgo = determinarNivelRiesgo(nueva.iqca);

        int es_mismo_dia = 0;
        int i_ult = -1;
        for(int i = 0; i < zona->num_lecturas; i++) {
            if(zona->historico[i].fecha.dia == d &&
               zona->historico[i].fecha.mes == m &&
               zona->historico[i].fecha.anio == y) {
                es_mismo_dia = 1;
                i_ult = i;
                break;
            }
        }

        if (es_mismo_dia) {
            zona->historico[i_ult] = nueva;
            zona->actual = nueva;
            zona->tiene_actual = 1;
            sobreescritos++;
        } else {
            zona->actual = nueva;
            zona->tiene_actual = 1;
            if (zona->num_lecturas < NUM_DIAS) {
                zona->historico[zona->num_lecturas] = nueva;
                zona->num_lecturas++;
            } else {
                for (int i = 0; i < NUM_DIAS - 1; i++) {
                    zona->historico[i] = zona->historico[i + 1];
                }
                zona->historico[NUM_DIAS - 1] = nueva;
            }
            agregados++;
        }
    }
    fclose(archivo);
    printf("\n[INFO] Importacion completada. Nuevos: %d, Sobreescritos: %d\n", agregados, sobreescritos);
}

void ejecutarSistema(void) {
    Zona zonas[MAX_ZONAS];
    int  num_zonas;
    int  opcion;

    if (!cargarHistorico(zonas, &num_zonas)) {
        configuracionInicial(zonas, &num_zonas);
    }

    do {
        printf("\n========================================\n");
        printf("  SISTEMA DE MONITOREO DE CONTAMINACION\n");
        printf("========================================\n");
        printf("  1. Registrar lectura actual\n");
        printf("  2. Ver monitoreo actual\n");
        printf("  3. Ver promedios historicos (30 dias)\n");
        printf("  4. Predecir niveles proximas 24h\n");
        printf("  5. Ver alertas activas\n");
        printf("  6. Ver recomendaciones\n");
        printf("  7. Exportar historial a CSV\n");
        printf("  8. Gestionar zonas\n");
        printf("  9. Cargar Datos\n");
        printf("  0. Salir\n");
        printf("========================================\n");
        printf("  Opcion: ");

        if (scanf("%d", &opcion) != 1) opcion = -1;
        limpiarBuffer();

        switch (opcion) {
            case 1:  registrarLectura(zonas, num_zonas);         presioneContinuar(); break;
            case 2:  verMonitoreoActual(zonas, num_zonas);       presioneContinuar(); break;
            case 3:  verPromediosHistoricos(zonas, num_zonas);   presioneContinuar(); break;
            case 4:  predecirNiveles(zonas, num_zonas);          presioneContinuar(); break;
            case 5:  verAlertasActivas(zonas, num_zonas);        presioneContinuar(); break;
            case 6:  verRecomendaciones(zonas, num_zonas);       presioneContinuar(); break;
            case 7:  exportarHistorialCSV(zonas, num_zonas);     presioneContinuar(); break;
            case 8:  gestionarZonas(zonas, &num_zonas);          break;
            case 9: importarDatosREMAQ(zonas, num_zonas);        presioneContinuar(); break;
            case 99: alterarDatosExposicion(zonas, num_zonas);   presioneContinuar(); break;
            case 0:  printf("\nGuardando datos y saliendo...\n"); break;

            default: printf("\nOpcion no valida.\n");     presioneContinuar(); break;
        }
    } while (opcion != 0);

    guardarHistorico(zonas, num_zonas);
    printf("Sistema finalizado.\n");
    presioneContinuar();
}
