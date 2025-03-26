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
    
    char *arr[] = {"Negro", "Marron", "Rojo", "Naranja", "Amarillo", 
                    "Verde", "Azul", "Violeta", "Gris", "Blanco"};
    int n = sizeof(arr) / sizeof(arr[0]);

   
    qsort(arr, n, sizeof(char *), comparar);

    
    for (int i = 0; i < n; i++) {
        if (i == n - 1) {
            printf("%s", arr[i]); 
        } else {
            printf("%s\\ ", arr[i]); 
        }
    }
    printf("\n"); 

    return 0;
}