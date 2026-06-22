//
// Created by Andres on 22/6/2026.
//

#include "funciones.h"

// ==========================================
// UTILIDADES Y VALIDACIONES
// ==========================================

void limpiarBuffer() {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

void presioneContinuar() {
    printf("\nPresione ENTER para continuar...");
    getchar();
}

Fecha obtenerFechaActual() {
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
        cadena[i] = toupper((unsigned char)cadena[i]);
    }
}

int nombreExiste(Zona zonas[], int num_zonas, const char *nombre_buscar, int indice_ignorar) {
    for (int i = 0; i < num_zonas; i++) {
        if (i != indice_ignorar && strcmp(zonas[i].nombre, nombre_buscar) == 0) {
            return 1; // El nombre ya existe
        }
    }
    return 0;
}

// ==========================================
// MANEJO DE ARCHIVOS Y CONFIGURACIÓN INICIAL
// ==========================================

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
            temp_nombre[strcspn(temp_nombre, "\n")] = '\0'; // Eliminar salto de linea

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

    if (fread(zonas, sizeof(Zona), *n, archivo) != (size_t)*n) {
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

// ==========================================
// MÓDULOS DE GESTIÓN Y LECTURA
// ==========================================

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
                    printf("\nLímite máximo de zonas alcanzado (%d).\n", MAX_ZONAS);
                } else {
                    printf("\nIngrese nombre de la nueva zona: ");
                    fgets(temp_nombre, NOMBRE_MAX, stdin);
                    temp_nombre[strcspn(temp_nombre, "\n")] = '\0';
                    aMayusculas(temp_nombre);

                    if (strlen(temp_nombre) > 0 && !nombreExiste(zonas, *num_zonas, temp_nombre, -1)) {
                        strncpy(zonas[*num_zonas].nombre, temp_nombre, NOMBRE_MAX - 1);
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
                        // Desplazamiento para cubrir el hueco
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
    nueva.co2  = leerFloatPositivo("CO2   (ppm)  ");
    nueva.so2  = leerFloatPositivo("SO2   (ug/m3)");
    nueva.no2  = leerFloatPositivo("NO2   (ug/m3)");
    nueva.pm25 = leerFloatPositivo("PM2.5 (ug/m3)");
    nueva.fecha = obtenerFechaActual();

    printf("\nFactores climaticos en %s:\n", zona->nombre);
    clima_nuevo.temperatura = leerFloatPositivo("Temperatura (C)   ");
    clima_nuevo.viento      = leerFloatPositivo("Viento      (km/h)");
    clima_nuevo.humedad     = leerFloatPositivo("Humedad     (%)   ");

    // Verificar si el ultimo registro es del mismo dia
    int es_mismo_dia = 0;
    if (zona->num_lecturas > 0) {
        Fecha ult = zona->historico[zona->num_lecturas - 1].fecha;
        if (ult.dia == nueva.fecha.dia && ult.mes == nueva.fecha.mes && ult.anio == nueva.fecha.anio) {
            es_mismo_dia = 1;
        }
    }

    if (es_mismo_dia) {
        // Promediar con los datos existentes del dia
        int i_ult = zona->num_lecturas - 1;
        zona->historico[i_ult].co2  = (zona->historico[i_ult].co2 + nueva.co2) / 2.0f;
        zona->historico[i_ult].so2  = (zona->historico[i_ult].so2 + nueva.so2) / 2.0f;
        zona->historico[i_ult].no2  = (zona->historico[i_ult].no2 + nueva.no2) / 2.0f;
        zona->historico[i_ult].pm25 = (zona->historico[i_ult].pm25 + nueva.pm25) / 2.0f;

        zona->clima.temperatura = (zona->clima.temperatura + clima_nuevo.temperatura) / 2.0f;
        zona->clima.viento      = (zona->clima.viento + clima_nuevo.viento) / 2.0f;
        zona->clima.humedad     = (zona->clima.humedad + clima_nuevo.humedad) / 2.0f;

        zona->actual = zona->historico[i_ult];
        zona->tiene_actual = 1;
        printf("\n[INFO] Ya existia un registro de hoy. Los datos fueron promediados exitosamente.\n");
    } else {
        // Logica normal de ventana deslizante para un dia nuevo
        zona->clima = clima_nuevo;
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
        printf("\nLectura registrada: %s [%02d/%02d/%04d]\n", zona->nombre, nueva.fecha.dia, nueva.fecha.mes, nueva.fecha.anio);
    }
}

void verMonitoreoActual(Zona zonas[], int num_zonas) {
    printf("\n==================================================\n");
    printf("         MONITOREO ACTUAL DE CONTAMINACION        \n");
    printf("==================================================\n");

    long max_fecha_val = 0;

    // PASO 1: Buscar la fecha mas reciente en todo el sistema (Formato AAAAMMDD)
    for (int i = 0; i < num_zonas; i++) {
        if (zonas[i].tiene_actual) {
            long fecha_val = (zonas[i].actual.fecha.anio * 10000) +
                             (zonas[i].actual.fecha.mes * 100) +
                             zonas[i].actual.fecha.dia;
            if (fecha_val > max_fecha_val) {
                max_fecha_val = fecha_val;
            }
        }
    }

    // Si max_fecha_val es 0, significa que ninguna zona tiene datos
    if (max_fecha_val == 0) {
        printf("\nNo hay lecturas actuales registradas.\nPor favor registre una en la Opcion 1.\n");
        printf("==================================================\n");
        return;
    }

    int encontrados = 0;

    // PASO 2: Mostrar solo las zonas que coincidan con la fecha mas reciente
    for (int i = 0; i < num_zonas; i++) {
        if (zonas[i].tiene_actual) {
            long fecha_val = (zonas[i].actual.fecha.anio * 10000) +
                             (zonas[i].actual.fecha.mes * 100) +
                             zonas[i].actual.fecha.dia;

            if (fecha_val == max_fecha_val) {
                encontrados = 1;
                printf("\n  Zona: %-10s | Fecha: %02d/%02d/%04d\n", zonas[i].nombre, zonas[i].actual.fecha.dia, zonas[i].actual.fecha.mes, zonas[i].actual.fecha.anio);
                printf("  - CO2:   %7.2f | Limite: %7.2f | %s\n", zonas[i].actual.co2, LIMITE_CO2, (zonas[i].actual.co2 > LIMITE_CO2) ? "EXCEDE" : "NORMAL");
                printf("  - SO2:   %7.2f | Limite: %7.2f | %s\n", zonas[i].actual.so2, LIMITE_SO2, (zonas[i].actual.so2 > LIMITE_SO2) ? "EXCEDE" : "NORMAL");
                printf("  - NO2:   %7.2f | Limite: %7.2f | %s\n", zonas[i].actual.no2, LIMITE_NO2, (zonas[i].actual.no2 > LIMITE_NO2) ? "EXCEDE" : "NORMAL");
                printf("  - PM2.5: %7.2f | Limite: %7.2f | %s\n", zonas[i].actual.pm25, LIMITE_PM25, (zonas[i].actual.pm25 > LIMITE_PM25) ? "EXCEDE" : "NORMAL");
            }
        }
    }

    if (!encontrados) {
        printf("\nNo hay datos visibles para la fecha calculada.\n");
    }
    printf("==================================================\n");
}

void verPromediosHistoricos(Zona zonas[], int num_zonas) {
    printf("\n==================================================\n");
    printf("     PROMEDIOS HISTORICOS (HASTA 30 DIAS)         \n");
    printf("==================================================\n");

    for (int i = 0; i < num_zonas; i++) {
        printf("\n  Zona: %s | Dias registrados: %d\n", zonas[i].nombre, zonas[i].num_lecturas);

        if (zonas[i].num_lecturas == 0) {
            printf("  -> No hay datos historicos suficientes para evaluar.\n");
            continue;
        }

        Fecha inicio = zonas[i].historico[0].fecha;
        Fecha fin = zonas[i].historico[zonas[i].num_lecturas - 1].fecha;
        printf("  Periodo evaluado: %02d/%02d/%04d al %02d/%02d/%04d\n",
               inicio.dia, inicio.mes, inicio.anio, fin.dia, fin.mes, fin.anio);

        float sum_co2 = 0, sum_so2 = 0, sum_no2 = 0, sum_pm25 = 0;

        for (int j = 0; j < zonas[i].num_lecturas; j++) {
            sum_co2  += zonas[i].historico[j].co2;
            sum_so2  += zonas[i].historico[j].so2;
            sum_no2  += zonas[i].historico[j].no2;
            sum_pm25 += zonas[i].historico[j].pm25;
        }

        float prom_co2  = sum_co2 / zonas[i].num_lecturas;
        float prom_so2  = sum_so2 / zonas[i].num_lecturas;
        float prom_no2  = sum_no2 / zonas[i].num_lecturas;
        float prom_pm25 = sum_pm25 / zonas[i].num_lecturas;

        printf("  - Promedio CO2:   %7.2f | Limite: %6.2f | %s\n",
               prom_co2, LIMITE_CO2, (prom_co2 > LIMITE_CO2) ? "EXCEDE" : "NORMAL");

        printf("  - Promedio SO2:   %7.2f | Limite: %6.2f | %s\n",
               prom_so2, LIMITE_SO2, (prom_so2 > LIMITE_SO2) ? "EXCEDE" : "NORMAL");

        printf("  - Promedio NO2:   %7.2f | Limite: %6.2f | %s\n",
               prom_no2, LIMITE_NO2, (prom_no2 > LIMITE_NO2) ? "EXCEDE" : "NORMAL");

        printf("  - Promedio PM2.5: %7.2f | Limite: %6.2f | %s\n",
               prom_pm25, LIMITE_PM25, (prom_pm25 > LIMITE_PM25) ? "EXCEDE" : "NORMAL");
    }
    printf("\n==================================================\n");
}

void generarSemillaOculta(Zona zonas[], int num_zonas) {
    int nivel;
    printf("\n--- GENERADOR DE DATOS (SEMILLA) ---\n");
    printf("Seleccione el nivel de contaminacion a simular:\n");
    printf("  1. BAJO  (Valores normales, ideales para probar que NO hay alertas)\n");
    printf("  2. MEDIO (Valores limitrofes, probocara algunas alertas esporadicas)\n");
    printf("  3. ALTO  (Valores criticos, forzara el disparo de todas las alarmas)\n");
    printf("  Opcion: ");

    if (scanf("%d", &nivel) != 1) nivel = 1;
    limpiarBuffer();
    if (nivel < 1 || nivel > 3) nivel = 1; // Por defecto a BAJO si ingresan algo invalido

    srand((unsigned int)time(NULL));
    time_t t_actual = time(NULL);

    for (int i = 0; i < num_zonas; i++) {
        zonas[i].num_lecturas = NUM_DIAS;
        for (int j = 0; j < NUM_DIAS; j++) {
            time_t t_hist = t_actual - (86400 * (NUM_DIAS - j));
            struct tm *tm_hist = localtime(&t_hist);

            zonas[i].historico[j].fecha.dia = tm_hist->tm_mday;
            zonas[i].historico[j].fecha.mes = tm_hist->tm_mon + 1;
            zonas[i].historico[j].fecha.anio = tm_hist->tm_year + 1900;

            // Inyeccion de datos segun el nivel seleccionado
            if (nivel == 1) { // BAJO
                zonas[i].historico[j].co2  = 400.0f + (rand() % 400); // Max 800 (Limite 1000)
                zonas[i].historico[j].so2  = 2.0f + (rand() % 10);    // Max 12 (Limite 20)
                zonas[i].historico[j].no2  = 5.0f + (rand() % 15);    // Max 20 (Limite 25)
                zonas[i].historico[j].pm25 = 5.0f + (rand() % 8);     // Max 13 (Limite 15)
            } else if (nivel == 2) { // MEDIO
                zonas[i].historico[j].co2  = 800.0f + (rand() % 400); // Max 1200
                zonas[i].historico[j].so2  = 15.0f + (rand() % 15);   // Max 30
                zonas[i].historico[j].no2  = 15.0f + (rand() % 20);   // Max 35
                zonas[i].historico[j].pm25 = 12.0f + (rand() % 15);   // Max 27
            } else { // ALTO
                zonas[i].historico[j].co2  = 1100.0f + (rand() % 900); // Max 2000
                zonas[i].historico[j].so2  = 25.0f + (rand() % 35);    // Max 60
                zonas[i].historico[j].no2  = 30.0f + (rand() % 40);    // Max 70
                zonas[i].historico[j].pm25 = 20.0f + (rand() % 30);    // Max 50
            }
        }

        // El clima no genera alarmas, asi que puede ser generico
        zonas[i].clima.temperatura = 15.0f + (rand() % 15);
        zonas[i].clima.viento = 5.0f + (rand() % 20);
        zonas[i].clima.humedad = 40.0f + (rand() % 40);

        // Clonar el registro del ultimo dia inyectado como el "actual"
        zonas[i].actual = zonas[i].historico[NUM_DIAS - 1];
        zonas[i].tiene_actual = 1;
    }

    char nombres_nivel[3][10] = {"BAJO", "MEDIO", "ALTO"};
    printf("\n[SISTEMA] Se inyectaron 30 dias de datos (Nivel %s) en las %d zonas.\n", nombres_nivel[nivel - 1], num_zonas);
}

void predecirNiveles(Zona zonas[], int num_zonas) {
    printf("\n==================================================\n");
    printf("     PREDICCION A 24 HORAS (PROMEDIO PONDERADO)   \n");
    printf("==================================================\n");

    time_t t_actual = time(NULL);

    for (int i = 0; i < num_zonas; i++) {
        if (zonas[i].num_lecturas == 0) {
            printf("\n  Zona: %s\n  -> [ERROR] Sin datos para predecir.\n", zonas[i].nombre);
            continue;
        }

        printf("\n  Zona: %s\n", zonas[i].nombre);

        // ADVERTENCIA 1: Volumen de datos insuficiente
        if (zonas[i].num_lecturas < 7) {
            printf("  [!] ADVERTENCIA: Pocos registros (%d dias). Prediccion poco fiable.\n", zonas[i].num_lecturas);
        }

        // ADVERTENCIA 2: Datos desactualizados
        Fecha ultima = zonas[i].historico[zonas[i].num_lecturas - 1].fecha;
        struct tm tm_ultima = {0};
        tm_ultima.tm_mday = ultima.dia;
        tm_ultima.tm_mon  = ultima.mes - 1;
        tm_ultima.tm_year = ultima.anio - 1900;
        time_t t_ultima = mktime(&tm_ultima);

        double diff_dias = difftime(t_actual, t_ultima) / 86400.0;
        if (diff_dias > 2.0) {
            printf("  [!] ADVERTENCIA: Datos desactualizados. Ultimo registro hace %.0f dias.\n", diff_dias);
        }

        // CALCULO: Promedio Movil Ponderado (WMA)
        float sum_co2 = 0, sum_so2 = 0, sum_no2 = 0, sum_pm25 = 0;
        int sum_pesos = 0;

        for (int j = 0; j < zonas[i].num_lecturas; j++) {
            int peso = j + 1; // Los dias mas recientes (j mayor) tienen mayor multiplicador
            sum_pesos += peso;
            sum_co2  += zonas[i].historico[j].co2 * peso;
            sum_so2  += zonas[i].historico[j].so2 * peso;
            sum_no2  += zonas[i].historico[j].no2 * peso;
            sum_pm25 += zonas[i].historico[j].pm25 * peso;
        }

        float pred_co2  = sum_co2 / sum_pesos;
        float pred_so2  = sum_so2 / sum_pesos;
        float pred_no2  = sum_no2 / sum_pesos;
        float pred_pm25 = sum_pm25 / sum_pesos;

        // APLICACION DE FACTORES CLIMATICOS
        float factor = 1.0f;
        if (zonas[i].clima.viento > 15.0f) factor -= 0.10f;       // El viento limpia el aire (-10%)
        if (zonas[i].clima.temperatura > 25.0f) factor += 0.05f;  // El calor aumenta el smog (+5%)
        if (zonas[i].clima.humedad > 80.0f) factor += 0.05f;      // Alta humedad retiene particulas (+5%)

        pred_co2  *= factor;
        pred_so2  *= factor;
        pred_no2  *= factor;
        pred_pm25 *= factor;

        printf("  - Prediccion CO2:   %7.2f | Limite: %6.2f | %s\n", pred_co2, LIMITE_CO2, (pred_co2 > LIMITE_CO2) ? "ALERTA" : "SEGURO");
        printf("  - Prediccion SO2:   %7.2f | Limite: %6.2f | %s\n", pred_so2, LIMITE_SO2, (pred_so2 > LIMITE_SO2) ? "ALERTA" : "SEGURO");
        printf("  - Prediccion NO2:   %7.2f | Limite: %6.2f | %s\n", pred_no2, LIMITE_NO2, (pred_no2 > LIMITE_NO2) ? "ALERTA" : "SEGURO");
        printf("  - Prediccion PM2.5: %7.2f | Limite: %6.2f | %s\n", pred_pm25, LIMITE_PM25, (pred_pm25 > LIMITE_PM25) ? "ALERTA" : "SEGURO");
    }
    printf("\n==================================================\n");
}

// ==========================================
// BUCLE PRINCIPAL
// ==========================================

void ejecutarSistema() {
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
        printf("  7. Exportar reporte a archivo\n");
        printf("  8. Gestionar Zonas (Agregar/Editar)\n");
        printf("  9. Salir\n");
        printf("========================================\n");
        printf("  Opcion: ");

        if (scanf("%d", &opcion) != 1) opcion = -1;
        limpiarBuffer();

        switch (opcion) {
            case 1: registrarLectura(zonas, num_zonas); presioneContinuar(); break;
            case 2: verMonitoreoActual(zonas, num_zonas); presioneContinuar(); break;
            case 3: verPromediosHistoricos(zonas, num_zonas); presioneContinuar(); break;
            case 4: predecirNiveles(zonas, num_zonas); presioneContinuar(); break;
            case 5: printf("\nModulo en construccion...\n"); presioneContinuar(); break;
            case 6: printf("\nModulo en construccion...\n"); presioneContinuar(); break;
            case 7: printf("\nModulo en construccion...\n"); presioneContinuar(); break;
            case 8: gestionarZonas(zonas, &num_zonas); break;
            case 9: printf("\nGuardando datos y saliendo...\n"); break;
            case 99: generarSemillaOculta(zonas, num_zonas); presioneContinuar(); break;
            default: printf("\nOpcion no valida. Intente de nuevo.\n"); presioneContinuar();
        }
    } while (opcion != 9);

    guardarHistorico(zonas, num_zonas);
    printf("Sistema finalizado.\n");
    presioneContinuar();
}