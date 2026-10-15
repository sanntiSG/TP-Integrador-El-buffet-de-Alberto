#include <iostream>
#include <cstring>
#include <cstdio>

using namespace std;

//  cierre.cpp  —  TP "El buffet de Alberto"
//
//  Junta las planillas de todos los días de la semana en una sola
//  planilla semanal (comandas_semana_sX-mm.dat), ordenada por mozo.
//
//  Lee: comandas_dd-mm-aaaa.dat (varios)
//  Genera: comandas_semana_sX-mm.dat

struct Comanda {
    int   idMozo;
    int   codigoProducto;
    int   cantidad;
    float comision;
};

// Cantidad de días en la semana (para saber cuántos archivos leer)
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
int pedirFechas(char fechas[][11], int len) //el len seria nuestro tope
{
    int cantFechas;
    cout << "Ingrese la cantidad de dias de la semana (max " << len << "): ";
    cin >> cantFechas;

    for (int i = 0; i < cantFechas; i++)
    {
        cout << "Ingrese la fecha del dia " << i + 1 << " (DD-MM-AAAA): ";
        cin >> fechas[i];
    }

    return cantFechas;
}

//ordenamiento burbuja por idMozo
void ordenarComandas(Comanda comandas[], int cantidad)
{
    for (int i = 0; i < cantidad - 1; i++)
    {
        for (int j = i + 1; j < cantidad; j++)
        {
            if (comandas[i].idMozo > comandas[j].idMozo)
            {
                Comanda aux = comandas[i];
                comandas[i] = comandas[j];
                comandas[j] = aux;
            }
        }
    }
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
        cout << "#" << nro
             << " | idMozo=" << c.idMozo
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

    // 5) Ir leyendo cada archivo de dia y volcando todo en un array grande
    Comanda todasLasComandas[10000];
    int cantComandas = 0;

    for (int i = 0; i < cantFechas; i++)
    {
        // armamos el nombre del archivo del dia
        char archivoDia[64];
        strcpy(archivoDia, "../datos/comandas_");
        agregarTexto(archivoDia, fechas[i]);
        agregarTexto(archivoDia, ".dat");

        FILE* fDia = fopen(archivoDia, "rb");

        if (fDia == NULL)
        {
            cout << "AVISO: no existe la planilla del dia "
                << fechas[i] << ". Se omite.\n";
        }
        else
        {
            // leemos todas las comandas de ese dia
            Comanda c;
            while (fread(&c, sizeof(Comanda), 1, fDia) == 1)
            {
                todasLasComandas[cantComandas] = c;
                cantComandas++;
            }

            fclose(fDia);
        }
    }

    if (cantComandas == 0)
    {
        cout << "No se encontraron comandas para la semana. Fin." << endl;
        return 0;
    }

    // 6) Ordenar todo por idMozo
    ordenarComandas(todasLasComandas, cantComandas);

    // 7) Escribir el archivo semanal
    FILE* fSemana = fopen(archivoSemana, "wb");
    if (fSemana == NULL)
    {
        cout << "No se pudo crear el archivo semanal." << endl;
        return 0;
    }

    for (int i = 0; i < cantComandas; i++)
    {
        fwrite(&todasLasComandas[i], sizeof(Comanda), 1, fSemana);
    }
    fclose(fSemana);

    cout << "Archivo semanal creado: " << archivoSemana << endl;

    // 8) Mostrar el contenido para verificar
    mostrarArchivoSemana(archivoSemana);

    return 0;
}