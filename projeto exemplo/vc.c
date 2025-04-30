/********************************************
 *                                          *
 *   INSTITUTO POLITÉCNICO DO CÁVADO E DO AVE *
 *             2022/2023                     *
 *   ENGENHARIA DE SISTEMAS INFORMÁTICOS     *
 *                                          *
 *   [  ROBERTO PEREIRA - A20731@ALUNOS.IPCA.PT  ]     *
 *                                          *
 ********************************************/

// Desabilita (no MSVC++) warnings de funções não seguras (fopen, sscanf, etc...)
#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <malloc.h>
#include "vc.h"
#include "math.h"


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//            FUNÇÕES: ALOCAR E LIBERTAR UMA IMAGEM
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++


// Alocar memória para uma imagem
IVC *vc_image_new(int width, int height, int channels, int levels)
{
	IVC *image = (IVC *) malloc(sizeof(IVC));

	if(image == NULL) return NULL;
	if((levels <= 0) || (levels > 255)) return NULL;

	image->width = width;
	image->height = height;
	image->channels = channels;
	image->levels = levels;
	image->bytesperline = image->width * image->channels;
	image->data = (unsigned char *) malloc(image->width * image->height * image->channels * sizeof(char));

	if(image->data == NULL)
	{
		return vc_image_free(image);
	}

	return image;
}


// Libertar memória de uma imagem
IVC *vc_image_free(IVC *image)
{
	if(image != NULL)
	{
		if(image->data != NULL)
		{
			free(image->data);
			image->data = NULL;
		}

		free(image);
		image = NULL;
	}

	return image;
}


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//    FUNÇÕES: LEITURA E ESCRITA DE IMAGENS (PBM, PGM E PPM)
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++


char *netpbm_get_token(FILE *file, char *tok, int len)
{
	char *t;
	int c;
	
	for(;;)
	{
		while(isspace(c = getc(file)));
		if(c != '#') break;
		do c = getc(file);
		while((c != '\n') && (c != EOF));
		if(c == EOF) break;
	}
	
	t = tok;
	
	if(c != EOF)
	{
		do
		{
			*t++ = c;
			c = getc(file);
		} while((!isspace(c)) && (c != '#') && (c != EOF) && (t - tok < len - 1));
		
		if(c == '#') ungetc(c, file);
	}
	
	*t = 0;
	
	return tok;
}


long int unsigned_char_to_bit(unsigned char *datauchar, unsigned char *databit, int width, int height)
{
	int x, y;
	int countbits;
	long int pos, counttotalbytes;
	unsigned char *p = databit;

	*p = 0;
	countbits = 1;
	counttotalbytes = 0;

	for(y=0; y<height; y++)
	{
		for(x=0; x<width; x++)
		{
			pos = width * y + x;

			if(countbits <= 8)
			{
				// Numa imagem PBM:
				// 1 = Preto
				// 0 = Branco
				//*p |= (datauchar[pos] != 0) << (8 - countbits);
				
				// Na nossa imagem:
				// 1 = Branco
				// 0 = Preto
				*p |= (datauchar[pos] == 0) << (8 - countbits);

				countbits++;
			}
			if((countbits > 8) || (x == width - 1))
			{
				p++;
				*p = 0;
				countbits = 1;
				counttotalbytes++;
			}
		}
	}

	return counttotalbytes;
}


void bit_to_unsigned_char(unsigned char *databit, unsigned char *datauchar, int width, int height)
{
	int x, y;
	int countbits;
	long int pos;
	unsigned char *p = databit;

	countbits = 1;

	for(y=0; y<height; y++)
	{
		for(x=0; x<width; x++)
		{
			pos = width * y + x;

			if(countbits <= 8)
			{
				// Numa imagem PBM:
				// 1 = Preto
				// 0 = Branco
				//datauchar[pos] = (*p & (1 << (8 - countbits))) ? 1 : 0;

				// Na nossa imagem:
				// 1 = Branco
				// 0 = Preto
				datauchar[pos] = (*p & (1 << (8 - countbits))) ? 0 : 1;
				
				countbits++;
			}
			if((countbits > 8) || (x == width - 1))
			{
				p++;
				countbits = 1;
			}
		}
	}
}


IVC *vc_read_image(char *filename)
{
	FILE *file = NULL;
	IVC *image = NULL;
	unsigned char *tmp;
	char tok[20];
	long int size, sizeofbinarydata;
	int width, height, channels;
	int levels = 255;
	int v;
	
	// Abre o ficheiro
	if((file = fopen(filename, "rb")) != NULL)
	{
		// Efectua a leitura do header
		netpbm_get_token(file, tok, sizeof(tok));

		if(strcmp(tok, "P4") == 0) { channels = 1; levels = 1; }	// Se PBM (Binary [0,1])
		else if(strcmp(tok, "P5") == 0) channels = 1;				// Se PGM (Gray [0,MAX(level,255)])
		else if(strcmp(tok, "P6") == 0) channels = 3;				// Se PPM (RGB [0,MAX(level,255)])
		else
		{
			#ifdef VC_DEBUG
			printf("ERROR -> vc_read_image():\n\tFile is not a valid PBM, PGM or PPM file.\n\tBad magic number!\n");
			#endif

			fclose(file);
			return NULL;
		}
		
		if(levels == 1) // PBM
		{
			if(sscanf(netpbm_get_token(file, tok, sizeof(tok)), "%d", &width) != 1 || 
			   sscanf(netpbm_get_token(file, tok, sizeof(tok)), "%d", &height) != 1)
			{
				#ifdef VC_DEBUG
				printf("ERROR -> vc_read_image():\n\tFile is not a valid PBM file.\n\tBad size!\n");
				#endif

				fclose(file);
				return NULL;
			}

			// Aloca memória para imagem
			image = vc_image_new(width, height, channels, levels);
			if(image == NULL) return NULL;

			sizeofbinarydata = (image->width / 8 + ((image->width % 8) ? 1 : 0)) * image->height;
			tmp = (unsigned char *) malloc(sizeofbinarydata);
			if(tmp == NULL) return 0;

			#ifdef VC_DEBUG
			printf("\nchannels=%d w=%d h=%d levels=%d\n", image->channels, image->width, image->height, levels);
			#endif

			if((v = fread(tmp, sizeof(unsigned char), sizeofbinarydata, file)) != sizeofbinarydata)
			{
				#ifdef VC_DEBUG
				printf("ERROR -> vc_read_image():\n\tPremature EOF on file.\n");
				#endif

				vc_image_free(image);
				fclose(file);
				free(tmp);
				return NULL;
			}

			bit_to_unsigned_char(tmp, image->data, image->width, image->height);

			free(tmp);
		}
		else // PGM ou PPM
		{
			if(sscanf(netpbm_get_token(file, tok, sizeof(tok)), "%d", &width) != 1 || 
			   sscanf(netpbm_get_token(file, tok, sizeof(tok)), "%d", &height) != 1 || 
			   sscanf(netpbm_get_token(file, tok, sizeof(tok)), "%d", &levels) != 1 || levels <= 0 || levels > 255)
			{
				#ifdef VC_DEBUG
				printf("ERROR -> vc_read_image():\n\tFile is not a valid PGM or PPM file.\n\tBad size!\n");
				#endif

				fclose(file);
				return NULL;
			}

			// Aloca memória para imagem
			image = vc_image_new(width, height, channels, levels);
			if(image == NULL) return NULL;

			#ifdef VC_DEBUG
			printf("\nchannels=%d w=%d h=%d levels=%d\n", image->channels, image->width, image->height, levels);
			#endif

			size = image->width * image->height * image->channels;

			if((v = fread(image->data, sizeof(unsigned char), size, file)) != size)
			{
				#ifdef VC_DEBUG
				printf("ERROR -> vc_read_image():\n\tPremature EOF on file.\n");
				#endif

				vc_image_free(image);
				fclose(file);
				return NULL;
			}
		}
		
		fclose(file);
	}
	else
	{
		#ifdef VC_DEBUG
		printf("ERROR -> vc_read_image():\n\tFile not found.\n");
		#endif
	}
	
	return image;
}


int vc_write_image(char *filename, IVC *image)
{
	FILE *file = NULL;
	unsigned char *tmp;
	long int totalbytes, sizeofbinarydata;
	
	if(image == NULL) return 0;

	if((file = fopen(filename, "wb")) != NULL)
	{
		if(image->levels == 1)
		{
			sizeofbinarydata = (image->width / 8 + ((image->width % 8) ? 1 : 0)) * image->height + 1;
			tmp = (unsigned char *) malloc(sizeofbinarydata);
			if(tmp == NULL) return 0;
			
			fprintf(file, "%s %d %d\n", "P4", image->width, image->height);
			
			totalbytes = unsigned_char_to_bit(image->data, tmp, image->width, image->height);
			printf("Total = %ld\n", totalbytes);
			if(fwrite(tmp, sizeof(unsigned char), totalbytes, file) != totalbytes)
			{
				#ifdef VC_DEBUG
				fprintf(stderr, "ERROR -> vc_read_image():\n\tError writing PBM, PGM or PPM file.\n");
				#endif

				fclose(file);
				free(tmp);
				return 0;
			}

			free(tmp);
		}
		else
		{
			fprintf(file, "%s %d %d 255\n", (image->channels == 1) ? "P5" : "P6", image->width, image->height);
		
			if(fwrite(image->data, image->bytesperline, image->height, file) != image->height)
			{
				#ifdef VC_DEBUG
				fprintf(stderr, "ERROR -> vc_read_image():\n\tError writing PBM, PGM or PPM file.\n");
				#endif

				fclose(file);
				return 0;
			}
		}
		
		fclose(file);

		return 1;
	}
	
	return 0;
}

// FUNÇÃO AULA ESPAÇOS DE COR  ---GRAY---
// VC04_15
int vc_gray_negative(IVC* srcdst)
{
	unsigned char* data = (unsigned char*)srcdst->data;
	int with = srcdst->width;
	int height = srcdst->height;
	int bytesperline = srcdst->bytesperline;
	int channels = srcdst->channels;
	int x, y;
	long int pos;

	//verificaçao de erros
	if ((srcdst->width <= 0) || (srcdst->height <= 0) || (srcdst->data == NULL)) return 0;
	if (channels != 1) return 0;

	//inverter imagem Gray

	for (y = 0; y < height; y ++)
	{
		for (x = 0; x < with; x++)
		{
			pos = y * bytesperline + x * channels;
			data[pos] = 255 - data[pos];
		}
	}
	return 1;
}

// FUNÇÃO AULA ESPAÇOS DE COR ---RGB---
// VC04_16
int vc_rgb_negative(IVC* srcdst)
{
	unsigned char* data = (unsigned char*)srcdst->data;
	int with = srcdst->width;
	int height = srcdst->height;
	int bytesperline = srcdst->bytesperline;
	int channels = srcdst->channels;
	int x, y;
	long int pos;

	//verificaçao de erros
	if ((srcdst->width <= 0) || (srcdst->height <= 0) || (srcdst->data == NULL)) return 0;
	if (channels != 3) return 0;

	//inverter imagem RGB

	for (y = 0; y < height; y++)
	{
		for (x = 0; x < with; x++)
		{
			pos = y * bytesperline + x * channels;
			data[pos] = 255 - data[pos];
			data[pos+1] = 255 - data[pos+1];
			data[pos+2] = 255 - data[pos+2];
		}
	}
	return 1;
}

int vc_rgb_get_red(IVC* srcdst)
{
	unsigned char* data = (unsigned char*)srcdst->data;
	int with = srcdst->width;
	int height = srcdst->height;
	int bytesperline = srcdst->bytesperline;
	int channels = srcdst->channels;
	int x, y;
	long int pos;

	//verificaçao de erros
	if ((srcdst->width <= 0) || (srcdst->height <= 0) || (srcdst->data == NULL)) return 0;
	if (channels != 3) return 0;

	//inverter imagem RGB

	for (y = 0; y < height; y++)
	{
		for (x = 0; x < with; x++)
		{
			pos = y * bytesperline + x * channels;
			
			data[pos + 1] =0;
			data[pos + 2] = 0;
		}
	}
	return 1;
}

int vc_rgb_get_green(IVC* srcdst)
{
	unsigned char* data = (unsigned char*)srcdst->data;
	int with = srcdst->width;
	int height = srcdst->height;
	int bytesperline = srcdst->bytesperline;
	int channels = srcdst->channels;
	int x, y;
	long int pos;

	//verificaçao de erros
	if ((srcdst->width <= 0) || (srcdst->height <= 0) || (srcdst->data == NULL)) return 0;
	if (channels != 3) return 0;

	//inverter imagem RGB

	for (y = 0; y < height; y++)
	{
		for (x = 0; x < with; x++)
		{
			pos = y * bytesperline + x * channels;
			data[pos] = 0;
			
			data[pos + 2] = 0;
		}
	}
	return 1;
}

int vc_rgb_get_blue(IVC* srcdst)
{
	unsigned char* data = (unsigned char*)srcdst->data;
	int with = srcdst->width;
	int height = srcdst->height;
	int bytesperline = srcdst->bytesperline;
	int channels = srcdst->channels;
	int x, y;
	long int pos;

	//verificaçao de erros
	if ((srcdst->width <= 0) || (srcdst->height <= 0) || (srcdst->data == NULL)) return 0;
	if (channels != 3) return 0;

	//inverter imagem RGB

	for (y = 0; y < height; y++)
	{
		for (x = 0; x < with; x++)
		{
			pos = y * bytesperline + x * channels;
			data[pos] = 0;
			data[pos + 1] = 0;
			
		}
	}
	return 1;
}

int vc_rgb_get_red_gray(IVC* srcdst)
{
	unsigned char* data = (unsigned char*)srcdst->data;
	int with = srcdst->width;
	int height = srcdst->height;
	int bytesperline = srcdst->bytesperline;
	int channels = srcdst->channels;
	int x, y;
	long int pos;

	//verificaçao de erros
	if ((srcdst->width <= 0) || (srcdst->height <= 0) || (srcdst->data == NULL)) return 0;
	if (channels != 3) return 0;

	//inverter imagem RGB

	for (y = 0; y < height; y++)
	{
		for (x = 0; x < with; x++)
		{
			pos = y * bytesperline + x * channels;
			data[pos+1] = data[pos];
			data[pos + 2] = data[pos];

		}
	}
	return 1;
}

int vc_rgb_get_green_gray(IVC* srcdst)
{
	unsigned char* data = (unsigned char*)srcdst->data;
	int with = srcdst->width;
	int height = srcdst->height;
	int bytesperline = srcdst->bytesperline;
	int channels = srcdst->channels;
	int x, y;
	long int pos;

	//verificaçao de erros
	if ((srcdst->width <= 0) || (srcdst->height <= 0) || (srcdst->data == NULL)) return 0;
	if (channels != 3) return 0;

	//inverter imagem RGB

	for (y = 0; y < height; y++)
	{
		for (x = 0; x < with; x++)
		{
			pos = y * bytesperline + x * channels;
			data[pos] = data[pos+1];
			data[pos + 2] = data[pos+1];

		}
	}
	return 1;
}


int vc_rgb_get_blue_gray(IVC* srcdst)
{
	unsigned char* data = (unsigned char*)srcdst->data;
	int with = srcdst->width;
	int height = srcdst->height;
	int bytesperline = srcdst->bytesperline;
	int channels = srcdst->channels;
	int x, y;
	long int pos;

	//verificaçao de erros
	if ((srcdst->width <= 0) || (srcdst->height <= 0) || (srcdst->data == NULL)) return 0;
	if (channels != 3) return 0;

	//inverter imagem RGB

	for (y = 0; y < height; y++)
	{
		for (x = 0; x < with; x++)
		{
			pos = y * bytesperline + x * channels;
			data[pos] = data[pos + 2];
			data[pos + 1] = data[pos + 2];

		}
	}
	return 1;
}

int vc_rgb_to_gray(IVC *src,IVC *dst)
{
	unsigned char *datasrc = (unsigned char*)src->data; 
	int bytesperline_src = src->width * src->channels;
	int channels_src = src->channels;
	unsigned char *datadst= (unsigned char*)dst->data;
	int bytesperline_dst = dst->width * dst->channels;
	int channels_dst = dst->channels;
	int width = src->width;
	int height = src->height;
	int x, y;
	long int pos_src, pos_dst;
	float rf, gf, bf;
	
	//verificaçao de erros
	if ((src->width <= 0) || (src->height <= 0) || (src->data == NULL)) return 0;
	if ((src->width != dst->width) || (src->height != dst->height))  return 0;
	if ((src->channels != 3)||(dst->channels !=1)) return 0;

	//inverter imagem RGB

	for (y = 0; y < height; y++)
	{
		for (x = 0; x < width; x++)
		{
			pos_src = y * bytesperline_src + x * channels_src;
			pos_dst = y * bytesperline_dst + x * channels_dst;
			rf = (float)datasrc[pos_src];
			gf = (float)datasrc[pos_src + 1];
			bf = (float)datasrc[pos_src + 2];
			datadst[pos_dst] = (unsigned char)((rf * 0.299) + (gf * 0.587) + (bf * 0.114));
		}
	}
	return 1;
}

int vc_hsv_segmentation(IVC* src, IVC* dst, int hmin, int hmax, int smin, int smax, int vmin, int vmax)
{

	unsigned char* datasrc = (unsigned char*)src->data;
	int bytesperline_src = src->width * src->channels;
	int channels_src = src->channels;
	unsigned char* datadst = (unsigned char*)dst->data;
	int bytesperline_dst = dst->width * dst->channels;
	int channels_dst = dst->channels;
	int width = src->width;
	int height = src->height;
	int x, y;
	long int pos_src, pos_dst;

	hmax = (hmax * 255) / 360;
	hmin = (hmin * 255) / 360;
	smax = (smax * 255) / 100;
	smin = (smin * 255) / 100;
	vmax = (vmax * 255) / 100;
	vmin = (vmin * 255) / 100;
	//verificaçao de erros
	if ((src->width <= 0) || (src->height <= 0) || (src->data == NULL)) return 0;
	if ((src->width != dst->width) || (src->height != dst->height))  return 0;
	if ((src->channels != 3) || (dst->channels != 1)) return 0;

	//precore imagem

	for (y = 0; y < height; y++)
	{
		for (x = 0; x < width; x++)
		{
			pos_src = y * bytesperline_src + x * channels_src;
			pos_dst = y * bytesperline_dst + x * channels_dst;

			if (
				((datasrc[pos_src] >= hmin) && (datasrc[pos_src] <= hmax))
				
				&& ((datasrc[pos_src+1] >= smin)) && ((datasrc[pos_src+1] <= smax))
				&& ((datasrc[pos_src+2] >= vmin) && (datasrc[pos_src+2] <= vmax))
				
				)
			{
			//datadst[pos_dst] = 255;
				datadst[pos_dst] = 1;
			
			}
			else
			{
			datadst[pos_dst] = 0;
			}


		}



	}
	return 1;
}

//int vc_rgb_to_hsv(IVC* src, IVC* dst) {
//
//	// Info da Source Image (src)
//	unsigned char* datasrc = (unsigned char*)src->data;
//	int bytesperline_src = src->width * src->channels;
//	int channels_src = src->channels;
//
//	// Info da Imagem Destino (dst)
//	unsigned char* datadst = (unsigned char*)dst->data;
//	int bytesperline_dst = dst->width * dst->channels;
//	int channels_dst = dst->channels;
//
//	// Width e Height da imagem Src
//	int width = src->width;
//	int height = src->height;
//
//	// Aux Variable
//	int x, y;
//	long int pos_src, pos_dst;
//	float red, green, blue;
//
//	// Hsv floats
//	float hue, saturation, value;
//	float max, min;
//
//	// Verificação de erros
//	if ((src->width <= 0) || (src->height <= 0) || (src->data == NULL)) return 0;
//	if ((src->width != dst->width) || (src->height != dst->height)) return 0;
//	if ((src->channels != 3) || (dst->channels != 3)) return 0;
//
//	for (y = 0; y < height; y++) {
//
//		for (x = 0; x < width; x++) {
//
//			pos_src = y * bytesperline_src + x * channels_src; // Posição src
//			pos_dst = y * bytesperline_dst + x * channels_dst; // Posição dst
//
//
//			red = (float)datasrc[pos_src]; // red
//			green = (float)datasrc[pos_src + 1]; // green
//			blue = (float)datasrc[pos_src + 2]; // blue
//
//
//			// Cálcular Máximo e Mínimo
//			max = (red >= green && red >= blue) ? red : (green >= red && green >= blue) ? green : blue;
//			min = (red <= green&& red <= blue) ? red : (green <= red&& green <= blue) ? green : blue;
//
//			// Declarar os valores para o HSV
//			value = max;
//			saturation = (max - min) / value;
//
//			// Calcular o Hue
//			if ((max == red) && (green >= blue)) {
//				hue = 60 * (green - blue) / (max - min);
//			}
//			else if ((max == red) && (blue >= green)) {
//				hue = 360 + 60 * (green - blue) / (max - min);
//			}
//			else if (max == green) {
//				hue = 120 + 60 * (blue - red) / (max - min);
//			}
//			else if (max == blue) {
//				hue = 240 + 60 * (red - green) / (max - min);
//			}
//
//
//			dst->data[pos_dst] = (unsigned char)((hue * 255) / 360);
//			dst->data[pos_dst + 1] = (unsigned char)(saturation * 255.0f);
//			dst->data[pos_dst + 2] = (unsigned char)(value);
//
//		}
//	}
//
//	return 1;
//}  //minha funcao vou alterar pela prof

int vc_rgb_to_hsv(IVC* srcdst)
{
	unsigned char* data = (unsigned char*)srcdst->data;
	int width = srcdst->width;
	int height = srcdst->height;
	int bytesperline = srcdst->bytesperline;
	int channels = srcdst->channels;
	float r, g, b, hue, saturation, value;
	float rgb_max, rgb_min;
	int i, size;

	// Verificação de erros
	if ((width <= 0) || (height <= 0) || (data == NULL)) return 0;
	if (channels != 3) return 0;

	size = width * height * channels;

	for (i = 0; i < size; i = i + channels)
	{
		r = (float)data[i];
		g = (float)data[i + 1];
		b = (float)data[i + 2];

		// Calcula valores máximo e mínimo dos canais de cor R, G e B
		rgb_max = (r > g ? (r > b ? r : b) : (g > b ? g : b));
		rgb_min = (r < g ? (r < b ? r : b) : (g < b ? g : b));

		// Value toma valores entre [0,255]
		value = rgb_max;
		if (value == 0.0f)
		{
			hue = 0.0f;
			saturation = 0.0f;
		}
		else
		{
			// Saturation toma valores entre [0,255]
			saturation = ((rgb_max - rgb_min) / rgb_max) * 255.0f;

			if (saturation == 0.0f)
			{
				hue = 0.0f;
			}
			else
			{
				// Hue toma valores entre [0,360]
				if ((rgb_max == r) && (g >= b))
				{
					hue = 60.0f * (g - b) / (rgb_max - rgb_min);
				}
				else if ((rgb_max == r) && (b > g))
				{
					hue = 360.0f + 60.0f * (g - b) / (rgb_max - rgb_min);
				}
				else if (rgb_max == g)
				{
					hue = 120.0f + 60.0f * (b - r) / (rgb_max - rgb_min);
				}
				else
				{
					hue = 240.0f + 60.0f * (r - g) / (rgb_max - rgb_min);
				}
			}
		}

		// Atribui valores entre [0,255]
		data[i] = (unsigned char)(hue / 360.0f * 255.0f);
		data[i + 1] = (unsigned char)(saturation);
		data[i + 2] = (unsigned char)(value);

	}

	return 1;
}

int vc_scale_gray_to_rgb(IVC* src, IVC* dst)
{
	// info source
	unsigned char* datasrc = (unsigned char*)src->data;
	int bytesperline_src = src->width * src->channels;
	int channels_src = src->channels;

	//info destino
	unsigned char* datadst = (unsigned char*)dst->data;
	int bytesperline_dst = dst->width * dst->channels;
	int channels_dst = dst->channels;

	// medidas
	int width = src->width;
	int height = src->height;

	//auxiliares gerais
	int x, y;
	long int pos_src, pos_dst;
	float gray;


	//verificação de erros
	if ((src->width <= 0) || (src->height <= 0) || (src->data == NULL)) return 0;
	if ((src->width != dst->width) || (src->height != dst->height)) return 0;
	if ((src->channels != 1) || (dst->channels != 3)) return 0;

	for (y = 0; y < height; y++)
	{
		for (x = 0; x < width; x++)
		{
			pos_src = y * bytesperline_src + x * channels_src;//posicao da source
			pos_dst = y * bytesperline_dst + x * channels_dst;//posicao destino

			gray = (float)datasrc[pos_src];

			if (gray < 64)
			{
				datadst[pos_dst] = (unsigned char)0;
				datadst[pos_dst + 1] = (unsigned char)(gray * 4);
				datadst[pos_dst + 2] = (unsigned char)255;
			}
			else if (gray < 128)
			{
				datadst[pos_dst] = (unsigned char)0;
				datadst[pos_dst + 1] = (unsigned char)255;
				datadst[pos_dst + 2] = (unsigned char)(255 - gray * 4);
			}
			else if (gray < 192)
			{
				datadst[pos_dst] = (unsigned char)(gray * 4);
				datadst[pos_dst + 1] = (unsigned char)255;
				datadst[pos_dst + 2] = (unsigned char)0;
			}
			else if (gray <= 255)
			{
				datadst[pos_dst] = (unsigned char)255;
				datadst[pos_dst + 1] = (unsigned char)(255 - gray * 4);
				datadst[pos_dst + 2] = (unsigned char)0;
			}
		}
	}
	return 1;
}


int vc_rgb_temp_get_percentage(IVC* srcdst)
{
	unsigned char* data = (unsigned char*)srcdst->data;
	int with = srcdst->width;
	int height = srcdst->height;
	int bytesperline = srcdst->bytesperline;
	int channels = srcdst->channels;
	int x, y;
	long int pos;
	int RedCount=0,Allcount=0,YellowCount=0,BlueCount=0, GreenCount=0;

	//verificaçao de erros
	if ((srcdst->width <= 0) || (srcdst->height <= 0) || (srcdst->data == NULL)) return 0;
	if (channels != 3) return 0;
	
	//inverter imagem RGB

	for (y = 0; y < height; y++)
	{
		for (x = 0; x < with; x++)
		{
			//Allcount++;
			//-Cor vermelha : entre 1 e 30; Corresponde a 76 a 100 % atividade cerebral;

			//-Cor amarela : entre 31 e 90; Corresponde a 50 a 75 % atividade cerebral;

			//-Cor verde : entre 91 e 180; Corresponde a 25 a 50 % atividade cerebral;

			//-Cor azul : entre 180 e 360; Corresponde a 0 a 25 % atividade cerebral;
			pos = y * bytesperline + x * channels;
			if (data[pos] > (1*255)/360 && data[pos] <= (30*255)/360)
				
			{
				Allcount++;
				RedCount++;
				data[pos] = 255;
				data[pos+1] = 255;
				data[pos+2] = 255;
			}
			
			if (data[pos] >= (31 * 255) / 360 && data[pos] <= (90 * 255) / 360)
			{
				Allcount++;
				YellowCount++;
				data[pos] = 255;
				data[pos + 1] = 255;
				data[pos + 2] = 255;
			}
			
			if (data[pos] >= (91 * 255) / 360 && data[pos] <= (180 * 255) / 360)
			{
				Allcount++;
				GreenCount++;
				data[pos] = 255;
				data[pos + 1] = 255;
				data[pos + 2] = 255;
			}
			if (data[pos] > (180 * 255) / 360)
			{
				Allcount++;
				BlueCount++;
				data[pos] = 255;
				data[pos + 1] = 255;
				data[pos + 2] = 255;
			}
		}
	}
	printf("- Cor vermelha: entre 1 e 30 Corresponde a 76 a 100 atividade cerebral -quant a vermelho: %d / %d  = %.2f \n", RedCount,Allcount, (float)RedCount * 100 / Allcount);
	printf("- Cor amarela: entre 31 e 90 Corresponde a 50 a 75 atividade cerebral -quant a pixeis a Amarelo: %d / %d = %.2f \n", YellowCount, Allcount, (float)YellowCount*100 / Allcount);
	printf("- Cor verde: entre 91 e 180 Corresponde a 25 a 50 atividade cerebral -quant a pixeis a Verde: %d / %d = %.2f \n", GreenCount, Allcount, (float)GreenCount*100 / Allcount);
	printf("- Cor azul: entre 180 e 360 Corresponde a 0 a 25 atividade cerebral-quant a pixeis a Azul: %d / %d = %.2f \n", BlueCount, Allcount, (float)BlueCount*100 / Allcount);
	printf("quantidade pixeisalterados %d / %d = %.2f \n", RedCount + YellowCount + GreenCount + BlueCount, Allcount, (float)((RedCount + YellowCount + GreenCount + BlueCount) * 100 / Allcount));

	return 1;
}
int vc_gray_to_binary(IVC* srcdst, int threshold)
{
	unsigned char* data = (unsigned char*)srcdst->data;
	int with = srcdst->width;
	int height = srcdst->height;
	int bytesperline = srcdst->bytesperline;
	int channels = srcdst->channels;
	int x, y;
	long int pos;

	//verificaçao de erros
	if ((srcdst->width <= 0) || (srcdst->height <= 0) || (srcdst->data == NULL)) return 0;
	if (channels != 1) return 0;

	//inverter imagem Gray

	for (y = 0; y < height; y++)
	{
		for (x = 0; x < with; x++)
		{
			pos = y * bytesperline + x * channels;
			if (data[pos] <= threshold)
			{
				data[pos] = 0;
			}
			else
				data[pos] = 255;
		}
	}
	return 1;
}
int vc_gray_to_binary_global_mean(IVC* srcdst)
{
	unsigned char* data = (unsigned char*)srcdst->data;
	int with = srcdst->width;
	int height = srcdst->height;
	int bytesperline = srcdst->bytesperline;
	int channels = srcdst->channels;
	int x, y;
	long int pos;
	int somaval=0,quantidade=0;
	//verificaçao de erros
	if ((srcdst->width <= 0) || (srcdst->height <= 0) || (srcdst->data == NULL)) return 0;
	if (channels != 1) return 0;
	
//inverter imagem Gray
	for (y = 0; y < height; y++)
	{
		for (x = 0; x < with; x++)
		{
			pos = y * bytesperline + x * channels;
			somaval = somaval + data[pos];
			quantidade++;		
		}
	}
	float media = 0;
	media = (float)somaval / quantidade; 
	for (y = 0; y < height; y++)
	{
		for (x = 0; x < with; x++)
		{
			pos = y * bytesperline + x * channels;
			
			if (data[pos] <= (unsigned char) media)
			{
				data[pos] = 255;
			}
			else
				data[pos] = 0;
		}
	}
}

int vc_gray_to_binary_midpoint(IVC* srcdst, int kernel)
{
	unsigned char* data = (unsigned char*)srcdst->data;
	int with = srcdst->width;
	int height = srcdst->height;
	int bytesperline = srcdst->bytesperline;
	int channels = srcdst->channels;
	int x, y, wx, wy;
	long int pos;

	// verificaçao de erros
	if ((srcdst->width <= 0) || (srcdst->height <= 0) || (srcdst->data == NULL)) return 0;
	if (channels != 1) return 0;


	for (y = 0; y < height; y++)
	{
		for (x = 0; x < with; x++)
		{
			int soma = 0, quantidade = 0;
			int threshold;
			//calcular media da janela
			for (wy = y-((kernel-1)/2); wy <= y + ((kernel - 1) / 2); wy++)
			{
				for (wx = x - ((kernel - 1) / 2); wx <= x + ((kernel - 1) / 2); wx++)
				{
					if (wy < height &&  wx < with && wy>=0 && wx>=0)
					{
						pos = (wy) * bytesperline  + wx * channels;
						soma += data[pos];
						quantidade++;
					}
				}	
			}

			if (soma > 0)
			{
				threshold = soma / quantidade;
				pos = y * bytesperline + x * channels;
				if (data[pos] <= threshold)
				{
					data[pos] = 0;
				}
				else
				{
					data[pos] = 255;
				}
			}
			
		}

	}





	//// percorrer cada janela
	//for (y = 0; y < height; y += kernel)
	//{
	//	for (x = 0; x < with; x += kernel)
	//	{
	//		

	//		// calcular valor médio da janela
	//		

	//		// definir limiar adaptativo
	//		threshold = soma / quantidade;

	//		// binarização adaptativa da janela
	//		for (wy = 0; wy < kernel; wy++)
	//		{
	//			for (wx = 0; wx < kernel; wx++)
	//			{
	//				if (y + wy < height && x + wx < with)
	//				{
	//					pos = (y + wy) * bytesperline + (x + wx) * channels;
	//					if (data[pos] <= threshold)
	//					{
	//						data[pos] = 0;
	//					}
	//					else
	//					{
	//						data[pos] = 255;
	//					}
	//				}
	//			}
	//		}
	//	}
	//}

	//return 1;
}
//int vc_binary_dilate(IVC* src, IVC* dst, int size)
//{
//	// info source
//	unsigned char* datasrc = (unsigned char*)src->data;
//	int bytesperline_src = src->width * src->channels;
//	int channels_src = src->channels;
//
//	// info destino
//	unsigned char* datadst = (unsigned char*)dst->data;
//	int bytesperline_dst = dst->width * dst->channels;
//	int channels_dst = dst->channels;
//
//	// medidas
//	int width = src->width;
//	int height = src->height;
//
//	// auxiliares gerais
//	int x, y, xk, yk, i, j, s1, s2;
//	long int pos_src, posk;
//	unsigned char pixel;
//
//	// verificação de erros
//	if ((src->width <= 0) || (src->height <= 0) || (src->data == NULL)) return 0;
//	if ((src->width != dst->width) || (src->height != dst->height)) return 0;
//	if ((src->channels != 1) || (dst->channels != 1)) return 0;
//
//	// inicializa a imagem de destino com pixels pretos
//	memset(datadst, 0, bytesperline_dst * height);
//
//	s2 = (size - 1) / 2;
//	s1 = -s2;
//	for (y = 0; y < height; y++)
//	{
//		for (x = 0; x < width; x++)
//		{
//			pos_src = y * bytesperline_src + x * channels_src;
//			pixel = datasrc[pos_src];
//			for (yk = s1; yk <= s2; yk++)
//			{
//				j = y + yk;
//				if ((j < 0) || (j >= height)) continue;
//				for (xk = s1; xk <= s2; xk++)
//				{
//					i = x + xk;
//					if ((i < 0) || (i >= width)) continue;
//					posk = j * bytesperline_src + i * channels_src;
//					pixel &= datasrc[posk];
//				}
//			}
//
//			if (pixel != 0) datadst[pos_src] = 255;
//		}
//	}
//
//	return 1;
//}
//
//int vc_binary_erode(IVC* src, IVC* dst, int size)
//{
//	// info source
//	unsigned char* datasrc = (unsigned char*)src->data;
//	int bytesperline_src = src->width * src->channels;
//	int channels_src = src->channels;
//
//	// info destino
//	unsigned char* datadst = (unsigned char*)dst->data;
//	int bytesperline_dst = dst->width * dst->channels;
//	int channels_dst = dst->channels;
//
//	// medidas
//	int width = src->width;
//	int height = src->height;
//
//	// auxiliares gerais
//	int x, y, xk, yk, i, j, s1, s2;
//	long int pos_src, posk;
//	unsigned char pixel;
//
//	// verificação de erros
//	if ((src->width <= 0) || (src->height <= 0) || (src->data == NULL)) return 0;
//	if ((src->width != dst->width) || (src->height != dst->height)) return 0;
//	if ((src->channels != 1) || (dst->channels != 1)) return 0;
//
//	s2 = (size - 1) / 2;
//	s1 = -s2;
//	for (y = 0; y < height; y++)
//	{
//		for (x = 0; x < width; x++)
//		{
//			pos_src = y * bytesperline_src + x * channels_src;
//			pixel = datasrc[pos_src];
//			for (yk = s1; yk <= s2; yk++)
//			{
//				j = y + yk;
//				if ((j < 0) || (j >= height)) continue;
//				for (xk = s1; xk <= s2; xk++)
//				{
//					i = x + xk;
//					if ((i < 0) || (i >= width)) continue;
//					posk = j * bytesperline_src + i * channels_src;
//					pixel &= datasrc[posk];
//				}
//			}
//
//			if (pixel == 0) datadst[pos_src] = 0;
//		}
//	}
//
//	return 1;
//}


//
//int vc_binary_dilate(IVC* src, IVC* dst, int kernel)
//{
//	unsigned char* datasrc = (unsigned char*)src->data;
//	int bytesperline_src = src->width * src->channels;
//	int channels_src = src->channels;
//
//	unsigned char* datadst = (unsigned char*)dst->data;
//	int bytesperline_dst = dst->width * dst->channels;
//	int channels_dst = dst->channels;
//
//	int width = src->width;
//	int height = src->height;
//
//	long int pos;
//	long int pos_2;
//	int offset = kernel / 2;
//
//	int is_first_plan;
//
//	// Verificação de erros
//	if ((src->width <= 0) || (src->height <= 0) || (src->data == NULL))
//		return 0;
//	if ((src->width != dst->width) || (src->height != dst->height))
//		return 0;
//	if ((src->channels != 1) || (dst->channels != 1))
//		return 0;
//
//	for (int y = 0; y < height; y++)
//	{
//		for (int x = 0; x < width; x++)
//		{
//			is_first_plan = 0;
//			for (int k_y = -offset; k_y <= offset; k_y++)
//			{
//				for (int k_x = -offset; k_x <= offset; k_x++)
//				{
//
//					pos_2 = (y + k_y) * bytesperline_src + (x + k_x) * channels_src;
//					if ((y + k_y) > 0 && (x + k_x) > 0 && (y + k_y) < height && (x + k_x) < width)
//					{
//						if (datasrc[pos_2] == 0)
//						{
//							is_first_plan = 1;
//						}
//					}
//				}
//			}
//
//			pos = y * bytesperline_src + x * channels_src;
//			if (is_first_plan == 1)
//			{
//				datadst[pos] = 0;
//			}
//			else
//			{
//				datadst[pos] = dst->levels;
//			}
//		}
//	}
//	return 1;
//}
int vc_binary_dilate(IVC* src, IVC* dst, int kernel) {

	unsigned char* datasrc = (unsigned char*)src->data;
	int bytesperline_src = src->width * src->channels;
	int channels_src = src->channels;
	unsigned char* datadst = (unsigned char*)dst->data;
	int width = src->width;
	int height = src->height;
	int x, y, yy, xx;
	long int pos_src, posk;

	// Verificação de Erros
	if ((src->width <= 0) || (src->height <= 0) || (src->data == NULL)) return 0;
	if ((src->width != dst->width) || (src->height != dst->height)) return 0;
	if ((src->channels != 1) || (dst->channels != 1)) return 0;

	int offset = kernel / 2;
	int verifica;

	for (y = offset; y <= height - offset; y++) {

		for (x = offset; x <= width - offset; x++) {

			pos_src = y * bytesperline_src + x * channels_src;

			verifica = 0;

			for (yy = y - offset; yy <= y + offset; yy++) {

				for (xx = x - offset; xx <= x + offset; xx++) {

					posk = yy * bytesperline_src + xx * channels_src;

					if (datasrc[posk] == 255) { verifica = 255; }

				}
			}

			datadst[pos_src] = verifica;
		}
	}

	return 1;

}

int vc_binary_erode(IVC* src, IVC* dst, int kernel) {
	unsigned char* datasrc = (unsigned char*)src->data;
	int bytesperline_src = src->width * src->channels;
	int channels_src = src->channels;
	unsigned char* datadst = (unsigned char*)dst->data;
	int width = src->width;
	int height = src->height;
	int bytesperline_dst = dst->width * dst->channels;
	int channels_dst = dst->channels;
	int x, y, x2, y2;
	long int pos_src, pos_dst;
	int verifica;
	int offset = kernel / 2;

	if ((src->width <= 0) || (src->height <= 0) || (src->data == NULL)) return 0;
	if ((src->width != dst->width) || (src->height != dst->height)) return 0;
	if ((src->channels != 1) || (dst->channels != 1)) return 0;

	for (y = 0; y < height; y++) {
		for (x = 0; x < width; x++) {
			pos_dst = y * bytesperline_dst + x * channels_dst;
			verifica = 255;

			// Verifica se está nas bordas
			if (y < offset || y >= height - offset || x < offset || x >= width - offset) {
				datadst[pos_dst] = 0;  // Define como preto nas bordas
				continue;
			}

			for (y2 = y - offset; y2 <= y + offset; y2++) {
				for (x2 = x - offset; x2 <= x + offset; x2++) {
					pos_src = y2 * bytesperline_src + x2 * channels_src;

					if (datasrc[pos_src] == 0) {
						verifica = 0;
						break;  // Termina o loop interno se encontrar um valor preto
					}
				}

				if (verifica == 0) {
					break;  // Termina o loop externo se encontrar um valor preto
				}
			}

			datadst[pos_dst] = verifica;
		}
	}

	return 1;
}




//
//
//int vc_binary_erode(IVC* src, IVC* dst, int kernel)
//{
//	unsigned char* datasrc = (unsigned char*)src->data;
//	int bytesperline_src = src->width * src->channels;
//	int channels_src = src->channels;
//
//	unsigned char* datadst = (unsigned char*)dst->data;
//	int bytesperline_dst = dst->width * dst->channels;
//	int channels_dst = dst->channels;
//
//	int width = src->width;
//	int height = src->height;
//
//	long int pos;
//	long int pos_2;
//	int offset = kernel / 2;
//
//	int is_not_first_plan;
//
//	// Verificação de erros
//	if ((src->width <= 0) || (src->height <= 0) || (src->data == NULL))
//		return 0;
//	if ((src->width != dst->width) || (src->height != dst->height))
//		return 0;
//	if ((src->channels != 1) || (dst->channels != 1))
//		return 0;
//
//	for (int y = 0; y < height; y++)
//	{
//		for (int x = 0; x < width; x++)
//		{
//			is_not_first_plan = 0;
//			for (int k_y = -offset; k_y <= offset; k_y++)
//			{
//				for (int k_x = -offset; k_x <= offset; k_x++)
//				{
//
//					pos_2 = (y + k_y) * bytesperline_src + (x + k_x) * channels_src;
//					if ((y + k_y) > 0 && (x + k_x) > 0 && (y + k_y) < height && (x + k_x) < width)
//					{
//						if (datasrc[pos_2] == 255)
//						{
//							is_not_first_plan = 1;
//						}
//					}
//				}
//			}
//
//			pos = y * bytesperline_src + x * channels_src;
//			if (is_not_first_plan == 1)
//			{
//				datadst[pos] = dst->levels;
//			}
//			else
//			{
//				datadst[pos] = 0;
//			}
//		}
//	}
//	return 1;
//}

//Operadores Morfologicos (Binarios): Open
//E obtida por uma erosao, seguida de uma dilatacao.Utilizada para remover pequenas regiões de primeiro plano.
int vc_bin_open(IVC* src, IVC* dst, int kernel)
{
	int open = 1;
	IVC* aux = vc_image_new(src->width, src->height, 1, src->levels);

	open &= vc_binary_erode(src, aux, kernel);
	open &= vc_binary_dilate(aux, dst, kernel);

	vc_image_free(aux);
	return open;
}

//Operadores Morfologicos (Binarios): Close
//E obtida por uma dilatacao, seguida de uma erosao. Utilizada para preencher falhas dentro de regiões de primeiro plano.
int vc_bin_close(IVC* src, IVC* dst, int kernel)
{
	int close = 1;
	IVC* aux = vc_image_new(src->width, src->height, 1, src->levels);

	close &= vc_binary_dilate(src, aux, kernel);
	close &= vc_binary_erode(aux, dst, kernel);

	vc_image_free(aux);
	return close;
}

//int vc_gray_erode(IVC* src, IVC* dst, int kernel)
//{
//	unsigned char* datasrc = (unsigned char*)src->data;
//	int bytesperline_src = src->width * src->channels;
//	int channels_src = src->channels;
//
//	unsigned char* datadst = (unsigned char*)dst->data;
//	int bytesperline_dst = dst->width * dst->channels;
//	int channels_dst = dst->channels;
//
//	int width = src->width;
//	int height = src->height;
//
//	long int pos;
//	long int pos_2;
//	int offset = kernel / 2;
//
//	int min;
//
//	// Verificação de erros
//	if ((src->width <= 0) || (src->height <= 0) || (src->data == NULL))
//		return 0;
//	if ((src->width != dst->width) || (src->height != dst->height))
//		return 0;
//	if ((src->channels != 1) || (dst->channels != 1))
//		return 0;
//
//	for (int y = 0; y < height; y++)
//	{
//		for (int x = 0; x < width; x++)
//		{
//			min = src->levels;
//			for (int k_y = -offset; k_y <= offset; k_y++)
//			{
//				for (int k_x = -offset; k_x <= offset; k_x++)
//				{
//
//					pos_2 = (y + k_y) * bytesperline_src + (x + k_x) * channels_src;
//					if ((y + k_y) > 0 && (x + k_x) > 0 && (y + k_y) < height && (x + k_x) < width)
//					{
//						if (datasrc[pos_2] < min)
//						{
//							min = datasrc[pos_2];
//						}
//					}
//				}
//			}
//
//			pos = y * bytesperline_src + x * channels_src;
//			datadst[pos] = min;
//		}
//	}
//	return 1;
//}
int vc_gray_erode(IVC* src, IVC* dst, int kernel)
{
	unsigned char* datasrc = (unsigned char*)src->data;
	int bytesperline_src = src->width * src->channels;
	int channels_src = src->channels;

	unsigned char* datadst = (unsigned char*)dst->data;
	int bytesperline_dst = dst->width * dst->channels;
	int channels_dst = dst->channels;

	int width = src->width;
	int height = src->height;

	long int pos;
	long int pos_2;
	int offset = kernel / 2;

	int min;

	// Verificação de erros
	if ((src->width <= 0) || (src->height <= 0) || (src->data == NULL))
		return 0;
	if ((src->width != dst->width) || (src->height != dst->height))
		return 0;
	if (src->channels != 1 || dst->channels != 1)
		return 0;

	for (int y = 0; y < height; y++)
	{
		for (int x = 0; x < width; x++)
		{
			min = 255;
			for (int k_y = -offset; k_y <= offset; k_y++)
			{
				for (int k_x = -offset; k_x <= offset; k_x++)
				{
					pos_2 = (y + k_y) * bytesperline_src + (x + k_x) * channels_src;
					if ((y + k_y) >= 0 && (x + k_x) >= 0 && (y + k_y) < height && (x + k_x) < width)
					{
						if (datasrc[pos_2] < min)
						{
							min = datasrc[pos_2];
						}
					}
				}
			}

			pos = y * bytesperline_src + x * channels_src;
			datadst[pos] = min;
		}
	}

	return 1;
}


int vc_gray_dilate(IVC* src, IVC* dst, int kernel)
{
	unsigned char* datasrc = (unsigned char*)src->data;
	int bytesperline_src = src->width * src->channels;
	int channels_src = src->channels;

	unsigned char* datadst = (unsigned char*)dst->data;
	int bytesperline_dst = dst->width * dst->channels;
	int channels_dst = dst->channels;

	int width = src->width;
	int height = src->height;

	long int pos;
	long int pos_2;
	int offset = kernel / 2;

	int max;

	// Verificação de erros
	if ((src->width <= 0) || (src->height <= 0) || (src->data == NULL))
		return 0;
	if ((src->width != dst->width) || (src->height != dst->height))
		return 0;
	if (src->channels != 1 || dst->channels != 1)
		return 0;

	for (int y = 0; y < height; y++)
	{
		for (int x = 0; x < width; x++)
		{
			max = 0;
			for (int k_y = -offset; k_y <= offset; k_y++)
			{
				for (int k_x = -offset; k_x <= offset; k_x++)
				{
					pos_2 = (y + k_y) * bytesperline_src + (x + k_x) * channels_src;
					if ((y + k_y) >= 0 && (x + k_x) >= 0 && (y + k_y) < height && (x + k_x) < width)
					{
						if (datasrc[pos_2] > max)
						{
							max = datasrc[pos_2];
						}
					}
				}
			}

			pos = y * bytesperline_src + x * channels_src;
			datadst[pos] = max;
		}
	}

	return 1;
}


int vc_gray_open(IVC* src, IVC* dst, int kernel)
{
	IVC* image_aux;
	image_aux = vc_image_new(src->width, src->height, src->channels, src->levels);
	vc_gray_erode(src, image_aux, kernel);
	vc_gray_dilate(image_aux, dst, kernel);
	vc_image_free(image_aux);
}

int vc_gray_close(IVC* src, IVC* dst, int kernel)
{
	IVC* image_aux;
	image_aux = vc_image_new(src->width, src->height, src->channels, src->levels);
	vc_gray_dilate(src, image_aux, kernel);
	vc_gray_erode(image_aux, dst, kernel);
	vc_image_free(image_aux);
}

int vc_gray_histogram_show(IVC* src, IVC* dst)
{
	unsigned char* datasrc = (unsigned char*)src->data;
	int bytesperline_src = src->width * src->channels;
	int channels_src = src->channels;

	unsigned char* datadst = (unsigned char*)dst->data;
	int bytesperline_dst = dst->width * dst->channels;
	int channels_dst = dst->channels;
	
	int width = src->width;
	int height = src->height;
	
	int x, y;
	long int pos_src, pos_dst;
	float rf, gf, bf;
	
	int his[256] = { 0 };
	float pdf[256] = { 0 };
	int total = 0;
	float max = 0;
	//verificaçao de erros
	if ((src->width <= 0) || (src->height <= 0) || (src->data == NULL)) return 0;
	if ((src->width != dst->width) || (src->height != dst->height))  return 0;
	if ((src->channels != 1) || (dst->channels != 1)) return 0;
	total = src->width * src->height;


	//inverter imagem RGB

	for (y = 0; y < height; y++)
	{
		for (x = 0; x < width; x++)
		{
			pos_src = y * bytesperline_src + x * channels_src;
			his[datasrc[pos_src]] ++;
			
		}
	}



	for (int i = 0; i < 256; i++)
	{
		if (his[i] > max)
		{
			max = (his[i]);
		}
	}


	for (int i = 0; i < 256; i++)
	{
		pdf[i] = (float)his[i] / max;
		//pdf[i] = (float)his[i] /(float) total;
	}


	// Gera o gr�fico com o histograma
	for (int i = 0, x = (width - 256) / 2; i < 256; i++, x++)
	{
		for (y = height - 1; y > height - 1 - pdf[i] * height; y--)
		{
			datadst[y * bytesperline_dst + x * channels_dst] = 255;
		}
	}

	// Desenha linhas de in�cio (itensidade = 0) e fim (intensidade = 255)
	for (y = 0; y < height - 1; y++)
	{
		datadst[y * bytesperline_dst + ((width - 256) / 2 - 1) * channels_dst] = 127;
		datadst[y * bytesperline_dst + ((width + 256) / 2 + 1) * channels_dst] = 127;
	}
	return 1;


}


int equalize(unsigned char* datasrc, unsigned char* datadst, int bytesperline, int channels, int offset, int width, int height)
{

	if (channels != 1 && channels != 3)
		return 0;

	if (channels == 1)
	{
		int ni[256] = { 0 };

		for (int y = 0; y < height; y++)
		{
			for (int x = 0; x < width; x++)
			{
				int pos = y * bytesperline + x * channels;
				ni[datasrc[pos]]++;
			}
		}

		float pdf[256] = { 0 };
		int n = width * height;

		for (int i = 0; i < 256; i++)
		{
			pdf[i] = (float)ni[i] / (float)n;
		}

		float pdfmax = pdf[0];
		for (int i = 1; i < 256; i++)
		{
			if (pdf[i] > pdfmax)
			{
				pdfmax = pdf[i];
			}
		}
		float cdf[256];
		float sum = 0;
		for (int i = 0; i < 256; i++)
		{
			sum += pdf[i];
			cdf[i] = sum;
		}

		float cdfmin = -1;
		for (int i = 0; i < 256 && cdfmin == -1; i++)
		{
			cdfmin = cdf[i];
		}

		for (int y = 0; y < height; y++)
		{
			for (int x = 0; x < width; x++)
			{
				long int pos = y * bytesperline + x * channels;
				datadst[pos] = ((cdf[datasrc[pos]] - cdfmin) / (1 - cdfmin)) * 255;
			}
		}
	}
	else if (channels == 3)
	{
		for (int c = 0; c < channels; c++)
		{
			int ni[256] = { 0 };

			for (int y = 0; y < height; y++)
			{
				for (int x = 0; x < width; x++)
				{
					int pos = y * bytesperline + x * channels + c;
					ni[datasrc[pos]]++;
				}
			}

			float pdf[256] = { 0 };
			int n = width * height;

			for (int i = 0; i < 256; i++)
			{
				pdf[i] = (float)ni[i] / (float)n;
			}

			float pdfmax = pdf[0];
			for (int i = 1; i < 256; i++)
			{
				if (pdf[i] > pdfmax)
				{
					pdfmax = pdf[i];
				}
			}
			float cdf[256];
			float sum = 0;
			for (int i = 0; i < 256; i++)
			{
				sum += pdf[i];
				cdf[i] = sum;
			}

			float cdfmin = -1;
			for (int i = 0; i < 256 && cdfmin == -1; i++)
			{
				cdfmin = cdf[i];
			}

			for (int y = 0; y < height; y++)
			{
				for (int x = 0; x < width; x++)
				{
					long int pos = y * bytesperline + x * channels + c;
					datadst[pos] = ((cdf[datasrc[pos]] - cdfmin) / (1 - cdfmin)) * 255;
				}
			}
		}
	}

	return 1;
}

int vc_gray_histogram_equalization(IVC* src, IVC* dst)
{
	unsigned char* datasrc = (unsigned char*)src->data;
	unsigned char* datadst = (unsigned char*)dst->data;
	int width = src->width;
	int height = src->height;
	int bytesperline = src->bytesperline;
	int channels = src->channels;

	// Verifica��o de erros
	if ((src->width <= 0) || (src->height <= 0) || (src->data == NULL) || (src->width != dst->width) || src->height != dst->height)
		return 0;
	if (channels != 1)
		return 0;
	return equalize(datasrc, datadst, bytesperline, channels, 0, width, height);
}




int vc_rgb_histogram_equalization(IVC* src, IVC* dst)
{
	unsigned char* datasrc = (unsigned char*)src->data;
	unsigned char* datadst = (unsigned char*)dst->data;
	int width = src->width;
	int height = src->height;
	int bytesperline = src->bytesperline;
	int channels = src->channels;
	long int pos;

	if (channels != 3)
		return 0;

	int res = equalize(datasrc, datadst, bytesperline, channels, 0, width, height);

	return res;
}


// Etiquetagem de blobs
// src		: Imagem bin�ria
// dst		: Imagem grayscale (ir� conter as etiquetas)
// nlabels	: Endere�o de mem�ria de uma vari�vel inteira. Recebe o n�mero de etiquetas encontradas.
// OVC*		: Retorna lista de estruturas de blobs (objectos), com respectivas etiquetas. � necess�rio libertar posteriormente esta mem�ria.
OVC* vc_binary_blob_labelling(IVC* src, IVC* dst, int* nlabels)
{
	unsigned char* datasrc = (unsigned char*)src->data;
	unsigned char* datadst = (unsigned char*)dst->data;
	int width = src->width;
	int height = src->height;
	int bytesperline = src->bytesperline;
	int channels = src->channels;
	int x, y, a, b;
	long int i, size;
	long int posX, posA, posB, posC, posD;
	int labeltable[256] = { 0 };
	int labelarea[256] = { 0 };
	int label = 1; // Etiqueta inicial.
	int num, tmplabel;
	OVC* blobs; // Apontador para lista de blobs (objectos) que ser� retornada desta fun��o.

	// Verifica��o de erros
	if ((src->width <= 0) || (src->height <= 0) || (src->data == NULL)) return 0;
	if ((src->width != dst->width) || (src->height != dst->height) || (src->channels != dst->channels)) return NULL;
	if (channels != 1) return NULL;

	// Copia dados da imagem bin�ria para imagem grayscale
	memcpy(datadst, datasrc, bytesperline * height);

	// Todos os pix�is de plano de fundo devem obrigat�riamente ter valor 0
	// Todos os pix�is de primeiro plano devem obrigat�riamente ter valor 255
	// Ser�o atribu�das etiquetas no intervalo [1,254]
	// Este algoritmo est� assim limitado a 254 labels
	for (i = 0, size = bytesperline * height; i < size; i++)
	{
		if (datadst[i] != 0) datadst[i] = 255;
	}

	// Limpa os rebordos da imagem bin�ria
	for (y = 0; y < height; y++)
	{
		datadst[y * bytesperline + 0 * channels] = 0;
		datadst[y * bytesperline + (width - 1) * channels] = 0;
	}
	for (x = 0; x < width; x++)
	{
		datadst[0 * bytesperline + x * channels] = 0;
		datadst[(height - 1) * bytesperline + x * channels] = 0;
	}

	// Efectua a etiquetagem
	for (y = 1; y < height - 1; y++)
	{
		for (x = 1; x < width - 1; x++)
		{
			// Kernel:
			// A B C
			// D X

			posA = (y - 1) * bytesperline + (x - 1) * channels; // A
			posB = (y - 1) * bytesperline + x * channels; // B
			posC = (y - 1) * bytesperline + (x + 1) * channels; // C
			posD = y * bytesperline + (x - 1) * channels; // D
			posX = y * bytesperline + x * channels; // X

			// Se o pixel foi marcado
			if (datadst[posX] != 0)
			{
				if ((datadst[posA] == 0) && (datadst[posB] == 0) && (datadst[posC] == 0) && (datadst[posD] == 0))
				{
					datadst[posX] = label;
					labeltable[label] = label;
					label++;
				}
				else
				{
					num = 255;

					// Se A est� marcado
					if (datadst[posA] != 0) num = labeltable[datadst[posA]];
					// Se B est� marcado, e � menor que a etiqueta "num"
					if ((datadst[posB] != 0) && (labeltable[datadst[posB]] < num)) num = labeltable[datadst[posB]];
					// Se C est� marcado, e � menor que a etiqueta "num"
					if ((datadst[posC] != 0) && (labeltable[datadst[posC]] < num)) num = labeltable[datadst[posC]];
					// Se D est� marcado, e � menor que a etiqueta "num"
					if ((datadst[posD] != 0) && (labeltable[datadst[posD]] < num)) num = labeltable[datadst[posD]];

					// Atribui a etiqueta ao pixel
					datadst[posX] = num;
					labeltable[num] = num;

					// Actualiza a tabela de etiquetas
					if (datadst[posA] != 0)
					{
						if (labeltable[datadst[posA]] != num)
						{
							for (tmplabel = labeltable[datadst[posA]], a = 1; a < label; a++)
							{
								if (labeltable[a] == tmplabel)
								{
									labeltable[a] = num;
								}
							}
						}
					}
					if (datadst[posB] != 0)
					{
						if (labeltable[datadst[posB]] != num)
						{
							for (tmplabel = labeltable[datadst[posB]], a = 1; a < label; a++)
							{
								if (labeltable[a] == tmplabel)
								{
									labeltable[a] = num;
								}
							}
						}
					}
					if (datadst[posC] != 0)
					{
						if (labeltable[datadst[posC]] != num)
						{
							for (tmplabel = labeltable[datadst[posC]], a = 1; a < label; a++)
							{
								if (labeltable[a] == tmplabel)
								{
									labeltable[a] = num;
								}
							}
						}
					}
					if (datadst[posD] != 0)
					{
						if (labeltable[datadst[posD]] != num)
						{
							for (tmplabel = labeltable[datadst[posD]], a = 1; a < label; a++)
							{
								if (labeltable[a] == tmplabel)
								{
									labeltable[a] = num;
								}
							}
						}
					}
				}
			}
		}
	}

	// Volta a etiquetar a imagem
	for (y = 1; y < height - 1; y++)
	{
		for (x = 1; x < width - 1; x++)
		{
			posX = y * bytesperline + x * channels; // X

			if (datadst[posX] != 0)
			{
				datadst[posX] = labeltable[datadst[posX]];
			}
		}
	}


	// Contagem do n�mero de blobs
	// Passo 1: Eliminar, da tabela, etiquetas repetidas
	for (a = 1; a < label - 1; a++)
	{
		for (b = a + 1; b < label; b++)
		{
			if (labeltable[a] == labeltable[b]) labeltable[b] = 0;
		}
	}
	// Passo 2: Conta etiquetas e organiza a tabela de etiquetas, para que n�o hajam valores vazios (zero) entre etiquetas
	*nlabels = 0;
	for (a = 1; a < label; a++)
	{
		if (labeltable[a] != 0)
		{
			labeltable[*nlabels] = labeltable[a]; // Organiza tabela de etiquetas
			(*nlabels)++; // Conta etiquetas
		}
	}

	// Se n�o h� blobs
	if (*nlabels == 0) return NULL;


	// Cria lista de blobs (objectos) e preenche a etiqueta
	blobs = (OVC*)calloc((*nlabels), sizeof(OVC));
	if (blobs != NULL)
	{
		for (a = 0; a < (*nlabels); a++) blobs[a].label = labeltable[a];
	}
	else return NULL;

	return blobs;
}


////int vc_binary_blob_labelling(IVC* src, IVC* dst, int size)
//{
//	unsigned char* datasrc = (unsigned char*)src->data;
//	unsigned char* datadst = (unsigned char*)dst->data;
//	int width = src->width;
//	int height = src->height;
//	int bytesperline = src->bytesperline;
//	int channels = src->channels;
//	int x, y, a, b;
//	long int i, size;
//	long int posX, posA, posB, posC, posD;
//	int labeltable[256] = { 0 };
//	int labelarea[256] = { 0 };
//	int label = 1; //etiqueta inicial
//	int num, tmplabel;
//	OVC* blobs; //apontador para lista de blobs "2632"
//
//
//
//	// verificação de erros
//	if ((src->width <= 0) || (src->height <= 0) || (src->data == NULL)) return 0;
//	if ((src->width != dst->width) || (src->height != dst->height) || (src->channels !=dst->channels)) return NULL;
//	if (channels != 1)  return NULL;
//
//	//copia dados imagem binaria para imagem grayscale
//	memcpy(datadst, datasrc, bytesperline * height);
//
//	for (i = 0, size = bytesperline * height; i < size; i++)
//	{
//		if (datadst[i] != 0)
//		{
//			datadst[i] = 255;
//		}
//
//		
//	}
//
//
//
//		// Limpa a borda da imagem
//	// Limpa a borda da imagem
//	for (y = 0; y < height; y++) {
//		datadst[y * bytesperline] = 0; // Define o primeiro pixel da linha como preto
//		datadst[y * bytesperline + width - 1] = 0; // Define o último pixel da linha como preto
//	}
//
//	for (x = 0; x < width; x++) {
//		datadst[x] = 0; // Define o primeiro pixel da coluna como preto
//		datadst[(height - 1) * bytesperline + x] = 0; // Define o último pixel da coluna como preto
//	}
//
//
//
//
//
////efetua etiquetagem
//	for (y = 1; y < height-1; y++)
//	{
//		for (x = 1; x < width-1; x++)
//		{
//			
//			posA = (y - 1) * bytesperline + (x - 1) * channels;
//			posB= (y - 1) * bytesperline + x * channels;
//			posC= (y - 1) * bytesperline + (x + 1) * channels; 
//			posC = y* bytesperline + (x - 1) * channels; 
//			posX = y * bytesperline + x * channels;
//
//			if (datadst[posX] != 0)
//			{
//				if ((datadst[posA]) == 0 && (datadst[posB] == 0) && (datadst[posC] == 0) && (datadst[posD] == 0))
//				{
//					datadst[posX] = label;
//					labeltable[label] = label;
//					label++;
//
//				}
//				else
//				{
//					num = 255;
//					//Se a esta marcado
//					if (datadst[posA] != 0) {
//						num = labeltable[datadst[posA]];
//					}
//
//					if ((datadst[posB] != 0) && (labeltable[datadst[posB]]<num))
//					{
//						num = labeltable[datadst[posB]];
//					}
//					if ((datadst[posC] != 0) && (labeltable[datadst[posC]] < num))
//					{
//						num = labeltable[datadst[posC]];
//					}
//					if ((datadst[posD] != 0) && (labeltable[datadst[posD]] < num))
//					{
//						num = labeltable[datadst[posD]];
//					}
//
//
//					//ATRIBUI ETIQUETA AO PIXEL
//					datadst[posX] = num;
//					labeltable[num] = num;
//
//					//Atualiza a table de etiquetas
//					if (datadst[posA] != 0)
//					{
//						if (labeltable[datadst[posA]] != num)
//						{
//							for (tmplabel = labeltable[datadst[posA]], a = 1; a < label; a++)
//							{
//								if (labeltable[a] == tmplabel)
//								{
//									labeltable[a] = num;
//								}
//							}
//
//						}
//					}
//					if (datadst[posB] != 0)
//					{
//						if (labeltable[datadst[posB]] != num)
//						{
//							for (tmplabel = labeltable[datadst[posB]], a = 1; a < label; a++)
//							{
//								if (labeltable[a] == tmplabel)
//								{
//									labeltable[a] = num;
//								}
//							}
//						}
//					}
//					if (datadst[posC] != 0)
//					{
//						if (labeltable[datadst[posC]] != num)
//						{
//							for (tmplabel = labeltable[datadst[posC]], a = 1; a < label; a++)
//							{
//								if (labeltable[a] == tmplabel)
//								{
//									labeltable[a] = num;
//								}
//							}
//
//						}
//					}
//
//
//
//					if (datadst[posD] != 0)
//					{
//						if (labeltable[datadst[posD]] != num)
//						{
//							for (tmplabel = labeltable[datadst[posD]], a = 1; a < label; a++)
//							{
//								if (labeltable[a] == tmplabel)
//								{
//									labeltable[a] = num;
//								}
//							}
//
//						}
//					}
//
//
//				}
//			}
//		}
//
//		
//	}
//	//VOLTA A ETIQUETAR
//	for (y = 1; y < height - 1; y++)
//	{
//		for (x = 1; x < width - 1; x++)
//		{
//			posX = y * bytesperline + x * channels;
//			if (datadst[posX != 0])
//			{
//				datadst[posX] = labeltable[datadst[posX]];
//			}
//
//		}
//	}
//
//	//contagem de numero de blobs 
//	//passo 1 apagar etiquetas repetidas
//
//	for (a = 1; a < label - 1; a++)
//	{
//		for (b = a + 1; b < label; b++)
//		{
//			if (labeltable[a] == labeltable[b]) {
//				labeltable[b] = 0;
//			}
//		}
//
//	}
//
//
//	return 1;
//}
//
//

// area numero de pixeis do objeto n 
//bounding box  []


        
//sabel [-1(x-1,y-1) 0 1(x+1,y-1) ]
//x	  [-2 0xy 2 ]
//	[-1 0 1]
//
//sabel[-1 2 -1]
//x	[0 0 0]
//	[1 2 1]

//
//prewitt
//x [-1 0 1]
//  [-1 0 1]
//  [-1 0 1]
//
//y [-1 -1 -1]
//  [0 0 0]
//  [1 1 1]
//
//





int vc_gray_edge_sobel(IVC* src, IVC* dst, float th)
{
	// info source
	unsigned char* datasrc = (unsigned char*)src->data;
	int bytesperline_src = src->width * src->channels;
	int channels_src = src->channels;

	//info destino
	unsigned char* datadst = (unsigned char*)dst->data;
	int bytesperline_dst = dst->width * dst->channels;
	int channels_dst = dst->channels;

	// medidas
	int width = src->width;
	int height = src->height;

	//auxiliares gerais
	int x, y;
	long int pos_src, pos_dst;
	float valuepixel,somapixeisx,somapixeisy,mag;


	//verificação de erros
	if ((src->width <= 0) || (src->height <= 0) || (src->data == NULL)) return 0;
	if ((src->width != dst->width) || (src->height != dst->height)) return 0;
	if ((src->channels != 1) || (dst->channels != 1)) return 0;
	somapixeisx = 0;
	somapixeisy = 0;
	for (y = 1; y < height-1; y++)
	{
		for (x = 1; x < width-1; x++)
		{
			somapixeisx = 0;
			somapixeisy = 0;
			//calculos para x
			
			pos_src = (y-1 )* bytesperline_src + (x-1) * channels_src;// primeira posicao
			somapixeisx = somapixeisx + ((float)datasrc[pos_src] * -1);

			pos_src = (x-1) * bytesperline_src + x  * channels_src;//  segunda
			somapixeisx = somapixeisx + (float)datasrc[pos_src] * 0;//que vai dar zero

			pos_src =( y - 1) * bytesperline_src + (x+1) * channels_src;//  terceira
			somapixeisx = somapixeisx + (float)datasrc[pos_src] * 1;

			pos_src = y  * bytesperline_src + (x - 1) * channels_src;//  quarta
			somapixeisx = somapixeisx + (float)datasrc[pos_src] * -2;

			pos_src = y * bytesperline_src + x * channels_src;//posicao da source //pixel central
			somapixeisx = somapixeisx + (float)datasrc[pos_src] * 0;//que vai dar zero


			pos_src = y * bytesperline_src + (x+1) * channels_src;//sexta
			somapixeisx = somapixeisx + (float)datasrc[pos_src] * 2;
			
			pos_src = (y+1) * bytesperline_src + (x - 1) * channels_src;//7
			somapixeisx = somapixeisx + (float)datasrc[pos_src] * -1;

			pos_src = (y + 1) * bytesperline_src + x  * channels_src;//8
			somapixeisx = somapixeisx + (float)datasrc[pos_src] * 0;// vai dar zero

			pos_src = (y + 1 )* bytesperline_src + (x +1) * channels_src;//9
			somapixeisx = somapixeisx + (float)datasrc[pos_src] * 1;
			
			
			//calculos para y



			pos_src = (y - 1) * bytesperline_src + (x - 1 )* channels_src;// primeira posicao
			somapixeisy = somapixeisy + (float)datasrc[pos_src] * -1;

			pos_src =( y - 1 )* bytesperline_src + x * channels_src;//  segunda
			somapixeisy = somapixeisy + (float)datasrc[pos_src] * -2;//que vai dar zero

			pos_src = (y - 1) * bytesperline_src +(x + 1) * channels_src;//  terceira
			somapixeisy = somapixeisy + (float)datasrc[pos_src] * -1;

			pos_src = y * bytesperline_src + (x - 1) * channels_src;//  quarta
			somapixeisy = somapixeisy + (float)datasrc[pos_src] * 0;

			pos_src = y * bytesperline_src + x * channels_src;//posicao da source //pixel central
			somapixeisy = somapixeisy + (float)datasrc[pos_src] * 0;//que vai dar zero


			pos_src = y * bytesperline_src + (x + 1) * channels_src;//sexta
			somapixeisy = somapixeisy + (float)datasrc[pos_src] * 0;

			pos_src = (y + 1) * bytesperline_src + (x - 1) * channels_src;//7
			somapixeisy = somapixeisy + (float)datasrc[pos_src] * 1;

			pos_src = (y + 1) * bytesperline_src + x * channels_src;//8
			somapixeisy = somapixeisy + (float)datasrc[pos_src] * 2;// vai dar zero

			pos_src = (y + 1) * bytesperline_src + (x + 1) * channels_src;//9
			somapixeisy = somapixeisy + (float)datasrc[pos_src] * 1;


			mag = (somapixeisx * somapixeisx) + (somapixeisy * somapixeisy);
			mag = sqrt(mag);

			pos_dst = y * bytesperline_dst + x * channels_dst;//posicao destino
			//datadst[pos_dst] = (unsigned char)mag;

			//100 thressold do professor;
			if (mag>th)
			{
				datadst[pos_dst] = 255;

			}
			else
				datadst[pos_dst] = 0;
		
		}


	}
	return 1;


}

int vc_gray_lowpass_mean_filter(IVC* src, IVC* dst, int size)
{
	// info source
	unsigned char* datasrc = (unsigned char*)src->data;
	int bytesperline_src = src->width * src->channels;
	int channels_src = src->channels;

	//info destino
	unsigned char* datadst = (unsigned char*)dst->data;
	int bytesperline_dst = dst->width * dst->channels;
	int channels_dst = dst->channels;

	// medidas
	int width = src->width;
	int height = src->height;

	// auxiliares gerais
	int x, y, xk, yk, i, j, s1, s2;
	long int pos_src, posk, pos_dst;
	unsigned char pixel;
	float soma,media;
	// verificação de erros
	if ((src->width <= 0) || (src->height <= 0) || (src->data == NULL)) return 0;
	if ((src->width != dst->width) || (src->height != dst->height)) return 0;
	if ((src->channels != 1) || (dst->channels != 1)) return 0;

	s2 = (size - 1) / 2;
	s1 = -s2;
	for (y = 0; y < height; y++)
	{
		for (x = 0; x < width; x++)
		{
			soma = 0;
			pos_src = y * bytesperline_src + x * channels_src;
			pixel = datasrc[pos_src];
			for (yk = s1; yk <= s2; yk++)
			{
				j = y + yk;
				if ((j < 0) || (j >= height)) continue;
				for (xk = s1; xk <= s2; xk++)
				{
					i = x + xk;
					if ((i < 0) || (i >= width)) continue;
					posk = j * bytesperline_src + i * channels_src;
					pixel = datasrc[posk];
					soma = soma + (pixel);

				}
			}
			media = soma / (size * size);
			pos_dst = y * bytesperline_dst + x * channels_dst;//posicao destino
			datadst[pos_dst] = (unsigned char)media;

			
		}
	}

	return 1;


}

int vc_gray_lowpass_median_filter(IVC* src, IVC* dst, int size)
{
	// info source
	unsigned char* datasrc = (unsigned char*)src->data;
	int bytesperline_src = src->width * src->channels;
	int channels_src = src->channels;

	//info destino
	unsigned char* datadst = (unsigned char*)dst->data;
	int bytesperline_dst = dst->width * dst->channels;
	int channels_dst = dst->channels;

	// medidas
	int width = src->width;
	int height = src->height;

	// auxiliares gerais
	int x, y, xk, yk, i, j, s1, s2;
	long int pos_src, posk, pos_dst;
	unsigned char pixel;
	float soma, median;
	long int valores[100];
	int valoresI;
	// verificação de erros
	if ((src->width <= 0) || (src->height <= 0) || (src->data == NULL)) return 0;
	if ((src->width != dst->width) || (src->height != dst->height)) return 0;
	if ((src->channels != 1) || (dst->channels != 1)) return 0;

	s2 = (size - 1) / 2;
	s1 = -s2;
	for (y = 0; y < height; y++)
	{
		for (x = 0; x < width; x++)
		{
			soma = 0;
			pos_src = y * bytesperline_src + x * channels_src;
			pixel = datasrc[pos_src];
			valoresI = 0;
			for (yk = s1; yk <= s2; yk++)
			{
				j = y + yk;
				if ((j < 0) || (j >= height)) continue;
				for (xk = s1; xk <= s2; xk++)
				{
					i = x + xk;
					if ((i < 0) || (i >= width)) continue;
					posk = j * bytesperline_src + i * channels_src;
					pixel = datasrc[posk];
					
						valores[valoresI] = pixel;
			
					valoresI++;

				}

			}
			// ordenaçao
			int temp = 0;
			for (i = 0; i < valoresI - 1; i++)
			{
				for (j = 0; j < valoresI - i - 1; j++)
				{
					if (valores[j] > valores[j + 1])
					{
						temp = valores[j];
						valores[j] = valores[j + 1];
						valores[j + 1] = temp;
					}
				}
			}
			median = valores[valoresI / 2];
			pos_dst = y * bytesperline_dst + x * channels_dst;//posicao destino
			datadst[pos_dst] = (unsigned char)median;


		}
	}

	return 1;


}

int vc_gray_highpass_filter(IVC* src, IVC* dst,int tipo)
{

	// info source
	unsigned char* datasrc = (unsigned char*)src->data;
	int bytesperline_src = src->width * src->channels;
	int channels_src = src->channels;

	//info destino
	unsigned char* datadst = (unsigned char*)dst->data;
	int bytesperline_dst = dst->width * dst->channels;
	int channels_dst = dst->channels;

	// medidas
	int width = src->width;
	int height = src->height;

	// auxiliares gerais
	int x, y, xk, yk, i, j, s1, s2;
	long int pos_src, posk, pos_dst;
	unsigned char pixel;
	float soma, media;
	float valuepixel, somapixeisx, somapixeisy, mag;
	// verificação de erros
	if ((src->width <= 0) || (src->height <= 0) || (src->data == NULL)) return 0;
	if ((src->width != dst->width) || (src->height != dst->height)) return 0;
	if ((src->channels != 1) || (dst->channels != 1)) return 0;

	s2 = (3 - 1) / 2;
	s1 = -s2;
	for (y = 0; y < height; y++)
	{
		for (x = 0; x < width; x++)
		{
			soma = 0;
			pos_src = y * bytesperline_src + x * channels_src;
			pixel = datasrc[pos_src];
			


			somapixeisx = 0;
		
			//calculos para x
			if (tipo = 1)
			{

	
			pos_src = (y - 1) * bytesperline_src + (x - 1) * channels_src;// primeira posicao
			somapixeisx = somapixeisx + ((float)datasrc[pos_src] * 0);

			pos_src = (x - 1) * bytesperline_src + x * channels_src;//  segunda
			somapixeisx = somapixeisx + (float)datasrc[pos_src] * -1;//que vai dar zero

			pos_src = (y - 1) * bytesperline_src + (x + 1) * channels_src;//  terceira
			somapixeisx = somapixeisx + (float)datasrc[pos_src] * 0;

			pos_src = y * bytesperline_src + (x - 1) * channels_src;//  quarta
			somapixeisx = somapixeisx + (float)datasrc[pos_src] * -1;

			pos_src = y * bytesperline_src + x * channels_src;//posicao da source //pixel central
			somapixeisx = somapixeisx + (float)datasrc[pos_src] * 4;//que vai dar zero


			pos_src = y * bytesperline_src + (x + 1) * channels_src;//sexta
			somapixeisx = somapixeisx + (float)datasrc[pos_src] * -1;

			pos_src = (y + 1) * bytesperline_src + (x - 1) * channels_src;//7
			somapixeisx = somapixeisx + (float)datasrc[pos_src] * 0;

			pos_src = (y + 1) * bytesperline_src + x * channels_src;//8
			somapixeisx = somapixeisx + (float)datasrc[pos_src] * -1;// vai dar zero

			pos_src = (y + 1) * bytesperline_src + (x + 1) * channels_src;//9
			somapixeisx = somapixeisx + (float)datasrc[pos_src] * 0;

			if (somapixeisx > 255)
			{
				somapixeisx = 255;
			}

			if (somapixeisx < 0)
			{
				somapixeisx = 0;
			}
			somapixeisx = somapixeisx / 6;
			}

			if (tipo = 2)
			{


				pos_src = (y - 1) * bytesperline_src + (x - 1) * channels_src;// primeira posicao
				somapixeisx = somapixeisx + ((float)datasrc[pos_src] * -1);

				pos_src = (x - 1) * bytesperline_src + x * channels_src;//  segunda
				somapixeisx = somapixeisx + (float)datasrc[pos_src] * -1;//que vai dar zero

				pos_src = (y - 1) * bytesperline_src + (x + 1) * channels_src;//  terceira
				somapixeisx = somapixeisx + (float)datasrc[pos_src] * -1;

				pos_src = y * bytesperline_src + (x - 1) * channels_src;//  quarta
				somapixeisx = somapixeisx + (float)datasrc[pos_src] * -1;

				pos_src = y * bytesperline_src + x * channels_src;//posicao da source //pixel central
				somapixeisx = somapixeisx + (float)datasrc[pos_src] * 8;//que vai dar zero


				pos_src = y * bytesperline_src + (x + 1) * channels_src;//sexta
				somapixeisx = somapixeisx + (float)datasrc[pos_src] * -1;

				pos_src = (y + 1) * bytesperline_src + (x - 1) * channels_src;//7
				somapixeisx = somapixeisx + (float)datasrc[pos_src] * -1;

				pos_src = (y + 1) * bytesperline_src + x * channels_src;//8
				somapixeisx = somapixeisx + (float)datasrc[pos_src] * -1;// vai dar zero

				pos_src = (y + 1) * bytesperline_src + (x + 1) * channels_src;//9
				somapixeisx = somapixeisx + (float)datasrc[pos_src] * -1;

				if (somapixeisx > 255)
				{
					somapixeisx = 255;
				}

				if (somapixeisx < 0)
				{
					somapixeisx = 0;
				}
				somapixeisx = somapixeisx / 9	;
			}


			
			
			pos_dst = y * bytesperline_dst + x * channels_dst;//posicao destino
			datadst[pos_dst] = (unsigned char)somapixeisx;


		}
	}

	return 1;

}

int vc_binary_blob_info(IVC* src, OVC* blobs, int nblobs)
{
	unsigned char* data = (unsigned char*)src->data;
	int width = src->width;
	int height = src->height;
	int bytesperline = src->bytesperline;
	int channels = src->channels;
	int x, y, i;
	long int pos;
	int xmin, ymin, xmax, ymax;

	// Verificação de erros
	if ((src->width <= 0) || (src->height <= 0) || (src->data == NULL))
		return 0;
	if (channels != 1)
		return 0;

	// Conta área de cada blob
	for (i = 0; i < nblobs; i++)
	{
		xmin = width - 1;
		ymin = height - 1;
		xmax = 0;
		ymax = 0;

		blobs[i].area = 0;

		for (y = 0; y < height; y++)
		{
			for (x = 0; x < width; x++)
			{
				pos = y * bytesperline + x * channels;

				if (data[pos] == blobs[i].label)
				{
					// Área
					blobs[i].area++;

					// Bounding Box
					if (xmin > x)
						xmin = x;
					if (ymin > y)
						ymin = y;
					if (xmax < x)
						xmax = x;
					if (ymax < y)
						ymax = y;
				}
			}
		}

		// Bounding Box
		blobs[i].x = xmin;
		blobs[i].y = ymin;
		blobs[i].width = (xmax - xmin) + 1;
		blobs[i].heigh = (ymax - ymin) + 1;
	}

	return 1;
}



// Desenha a caixa delimitadora de um objecto
int vc_draw_boundingbox(IVC* srcdst, OVC* blob)
{
	int c;
	int x, y;

	for (y = blob->y; y < blob->y + blob->heigh; y++)
	{
		for (c = 0; c < srcdst->channels; c++)
		{
			srcdst->data[y * srcdst->bytesperline + blob->x * srcdst->channels] = 255;
			srcdst->data[y * srcdst->bytesperline + (blob->x + blob->width - 1) * srcdst->channels] = 255;
		}
	}

	for (x = blob->x; x < blob->x + blob->width; x++)
	{
		for (c = 0; c < srcdst->channels; c++)
		{
			srcdst->data[blob->y * srcdst->bytesperline + x * srcdst->channels] = 255;
			srcdst->data[(blob->y + blob->heigh - 1) * srcdst->bytesperline + x * srcdst->channels] = 255;
		}
	}

	return 1;
}




// Desenha o centro de gravidade de um objecto
int vc_draw_centerofgravity(IVC* srcdst, OVC* blob)
{
	int c;
	int x, y;
	int xmin, xmax, ymin, ymax;
	int s = 3;

	xmin = blob->xc - s;
	ymin = blob->yc - s;
	xmax = blob->xc + s;
	ymax = blob->yc + s;

	if (xmin < blob->x) xmin = blob->x;
	if (ymin < blob->y) ymin = blob->y;
	if (xmax > blob->x + blob->width - 1) xmax = blob->x + blob->width - 1;
	if (ymax > blob->y + blob->heigh - 1) ymax = blob->y + blob->heigh - 1;

	for (y = ymin; y <= ymax; y++)
	{
		for (c = 0; c < srcdst->channels; c++)
		{
			srcdst->data[y * srcdst->bytesperline + blob->xc * srcdst->channels] = 255;
		}
	}

	for (x = xmin; x <= xmax; x++)
	{
		for (c = 0; c < srcdst->channels; c++)
		{
			srcdst->data[blob->yc * srcdst->bytesperline + x * srcdst->channels] = 255;
		}
	}

	return 1;
}


int vc_hsv_segmentationalterada(IVC* src, IVC* dst, int hmin, int hmax, int smin, int smax, int vmin, int vmax)
{
	unsigned char* datasrc = (unsigned char*)src->data;
	int bytesperline_src = src->width * src->channels;
	int channels_src = src->channels;
	unsigned char* datadst = (unsigned char*)dst->data;
	int bytesperline_dst = dst->width * dst->channels;
	int channels_dst = dst->channels;
	int width = src->width;
	int height = src->height;
	int x, y;
	long int pos_src, pos_dst;

	hmax = (hmax * 255) / 360;
	hmin = (hmin * 255) / 360;
	smax = (smax * 255) / 100;
	smin = (smin * 255) / 100;
	vmax = (vmax * 255) / 100;
	vmin = (vmin * 255) / 100;

	// Verificação de erros
	if ((src->width <= 0) || (src->height <= 0) || (src->data == NULL)) return 0;
	if ((src->width != dst->width) || (src->height != dst->height))  return 0;
	if ((src->channels != 3) || (dst->channels != 1)) return 0;

	// Pré-processamento da imagem
	for (y = 0; y < height; y++)
	{
		for (x = 0; x < width; x++)
		{
			pos_src = y * bytesperline_src + x * channels_src;
			pos_dst = y * bytesperline_dst + x * channels_dst;

			if (
				((datasrc[pos_src] >= hmin) && (datasrc[pos_src] <= hmax)) &&
				((datasrc[pos_src + 1] >= smin) && (datasrc[pos_src + 1] <= smax)) &&
				((datasrc[pos_src + 2] >= vmin) && (datasrc[pos_src + 2] <= vmax))
				)
			{
				datadst[pos_dst] = 255;
			}
			else
			{
				datadst[pos_dst] = 0;
			}
		}
	}

	return 1;
}

int vc_remove_bg(IVC* srcOriginal, IVC* srcMask, IVC* dst) {
	unsigned char* dataOriginal = (unsigned char*)srcOriginal->data;
	int bytesperlineOriginal = srcOriginal->width * srcOriginal->channels;
	int channelsOriginal = srcOriginal->channels;

	unsigned char* dataMask = (unsigned char*)srcMask->data;
	int bytesperlineMask = srcMask->width * srcMask->channels;
	int channelsMask = srcMask->channels;

	unsigned char* dataDst = (unsigned char*)dst->data;
	int bytesperlineDst = dst->width * dst->channels;
	int channelsDst = dst->channels;

	int width = srcOriginal->width;
	int height = srcOriginal->height;

	int x, y;
	long int pos;

	if ((srcOriginal->width <= 0) || (srcOriginal->height <= 0) || (srcOriginal->data == NULL))
		return 0;
	if ((srcOriginal->width != srcMask->width) || (srcOriginal->height != srcMask->height))
		return 0;
	if ((srcOriginal->channels != 3) || (srcMask->channels != 1) || (dst->channels != 3))
		return 0;

	for (y = 0; y < height; y++)
	{
		for (x = 0; x < width; x++)
		{
			pos = y * bytesperlineOriginal + x * channelsOriginal;

			if (dataMask[y * bytesperlineMask + x * channelsMask] == 255)
			{
				dataDst[pos] = dataOriginal[pos];
				dataDst[pos + 1] = dataOriginal[pos + 1];
				dataDst[pos + 2] = dataOriginal[pos + 2];
			}
			else
			{
				dataDst[pos] = 0;
				dataDst[pos + 1] = 0;
				dataDst[pos + 2] = 0;
			}
		}
	}

	return 1;
}
float cv_segmentAndCalculateredAreaPercentage(IVC* src, IVC* dst, int hmin, int hmax, int smin, int smax, int vmin, int vmax)
{
	unsigned char* datasrc = (unsigned char*)src->data;
	int bytesperline_src = src->width * src->channels;
	int channels_src = src->channels;
	unsigned char* datadst = (unsigned char*)dst->data;
	int bytesperline_dst = dst->width * dst->channels;
	int channels_dst = dst->channels;
	int width = src->width;
	int height = src->height;
	int x, y;
	long int pos_src, pos_dst;

	hmax = (hmax * 255) / 360;
	hmin = (hmin * 255) / 360;
	smax = (smax * 255) / 100;
	smin = (smin * 255) / 100;
	vmax = (vmax * 255) / 100;
	vmin = (vmin * 255) / 100;

	int redPixels = 0;
	int totalPixels = width * height;

	// Percorrer a imagem original e realizar a segmentação
	for (y = 0; y < height; y++)
	{
		for (x = 0; x < width; x++)
		{
			pos_src = y * bytesperline_src + x * channels_src;
			pos_dst = y * bytesperline_dst + x * channels_dst;

			if (
				((datasrc[pos_src] >= hmin) && (datasrc[pos_src] <= hmax))
				&& ((datasrc[pos_src + 1] >= smin)) && ((datasrc[pos_src + 1] <= smax))
				&& ((datasrc[pos_src + 2] >= vmin) && (datasrc[pos_src + 2] <= vmax))
				)
			{
				datadst[pos_dst] = 255;
				redPixels++;
			}
			else
			{
				datadst[pos_dst] = 0;
			}
		}
	}

	float blueAreaPercentage = (float)redPixels / totalPixels * 100;
	return blueAreaPercentage;
}


float calculateNonRedAreaPercentage(IVC* image) //devolve area nao vermelha logo e azul
{
	// Cria uma estrutura IVC para a imagem segmentada
	IVC* segmentedIvc = vc_image_new(image->width, image->height, image->channels, 1);

	// Converte a imagem para o espaço de cor HSV
	vc_rgb_to_hsv(image);

	// Define os valores estáticos para a faixa de cor azul no espaço de cor HSV
	int hmin = 0;
	int hmax = 91;
	int smin = 50;
	int smax = 100;
	int vmin = 50;
	int vmax = 100;

	// Calcula a porcentagem de área azul
	float nonRedAreaPercentage = cv_segmentAndCalculateredAreaPercentage(image, segmentedIvc, hmin, hmax, smin, smax, vmin, vmax);

	// Libera a memória da estrutura IVC segmentada
	vc_image_free(segmentedIvc);

	return nonRedAreaPercentage;
}

//função que realiza a binarização, por thresholding obtido pelos valores de uma dada vizinhanca, de uma imagem em tons de cinzento
int vc_gray_to_binary_bernsen(IVC* src, IVC* dst, int kernel, int Cmin) {
	unsigned char* datasrc = (unsigned char*)src->data; //verificar se a imagem é mesmo cinzenta!
	int bytesperline_src = src->width * src->channels;
	int channels_src = src->channels;

	unsigned char* datadst = (unsigned char*)dst->data;
	int bytesperline_dst = dst->width * dst->channels;
	int channels_dst = dst->channels;

	int width = src->width;
	int height = src->height;

	int x, y;
	long int pos_src, pos_dst;

	//verificação de erros:
	if ((src->width <= 0) || (src->height <= 0) || (src->data == NULL)) return 0;
	if ((src->width != dst->width) || (src->height != dst->height)) return 0;
	if ((src->channels != 1) || (dst->channels != 1)) return 0;

	//verificação do valor do Kernel (ímpar!)
	if (kernel % 2 == 0) {
		kernel += 1;
		printf("Atencao!\nO valor do kernel foi alterado para %d\n", kernel);
	}
	int aux = (kernel - 1) / 2;

	//verificação do valor de Cmin
	if (Cmin < 1) {
		printf("Valor de Cmin negativo!");
		return 0;
	}

	//binarização automática (método Adaptativo de Bernsen)
	int pos_viz = 0;
	float threshold = 0;

	for (y = aux; y < height - aux; y++)
	{
		for (x = aux; x < width - aux; x++)
		{
			pos_src = y * bytesperline_src + x * channels_src;
			pos_dst = y * bytesperline_dst + x * channels_dst;
			float min = datasrc[pos_src];
			float max = datasrc[pos_src];

			for (int i = y - aux; i < y + aux; i++)
			{
				for (int j = x - aux; j < x + aux; j++)
				{
					pos_viz = i * bytesperline_src + j * channels_src;
					if (datasrc[pos_viz] < min)
					{
						min = datasrc[pos_viz];
					}
					if (datasrc[pos_viz] > max)
					{
						max = datasrc[pos_viz];
					}
				}
			}
			if ((max - min) < Cmin)
			{
				threshold = (src->levels) / 2.0f;
			}
			else
			{
				threshold = 0.5f * (min + max);
			}
			if (datasrc[pos_src] > threshold)
			{
				datadst[pos_dst] = 255;
			}
			else
			{
				datadst[pos_dst] = 0;
			}
		}
	}
	dst->levels = 1;
	return 1;
}

int getobjectmaskbyboundingbox(IVC* srcdst, OVC* blob)
{
	int x, y, c;

	for (y = 0; y < srcdst->height; y++)
	{
		for (x = 0; x < srcdst->width; x++)
		{
			int pos = y * srcdst->bytesperline + x * srcdst->channels;
			int isInsideBox = (x >= blob->x && x < blob->x + blob->width && y >= blob->y && y < blob->y + blob->heigh);

			for (c = 0; c < srcdst->channels; c++)
			{
				if (isInsideBox)
				{
					//srcdst->data[pos + c] = 255;  // Define o pixel como branco (255) se estiver dentro da caixa delimitadora
				}
				else
				{
					srcdst->data[pos + c] = 0;  // Define o pixel como preto (0) se estiver fora da caixa delimitadora
				}
			}
		}
	}

	return 1;
}
int cv_segmentAndCalculateColorAreaPercentageV2(IVC* image, int hmin_red, int hmax_red, int smin_red, int smax_red, int vmin_red, int vmax_red, int hmin_blue, int hmax_blue, int smin_blue, int smax_blue, int vmin_blue, int vmax_blue)
{
	// Cria uma estrutura IVC para a imagem segmentada
	IVC* segmentedIvc_red = vc_image_new(image->width, image->height, image->channels, 1);
	IVC* segmentedIvc_blue = vc_image_new(image->width, image->height, image->channels, 1);

	// Converte a imagem para o espaço de cor HSV
	vc_rgb_to_hsv(image);

	// Calcula a porcentagem de área vermelha
	float redAreaPercentage = cv_segmentAndCalculateredAreaPercentage(image, segmentedIvc_red, hmin_red, hmax_red, smin_red, smax_red, vmin_red, vmax_red);

	// Calcula a porcentagem de área azul
	float blueAreaPercentage = cv_segmentAndCalculateredAreaPercentage(image, segmentedIvc_blue, hmin_blue, hmax_blue, smin_blue, smax_blue, vmin_blue, vmax_blue);

	// Libera a memória das estruturas IVC segmentadas
	vc_image_free(segmentedIvc_red);
	vc_image_free(segmentedIvc_blue);

	if (redAreaPercentage > blueAreaPercentage)
	{
		return 1; // Vermelho
	}
	else if (blueAreaPercentage > redAreaPercentage)
	{
		return 2; // Azul
	}
	else
	{
		return 3; // Outra cor
	}
}
int getColorCode(IVC* image)
{
	// Cria uma estrutura IVC para a imagem segmentada
	IVC* segmentedIvc = vc_image_new(image->width, image->height, image->channels, 1);

	// Converte a imagem para o espaço de cor HSV
	vc_rgb_to_hsv(image);

	// Define os valores para a faixa de cor vermelha no espaço de cor HSV
	int hmin_red = 0;
	int hmax_red = 10;
	int smin_red = 120;
	int smax_red = 255;
	int vmin_red = 120;
	int vmax_red = 255;

	// Define os valores para a faixa de cor azul no espaço de cor HSV
	int hmin_blue = 200;
	int hmax_blue = 260;
	int smin_blue = 50;
	int smax_blue = 255;
	int vmin_blue = 50;
	int vmax_blue = 255;

	// Calcula o código de cor
	int colorCode = cv_segmentAndCalculateColorAreaPercentageV2(image, hmin_red, hmax_red, smin_red, smax_red, vmin_red, vmax_red, hmin_blue, hmax_blue, smin_blue, smax_blue, vmin_blue, vmax_blue);

	// Libera a memória da estrutura IVC segmentada
	vc_image_free(segmentedIvc);

	return colorCode;
}


float calculateNonBlueAreaPercentage(IVC* image)
{
	// Cria uma estrutura IVC para a imagem segmentada
	IVC* segmentedIvc = vc_image_new(image->width, image->height, image->channels, 1);

	// Converte a imagem para o espaço de cor HSV
	vc_rgb_to_hsv(image);

	// Define os valores estáticos para a faixa de cor azul no espaço de cor HSV
	int hmin_blue = 200;
	int hmax_blue = 260;
	int smin_blue = 50;
	int smax_blue = 255;
	int vmin_blue = 50;
	int vmax_blue = 255;
	

	// Calcula a porcentagem de área não azul
	float nonBlueAreaPercentage = 1.0 - cv_segmentAndCalculateredAreaPercentage(image, segmentedIvc, hmin_blue, hmax_blue, smin_blue, smax_blue, vmin_blue, vmax_blue);

	// Libera a memória da estrutura IVC segmentada
	vc_image_free(segmentedIvc);

	return nonBlueAreaPercentage;
}

bool isSquare(const OVC* blob) {
	double aspectRatio = (double)blob->width / blob->heigh;
	double perimeterToAreaRatio = (double)blob->perimeter / blob->area;

	// Verificar se a razão de aspecto está próxima de 1 e o perímetro em relação à área é menor que um certo valor
	if (fabs(1.0 - aspectRatio) <= 0.1 && perimeterToAreaRatio <= 1.2) {
		return true;
	}

	return false;
}

bool isRound(const OVC* blob) {
	double aspectRatio = (double)(blob->width) / blob->heigh;
	double roundness = (double)(blob->perimeter * blob->perimeter) / (4.0 * 3.14159 * blob->area);

	// Verificar se a razão de aspecto está próxima de 1 e a redondeza é maior que um certo valor
	if (fabs(1.0 - aspectRatio) <= 0.1 && roundness > 0.6) {
		return true;
	}

	return false;
}


int vc_gray_to_binary_bernsen2(IVC* src, IVC* dst, int kernel, int Cmin) {
	unsigned char* datasrc = (unsigned char*)src->data;
	int bytesperline_src = src->width * src->channels;
	int channels_src = src->channels;

	unsigned char* datadst = (unsigned char*)dst->data;
	int bytesperline_dst = dst->width * dst->channels;
	int channels_dst = dst->channels;

	int width = src->width;
	int height = src->height;

	int x, y;
	long int pos_src, pos_dst;

	if ((src->width <= 0) || (src->height <= 0) || (src->data == NULL)) return 0;
	if ((src->width != dst->width) || (src->height != dst->height)) return 0;
	if ((src->channels != 1) || (dst->channels != 1)) return 0;

	if (kernel % 2 == 0) {
		kernel += 1;
		printf("Atenção!\nO valor do kernel foi alterado para %d\n", kernel);
	}
	int aux = (kernel - 1) / 2;

	if (Cmin < 1) {
		printf("Valor de Cmin negativo!");
		return 0;
	}

	int pos_viz = 0;
	float threshold = 0;

	for (y = aux; y < height - aux; y++) {
		for (x = aux; x < width - aux; x++) {
			pos_src = y * bytesperline_src + x * channels_src;
			pos_dst = y * bytesperline_dst + x * channels_dst;
			float min = datasrc[pos_src];
			float max = datasrc[pos_src];

			for (int i = y - aux; i <= y + aux; i++) {
				for (int j = x - aux; j <= x + aux; j++) {
					if (i >= 0 && i < height && j >= 0 && j < width) {
						pos_viz = i * bytesperline_src + j * channels_src;
						if (datasrc[pos_viz] < min) {
							min = datasrc[pos_viz];
						}
						if (datasrc[pos_viz] > max) {
							max = datasrc[pos_viz];
						}
					}
				}
			}
			if ((max - min) < Cmin) {
				threshold = (src->levels) / 2.0f;
			}
			else {
				threshold = 0.5f * (min + max);
			}
			if (datasrc[pos_src] > threshold) { // Alteração: inverte a condição novamente
				datadst[pos_dst] = 255;
			}
			else {
				datadst[pos_dst] = 0;
			}
		}
	}
	dst->levels = 1;
	return 1;
}

void vc_process_image(IVC* src, IVC* dst, int threshold) {
	unsigned char* datasrc = (unsigned char*)src->data;
	unsigned char* datadst = (unsigned char*)dst->data;
	int width = src->width;
	int height = src->height;
	int channels_src = src->channels;
	int channels_dst = dst->channels;

	if ((src->width <= 0) || (src->height <= 0) || (src->data == NULL) || (dst->data == NULL)) return;
	if ((src->width != dst->width) || (src->height != dst->height)) return;
	if ((src->channels != 1) || (dst->channels != 1)) return;

	IVC* aux = vc_image_new(width, height, 1, channels_src);
	unsigned char* dataaux = (unsigned char*)aux->data;
	memcpy(dataaux, datasrc, width * height);

	int stackSize = 0;

	for (int y = 0; y < height; y++) {
		for (int x = 0; x < width; x++) {
			long int pos_src = y * width + x;
			long int pos_dst = y * width + x;

			if (datasrc[pos_src] > threshold) {
				datadst[pos_dst] = 255;
			}
			else {
				datadst[pos_dst] = 0;
			}

			if (datadst[pos_dst] == 255 && dataaux[pos_dst] == 255) {
				int pos = pos_dst;

				if (x >= 0 && x < width && y >= 0 && y < width && datasrc[pos] == 255) {
					datadst[pos] = 0;
					dataaux[pos] = 0;

					int neighbors[8] = {
						pos - width - 1, pos - width, pos - width + 1,
						pos - 1,                      pos + 1,
						pos + width - 1, pos + width, pos + width + 1
					};

					for (int i = 0; i < 8; i++) {
						int neighborX = neighbors[i] % width;
						int neighborY = neighbors[i] / width;
						int neighborPos = neighborY * width + neighborX;

						if (neighborX >= 0 && neighborX < width && neighborY >= 0 && neighborY < height && dataaux[neighborPos] == 255) {
							vc_recursive_fill(datadst, dataaux, width, height, neighborX, neighborY);
						}
					}
				}
			}
		}
	}

	vc_image_free(aux);
}

void vc_recursive_fill(unsigned char* datadst, unsigned char* dataaux, int width, int height, int x, int y) {
	int pos = y * width + x;

	if (x >= 0 && x < width && y >= 0 && y < width && dataaux[pos] == 255) {
		datadst[pos] = 0;
		dataaux[pos] = 0;

		int neighbors[8] = {
			pos - width - 1, pos - width, pos - width + 1,
			pos - 1,                      pos + 1,
			pos + width - 1, pos + width, pos + width + 1
		};

		for (int i = 0; i < 8; i++) {
			int neighborX = neighbors[i] % width;
			int neighborY = neighbors[i] / width;
			int neighborPos = neighborY * width + neighborX;

			if (neighborX >= 0 && neighborX < width && neighborY >= 0 && neighborY < height && dataaux[neighborPos] == 255) {
				vc_recursive_fill(datadst, dataaux, width, height, neighborX, neighborY);
			}
		}
	}
}

int determinaSetaEsquerdaOuDireita(IVC* srcdst, OVC* blob)
{
	int x, y;
	int width = blob->width;
	int height = blob->heigh;
	int center_x = blob->x + width / 2;
	int left_count = 0;
	int right_count = 0;

	// Percorre as colunas da região delimitada pela bounding box
	for (y = blob->y; y < blob->y + height; y++)
	{
		// Percorre a metade esquerda da região delimitada
		for (x = blob->x; x < center_x; x++)
		{
			int index = y * srcdst->bytesperline + x * srcdst->channels;
			unsigned char pixel_value = srcdst->data[index];

			// Verifica se o pixel é branco (valor 255)
			if (pixel_value == 255)
			{
				left_count++;
			}
		}

		// Percorre a metade direita da região delimitada
		for (x = center_x; x < blob->x + width; x++)
		{
			int index = y * srcdst->bytesperline + x * srcdst->channels;
			unsigned char pixel_value = srcdst->data[index];

			// Verifica se o pixel é branco (valor 255)
			if (pixel_value == 255)
			{
				right_count++;
			}
		}
	}

	// Compara as contagens e retorna o resultado
	if (left_count > right_count)
	{
		return 1;  // Maior densidade à esquerda
	}
	else if (right_count > left_count)
	{
		return 2;  // Maior densidade à direita
	}
	else
	{
		return 0;  // Densidades iguais
	}
}