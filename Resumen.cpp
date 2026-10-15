#include <iostream>
#include <cstring>
#include <cstdio>

using namespace std;

//  resumen.cpp  —  TP "El buffet de Alberto"
//
//  Arma el resumen del cierre: por cada mozo, cuantos productos
//  vendio y cuanta comision le corresponde; y el total del buffet.
//
//  Lee: comandas_semana_sX-mm.dat
//  Genera: reporte por pantalla

struct Mozo {
    int   idMozo;
    char  nombre[50];
    float totalComision;
    char  password[20];
};

struct Comanda {
    int   idMozo;
    int   codigoProducto;
    int   cantidad;
    float comision;
};

// Ruta mas prolija y centrada
const char* RUTA_MOZOS = "../datos/mozos.dat";

//buscamos el nombre del mozo por su ID
bool buscarNombreMozo(int idBuscado, char* nombreDestino)
{
    FILE* f = fopen(RUTA_MOZOS, "rb");
    if (f == NULL)
    {
        return false;
    }

    Mozo r;
    while (fread(&r, sizeof(Mozo), 1, f) == 1)
    {
        if (r.idMozo == idBuscado)
        {
            strcpy(nombreDestino, r.nombre);
            fclose(f);
            return true;
        }
    }

    fclose(f);
    return false;
}

//recorremos el archivo semanal y mostramos su contenido por pantalla
void mostrarArchivoSemanal(const char* ruta)
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

    int len = 0;
    while (archivoSemana[len] != '\0') len++;
    int p = 0;
    while (semanaTexto[p] != '\0')
    {
        archivoSemana[len] = semanaTexto[p];
        len++;
        p++;
    }
    archivoSemana[len] = '-';
    len++;
    archivoSemana[len] = '\0';

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

    len = 0;
    while (archivoSemana[len] != '\0') len++;
    p = 0;
    while (mesTexto[p] != '\0')
    {
        archivoSemana[len] = mesTexto[p];
        len++;
        p++;
    }
    archivoSemana[len] = '.';
    archivoSemana[len + 1] = 'd';
    archivoSemana[len + 2] = 'a';
    archivoSemana[len + 3] = 't';
    archivoSemana[len + 4] = '\0';

    // 4) Mostrar el contenido del archivo semanal (para verificar)
    mostrarArchivoSemanal(archivoSemana);

    // 5) Cargar todas las comandas del archivo semanal
    FILE* f = fopen(archivoSemana, "rb");
    if (f == NULL)
    {
        cout << "No se pudo abrir " << archivoSemana << endl;
        return 0;
    }

    Comanda comandas[10000];
    int cantidad = 0;

    while (cantidad < 10000 && fread(&comandas[cantidad], sizeof(Comanda), 1, f) == 1)
    {
        cantidad++;
    }
    fclose(f);

    if (cantidad == 0)
    {
        cout << "El archivo semanal esta vacio." << endl;
        return 0;
    }

    // 6) Corte de control por idMozo
    //    Recorremos todo y mostramos un bloque por mozo.
    //    El "caso raro" del ultimo mozo se maneja bien porque el
    //    while externo corta cuando i llega a cantidad.
    int i = 0;
    int totalProductosBuffet = 0;

    cout << "\nRESUMEN DE LA SEMANA" << endl;

    while (i < cantidad)
    {
        int idActual = comandas[i].idMozo;

        //buscamos el nombre del mozo
        char nombreActual[50];
        if (!buscarNombreMozo(idActual, nombreActual))
        {
            strcpy(nombreActual, "(desconocido)");
        }

        //acumuladores de este mozo
        int   totalProductos = 0;
        float totalComision  = 0;

        //bucle interno: procesamos todas las comandas de este mozo
        while (i < cantidad && comandas[i].idMozo == idActual)
        {
            totalProductos += comandas[i].cantidad;
            totalComision  += comandas[i].comision;
            i++;
        }

        //mostramos el bloque del mozo
        cout << "\nMozo ID " << idActual << " - " << nombreActual << endl;
        cout << "  Productos vendidos: " << totalProductos << endl;
        cout << "  Comision a pagar: $" << totalComision << endl;

        //acumulamos al total del buffet
        totalProductosBuffet += totalProductos;
    }

    // 7) Total del buffet
    cout << "TOTAL DE PRODUCTOS DEL BUFFET: " << totalProductosBuffet << endl;
    return 0;
}