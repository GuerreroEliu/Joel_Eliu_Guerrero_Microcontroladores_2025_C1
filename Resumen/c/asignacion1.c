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


int comparar(const void *a, const void *b) {
    return strcmp(*(const char **)a, *(const char **)b);
}

int main() {
    char mat[][20] = {"Negro", "Marron", "Rojo", "Naranja", "Amarillo", 
                      "Verde", "Azul", "Violeta", "Gris", "Blanco"};
    int n = sizeof(mat) / sizeof(mat[0]);

    char *arr[n];
    for (int i = 0; i < n; i++) {
        arr[i] = mat[i];
    }

    qsort(arr, n, sizeof(char *), comparar);

    for (int i = 0; i < n; i++) {
        printf("%s\n", arr[i]);
    }

    return 0;
}