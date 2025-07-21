#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>

#define CHECKPOINT_FILE "checkpoints/checkpoint_rank%d.dat"
#define MAX_ITER 20
#define CHECKPOINT_INTERVAL 5
#define FAIL_AT_ITER 10

// Estructura para el estado del proceso
typedef struct {
    int iter;
    int partial_sum;
} State;

// Función para guardar checkpoint
void save_checkpoint(int rank, State *state) {
    char filename[100];
    sprintf(filename, CHECKPOINT_FILE, rank);
    FILE *fp = fopen(filename, "wb");
    if (fp != NULL) {
        fwrite(state, sizeof(State), 1, fp);
        fclose(fp);
        printf("[Proceso %d] Checkpoint guardado (iter: %d, suma: %d)\n", rank, state->iter, state->partial_sum);
        fflush(stdout);
    }
}

// Función para cargar checkpoint
bool load_checkpoint(int rank, State *state) {
    char filename[100];
    sprintf(filename, CHECKPOINT_FILE, rank);
    FILE *fp = fopen(filename, "rb");
    if (fp != NULL) {
        fread(state, sizeof(State), 1, fp);
        fclose(fp);
        printf("[Proceso %d] Checkpoint cargado (iter: %d, suma: %d)\n", rank, state->iter, state->partial_sum);
        fflush(stdout);
        return true;
    }
    return false;
}

int main(int argc, char** argv) {
    int rank, size;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    printf("Proceso %d iniciado\n", rank);
    fflush(stdout);

    State state = {0, 0};

    // Recuperar si hay checkpoint
    load_checkpoint(rank, &state);

    for (int i = state.iter; i < MAX_ITER; i++) {
        // Cómputo simulado
        state.partial_sum += rank + i;
        state.iter = i + 1;

        printf("[Proceso %d] Iteración %d, suma parcial: %d\n", rank, i, state.partial_sum);
        fflush(stdout);
        sleep(1); // Simular tiempo de cómputo

        // Simular fallo en una iteración
        if (i == FAIL_AT_ITER && rank == 1) {
            printf("[Proceso %d] Fallo simulado\n", rank);
            fflush(stdout);
            exit(1); // Fallo intencional
        }

        // Checkpoint coordinado
        if ((i + 1) % CHECKPOINT_INTERVAL == 0) {
            MPI_Barrier(MPI_COMM_WORLD); // Sincronización
            save_checkpoint(rank, &state);
        }
    }

    printf("[Proceso %d] Finalizó. Suma final: %d\n", rank, state.partial_sum);
    fflush(stdout);
    MPI_Finalize();
    return 0;
}
