/***********************************************************/
/*           Programación para mecatrónicos               */
/*  Nombre:    Joel Eliu Guerrero                                  */
/*  Matricula: 2022-0073                                               */
/*  Seccion:   Miercoles                                              */
/*  Practica:  Asignacion 1                                     */
/*  Fecha:     26/02/2025                                              */                       
/* Link Practica: http://www.linka-a-la-practica          */
/***********************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    char *color;
    int codigo;
} ColorCodigo;


int compararPorColor(const void *a, const void *b) {
    return strcmp(((ColorCodigo *)a)->color, ((ColorCodigo *)b)->color);
}


int compararPorCodigo(const void *a, const void *b) {
    return ((ColorCodigo *)a)->codigo - ((ColorCodigo *)b)->codigo;
}

int main() {
    
    char *arr[] = {"Negro", "Marron", "Rojo", "Naranja", "Amarillo", 
                    "Verde", "Azul", "Violeta", "Gris", "Blanco"};
    int code[] = {123, 520, 178, 963, 258, 120, 852, 412, 254, 456};
    int n = sizeof(arr) / sizeof(arr[0]);

    
    ColorCodigo coloresCodigos[n];
    for (int i = 0; i < n; i++) {
        coloresCodigos[i].color = arr[i];
        coloresCodigos[i].codigo = code[i];
    }

    
    qsort(coloresCodigos, n, sizeof(ColorCodigo), compararPorColor);

    
    printf("Ordenado por texto:\n");
    for (int i = 0; i < n; i++) {
        printf("%d, %s\n", coloresCodigos[i].codigo, coloresCodigos[i].color);
    }

    
    qsort(coloresCodigos, n, sizeof(ColorCodigo), compararPorCodigo);

    
    printf("\nOrdenado por código:\n");
    for (int i = 0; i < n; i++) {
        printf("%d, %s\n", coloresCodigos[i].codigo, coloresCodigos[i].color);
    }

    return 0;
}