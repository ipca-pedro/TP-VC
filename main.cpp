/**
 * @file main.c
 * @brief Programa para contar moedas em um v�deo
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

    // Processa o v�deo
    if (!vc_process_video(argv[1])) {
        printf("Erro ao processar o v�deo\n");
        return 1;
    }

    return 0;
}