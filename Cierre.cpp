#include <iostream>
#include <cstring>
#include <cstdio>

using namespace std;

//  cierre.cpp  —  TP "El buffet de Alberto"
//
//  Junta las planillas de todos los días de la semana en una sola
//  planilla semanal (comandas_semana_sX-mm.dat), ordenada por mozo.
//
//  Lee: comandas_dd-mm-aaaa.dat (varios, cada uno ya ordenado por mozo)
//  Genera: comandas_semana_sX-mm.dat
//
//  Usa apareo de N archivos: aprovecha que cada planilla diaria ya viene
//  ordenada por idMozo y las combina en una sola pasada sin reordenar.

struct Comanda {
    int   idMozo;
    int   codigoProducto;
    int   cantidad;
    float comision;
};

// Cantidad maxima de dias que se pueden ingresar
const int MAX_FECHAS = 100;

//agregamos texto al final de otro texto
void agregarTexto(char* destino, const char* origen)
{
    int i = 0;
    while (destino[i] != '\0')   // busco el final del texto actual
    {
        i++;
    }

    int j = 0;
    while (origen[j] != '\0')    // copio el nuevo texto desde ahi
    {
        destino[i] = origen[j];
        i++;
        j++;
    }
    destino[i] = '\0';           // cierro
}

//le pedimos al usuario las fechas de los dias de la semana (para saber que archivos de dia leer)
//si el usuario ingresa dos veces la misma fecha, la segunda se ignora para evitar duplicar ventas
int pedirFechas(char fechas[][11], int len) //el len seria nuestro tope
{
    int cantFechas;
    cout << "Ingrese la cantidad de dias de la semana (max " << len << "): ";
    cin >> cantFechas;

    int cantCargadas = 0;
    for (int i = 0; i < cantFechas; i++)
    {
        char fechaIngresada[11];
        cout << "Ingrese la fecha del dia " << i + 1 << " (DD-MM-AAAA): ";
        cin >> fechaIngresada;

        // verificar si ya fue cargada para evitar duplicados
        bool duplicada = false;
        int j = 0;
        while (j < cantCargadas && !duplicada)
        {
            if (strcasecmp(fechas[j], fechaIngresada) == 0)
            {
                duplicada = true;
            }
            j++;
        }

        if (!duplicada && cantCargadas < len)
        {
            strcpy(fechas[cantCargadas], fechaIngresada);
            cantCargadas++;
        }
        else if (duplicada)
        {
            cout << "AVISO: la fecha " << fechaIngresada << " ya fue cargada. Se omite.\n";
        }
    }

    return cantCargadas;
}

//recorremos el archivo semanal y mostramos su contenido por pantalla
void mostrarArchivoSemana(const char* ruta)
{
    FILE* f = fopen(ruta, "rb");
    if (f == NULL)
    {
        cout << "No se pudo abrir " << ruta << endl;
        return;
    }

    cout << "\nCONTENIDO DE " << ruta << endl;

    Comanda c;
    int nro = 0;
    while (fread(&c, sizeof(Comanda), 1, f) == 1)
    {
        nro++;
        cout << "#" << nro << " | idMozo=" << c.idMozo
             << " | prod=" << c.codigoProducto
             << " | cant=" << c.cantidad
             << " | com=$" << c.comision << endl;
    }

    fclose(f);
    cout << "Total de registros: " << nro << endl;
}

//  main
int main()
{
    // 1) Pedir al usuario el numero de semana
    int nroSemana;
    cout << "Ingrese el numero de semana (ej: 1): ";
    cin >> nroSemana;

    // 2) Pedir el mes de la semana
    int mes;
    cout << "Ingrese el mes de la semana (ej: 6): ";
    cin >> mes;

    // 3) Armar el nombre del archivo semanal
    // formato: comandas_semana_sX-mm.dat
    char archivoSemana[64];
    strcpy(archivoSemana, "../datos/comandas_semana_s");

    //pasamos nroSemana a texto
    char semanaTexto[10];
    if (nroSemana == 0)
    {
        semanaTexto[0] = '0';
        semanaTexto[1] = '\0';
    }
    else
    {
        char temp[10];
        int i = 0;
        int num = nroSemana;
        while (num > 0)
        {
            temp[i] = '0' + (num % 10);
            num = num / 10;
            i++;
        }
        int j = 0;
        for (int k = i - 1; k >= 0; k--)
        {
            semanaTexto[j] = temp[k];
            j++;
        }
        semanaTexto[j] = '\0';
    }

    agregarTexto(archivoSemana, semanaTexto);
    agregarTexto(archivoSemana, "-");

    //pasamos el mes a texto (2 digitos)
    char mesTexto[10];
    if (mes < 10)
    {
        mesTexto[0] = '0';
        mesTexto[1] = '0' + mes;
        mesTexto[2] = '\0';
    }
    else
    {
        char temp[10];
        int i = 0;
        int num = mes;
        while (num > 0)
        {
            temp[i] = '0' + (num % 10);
            num = num / 10;
            i++;
        }
        int j = 0;
        for (int k = i - 1; k >= 0; k--)
        {
            mesTexto[j] = temp[k];
            j++;
        }
        mesTexto[j] = '\0';
    }

    agregarTexto(archivoSemana, mesTexto);
    agregarTexto(archivoSemana, ".dat");

    // 4) Pedir las fechas de los dias de la semana
    char fechas[MAX_FECHAS][11];
    int cantFechas = pedirFechas(fechas, MAX_FECHAS);

    if (cantFechas == 0)
    {
        cout << "Cantidad de dias invalida. Fin." << endl;
        return 0;
    }

    // 5) Abrir todos los archivos de dia y leer el primer registro de cada uno (apareo)
    FILE* archivos[MAX_FECHAS];
    Comanda actuales[MAX_FECHAS];  // registro actual leido de cada archivo
    bool hayMas[MAX_FECHAS];       // indica si ese archivo todavia tiene registros

    for (int i = 0; i < cantFechas; i++)
    {
        archivos[i] = NULL;
        hayMas[i] = false;
    }

    for (int i = 0; i < cantFechas; i++)
    {
        char archivoDia[64];
        strcpy(archivoDia, "../datos/comandas_");
        agregarTexto(archivoDia, fechas[i]);
        agregarTexto(archivoDia, ".dat");

        archivos[i] = fopen(archivoDia, "rb");
        if (archivos[i] == NULL)
        {
            cout << "AVISO: no existe la planilla del dia "
                 << fechas[i] << ". Se omite.\n";
        }
        else
        {
            // lectura anticipada: cargamos el primer registro de cada archivo
            if (fread(&actuales[i], sizeof(Comanda), 1, archivos[i]) == 1)
            {
                hayMas[i] = true;
            }
            else
            {
                fclose(archivos[i]);
                archivos[i] = NULL;
            }
        }
    }

    // 6) Crear el archivo semanal y escribir usando el apareo
    FILE* fSemana = fopen(archivoSemana, "wb");
    if (fSemana == NULL)
    {
        for (int i = 0; i < cantFechas; i++)
        {
            if (archivos[i] != NULL)
            {
                fclose(archivos[i]);
            }
        }
        cout << "No se pudo crear el archivo semanal." << endl;
        return 0;
    }

    int cantEscritas = 0;
    bool hayAlguno = true;
    while (hayAlguno)
    {
        // buscar el archivo con el menor idMozo entre los que aun tienen registros
        int minPos = -1;
        for (int i = 0; i < cantFechas; i++)
        {
            if (hayMas[i])
            {
                if (minPos == -1 || actuales[i].idMozo < actuales[minPos].idMozo)
                {
                    minPos = i;
                }
            }
        }

        if (minPos == -1)
        {
            hayAlguno = false; // todos los archivos se agotaron
        }
        else
        {
            fwrite(&actuales[minPos], sizeof(Comanda), 1, fSemana);
            cantEscritas++;

            // avanzar en el archivo del que tomamos el registro
            if (fread(&actuales[minPos], sizeof(Comanda), 1, archivos[minPos]) != 1)
            {
                hayMas[minPos] = false;
                fclose(archivos[minPos]);
                archivos[minPos] = NULL;
            }
        }
    }
    fclose(fSemana);

    if (cantEscritas == 0)
    {
        cout << "No se encontraron comandas para la semana. Fin." << endl;
        return 0;
    }

    cout << "Archivo semanal creado: " << archivoSemana << endl;

    // 7) Mostrar el contenido para verificar
    mostrarArchivoSemana(archivoSemana);

    return 0;
}
