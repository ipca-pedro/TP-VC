/**
 * @file vc.c
 * @brief Implementação das funções de processamento de imagem
 * @author [Seu Nome]
 * @date 2024
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include "vc.h"
#include <opencv2/opencv.hpp>

 // Funções auxiliares para leitura de imagens NetPBM
char* netpbm_get_token(FILE* file, char* tok, int len) {
    char* t;
    int c;

    for (;;) {
        while (isspace(c = getc(file)));

        if (c != '#') break;

        do c = getc(file);
        while ((c != '\n') && (c != EOF));

        if (c == EOF) break;
    }

    t = tok;

    if (c != EOF) {
        *t++ = c;
        while (!isspace(c = getc(file)) && (c != '#') && (c != EOF) && (t - tok < len - 1))
            *t++ = c;

        if (c == '#') ungetc(c, file);
    }

    *t = 0;

    return tok;
}

// Funções básicas de manipulação de imagem
IVC* vc_image_new(int width, int height, int channels, int levels) {
    IVC* image = (IVC*)malloc(sizeof(IVC));
    if (image == NULL) return NULL;

    image->width = width;
    image->height = height;
    image->channels = channels;
    image->levels = levels;
    image->bytesperline = width * channels;
    image->data = (unsigned char*)malloc(width * height * channels);

    if (image->data == NULL) {
        free(image);
        return NULL;
    }

    return image;
}

void vc_image_free(IVC* image) {
    if (image != NULL) {
        if (image->data != NULL) free(image->data);
        free(image);
    }
}

IVC* vc_read_image(const char* filename) {
    FILE* file = fopen(filename, "rb");
    if (file == NULL) return NULL;

    char token[100];
    int width, height, maxval;
    IVC* image = NULL;

    // Lê o cabeçalho PPM
    if (strcmp(netpbm_get_token(file, token, sizeof(token)), "P6") != 0) {
        fclose(file);
        return NULL;
    }

    // Lê largura e altura
    if (sscanf_s(netpbm_get_token(file, token, sizeof(token)), "%d", &width) != 1 ||
        sscanf_s(netpbm_get_token(file, token, sizeof(token)), "%d", &height) != 1 ||
        sscanf_s(netpbm_get_token(file, token, sizeof(token)), "%d", &maxval) != 1) {
        fclose(file);
        return NULL;
    }

    // Cria a imagem
    image = vc_image_new(width, height, 3, maxval);
    if (image == NULL) {
        fclose(file);
        return NULL;
    }

    // Lê os dados
    if (fread(image->data, 1, width * height * 3, file) != width * height * 3) {
        vc_image_free(image);
        fclose(file);
        return NULL;
    }

    fclose(file);
    return image;
}

int vc_write_image(const char* filename, IVC* image) {
    FILE* file = fopen(filename, "wb");
    if (file == NULL) return 0;

    // Escreve o cabeçalho PPM
    fprintf(file, "P6\n%d %d\n%d\n", image->width, image->height, image->levels);

    // Escreve os dados
    if (fwrite(image->data, 1, image->width * image->height * image->channels, file) !=
        image->width * image->height * image->channels) {
        fclose(file);
        return 0;
    }

    fclose(file);
    return 1;
}

// Conversão RGB para HSV
int vc_rgb_to_hsv(IVC* src, IVC* dst) {
    unsigned char* data_src = (unsigned char*)src->data;
    unsigned char* data_dst = (unsigned char*)dst->data;
    int width = src->width;
    int height = src->height;
    int bytesperline_src = src->bytesperline;
    int bytesperline_dst = dst->bytesperline;
    int channels_src = src->channels;
    int channels_dst = dst->channels;
    float r, g, b, hue, sat, val;
    float rgb_max, rgb_min;
    int i, size;

    // Verificação de erros
    if ((src->width <= 0) || (src->height <= 0) || (src->data == NULL)) return 0;
    if ((src->width != dst->width) || (src->height != dst->height)) return 0;
    if ((src->channels != 3) || (dst->channels != 3)) return 0;

    size = width * height * channels_src;

    for (i = 0; i < size; i += channels_src) {
        r = (float)data_src[i] / 255.0f;
        g = (float)data_src[i + 1] / 255.0f;
        b = (float)data_src[i + 2] / 255.0f;

        rgb_max = (r > g) ? ((r > b) ? r : b) : ((g > b) ? g : b);
        rgb_min = (r < g) ? ((r < b) ? r : b) : ((g < b) ? g : b);

        // Value
        val = rgb_max;
        if (val == 0.0f) {
            hue = 0.0f;
            sat = 0.0f;
        }
        else {
            // Saturation
            sat = (rgb_max - rgb_min) / rgb_max;

            if (sat == 0.0f) {
                hue = 0.0f;
            }
            else {
                // Hue
                if (rgb_max == r) {
                    hue = 60.0f * (0.0f + (g - b) / (rgb_max - rgb_min));
                }
                else if (rgb_max == g) {
                    hue = 60.0f * (2.0f + (b - r) / (rgb_max - rgb_min));
                }
                else {
                    hue = 60.0f * (4.0f + (r - g) / (rgb_max - rgb_min));
                }

                if (hue < 0.0f) hue = hue + 360.0f;
            }
        }

        // Conversão para 8 bits
        data_dst[i] = (unsigned char)((hue / 360.0f) * 255.0f);
        data_dst[i + 1] = (unsigned char)(sat * 255.0f);
        data_dst[i + 2] = (unsigned char)(val * 255.0f);
    }

    return 1;
}

// Segmentação HSV
int vc_hsv_segmentation(IVC* src, IVC* dst, int hmin, int hmax, int smin, int smax, int vmin, int vmax) {
    unsigned char* data_src = (unsigned char*)src->data;
    unsigned char* data_dst = (unsigned char*)dst->data;
    int width = src->width;
    int height = src->height;
    int bytesperline_src = src->bytesperline;
    int bytesperline_dst = dst->bytesperline;
    int channels_src = src->channels;
    int pos_src, pos_dst;
    int h, s, v;
    int x, y;

    // Verificação de erros
    if ((src->width <= 0) || (src->height <= 0) || (src->data == NULL)) return 0;
    if ((src->width != dst->width) || (src->height != dst->height)) return 0;
    if ((src->channels != 3) || (dst->channels != 1)) return 0;

    for (y = 0; y < height; y++) {
        for (x = 0; x < width; x++) {
            pos_src = y * bytesperline_src + x * channels_src;
            pos_dst = y * bytesperline_dst + x;

            h = data_src[pos_src];
            s = data_src[pos_src + 1];
            v = data_src[pos_src + 2];

            if ((h >= hmin && h <= hmax) &&
                (s >= smin && s <= smax) &&
                (v >= vmin && v <= vmax)) {
                data_dst[pos_dst] = 255;
            }
            else {
                data_dst[pos_dst] = 0;
            }
        }
    }

    return 1;
}

// Rotulagem de componentes conectados
int vc_binary_labeling(IVC* src, int* labels) {
    int width = src->width;
    int height = src->height;
    int bytesperline = src->bytesperline;
    unsigned char* data = (unsigned char*)src->data;
    int label = 1;
    int pos;
    int x, y;

    // Primeira passagem: atribuição de rótulos
    for (y = 1; y < height - 1; y++) {
        for (x = 1; x < width - 1; x++) {
            pos = y * bytesperline + x;

            if (data[pos] == 255) {
                // Verifica vizinhos superiores e à esquerda
                int min_label = label;

                // Vizinho superior
                if (data[pos - bytesperline] == 255) {
                    int up_label = labels[(y - 1) * width + x];
                    if (up_label > 0 && up_label < min_label)
                        min_label = up_label;
                }

                // Vizinho esquerdo
                if (data[pos - 1] == 255) {
                    int left_label = labels[y * width + (x - 1)];
                    if (left_label > 0 && left_label < min_label)
                        min_label = left_label;
                }

                if (min_label == label) {
                    labels[y * width + x] = label++;
                }
                else {
                    labels[y * width + x] = min_label;
                }
            }
        }
    }

    return label - 1;  // Retorna o número de objetos encontrados
}

// Análise das moedas
void vc_analyze_coins(IVC* image, int* labels, CoinStats* stats) {
    int width = image->width;
    int height = image->height;
    int num_labels = 0;
    int i;

    // Encontra o número máximo de rótulos
    for (i = 0; i < width * height; i++) {
        if (labels[i] > num_labels)
            num_labels = labels[i];
    }

    // Para cada objeto encontrado
    for (int label = 1; label <= num_labels; label++) {
        int area = 0;
        int perimeter = 0;
        int sum_x = 0, sum_y = 0;
        int min_x = width, min_y = height;
        int max_x = 0, max_y = 0;

        // Calcula área e centro de massa
        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                if (labels[y * width + x] == label) {
                    area++;
                    sum_x += x;
                    sum_y += y;

                    // Atualiza bounding box
                    if (x < min_x) min_x = x;
                    if (x > max_x) max_x = x;
                    if (y < min_y) min_y = y;
                    if (y > max_y) max_y = y;
                }
            }
        }

        // Calcula perímetro
        for (int y = min_y; y <= max_y; y++) {
            for (int x = min_x; x <= max_x; x++) {
                if (labels[y * width + x] == label) {
                    // Verifica se é um pixel de borda
                    if (x == min_x || x == max_x || y == min_y || y == max_y)
                        perimeter++;
                }
            }
        }

        // Calcula circularidade
        #ifndef M_PI
        #define M_PI 3.14159265358979323846
        #endif

        float circularity = (4.0f * M_PI * area) / (perimeter * perimeter);

        // Identifica o tipo de moeda baseado na área e circularidade
        int coin_type = -1;
        if (circularity > 0.8f) {  // Verifica se é circular o suficiente
            if (area > 3000 && area <= 4000) coin_type = 0;      // 1 cent
            else if (area > 4000 && area <= 5000) coin_type = 1; // 2 cent
            else if (area > 5000 && area <= 6000) coin_type = 2; // 5 cent
            else if (area > 6000 && area <= 7000) coin_type = 3; // 10 cent
            else if (area > 7000 && area <= 8000) coin_type = 4; // 20 cent
            else if (area > 8000 && area <= 9000) coin_type = 5; // 50 cent
            else if (area > 9000 && area <= 10000) coin_type = 6;// 1 euro
            else if (area > 10000 && area <= 12000) coin_type = 7;// 2 euros
        }

        // Atualiza estatísticas se for uma moeda válida
        if (coin_type >= 0) {
            stats->total_coins++;
            stats->coins_by_type[coin_type]++;
        }
    }
}

// Funções de análise de moedas
void vc_init_coin_stats(CoinStats* stats) {
    stats->total_coins = 0;
    memset(stats->coins_by_type, 0, sizeof(stats->coins_by_type));
    stats->max_coins = 100; // Número máximo de moedas a serem rastreadas
    stats->coins = (CoinInfo*)malloc(stats->max_coins * sizeof(CoinInfo));
}

void vc_free_coin_stats(CoinStats* stats) {
    if (stats->coins != NULL) {
        free(stats->coins);
    }
}

int vc_identify_coin_type(int area, float circularity) {
    // Valores aproximados baseados em testes com as moedas
    // Estes valores precisam ser calibrados com o vídeo real
    if (circularity < 0.8) return -1; // Não é uma moeda

    if (area >= 10000 && area <= 12000) return 6; // 1€
    if (area >= 9000 && area <= 10000) return 7;  // 2€
    if (area >= 8000 && area <= 9000) return 5;   // 50c
    if (area >= 7000 && area <= 8000) return 4;   // 20c
    if (area >= 6000 && area <= 7000) return 3;   // 10c
    if (area >= 5000 && area <= 6000) return 2;   // 5c
    if (area >= 4000 && area <= 5000) return 1;   // 2c
    if (area >= 3000 && area <= 4000) return 0;   // 1c

    return -1; // Tipo desconhecido
}

// Função auxiliar para desenhar informações da moeda
static void vc_draw_coin_info(cv::Mat& frame, const CoinInfo& coin) {
    // Implementação aqui se necessário
}

int vc_process_video(const char* filename) {
    printf("Tentando abrir o vídeo: %s\n", filename);

    // Verifica se o arquivo existe
    FILE* test = fopen(filename, "rb");
    if (test == NULL) {
        printf("Erro: Arquivo não encontrado\n");
        return 0;
    }
    fclose(test);

    // Abre o vídeo
    cv::VideoCapture cap;
    try {
        cap.open(filename);
    if (!cap.isOpened()) {
            printf("Erro: OpenCV não conseguiu abrir o vídeo\n");
            return 0;
        }
    }
    catch (const cv::Exception& e) {
        printf("Erro OpenCV: %s\n", e.what());
        return 0;
    }

    // Obtém informações do vídeo
    int width = (int)cap.get(cv::CAP_PROP_FRAME_WIDTH);
    int height = (int)cap.get(cv::CAP_PROP_FRAME_HEIGHT);
    int fps = (int)cap.get(cv::CAP_PROP_FPS);
    int total_frames = (int)cap.get(cv::CAP_PROP_FRAME_COUNT);

    printf("Informações do vídeo:\n");
    printf("Dimensões: %dx%d\n", width, height);
    printf("FPS: %d\n", fps);
    printf("Total de frames: %d\n", total_frames);

    // Cria janela
    cv::namedWindow("Coins Detection", cv::WINDOW_AUTOSIZE);

    // Estruturas para processamento
    IVC* frame = vc_image_new(width, height, 3, 255);
    IVC* hsv = vc_image_new(width, height, 3, 255);
    IVC* binary = vc_image_new(width, height, 1, 255);
    int* labels = (int*)calloc(width * height, sizeof(int));
    CoinStats stats = { 0 };

    // Array para armazenar valores das moedas
    const float coin_values[] = { 0.01f, 0.02f, 0.05f, 0.10f, 0.20f, 0.50f, 1.0f, 2.0f };
    const char* coin_names[] = { "1c", "2c", "5c", "10c", "20c", "50c", "1€", "2€" };

    // Estrutura para rastrear moedas já contadas
    struct CoinTracker {
        int center_x;
        int center_y;
        int type;
        int frames_tracked;
    };
    std::vector<CoinTracker> tracked_coins;

    cv::Mat cv_frame;
    int frame_count = 0;

    // Criar janelas e trackbars
    cv::namedWindow("Controls", cv::WINDOW_AUTOSIZE);
    cv::namedWindow("Original", cv::WINDOW_AUTOSIZE);
    cv::namedWindow("HSV", cv::WINDOW_AUTOSIZE);
    cv::namedWindow("Binary", cv::WINDOW_AUTOSIZE);

    while (cap.read(cv_frame)) {
        frame_count++;

        // Converte frame OpenCV para nossa estrutura IVC
        memcpy(frame->data, cv_frame.data, width * height * 3);

        // Converte para HSV usando apenas cvtColor
        cv::Mat cv_hsv;
        cv::cvtColor(cv_frame, cv_hsv, cv::COLOR_BGR2HSV);
        memcpy(hsv->data, cv_hsv.data, width * height * 3);
        
        // Valores HSV ajustados para cada tipo de moeda
        // Moedas de cobre (1, 2, 5 cents) - tons marrom/cobre
        int hmin_copper = 10, hmax_copper = 40;    // Faixa para tons marrom/cobre
        int smin_copper = 80, smax_copper = 255;   // Alta saturação
        int vmin_copper = 50, vmax_copper = 220;   // Brilho médio-alto

        // Moedas douradas (10, 20, 50 cents) - tons amarelo/dourado
        int hmin_gold = 20, hmax_gold = 45;        // Faixa para tons dourados
        int smin_gold = 100, smax_gold = 255;      // Alta saturação
        int vmin_gold = 100, vmax_gold = 255;      // Alto brilho

        // Moedas prateadas (1, 2 euros) - tons cinza/prata
        int hmin_silver = 0, hmax_silver = 180;    // Qualquer matiz
        int smin_silver = 0, smax_silver = 70;     // Baixa saturação
        int vmin_silver = 150, vmax_silver = 255;  // Alto brilho

        // Cria uma cópia do frame binário para cada tipo de metal
        IVC* binary_copper = vc_image_new(width, height, 1, 255);
        IVC* binary_gold = vc_image_new(width, height, 1, 255);
        IVC* binary_silver = vc_image_new(width, height, 1, 255);
        
        // Detecta cada tipo de moeda
        vc_hsv_segmentation(hsv, binary_copper, 
            hmin_copper, hmax_copper, 
            smin_copper, smax_copper, 
            vmin_copper, vmax_copper);
        
        vc_hsv_segmentation(hsv, binary_gold,
            hmin_gold, hmax_gold,
            smin_gold, smax_gold,
            vmin_gold, vmax_gold);
        
        vc_hsv_segmentation(hsv, binary_silver,
            hmin_silver, hmax_silver,
            smin_silver, smax_silver,
            vmin_silver, vmax_silver);
        
        // Aplica operações morfológicas para remover ruído
        // Primeiro erosão para remover pequenos ruídos
        vc_binary_erode(binary_copper, binary_copper, 5);  // Aumentei kernel para 5x5
        vc_binary_erode(binary_gold, binary_gold, 5);
        vc_binary_erode(binary_silver, binary_silver, 5);
        
        // Depois dilatação para recuperar o formato
        vc_binary_dilate(binary_copper, binary_copper, 7); // Aumentei kernel para 7x7
        vc_binary_dilate(binary_gold, binary_gold, 7);
        vc_binary_dilate(binary_silver, binary_silver, 7);
        
        // Erosão final para refinar as bordas
        vc_binary_erode(binary_copper, binary_copper, 3);
        vc_binary_erode(binary_gold, binary_gold, 3);
        vc_binary_erode(binary_silver, binary_silver, 3);
        
        // Combina os resultados no binary final com prioridade
        unsigned char* data_copper = (unsigned char*)binary_copper->data;
        unsigned char* data_gold = (unsigned char*)binary_gold->data;
        unsigned char* data_silver = (unsigned char*)binary_silver->data;
        unsigned char* data_binary = (unsigned char*)binary->data;
        
        for(int i = 0; i < width * height; i++) {
            // Prioriza detecção de moedas prateadas
            if (data_silver[i] == 255) {
                data_binary[i] = 255;
            }
            // Depois moedas douradas
            else if (data_gold[i] == 255) {
                data_binary[i] = 255;
            }
            // Por fim moedas de cobre
            else if (data_copper[i] == 255) {
                data_binary[i] = 255;
            }
            else {
                data_binary[i] = 0;
            }
        }

        // Limpa o buffer de rastreamento a cada 30 frames
        if (frame_count % 30 == 0) {
            tracked_coins.clear();
        }

        int num_objects = vc_binary_labeling(binary, labels);

        // Constantes para conversão de diâmetro em pixels
        // Assumindo que 1mm = X pixels (precisa ser calibrado com base na resolução do vídeo)
        const float MM_TO_PIXELS = 4.0f; // Valor aproximado, ajuste conforme necessário
        
        // Diâmetros das moedas em milímetros
        const float DIAM_1CENT = 16.25f;
        const float DIAM_2CENT = 18.75f;
        const float DIAM_5CENT = 21.25f;
        const float DIAM_10CENT = 19.75f;
        const float DIAM_20CENT = 22.25f;
        const float DIAM_50CENT = 24.25f;
        const float DIAM_1EURO = 23.25f;
        const float DIAM_2EURO = 25.75f;

        // Calcula áreas esperadas em pixels (π * r²)
        const float PI = 3.14159265359f;
        const float AREA_1CENT = PI * pow(DIAM_1CENT * MM_TO_PIXELS / 2, 2);
        const float AREA_2CENT = PI * pow(DIAM_2CENT * MM_TO_PIXELS / 2, 2);
        const float AREA_5CENT = PI * pow(DIAM_5CENT * MM_TO_PIXELS / 2, 2);
        const float AREA_10CENT = PI * pow(DIAM_10CENT * MM_TO_PIXELS / 2, 2);
        const float AREA_20CENT = PI * pow(DIAM_20CENT * MM_TO_PIXELS / 2, 2);
        const float AREA_50CENT = PI * pow(DIAM_50CENT * MM_TO_PIXELS / 2, 2);
        const float AREA_1EURO = PI * pow(DIAM_1EURO * MM_TO_PIXELS / 2, 2);
        const float AREA_2EURO = PI * pow(DIAM_2EURO * MM_TO_PIXELS / 2, 2);

        // Margem de erro para área (±25%)
        const float AREA_MARGIN = 0.25f;

        // Para cada objeto detectado
        for (int i = 1; i <= num_objects; i++) {
            int area = 0;
            int sum_x = 0, sum_y = 0;
            int perimeter = 0;
            int min_x = width, min_y = height;
            int max_x = 0, max_y = 0;

            // Calcula área e centro de massa
            for (int y = 0; y < height; y++) {
                for (int x = 0; x < width; x++) {
                    if (labels[y * width + x] == i) {
                        area++;
                        sum_x += x;
                        sum_y += y;
                        min_x = std::min(min_x, x);
                        max_x = std::max(max_x, x);
                        min_y = std::min(min_y, y);
                        max_y = std::max(max_y, y);
                    }
                }
            }

            if (area > 0) {
                int center_x = sum_x / area;
                int center_y = sum_y / area;

                // Calcula perímetro
                for (int y = min_y; y <= max_y; y++) {
                    for (int x = min_x; x <= max_x; x++) {
                        if (labels[y * width + x] == i) {
                            if (x == min_x || x == max_x || y == min_y || y == max_y ||
                                labels[y * width + x - 1] != i ||
                                labels[y * width + x + 1] != i ||
                                labels[(y - 1) * width + x] != i ||
                                labels[(y + 1) * width + x] != i) {
                                perimeter++;
                            }
                        }
                    }
                }

                float circularity = (4.0f * PI * area) / (perimeter * perimeter);

                // Verifica se é uma moeda válida usando múltiplos critérios
                if (circularity > 0.65) {
                    // Calcula a razão de aspecto (width/height da bounding box)
                    float aspect_ratio = (float)(max_x - min_x) / (float)(max_y - min_y);
                    
                    // Moedas devem ser aproximadamente circulares (razão próxima de 1.0)
                    if (aspect_ratio >= 0.85 && aspect_ratio <= 1.15) {
                        // Calcula a densidade (área preenchida / área da bounding box)
                        float bbox_area = (max_x - min_x + 1) * (max_y - min_y + 1);
                        float density = area / bbox_area;
                        
                        // Moedas devem ter alta densidade (área preenchida)
                        if (density > 0.6) {
                            int coin_type = -1;
                            const char* coin_label = "";
                            
                            // Identifica o tipo de moeda baseado na área com margem de erro
                            if (area >= AREA_1CENT * (1 - AREA_MARGIN) && area <= AREA_1CENT * (1 + AREA_MARGIN)) 
                                { coin_type = 0; coin_label = "1c"; }
                            else if (area >= AREA_2CENT * (1 - AREA_MARGIN) && area <= AREA_2CENT * (1 + AREA_MARGIN))
                                { coin_type = 1; coin_label = "2c"; }
                            else if (area >= AREA_5CENT * (1 - AREA_MARGIN) && area <= AREA_5CENT * (1 + AREA_MARGIN))
                                { coin_type = 2; coin_label = "5c"; }
                            else if (area >= AREA_10CENT * (1 - AREA_MARGIN) && area <= AREA_10CENT * (1 + AREA_MARGIN))
                                { coin_type = 3; coin_label = "10c"; }
                            else if (area >= AREA_20CENT * (1 - AREA_MARGIN) && area <= AREA_20CENT * (1 + AREA_MARGIN))
                                { coin_type = 4; coin_label = "20c"; }
                            else if (area >= AREA_50CENT * (1 - AREA_MARGIN) && area <= AREA_50CENT * (1 + AREA_MARGIN))
                                { coin_type = 5; coin_label = "50c"; }
                            else if (area >= AREA_1EURO * (1 - AREA_MARGIN) && area <= AREA_1EURO * (1 + AREA_MARGIN))
                                { coin_type = 6; coin_label = "1€"; }
                            else if (area >= AREA_2EURO * (1 - AREA_MARGIN) && area <= AREA_2EURO * (1 + AREA_MARGIN))
                                { coin_type = 7; coin_label = "2€"; }

                            // Se identificou uma moeda válida
                            if (coin_type >= 0) {
                                // Verifica se esta moeda já foi contada
                                bool already_counted = false;
                                for (const auto& tracked : tracked_coins) {
                                    int dx = center_x - tracked.center_x;
                                    int dy = center_y - tracked.center_y;
                                    float distance = sqrt(dx*dx + dy*dy);
                                    
                                    if (distance < 30) {
                                        already_counted = true;
                                        break;
                                    }
                                }

                                if (!already_counted) {
                                    // Adiciona nova moeda ao rastreamento
                                    CoinTracker new_coin = {center_x, center_y, coin_type, 0};
                                    tracked_coins.push_back(new_coin);
                                    
                                    // Atualiza estatísticas
                                    stats.total_coins++;
                                    stats.coins_by_type[coin_type]++;

                                    // Debug: mostra informação da moeda detectada
                                    printf("Moeda detectada: %s, Area=%d, Circ=%.2f, Pos=(%d,%d)\n", 
                                           coin_label, area, circularity, center_x, center_y);
                                }
                            }
                        }
                    }
                }
            }
        }

        // Mostrar etapas do processamento
        cv::imshow("Original", cv_frame);
        cv::imshow("HSV", cv_hsv);
        cv::Mat binary_display(height, width, CV_8UC1, binary->data);
        cv::imshow("Binary", binary_display);

        // Mostra estatísticas no frame
        char info[200];
        sprintf(info, "Frame: %d/%d | Total: %.2f€ (%d moedas)", 
                frame_count, total_frames, 
                stats.coins_by_type[0] * 0.01f + 
                stats.coins_by_type[1] * 0.02f + 
                stats.coins_by_type[2] * 0.05f + 
                stats.coins_by_type[3] * 0.10f + 
                stats.coins_by_type[4] * 0.20f + 
                stats.coins_by_type[5] * 0.50f + 
                stats.coins_by_type[6] * 1.00f + 
                stats.coins_by_type[7] * 2.00f,
                stats.total_coins);
        cv::putText(cv_frame, info, cv::Point(20, 30), cv::FONT_HERSHEY_SIMPLEX, 0.7, cv::Scalar(0, 255, 0), 2);

        // Mostra o frame
        cv::imshow("Coins Detection", cv_frame);
        if (cv::waitKey(1000/fps) == 27) break;

        // Libera memória das imagens temporárias
        vc_image_free(binary_copper);
        vc_image_free(binary_gold);
        vc_image_free(binary_silver);
    }

    // Mostra estatísticas finais
    printf("\nEstatísticas finais:\n");
    printf("Total de moedas: %d\n", stats.total_coins);
    float total_value = 0.0f;
    for (int i = 0; i < 8; i++) {
        printf("%s: %d moedas (%.2f)\n",
            coin_names[i], stats.coins_by_type[i],
            stats.coins_by_type[i] * coin_values[i]);
        total_value += stats.coins_by_type[i] * coin_values[i];
    }
    printf("Valor total: %.2f€\n", total_value);

    // Libera recursos
    vc_image_free(frame);
    vc_image_free(hsv);
    vc_image_free(binary);
    free(labels);
    cv::destroyAllWindows();
    cap.release();

    return 1;
}

// Binary morphological operations
int vc_binary_erode(IVC* src, IVC* dst, int kernel_size) {
    unsigned char* data_src = (unsigned char*)src->data;
    unsigned char* data_dst = (unsigned char*)dst->data;
    int width = src->width;
    int height = src->height;
    int bytesperline = src->bytesperline;
    int channels = src->channels;
    int x, y, kx, ky;
    int offset = kernel_size / 2;

    // Verificação de erros
    if ((src->width <= 0) || (src->height <= 0) || (src->data == NULL)) return 0;
    if ((src->width != dst->width) || (src->height != dst->height)) return 0;
    if ((src->channels != 1) || (dst->channels != 1)) return 0;
    if (kernel_size % 2 == 0) return 0;

    // Copia a imagem fonte para destino
    memcpy(data_dst, data_src, width * height);

    // Para cada pixel da imagem
    for (y = offset; y < height - offset; y++) {
        for (x = offset; x < width - offset; x++) {
            int pos = y * bytesperline + x;
            bool erode = false;

            // Se o pixel for branco, verifica vizinhança
            if (data_src[pos] == 255) {
                // Para cada pixel do kernel
                for (ky = -offset; ky <= offset && !erode; ky++) {
                    for (kx = -offset; kx <= offset && !erode; kx++) {
                        int pos_k = (y + ky) * bytesperline + (x + kx);
                        if (data_src[pos_k] == 0) {
                            erode = true;
                        }
                    }
                }
                if (erode) data_dst[pos] = 0;
            }
        }
    }
    return 1;
}

int vc_binary_dilate(IVC* src, IVC* dst, int kernel_size) {
    unsigned char* data_src = (unsigned char*)src->data;
    unsigned char* data_dst = (unsigned char*)dst->data;
    int width = src->width;
    int height = src->height;
    int bytesperline = src->bytesperline;
    int channels = src->channels;
    int x, y, kx, ky;
    int offset = kernel_size / 2;

    // Verificação de erros
    if ((src->width <= 0) || (src->height <= 0) || (src->data == NULL)) return 0;
    if ((src->width != dst->width) || (src->height != dst->height)) return 0;
    if ((src->channels != 1) || (dst->channels != 1)) return 0;
    if (kernel_size % 2 == 0) return 0;

    // Copia a imagem fonte para destino
    memcpy(data_dst, data_src, width * height);

    // Para cada pixel da imagem
    for (y = offset; y < height - offset; y++) {
        for (x = offset; x < width - offset; x++) {
            int pos = y * bytesperline + x;
            bool dilate = false;

            // Se o pixel for preto, verifica vizinhança
            if (data_src[pos] == 0) {
                // Para cada pixel do kernel
                for (ky = -offset; ky <= offset && !dilate; ky++) {
                    for (kx = -offset; kx <= offset && !dilate; kx++) {
                        int pos_k = (y + ky) * bytesperline + (x + kx);
                        if (data_src[pos_k] == 255) {
                            dilate = true;
                        }
                    }
                }
                if (dilate) data_dst[pos] = 255;
            }
        }
    }
    return 1;
}