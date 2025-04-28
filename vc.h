#pragma once
/**
 * @file vc.h
 * @brief Biblioteca de processamento de imagem
 * @author [Seu Nome]
 * @date 2024
 */

#ifndef VC_H
#define VC_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>

    // Estrutura para armazenar uma imagem
    typedef struct {
        unsigned char* data;    // Dados da imagem
        int width;             // Largura da imagem
        int height;            // Altura da imagem
        int channels;          // Número de canais (1=binário, 3=RGB)
        int levels;            // Número de níveis de cinzento
        int bytesperline;      // Bytes por linha
    } IVC;

    // Estrutura para armazenar informações de uma moeda
    typedef struct {
        int type;              // Tipo de moeda (1, 2, 5, 10, 20, 50 cent. ou 1, 2 euros)
        int area;              // Área em pixels
        int perimeter;         // Perímetro em pixels
        int x;                 // Coordenada x do centroide
        int y;                 // Coordenada y do centroide
        int min_x;             // Coordenada x mínima da caixa delimitadora
        int min_y;             // Coordenada y mínima da caixa delimitadora
        int max_x;             // Coordenada x máxima da caixa delimitadora
        int max_y;             // Coordenada y máxima da caixa delimitadora
        float circularity;     // Circularidade da moeda
    } CoinInfo;

    // Estrutura para armazenar estatísticas das moedas
    typedef struct {
        int total_coins;       // Total de moedas
        int coins_by_type[8];  // Contagem por tipo (0=1c, 1=2c, 2=5c, 3=10c, 4=20c, 5=50c, 6=1€, 7=2€)
        CoinInfo* coins;       // Array com informações de cada moeda
        int max_coins;         // Tamanho máximo do array coins
    } CoinStats;

    // Estrutura para armazenar informações do vídeo
    typedef struct {
        FILE* file;            // Ponteiro para o arquivo de vídeo
        int width;             // Largura do frame
        int height;            // Altura do frame
        int fps;               // Frames por segundo
        int total_frames;      // Número total de frames
        int current_frame;     // Frame atual
    } IVC_VIDEO;

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    //                    PROTÓTIPOS DE FUNÇÕES
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

    /**
     * @brief Cria uma nova imagem
     * @param width Largura da imagem
     * @param height Altura da imagem
     * @param channels Número de canais
     * @param levels Número de níveis de cinzento
     * @return Ponteiro para a nova imagem ou NULL em caso de erro
     */
    IVC* vc_image_new(int width, int height, int channels, int levels);

    /**
     * @brief Liberta a memória de uma imagem
     * @param image Ponteiro para a imagem
     */
    void vc_image_free(IVC* image);

    /**
     * @brief Lê uma imagem de um ficheiro
     * @param filename Nome do ficheiro
     * @return Ponteiro para a imagem ou NULL em caso de erro
     */
    IVC* vc_read_image(const char* filename);

    /**
     * @brief Escreve uma imagem para um ficheiro
     * @param filename Nome do ficheiro
     * @param image Ponteiro para a imagem
     * @return 1 em caso de sucesso, 0 em caso de erro
     */
    int vc_write_image(const char* filename, IVC* image);

    // FUNÇÕES: TRANSFORMAÇÕES DE IMAGENS
    int vc_gray_negative(IVC* srcdst);
    int vc_rgb_to_gray(IVC* src, IVC* dst);
    int vc_rgb_to_hsv(IVC* src, IVC* dst);
    int vc_hsv_segmentation(IVC* src, IVC* dst, int hmin, int hmax, int smin, int smax, int vmin, int vmax);

    // FUNÇÕES: OPERAÇÕES SOBRE CANAIS DE COR
    int vc_rgb_get_red_channel(IVC* src, IVC* dst);
    int vc_rgb_get_green_channel(IVC* src, IVC* dst);
    int vc_rgb_get_blue_channel(IVC* src, IVC* dst);

    // FUNÇÕES: BINARIZAÇÕES
    int vc_gray_to_binary(IVC* src, IVC* dst, int threshold);
    int vc_gray_to_binary_global_mean(IVC* src, IVC* dst);

    // FUNÇÕES: OPERAÇÕES MORFOLÓGICAS
    int vc_binary_dilate(IVC* src, IVC* dst, int kernel_size);
    int vc_binary_erode(IVC* src, IVC* dst, int kernel_size);

    /**
     * @brief Realiza a rotulagem de uma imagem binária
     * @param src Imagem de origem (binária)
     * @param labels Array para armazenar os rótulos
     * @return Número de objetos encontrados
     */
    int vc_binary_labeling(IVC* src, int* labels);

    /**
     * @brief Analisa os objetos numa imagem
     * @param image Imagem de origem
     * @param labels Array com os rótulos
     */
    void vc_analyze_objects(IVC* image, int* labels);

    /**
     * @brief Analisa as moedas em uma imagem binária
     * @param image Imagem binária de origem
     * @param labels Array com os rótulos
     * @param stats Estrutura para armazenar as estatísticas das moedas
     */
    void vc_analyze_coins(IVC* image, int* labels, CoinStats* stats);

    // Funções de processamento de vídeo
    IVC_VIDEO* vc_video_open(const char* filename);
    void vc_video_close(IVC_VIDEO* video);
    int vc_video_read_frame(IVC_VIDEO* video, IVC* frame);
    int vc_video_get_frame_count(IVC_VIDEO* video);
    int vc_video_get_fps(IVC_VIDEO* video);

    // Funções de processamento de vídeo
    int vc_process_video(const char* filename);

    // Funções de análise de moedas
    void vc_init_coin_stats(CoinStats* stats);
    void vc_free_coin_stats(CoinStats* stats);
    int vc_identify_coin_type(int area, float circularity);

#ifdef __cplusplus
}
#endif

#endif
