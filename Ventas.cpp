#include <iostream>
#include <cstring>
#include <cstdio>

using namespace std;

//  ventas.cpp  —  TP "El buffet de Alberto"
//
//  Pide la fecha del día una sola vez, abre (o crea) la planilla
//  de ese día. Después, por cada venta: pide el mozo y su clave,
//  los valida, busca el precio, calcula la comisión y agrega la
//  venta a la planilla del día. Al terminar, la deja ordenada
//  por mozo.

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

struct Producto {
    int   codigo;
    char  descripcion[50];
    float precio;
    int   stockActual;
};

// Rutas organizadas
const char* RUTA_INV       = "../datos/inventario.dat";
const char* RUTA_MOZOS     = "../datos/mozos.dat";

const int K = 5; //numero al azar para encriptar password
const float TASA_COMISION = 0.10f; //la comision de cada venta es el 10% de lo vendido

//funciones reutilizadas de normalizacion.cpp

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

//buscamos el mozo por su ID dentro de mozos.dat
bool buscarMozoPorId(int idBuscado, Mozo& resultado)
{
    FILE *f = fopen(RUTA_MOZOS, "rb");
    if (f == NULL)
    {
        return false;
    }

    Mozo r;
    while (fread(&r, sizeof(Mozo), 1, f) == 1)
    {
        if (r.idMozo == idBuscado)
        {
            resultado = r;
            fclose(f);
            return true;
        }
    }

    fclose(f);
    return false; //no lo encontro
}

//  Busqueda binaria en inventario.dat; devuelve la posicion (-1 si no existe) y carga 'resultado'.
//  Busqueda binaria en inventario.dat; devuelve la posicion (-1 si no existe) y carga 'resultado'.
int buscarProductoEnArchivo(int codigoBuscado, Producto& resultado)
{
    FILE* fInv = fopen(RUTA_INV, "rb");
    if (fInv == NULL) return -1;

    // Contamos cuantos productos hay leyendo el archivo entero
    long cantProductos = 0;
    Producto aux;
    while (fread(&aux, sizeof(Producto), 1, fInv) == 1)
    {
        cantProductos++;
    }
    fseek(fInv, 0, SEEK_SET);   // volver al principio

    long pri = 0;
    long ult = cantProductos - 1;
    long pos = -1;

    while (pri <= ult && pos == -1)
    {
        long med = (pri + ult) / 2;
        fseek(fInv, med * sizeof(Producto), SEEK_SET);
        Producto p;
        fread(&p, sizeof(Producto), 1, fInv);

        if (p.codigo == codigoBuscado)
        {
            pos = med;
            resultado = p;
        }
        else if (p.codigo < codigoBuscado)
        {
            pri = med + 1;
        }
        else
        {
            ult = med - 1;
        }
    }
    fclose(fInv);
    return pos;
}

//  Edicion in situ: descuenta 'cantidad' del stock del registro en la posicion 'pos'
void actualizarStockEnArchivo(long pos, int cantidad)
{
    FILE* fInv = fopen(RUTA_INV, "rb+");
    if (fInv == NULL) return;

    fseek(fInv, pos * sizeof(Producto), SEEK_SET);
    Producto p;
    fread(&p, sizeof(Producto), 1, fInv);

    p.stockActual -= cantidad;
    if (p.stockActual < 0)
    {
        p.stockActual = 0;
    }

    // el puntero quedo justo despues del registro leido, retrocedemos y sobreescribimos
    fseek(fInv, -(long)sizeof(Producto), SEEK_CUR);
    fwrite(&p, sizeof(Producto), 1, fInv);
    fclose(fInv);
}

//  Login del mozo
//  Pide id y clave, valida usando mozos.dat.
//  Devuelve el id del mozo logueado, o -1 si fallo.
int loginMozo()
{
    int idIngresado;
    char claveIngresada[20];

    cout << "Ingrese el ID del mozo (0 para cancelar): ";
    cin >> idIngresado;

    if (idIngresado == 0)
    {
        return -1; //el usuario cancelo
    }

    // 1) Buscar el mozo por id
    Mozo mozoEncontrado;
    if (!buscarMozoPorId(idIngresado, mozoEncontrado))
    {
        cout << "ERROR: el mozo con ID " << idIngresado << " no existe.\n";
        return -1;
    }

    // 2) Pedir la clave
    cout << "Ingrese la clave del mozo: ";
    cin >> claveIngresada;

    // 3) Encriptar la clave tipeada y compararla con la guardada
    char claveEncriptada[20];
    encriptarClave(claveIngresada, claveEncriptada);

    if (strcasecmp(claveEncriptada, mozoEncontrado.password) != 0)
    {
        cout << "ERROR: la clave no coincide.\n";
        return -1;
    }

    cout << "Login correcto. Bienvenido/a " << mozoEncontrado.nombre << ".\n";
    return mozoEncontrado.idMozo;
}

//  Ordenar un archivo de dia por idMozo
void OrdenarArchivoDia(const char* ruta)
{
    FILE *f = fopen(ruta, "rb");
    if (f == NULL) return;

    Comanda comandas[10000]; //cargar en array para ordenar por id del mozo
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

    f = fopen(ruta, "wb");
    if (f == NULL) return;

    for (int i = 0; i < cantidad; i++)
    {
        fwrite(&comandas[i], sizeof(Comanda), 1, f);
    }
    fclose(f);
}

//  Cargar todas las ventas del dia
void cargarVentasDelDia(const char* fecha)
{
    // 1) Armar el nombre del archivo del dia
    char archivoDia[64];
    strcpy(archivoDia, "../datos/comandas_");
    agregarTexto(archivoDia, fecha);
    agregarTexto(archivoDia, ".dat");

    // 2) Loop de carga mientras se ejecuta
    char seguir = 's';

    while (seguir == 's' || seguir == 'S')
    {
        cout << "\nNUEVA VENTA\n";

        // 2.1) Login del mozo
        int idMozo = loginMozo();

        if (idMozo == -1)
        {
            cout << "No se pudo validar el mozo. Venta cancelada.\n";
        }
        else
        {
            // 2.2) Pedir codigo de producto
            int codigoProducto = 0;
            cout << "Ingrese el codigo del producto: ";
            cin >> codigoProducto;

            Producto producto;
            int posProducto = buscarProductoEnArchivo(codigoProducto, producto);

            if (posProducto == -1)
            {
                cout << "ERROR: el producto " << codigoProducto << " no existe.\n";
            }
            else
            {
                // 2.3) Pedir cantidad
                int cantidad;
                cout << "Ingrese la cantidad: ";
                cin >> cantidad;

                // 2.4) Validar stock (caso raro: producto sin stock)
                if (cantidad <= 0)
                {
                    cout << "ERROR: la cantidad tiene que ser mayor a 0.\n";
                }
                else if (producto.stockActual < cantidad)
                {
                    cout << "ERROR: no hay stock suficiente. Disponible: " << producto.stockActual << "\n";
                }
                else
                {
                    // 2.5) Calcular comision
                    float comision = cantidad * producto.precio * TASA_COMISION;

                    // 2.6) Armar la comanda
                    Comanda c;
                    c.idMozo         = idMozo;
                    c.codigoProducto = codigoProducto;
                    c.cantidad       = cantidad;
                    c.comision       = comision;

                    // 2.7) Escribir en el archivo del dia
                    FILE *fDia = fopen(archivoDia, "ab"); //agregar binario si existe agrega al final o crea archivo
                    if (fDia == NULL)
                    {
                        //si no existe, lo creamos
                        fDia = fopen(archivoDia, "wb");
                    }

                    if (fDia != NULL)
                    {
                        fwrite(&c, sizeof(Comanda), 1, fDia);
                        fclose(fDia);

                        // 2.8) Descontar stock directamente en el archivo (edicion in situ)
                        actualizarStockEnArchivo(posProducto, cantidad);

                        cout << "Venta cargada. Comision: $" << comision << "\n";
                    }
                    else
                    {
                        cout << "No se pudo escribir en el archivo del dia.\n";
                    }
                }
            }
        }

        // 2.9) Preguntar si sigue
        cout << "Cargar otra venta? (s/n): ";
        cin >> seguir;
    }

    // 3) Ordenar el archivo del dia por idMozo
    OrdenarArchivoDia(archivoDia);

    cout << "\nCarga finalizada. Archivo " << archivoDia << " ordenado por mozo.\n";
}

//  main
int main()
{
    char fecha[11];

    cout << "Ingrese la fecha del dia (DD-MM-AAAA): ej: 02-06-2025 ";
    cin >> fecha;

    cargarVentasDelDia(fecha);

    return 0;
}
