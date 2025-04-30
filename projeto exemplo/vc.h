/********************************************
 *                                          *
 *   INSTITUTO POLITÉCNICO DO CÁVADO E DO AVE *
 *             2022/2023                     *
 *   ENGENHARIA DE SISTEMAS INFORMÁTICOS     *
 *                                          *
 *   [  ROBERTO PEREIRA - A20731@ALUNOS.IPCA.PT  ]     *
 *                                          *
 ********************************************/


#define VC_DEBUG

#define MAX(r, g, b) (r > b ? (r > g ? r : g) : (b > g ? b : g))
#define MIN(r, g, b) (r < b ? (r < g ? r : g) : (b < g ? b : g))

#define MAX_2(a, b) (a > b ? a : b)
#define MIN_2(a, b) (a < b ? a : b)

#define CONV_RANGE(value, range, new_range) (value / range) * new_range
#define HSV_2_RGB(value) CONV_RANGE(value, 360, 255)
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//                   ESTRUTURA DE UMA IMAGEM
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++


typedef struct {
	unsigned char *data;
	int width, height;
	int channels;			// Binário/Cinzentos=1; RGB=3
	int levels;				// Binário=1; Cinzentos [1,255]; RGB [1,255]
	int bytesperline;		// width * channels
} IVC;

typedef struct {
	int x, y, width, heigh;
	int area;
	int xc, yc;
	int perimeter;
	int label;

	unsigned char* mask;
		unsigned char* data;
	int channels;
	int levels;
}OVC;



#include <stdbool.h>;


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//                    PROTÓTIPOS DE FUNÇÕES
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

// FUNÇÕES: ALOCAR E LIBERTAR UMA IMAGEM
IVC *vc_image_new(int width, int height, int channels, int levels);
IVC *vc_image_free(IVC *image);

// FUNÇÕES: LEITURA E ESCRITA DE IMAGENS (PBM, PGM E PPM)
IVC *vc_read_image(char *filename);
int vc_write_image(char *filename, IVC *image);

// FUNÇÃO AULA ESPAÇOS DE COR VC04
int vc_gray_negative(IVC* srcdst);
int vc_rgb_negative(IVC* srcdst);
int vc_rgb_get_red(IVC* srcdst);
int vc_rgb_get_green(IVC* srcdst);
int vc_rgb_get_blue(IVC* srcdst);
int vc_rgb_get_red_gray(IVC* srcdst);
int vc_rgb_get_green_gray(IVC* srcdst);
int vc_rgb_get_blue_gray(IVC* srcdst);
int vc_rgb_to_gray(IVC* src, IVC* dst);
int vc_hsv_segmentation(IVC* src, IVC* dst, int hmin, int hmax, int smin, int smax, int vmin, int vmax);
int vc_rgb_to_hsv(IVC* srcdst);
int vc_rgb_temp_get_percentage(IVC* srcdst);
int vc_gray_to_binary(IVC* srcdst, int threshold);
int vc_gray_to_binary_global_mean(IVC* srcdst);
int vc_gray_to_binary_midpoint(IVC* srcdst, int kernel);
int vc_binary_dilate(IVC* src, IVC* dst, int size);

int vc_gray_open(IVC* src, IVC* dst, int kernel);
int vc_gray_close(IVC* src, IVC* dst, int kernel);
//int vc_binary_open(IVC* src, IVC* dst, int kernel);
int vc_bin_open(IVC* src, IVC* dst, int kernel);
//int vc_binary_close(IVC* src, IVC* dst, int kernel);
int vc_bin_close(IVC* src, IVC* dst, int kernel);
int vc_gray_erode(IVC* src, IVC* dst, int kernel);
int vc_gray_dilate(IVC* src, IVC* dst, int kernel);
int vc_binary_erode(IVC* src, IVC* dst, int kernel);
int vc_binary_dilate(IVC* src, IVC* dst, int kernel);
OVC* vc_binary_blob_labelling(IVC* src, IVC* dst, int* nlabels);
int vc_binary_blob_info(IVC* src, OVC* blobs, int nblobs);

int vc_gray_histogram_show(IVC* src, IVC* dst);
int vc_gray_edge_sobel(IVC* src, IVC* dst, float th);
int vc_gray_lowpass_mean_filter(IVC* src, IVC* dst, int kernelsize);
int vc_gray_lowpass_median_filter(IVC* src, IVC* dst, int size);
int vc_gray_highpass_filter(IVC* src, IVC* dst,int tipo);
int vc_gray_histogram_equalization(IVC* src, IVC* dst);
int equalize(unsigned char* datasrc, unsigned char* datadst, int bytesperline, int channels, int offset, int width, int height);
int vc_draw_boundingbox(IVC* srcdst, OVC* blob);
int vc_draw_centerofgravity(IVC* srcdst, OVC* blob);


int vc_hsv_segmentationalterada(IVC* src, IVC* dst, int hmin, int hmax, int smin, int smax, int vmin, int vmax);
int vc_remove_bg(IVC* srcOriginal, IVC* srcMask, IVC* dst);
float cv_segmentAndCalculateredAreaPercentage(IVC* src, IVC* dst, int hmin, int hmax, int smin, int smax, int vmin, int vmax);
float calculateNonRedAreaPercentage(IVC* image);
int vc_gray_to_binary_bernsen(IVC* src, IVC* dst, int kernel, int Cmin);
int getobjectmaskbyboundingbox(IVC* srcdst, OVC* blob);

int cv_segmentAndCalculateColorAreaPercentageV2(IVC* image, int hmin_red, int hmax_red, int smin_red, int smax_red, int vmin_red, int vmax_red, int hmin_blue, int hmax_blue, int smin_blue, int smax_blue, int vmin_blue, int vmax_blue);
int getColorCode(IVC* image);
float calculateNonBlueAreaPercentage(IVC* image);
bool isSquare(const OVC* blob);
bool isRound(const OVC* blob);
int vc_gray_to_binary_bernsen2(IVC* src, IVC* dst, int kernel, int Cmin);
bool isCircularPixel(unsigned char* data, int width, int height, int bytesperline, int channels, int x, int y);
void vc_recursive_fill(unsigned char* datadst, unsigned char* dataaux, int width, int height, int x, int y);
void vc_process_image(IVC* src, IVC* dst, int threshold);
int determinaSetaEsquerdaOuDireita(IVC* srcdst, OVC* blob);