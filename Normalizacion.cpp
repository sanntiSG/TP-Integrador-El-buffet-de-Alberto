#include <iostream>
#include <cstring>
#include <cstdio>

using namespace std;

//  normalizacion.cpp  —  TP "El buffet de Alberto"
//
//  1) Crear mozos.dat a partir de comandas_historicas.dat
//  2) Separar las comandas por día en comandas_dd-mm-aaaa.dat
//     (con idMozo en lugar de nombre, ordenadas por mozo)
//  3) Actualizar inventario.dat descontando lo vendido

struct ComandaHistorica {
    char  fecha[11];        // "DD-MM-AAAA"
    char  nombreMozo[50];
    int   codigoProducto;
    int   cantidad;
    float comision;
};

struct Mozo {
    int   idMozo;
    char  nombre[50];
    float totalComision;
    char password[20];
};

struct Comanda {
    int   idMozo;
    int   codigoProducto;
    int   cantidad;
    float comision;
};

struct Producto {
    int   codigo;
    char  descripcion[50];
    float precio;
    int   stockActual;
};
// Rutas centralizadas para trabajar mas ordenado
const char* RUTA_COM_HIST      = "../datos/comandas_historicas.dat";
const char* RUTA_INV       = "../datos/inventario.dat";
const char* RUTA_MOZOS     = "../datos/mozos.dat";//nombre que quiera ya que sera nuevo archivo

const int K = 5; //numero al alzar para encriptar password

//punto 1: antes de crear los mozos con sus ID, debemos convertir ese id a texto para ser encriptado
void numeroATexto(int numero, char* destino)
{
    if (numero == 0)
    {
        destino[0] = '0';
        destino[1] = '\0';
        return;
    }

    char temp[20];
    int i = 0;

    while (numero > 0)
    {
        temp[i] = '0' + (numero % 10);
        numero = numero / 10;
        i++;
    }

    int j = 0;
    for (int k = i - 1; k >= 0; k--)
    {
        destino[j] = temp[k];
        j++;
    }
    destino[j] = '\0';
}

//encriptamos la clave
void encriptarClave(const char* original, char* destino)
{
    int i = 0;
    while (original[i] != '\0' && i < 19)
    {
        destino[i] = original[i] + K;
        i++;
    }
    destino[i] = '\0';
}

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

//punto 1: Crear mozos.dat ordenado
void CrearMozos(const char* nombre);
void CrearMozos(const char* nombre)
{
    FILE *f = fopen(nombre, "rb");
    if(f == NULL)
    {
        cout << "No se pudo abrir comandas historicas.dat\n";
        return;
    }

    //Creamos un nuevo archivo (mozos.dat) mientras leemos comandas historicas y le damos su ubicacion:
    FILE *Fmozos = fopen(RUTA_MOZOS, "wb"); //ruta y permisos
    if (Fmozos == NULL)
    {
        cout << "No se pudo crear mozos.dat\n";
        fclose(f);
        return;
    }
    //Cargamos todas las comandas en un array para después ordenarlas por nombre de mozo (corte de control)
    ComandaHistorica arrDeComandas[10000];
    int cantidad = 0;
    while(cantidad < 10000 && fread(&arrDeComandas[cantidad], sizeof(ComandaHistorica), 1, f) == 1)
    {
        cantidad ++;
    }
    fclose(f);//ya leimos todo, cerramos

    //ordenamiento burbuja
    for(int i = 0; i < cantidad - 1; i++)
    {
        for(int j = i + 1; j < cantidad; j ++)
        {
            if(strcasecmp(arrDeComandas[i].nombreMozo, arrDeComandas[j].nombreMozo) > 0)
            {
                ComandaHistorica aux = arrDeComandas[i];
                arrDeComandas[i] = arrDeComandas[j];
                arrDeComandas[j] = aux; 
            }
        }
    }
    //corte de control
    int i = 0; //para el corte
    int id = 1; //para asignar un ID a cada mozo (ya estando todo el archivo ordenado por nombre)
    while(i < cantidad)
    {
        char NombreActual [50];
        strcpy(NombreActual, arrDeComandas[i].nombreMozo);

        float totalComisiones = 0;

        while(i < cantidad && strcasecmp(NombreActual, arrDeComandas[i].nombreMozo) == 0) //si son iguales
        {
            totalComisiones = totalComisiones + arrDeComandas[i].comision;
            i++;
        }
        //mientras siga siendo el mismo mozo le sumamos su comision y ahora le asignamos un ID
        Mozo mozoActual; //creamos una variable de tipo Mozo (con todos los artributos de su struct)
        mozoActual.idMozo = id; //ahora esa variable vale = ID (en la primer vuelta vale 1 xq eso declaramos arriba)
        strcpy(mozoActual.nombre, NombreActual); //copiamos el nombre actual ordenado de arriba y lo ponemos en la variable del nombre del mozo
        mozoActual.totalComision = totalComisiones; //lo mismo pero con su comision

        //password inicial = id como texto, ya encriptado
        char idTexto[20];
        numeroATexto(id, idTexto);
        encriptarClave(idTexto, mozoActual.password);

        id++;

        fwrite(&mozoActual, sizeof(Mozo), 1, Fmozos); //escribimos toda la data en Fmozos (el archivo nuevo)
    } //cerramos el while
    fclose(Fmozos); //cerramos el nuevo archivo cuando terminamos todos los trabajos sobre el, por eso al final
    cout << "Mozos.dat creado correctamente" << endl;
}

//PUNTO 2: Buscar ID de mozo dentro de mozosBYsanti.dat

int buscarIDdeMozo(const char* nombreBuscado)
{
    FILE *f = fopen(RUTA_MOZOS, "rb");
    if (f == NULL)
    {
        return -1;
    }
    Mozo r;
    while(fread(&r, sizeof(Mozo), 1, f) == 1)
    {
        if(strcasecmp(nombreBuscado, r.nombre) == 0) //si el nombre buscado es == a el nombre del mozo devuelve su ID
        {
            fclose(f);
            return r.idMozo;
        }
    }
    fclose(f);
    return -1; //no lo encontro o no coincide no son iguales los nombres
}

//solo creamos la funcion para luego ser utilizada en otra funcion
//llamaremos a buscarIDdeMozo cuando lo querramos unir con cada venta de ese mismo mozo (hay que ordenar) otra vez comanda historica por nombre, y cuando estemos en el ciclo y vayamos uno por uno en nombre actual, usaremos ese nombre actual para llamar a BuscarIDdeMozo y ahi hacemos que ese nombre sea usado por buscarIDdeMozo y ella hara el trabajo de buscar el ID en base a el nombre que la otra funcion le haya pasado en el ciclo de NombreActual

void separarYcrearComandaXdia(const char* RUTA_COM_HIST);
void separarYcrearComandaXdia(const char* RUTA_COM_HIST)
{
    FILE *f = fopen(RUTA_COM_HIST, "rb");
    if(f == NULL)
    {
        return;
    }
    //Cargamos todas las comandas en un array para después ordenarlas por nombre de mozo (corte de control)
    ComandaHistorica arrDeComandas[10000];
    int cantidad = 0;
    //mientras la cantidad sea menor que 10000 y puedas leer el arr por cada iteracion, puedas leer el tamaño de 1 registro del archivo f entonces cantidad++
    while(cantidad < 10000 && fread(&arrDeComandas[cantidad], sizeof(ComandaHistorica), 1, f) == 1)
    {
        cantidad++;
    }
    fclose(f); //ya leimos todo, cerramos

    //ordenamos
    for(int i = 0; i < cantidad - 1; i++)
    {
        for(int j = i + 1; j < cantidad; j++)
        {
            if(strcasecmp(arrDeComandas[i].nombreMozo, arrDeComandas[j].nombreMozo) > 0)
            {
                ComandaHistorica aux = arrDeComandas[i];
                arrDeComandas[i] = arrDeComandas[j];
                arrDeComandas[j] = aux;
            }
        }
    }
    //corte de control pero no para agrupar nombre y calcular comisiones, sino para agrupar por nombre, llamar a la funcion buscarID y que ella busque por nombre en cada iteracion, luego a ese nombre actual le asigna su ID correspondiente
    int i = 0;
    while(i < cantidad) //mientras i < cantidad
    {
        char NombreActual[50];
        strcpy(NombreActual, arrDeComandas[i].nombreMozo);
        //vas a buscar con la funcion buscarID el nombre de cada mozo y le mostras su ID
        int IDmozo = buscarIDdeMozo(NombreActual); //llamamos a la funcion anteriormente creada y le pasamos el nombre actual que hay en cada iteracion, y aqui busca en el array de comandas ya ordenado el nombre actual
        while(i < cantidad && strcasecmp(arrDeComandas[i].nombreMozo, NombreActual) == 0)
        {
            //si llego aqui es xq i < cant, busco un nombre y su id y este coincide con el nombre actual de cada iteracion
            if(IDmozo != -1) //sabemos que los IDs comienzan con 1, es por las dudas el IF
            {
                Comanda c; //creamos una variable c de tipo comada con sus atributos del struct y mostramos

                c.idMozo = IDmozo;
                c.codigoProducto = arrDeComandas[i].codigoProducto;
                c.cantidad = arrDeComandas[i].cantidad;
                c.comision = arrDeComandas[i].comision;

                //se envio mail preguntando al profe el 05/10/26 a las 19hs
                //armamos el nombre del archivo del dia sin sprintf
                char archivoDia[64];
                strcpy(archivoDia, "../datos/comandas_");
                agregarTexto(archivoDia, arrDeComandas[i].fecha);
                agregarTexto(archivoDia, ".dat");

                FILE* fDia = fopen(archivoDia, "ab");
                if (fDia != NULL)
                {
                    fwrite(&c, sizeof(Comanda), 1, fDia);
                    fclose(fDia);
                }
            }
            i++;
        }
    }
    cout << "Comandas por dia generadas." << endl;
}
//PUNTO 2: ordenar cada archivo de día por idMozo
void OrdenadorDeArchivos(const char* nombre) //tiene la logica para ordenar cada archivo de comada por dia
{
    FILE* f = fopen(nombre, "rb");
    if (f == NULL) return;
    //carga por array, ordena el archivo que le pasemos por ID
    Comanda comandas[10000];
    int cantidad = 0;

    while (cantidad < 10000 && fread(&comandas[cantidad], sizeof(Comanda), 1, f) == 1)
    {
        cantidad++;
    }
    fclose(f);

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
    //vuelve a abrir el mismo archivo que le pasamos, y lo reescribe ordenado, si estaba: id=1 [datos] id=2 [datos] id = 1 [datos] ahora esta: id=1 [datos] id=1 [datos] id=2 [datos] 
    f = fopen(nombre, "wb");
    if (f == NULL) return;

    for (int i = 0; i < cantidad; i++)
    {
        fwrite(&comandas[i], sizeof(Comanda), 1, f);
    }
    fclose(f);
}

void OrganizadorDeOrdenador(const char* nombre) //le dice a ordenadorDeArchivos que archivo ordenar
{
    FILE* f = fopen(RUTA_COM_HIST, "rb");
    if (f == NULL) return;

    char fechas[100][11];
    int cantFechas = 0;

    ComandaHistorica ch;

    while (fread(&ch, sizeof(ComandaHistorica), 1, f) == 1)
    {
        bool yaEsta = false;
        for (int i = 0; i < cantFechas; i++)
        {
            if (strcmp(fechas[i], ch.fecha) == 0)
            {
                yaEsta = true;
                break;
            }
        }

        if (!yaEsta && cantFechas < 100)
        {
            strcpy(fechas[cantFechas], ch.fecha);
            cantFechas++;
        }
    }
    fclose(f);

    for (int i = 0; i < cantFechas; i++)
    {
        //armamos el nombre del archivo del dia
        char archivoDia[64];
        strcpy(archivoDia, "../datos/comandas_");
        agregarTexto(archivoDia, fechas[i]);
        agregarTexto(archivoDia, ".dat");
        OrdenadorDeArchivos(archivoDia);
    }

    cout << "Archivos de dia ordenados por mozo." << endl;
}

//punto 3: Actualizar inventario descontando lo que se vendio con las comandas historicas

//creamos una busqueda binaria ya que inventario.dat esta ordenado por codigo de producto
int buscarProducto(Producto arrDeProductos[], int len, int codigoBuscado)
{
    int inicio = 0;
    int final = len -1;
    while(inicio <= final){
        int medio = (inicio + final) / 2;

        if(arrDeProductos[medio].codigo == codigoBuscado)
        {
            return medio;
        }
        else if(arrDeProductos[medio].codigo < codigoBuscado)
        {
            inicio = medio + 1; //pasa el siguiente
        }
        else{
            final = medio - 1;
        }
    }
    return -1;
}

void actualizarInventario(const char* RUTA_COM_HIST, const char* RUTA_INV);
void actualizarInventario(const char* RUTA_COM_HIST, const char* RUTA_INV)
{
    // 1) Leer el inventario
    FILE* fInv = fopen(RUTA_INV, "rb");
    if (fInv == NULL)
    {
        cout << "No se pudo abrir el inventario." << endl;
        return;
    }

    Producto productos[10000];
    int cantProductos = 0;

    while (cantProductos < 10000 && fread(&productos[cantProductos], sizeof(Producto), 1, fInv) == 1)
    {
        cantProductos++;
    }
    fclose(fInv);

    // 2) Descontar stock según las comandas históricas
    FILE* fHist = fopen(RUTA_COM_HIST, "rb");
    if (fHist == NULL) return;

    ComandaHistorica s;
    while (fread(&s, sizeof(ComandaHistorica), 1, fHist) == 1)
    {
        int pos = buscarProducto(productos, cantProductos, s.codigoProducto);

        if (pos != -1)
        {
            productos[pos].stockActual -= s.cantidad;

            if (productos[pos].stockActual < 0)
            {
                productos[pos].stockActual = 0;
            }
        }
    }
    fclose(fHist);

    // 3) Escribir el inventario actualizado
    FILE* fNuevo = fopen(RUTA_INV, "wb");
    if (fNuevo == NULL)
    {
        cout << "No se pudo escribir el inventario" << endl;
        return;
    }

    for (int i = 0; i < cantProductos; i++)
    {
        fwrite(&productos[i], sizeof(Producto), 1, fNuevo);
    }
    fclose(fNuevo);

    cout << "Inventario actualizado escrito en " << RUTA_INV << endl;
}

int main()
{
    //si mozos.dat ya existe, ya se normalizo (evita duplicar comandas y descontar el stock dos veces)
    FILE *fExiste = fopen(RUTA_MOZOS, "rb");
    if (fExiste != NULL)
    {
        fclose(fExiste);
        cout << "Ya se normalizo antes (existe mozos.dat). Borrar para volver a normalizar." << endl;
        return 0;
    }

    CrearMozos(RUTA_COM_HIST);
    separarYcrearComandaXdia(RUTA_COM_HIST);
    OrganizadorDeOrdenador(RUTA_COM_HIST);
    actualizarInventario(RUTA_COM_HIST, RUTA_INV);

    return 0;
}