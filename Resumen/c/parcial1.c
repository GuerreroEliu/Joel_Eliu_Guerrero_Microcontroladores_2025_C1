/***********************************************************/
/*           Programación para mecatrónicos               */
/*  Nombre:    Joel Eliu Guerrero                                  */
/*  Matricula: 2022-0073                                               */
/*  Seccion:   Miercoles                                              */
/*  Practica:  Parcial 1                                     */
/*  Fecha:     11/02/2025                                              */                       
/* Link Practica: http://www.linka-a-la-practica          */
/***********************************************************/

#include <stdio.h>

int main() {
    int arr[] = {9, 0, 7, 4, 2, 10};
    int n = sizeof(arr) / sizeof(arr[0]);
    int i, j, temp;

    printf("Arreglo original: \n");
    for (i = 0; i < n; i++) {
        printf("%d ", arr[i]);
    }
    printf("\n");

    // Algoritmo de ordenamiento burbuja
    for (i = 0; i < n-1; i++) {
        for (j = 0; j < n-i-1; j++) {
            if (arr[j] > arr[j+1]) {
                // Intercambiar elementos
                temp = arr[j];
                arr[j] = arr[j+1];
                arr[j+1] = temp;
            }
        }
    }

    printf("Arreglo ordenado: \n");
    for (i = 0; i < n; i++) {
        printf("%d ", arr[i]);
    }
    printf("\n");

    return 0;
}

