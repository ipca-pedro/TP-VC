/**
 * @file main.c
 * @brief Programa para contar moedas em um video
 * @author [Seu Nome]
 * @date 2024
 */

#include <stdio.h>
#include <stdlib.h>
#include "vc.h"

int main(int argc, char* argv[]) {
    if (argc != 2) {
        printf("Uso: %s <video.mp4>\n", argv[0]);
        return 1;
    }

    printf("Iniciando processamento de video...\n");
    printf("Pressione 'ESC' para sair\n");
    printf("Use as trackbars para ajustar os parâmetros de detecção\n");

    // Processa o video
    if (!vc_process_video(argv[1])) {
        printf("Erro ao processar o video\n");
        return 1;
    }

    printf("Processamento finalizado com sucesso\n");
    return 0;
}