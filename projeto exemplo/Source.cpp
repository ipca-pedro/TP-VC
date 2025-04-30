#include <iostream>
#include <string>
#include <opencv2\opencv.hpp>
#include <opencv2\core.hpp>
#include <opencv2\highgui.hpp>
#include <opencv2\videoio.hpp>

extern "C" {
#include "vc.h";
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <malloc.h>
#include "math.h"
#include <cstring>
}








int main_detecao_moveimento(void) {
	// V�deo

	cv::Mat frame1, frame2;
	
	char videofile[20] = "video.avi";
	cv::VideoCapture capture;
	struct
	{
		int width, height;
		int ntotalframes;
		int fps;
		int nframe;
	} video;

	
	// Outros
	std::string str;
	int key = 0;

	/* Leitura de v�deo de um ficheiro */
	/* NOTA IMPORTANTE:
	O ficheiro video.avi dever� estar localizado no mesmo direct�rio que o ficheiro de c�digo fonte.
	*/
	capture.open(videofile);

	/* Em alternativa, abrir captura de v�deo pela Webcam #0 */
	//capture.open(0, cv::CAP_DSHOW); // Pode-se utilizar apenas capture.open(0);

	/* Verifica se foi poss�vel abrir o ficheiro de v�deo */
	if (!capture.isOpened())
	{
		std::cerr << "Erro ao abrir o ficheiro de v�deo!\n";
		return 1;
	}

	/* N�mero total de frames no v�deo */
	video.ntotalframes = (int)capture.get(cv::CAP_PROP_FRAME_COUNT);
	/* Frame rate do v�deo */
	video.fps = (int)capture.get(cv::CAP_PROP_FPS);
	/* Resolu��o do v�deo */
	video.width = (int)capture.get(cv::CAP_PROP_FRAME_WIDTH);
	video.height = (int)capture.get(cv::CAP_PROP_FRAME_HEIGHT);

	/* Cria uma janela para exibir o v�deo */
	cv::namedWindow("Subtracao de plano de fundo", cv::WINDOW_AUTOSIZE);
	
		video.nframe = (int)capture.get(cv::CAP_PROP_POS_FRAMES);
		capture.read(frame1);
		while (capture.read(frame2))
		{
			//subtrair plano de fundo
			cv::Mat diff;
			cv::absdiff(frame1, frame2, diff);

			//converter para escla de cinza
			cv::cvtColor(diff, diff, cv::COLOR_BGR2GRAY);

			//aplicar um limiar para des
			cv::threshold(diff, diff, 80, 255, cv::THRESH_BINARY);

			//mostra o resultado
			cv::imshow("Subtracao de plano de fundo", diff);

			key = cv::waitKey(1);


		}


		/* Fecha a janela */
		cv::destroyWindow("Subtracao de plano de fundo");

		/* Fecha o ficheiro de v�deo */

		capture.release();

		return 0;
}

		///* Exemplo de inser��o texto na frame */
		//str = std::string("RESOLUCAO: ").append(std::to_string(video.width)).append("x").append(std::to_string(video.height));
		//cv::putText(frame, str, cv::Point(20, 25), cv::FONT_HERSHEY_SIMPLEX, 1.0, cv::Scalar(0, 0, 0), 2);
		//cv::putText(frame, str, cv::Point(20, 25), cv::FONT_HERSHEY_SIMPLEX, 1.0, cv::Scalar(255, 255, 255), 1);
		//str = std::string("TOTAL DE FRAMES: ").append(std::to_string(video.ntotalframes));
		//cv::putText(frame, str, cv::Point(20, 50), cv::FONT_HERSHEY_SIMPLEX, 1.0, cv::Scalar(0, 0, 0), 2);
		//cv::putText(frame, str, cv::Point(20, 50), cv::FONT_HERSHEY_SIMPLEX, 1.0, cv::Scalar(255, 255, 255), 1);
		//str = std::string("FRAME RATE: ").append(std::to_string(video.fps));
		//cv::putText(frame, str, cv::Point(20, 75), cv::FONT_HERSHEY_SIMPLEX, 1.0, cv::Scalar(0, 0, 0), 2);
		//cv::putText(frame, str, cv::Point(20, 75), cv::FONT_HERSHEY_SIMPLEX, 1.0, cv::Scalar(255, 255, 255), 1);
		//str = std::string("N. DA FRAME: ").append(std::to_string(video.nframe));
		//cv::putText(frame, str, cv::Point(20, 100), cv::FONT_HERSHEY_SIMPLEX, 1.0, cv::Scalar(0, 0, 0), 2);
		//cv::putText(frame, str, cv::Point(20, 100), cv::FONT_HERSHEY_SIMPLEX, 1.0, cv::Scalar(255, 255, 255), 1);


	


		/* Sai da aplica��o, se o utilizador premir a tecla 'q' */
		
int main_aula()//detecao_com filtro gauseano
{
	using namespace cv;
	using namespace std;

	Ptr<BackgroundSubtractor>pBackSub;
	//pBackSub = createBackgroundSubtractorMOG2(); // Cria um objeto de subtração de fundo MOG2
	pBackSub = createBackgroundSubtractorKNN();
	VideoCapture capture("video.avi"); // Abre o arquivo de vídeo
	if (!capture.isOpened()) {
		cerr << "unable to open: " << "video.avi" << endl; // Exibe uma mensagem de erro se o arquivo de vídeo não puder ser aberto
		return 0;
	}

	Mat frame, fgMask;
	while (true)
	{
		capture >> frame; // Lê o próximo quadro do vídeo
		if (frame.empty())
			break; // Sai do loop se o quadro estiver vazio (fim do vídeo)

		pBackSub->apply(frame, fgMask); // Aplica a subtração de fundo no quadro atual para obter a máscara de primeiro plano

		rectangle(frame, cv::Point(10, 2), cv::Point(100, 20), cv::Scalar(255, 255, 255, 255), -1); // Desenha um retângulo branco no canto superior esquerdo do quadro
		stringstream ss;
		ss << capture.get(CAP_PROP_POS_FRAMES); // Obtém o número do quadro atual
		string frameNumberString = ss.str(); // Converte o número do quadro para uma string
		putText(frame, frameNumberString.c_str(), cv::Point(15, 15), FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0, 0, 0)); // Escreve o número do quadro no quadro atual

		resize(frame, frame, cv::Size(600, 300)); // Redimensiona o quadro para uma largura de 600 pixels e altura de 300 pixels
		resize(fgMask, fgMask, cv::Size(600, 300)); // Redimensiona a máscara de primeiro plano para a mesma resolução do quadro
		cv::threshold(fgMask, fgMask, 254, 255, cv::THRESH_BINARY);
		imshow("frame", frame); // Exibe o quadro redimensionado em uma janela chamada "frame"
		imshow("fgmask", fgMask); // Exibe a máscara de primeiro plano redimensionada em uma janela chamada "fgmask"

		int keyboard = waitKey(1); // Aguarda uma tecla ser pressionada por 1 milissegundo
		if (keyboard == 'q' || keyboard == 27) {
			/* Fecha a janela */
			cv::destroyWindow("frame");
			/* Fecha o arquivo de vídeo */
			capture.release(); // Libera o recurso do arquivo de vídeo
		}
	}

	return 0;
}
int main_webcam()
{

		cv::Ptr<cv::BackgroundSubtractor> pBackSub;
		pBackSub = cv::createBackgroundSubtractorKNN();

		cv::VideoCapture capture(0); // Índice 0 para a webcam padrão
		if (!capture.isOpened()) {
			std::cerr << "Unable to open the webcam." << std::endl;
			return 0;
		}

		cv::Mat frame, fgMask;
		while (true) {
			capture >> frame; // Lê o próximo quadro da webcam
			if (frame.empty())
				break;

			pBackSub->apply(frame, fgMask);

			cv::rectangle(frame, cv::Point(10, 2), cv::Point(100, 20), cv::Scalar(255, 255, 255, 255), -1);
			std::stringstream ss;
			ss << capture.get(cv::CAP_PROP_POS_FRAMES);
			std::string frameNumberString = ss.str();
			cv::putText(frame, frameNumberString.c_str(), cv::Point(15, 15), cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0, 0, 0));

			cv::resize(frame, frame, cv::Size(600, 300));
			cv::resize(fgMask, fgMask, cv::Size(600, 300));
			cv::threshold(fgMask, fgMask, 254, 255, cv::THRESH_BINARY);
			cv::imshow("frame", frame);
			cv::imshow("fgmask", fgMask);

			int keyboard = cv::waitKey(1);
			if (keyboard == 'q' || keyboard == 27) {
				break;
			}
		}

		capture.release();
		cv::destroyAllWindows();

		return 0;
	

}

int main_veremlho_azul()
{
	// Carrega a imagem a partir do arquivo
	cv::Mat imagem = cv::imread("sinais/stop.jpg");
	if (imagem.empty())
		return -1;

	// Cria uma cópia da imagem carregada
	cv::Mat image2 = imagem.clone();
	cv::Mat image3= imagem.clone();;
	
	// Cria uma janela para exibir a imagem original
	//cv::namedWindow("Copia de Imagem");

	// Exibe a imagem original na janela "Copia de Imagem"
	//cv::imshow("Copia de Imagem", image2);

	
	

//supomos que este e o meu frame   esqueca hsv>segmentacao de vermelho>quantidade de nao vermelho >10 entao e azul

	IVC* image = vc_image_new(imagem.cols, imagem.rows, imagem.channels(), 255);
	memcpy(image->data, image2.data, image2.cols * image2.rows * 3);// Copia os dados da imagem para a estrutura IVC
	float percAreasegm;
	
	percAreasegm = calculateNonRedAreaPercentage(image);

	//printf("	\n %f   ESTOU AQUI \n", percAreasegm);
	memcpy(image3.data, image->data, image->bytesperline * image->height);


	std::string str = "Percentagem nao vermelha: " + std::to_string(percAreasegm);
	cv::putText(imagem, str, cv::Point(20, 25), cv::FONT_HERSHEY_SIMPLEX, 1.0, cv::Scalar(0, 0, 0), 2);

	if (percAreasegm > 10) {
		cv::putText(imagem, "Sinal azul", cv::Point(20, 50), cv::FONT_HERSHEY_SIMPLEX, 1.0, cv::Scalar(0, 0, 0), 2);


		//sei que e azul
		// acrescentra aqui logica azul



	}
	else {
		cv::putText(imagem, "Sinal vermelho", cv::Point(20, 50), cv::FONT_HERSHEY_SIMPLEX, 1.0, cv::Scalar(0, 0, 0), 2);

		//sei que e vermelhor
	// acrescentra aqui logica vermelho




	}
	
	cv::imshow("VC - VIDEO", imagem);


	// fim codigo para saber se e vermelho ou azul
	





	//cv::imshow("VC - VIDEO", image3);//mostra hsv segmentada que prova a cor azul presente.




	//IVC* imagesegmentadaIvc= vc_image_new(imagem.cols, imagem.rows, imagem.channels(), 1);																
	//memcpy(image->data, image2.data, image2.cols * image2.rows * 3);// Copia os dados da imagem para a estrutura IVC
	//vc_rgb_to_hsv(image);
	//float percAreasegm;
	//percAreasegm = cv_segmentAndCalculateredAreaPercentage(image, imagesegmentadaIvc, 0, 91, 50, 100, 50, 100); // com imagens azuis vai dar torno 30 - ou seja >10 e azul
	//printf("	\n %f   ESTOU AQUI \n", percAreasegm);
	//
	//chamar funcao que vai responder se imagem tem maior quantidade de vermelho ou maior quantidade de azul  (parametros de entrada sera *IVC SRC) SAIDA IS RED=1 IS BLUE=0
	//memcpy(image2.data, imagesegmentadaIvc->data, imagesegmentadaIvc->bytesperline * imagesegmentadaIvc->height);
	//memcpy(image2.data, image->data, image2.cols * image2.rows * 3);// Copia os dados da estrutura IVC de volta para a imagem2

	
	// Libera a memória da estrutura IVC
	//vc_image_free(image);

	// Exibe a imagem2 convertida para HSV usando o seu código
	
	//::imshow("VC - VIDEO", image3);

	// Cria uma nova matriz para armazenar a imagem convertida para HSV usando o OpenCV
	//cv::Mat image4;

	// Converte a imagem original para HSV usando a função do OpenCV
	//cv::cvtColor(imagem, image4, cv::COLOR_RGB2HSV);

	// Exibe a imagem3 convertida para HSV usando o OpenCV
	//cv::imshow("OpenCV - HSV", image4);

	// Aguarda até que uma tecla seja pressionada
	cv::waitKey(0);

	// Libera a memória das imagens
	imagem.release();
	image2.release();
	image3.release();

	// Fecha as janelas
	//cv::destroyWindow("Copia de Imagem");
	//cv::destroyWindow("VC - VIDEO");
	//cv::destroyWindow("OpenCV - HSV");

	return 0;
}
int main_com_ficheiro() {
	{// Vídeo
		cv::VideoCapture capture;
		struct
		{
			int width, height;
			int ntotalframes;
			int fps;
			int nframe;
		} video;
		// Outros
		std::string str;
		int key = 0;

		/* Abrir captura de vídeo pela Webcam #0 */
		capture.open(0);

		/* Verifica se foi possível abrir a captura de vídeo */
		if (!capture.isOpened())
		{
			std::cerr << "Erro ao abrir a captura de vídeo!\n";
			return 1;
		}

		/* Frame rate da captura de vídeo */
		video.fps = (int)capture.get(cv::CAP_PROP_FPS);
		/* Resolução da captura de vídeo */
		video.width = (int)capture.get(cv::CAP_PROP_FRAME_WIDTH);
		video.height = (int)capture.get(cv::CAP_PROP_FRAME_HEIGHT);

		/* Cria uma janela para exibir o vídeo */
		cv::namedWindow("VC - VIDEO", cv::WINDOW_AUTOSIZE);

		cv::Mat frame;
		while (key != 'q') {
			/* Leitura de um frame do vídeo */
			capture.read(frame);

			/* Verifica se conseguiu ler o frame */
			if (frame.empty()) break;

			/* Número do frame a processar */
			video.nframe = (int)capture.get(cv::CAP_PROP_POS_FRAMES);

			// Convertendo a imagem de BGR para RGB
			
			
			cv::cvtColor(frame, frame, cv::COLOR_BGR2RGB);
			
			/*  este codigo grava imagem ja convertiga rgb para ter a cerrteza que e mesmo rgb , ja confirmei esta ok 
			
			IVC* imagemteste= vc_image_new(video.width, video.height, 3, 255);
			memcpy(imagemteste->data, frame.data, video.width* video.height * 3);
		
			char filename[] = "teste2.pgm";
			vc_write_image(filename, imagemteste);*/
			IVC* imagemIvcAtratar = vc_image_new(video.width, video.height, 3, 255);
			IVC* imagemIvcAtratarGrayScale = vc_image_new(video.width, video.height, 1, 255);
			memcpy(imagemIvcAtratar->data, frame.data, video.width* video.height * 3);
			vc_rgb_to_gray(imagemIvcAtratar, imagemIvcAtratarGrayScale);
			//
			//vc_gray_negative(imagemIvcAtratarGrayScale);
			vc_gray_to_binary(imagemIvcAtratarGrayScale, 40);
			char filename[] = "teste3.pgm";
			vc_write_image(filename, imagemIvcAtratarGrayScale);
			//converter para bgr e fazer imshow no final 


			// Copia dados de imagem da estrutura IVC para uma estrutura cv::Mat
			cv::Mat frame_bgr;
			memcpy(frame_bgr.data, imagemIvcAtratarGrayScale->data, video.width * video.height * 1);

			//cv::Mat frame_gray;
			//cv::cvtColor(imagemIvcAtratarGrayScale, frame_gray, cv::COLOR_GRAY2BGR);
			//cv::imshow("VC - VIDEO", frame_gray);

			IVC* image = vc_image_new(video.width, video.height, 3, 255);

			// Copia dados de imagem da estrutura cv::Mat para uma estrutura IVC
			memcpy(image->data, frame.data, video.width* video.height * 3);
		
		
			//colocr aqui codigo reducao ruido Filling holes

			// Cria uma nova imagem IVC
	/*		
			IVC* imagegrayscale = vc_image_new(video.width, video.height, 1, 255);
			IVC* imagebin = vc_image_new(video.width, video.height, 1, 1);
		

			vc_gray_to_binary_midpoint(image, 30);
	*/
		
			//memcpy(imagebin->data, frame.data, video.width* video.height * 1);//copio para imagem binaria de 1 canal

			// Executa uma função da nossa biblioteca vc
			//memcpy(imagebin->data, frame.data, video.width* video.height * 1);

			// Copia dados de imagem da estrutura IVC para uma estrutura cv::Mat
			//memcpy(frame.data, imagebin->data, video.width * video.height * 1);
			// Liberta a memória da imagem IVC que havia sido criada
			//vc_image_free(image);

			// Convertendo a imagem de RGB de volta para BGR
			//cv::cvtColor(frame, frame, cv::COLOR_RGB2BGR);

			/* Exibe o frame */
		//	cv::imshow("VC - VIDEO", frame);

			/* Sai da aplicação se o usuário pressionar a tecla 'q' */
			key = cv::waitKey(9);
		}

		/* Fecha a janela */
		cv::destroyWindow("VC - VIDEO");

		/* Libera a captura de vídeo */
		capture.release();

		return 0;
		//
				// Vídeo
				// Vídeo

				// Abrir captura de vídeo pela Webcam #0
	}

}int main() {
		char videofile[20] = "video5.mp4";
		cv::VideoCapture capture;
		struct {
			int width, height;
			int ntotalframes;
			int fps;
			int nframe;
		} video;
		// Outros
		std::string str;
		int key = 0;

		/* Leitura de vídeo de um ficheiro */
		/* NOTA IMPORTANTE:
		O ficheiro video.avi deverá estar localizado no mesmo directório que o ficheiro de código fonte.
		*/
		capture.open(videofile);

		/* Em alternativa, abrir captura de vídeo pela Webcam #0 */
		//capture.open(0, cv::CAP_DSHOW); // Pode-se utilizar apenas capture.open(0);

		/* Verifica se foi possível abrir o ficheiro de vídeo */
		if (!capture.isOpened()) {
			std::cerr << "Erro ao abrir o ficheiro de vídeo!\n";
			return 1;
		}

		/* Número total de frames no vídeo */
		video.ntotalframes = (int)capture.get(cv::CAP_PROP_FRAME_COUNT);
		/* Frame rate do vídeo */
		video.fps = (int)capture.get(cv::CAP_PROP_FPS);
		/* Resolução do vídeo */
		video.width = (int)capture.get(cv::CAP_PROP_FRAME_WIDTH);
		video.height = (int)capture.get(cv::CAP_PROP_FRAME_HEIGHT);

		/* Cria uma janela para exibir o vídeo */
		cv::namedWindow("VC - VIDEO", cv::WINDOW_AUTOSIZE);
		cv::Mat framehsvsegeentadablue;
		cv::Mat frame;
		cv::Mat saida;
		cv::Mat saidaoriginal;
		int contador=0;
		while (key != 'q') {

			/* Leitura de uma frame do vídeo */
			capture.read(frame);

			//cv::resize(frame, frame, cv::Size(640, 480));
			/* Verifica se conseguiu ler a frame */
			if (frame.empty())
				break;

			/* Número da frame a processar */
			video.nframe = (int)capture.get(cv::CAP_PROP_POS_FRAMES);

			IVC* imagemIvcAtratar = vc_image_new(video.width, video.height, 3, 255); //imagem a ser usada pelas minhas funcoes
			IVC* imagemIvcAtrataralterada = vc_image_new(video.width, video.height, 1, 255);//imagem a ser usada pelas minhas funcoes em grayscale
			IVC* imagemIvcAtratar4 = vc_image_new(video.width, video.height, 3, 255);
			IVC* imagemIvcAtratar5 = vc_image_new(video.width, video.height, 3, 255);
			IVC* imagemIvcAtrataralterada2 = vc_image_new(video.width, video.height, 1, 255);//imagem a ser usada pelas minhas funcoes em grayscale
			IVC* imagemIvcAtrataralterada3 = vc_image_new(video.width, video.height, 3, 255);//imagem a ser usada pelas minhas funcoes em grayscale
			memcpy(imagemIvcAtratar->data, frame.data, video.width * video.height * 3);
			
			vc_rgb_to_gray(imagemIvcAtratar, imagemIvcAtrataralterada);
			//vc_gray_to_binary(imagemIvcAtrataralterada, 120);
			//vc_gray_to_binary(imagemIvcAtrataralterada,150);
			vc_gray_to_binary_global_mean(imagemIvcAtrataralterada); 
			//c_gray_to_binary_bernsen(imagemIvcAtrataralterada4, imagemIvcAtrataralterada, 9, 100);
			//vc_gray_to_binary_global_mean(imagemIvcAtrataralterada);
			vc_binary_dilate(imagemIvcAtrataralterada, imagemIvcAtrataralterada2, 3);
			vc_binary_erode(imagemIvcAtrataralterada2, imagemIvcAtrataralterada,3);








			//memcpy(imagemIvcAtratar->data, imagemIvcAtrataralterada2->data, video.width* video.height);

			//vc_remove_bg(imagemIvcAtratar, imagemIvcAtrataralterada, imagemIvcAtrataralterada3);
			//memcpy(imagemIvcAtrataralterada->data, imagemIvcAtrataralterada2->data, video.width* video.height);

			//cv::Mat binaryFrame2(video.height, video.width, CV_8UC3, imagemIvcAtrataralterada3->data);
			//
			//cv::Mat bgrFrame2; 
			//cv::cvtColor(binaryFrame2, bgrFrame2, cv::COLOR_RGB2BGR); 

			vc_remove_bg(imagemIvcAtratar, imagemIvcAtrataralterada, imagemIvcAtrataralterada3);
			
			
			
			// ate aqui tenho ja minha imagem apos remover mascara


			memcpy(imagemIvcAtratar4->data, imagemIvcAtrataralterada3->data, video.width* video.height * 3);
			memcpy(imagemIvcAtratar5->data, imagemIvcAtrataralterada3->data, video.width* video.height * 3);
			//int corsinal = getColorCode(imagemIvcAtratar4);
			float percAreasegmblue = calculateNonRedAreaPercentage(imagemIvcAtratar4);
			float percAreasegmRed = calculateNonBlueAreaPercentage(imagemIvcAtratar5);

			printf("	\n %f   blue   %f   Red \n", percAreasegmblue,percAreasegmRed);

			if (percAreasegmblue > 5 && percAreasegmRed>-4)
			
			{
				printf("azul");

			//imagem imagem original e imagemIvcAtratar
			//imagem apos thresold e tramento e imagemIvcAtrataralterada3


				IVC* imagemAposProcessamento = vc_image_new(video.width, video.height, 3, 255);
				IVC* imagemOriginal = vc_image_new(video.width, video.height, 3, 255);
				memcpy(imagemOriginal->data, imagemIvcAtratar->data, video.width * video.height * 3);
				memcpy(imagemAposProcessamento->data, imagemIvcAtrataralterada3->data, video.width * video.height * 3);
				IVC* imagemAposProcessamentoDst = vc_image_new(video.width, video.height, 3, 255);
				IVC* imagemAposProcessamentoDst2 = vc_image_new(video.width, video.height, 1, 1);
				IVC* imagemAposProcessamentoDst3 = vc_image_new(video.width, video.height, 1, 1);
				IVC* imagemAposProcessamentoDst4 = vc_image_new(video.width, video.height, 1, 1);
				//image = vc_read_image("Images/Sinais/Stop.ppm");
				char objetoSinal[]="indefenido";
				memcpy(imagemAposProcessamentoDst->data, imagemAposProcessamento->data, imagemAposProcessamento->bytesperline * imagemAposProcessamento->height);

				vc_rgb_to_hsv(imagemAposProcessamentoDst);
				vc_hsv_segmentationalterada(imagemAposProcessamentoDst, imagemAposProcessamentoDst2, 0, 91, 50, 100, 50, 100); // 47, 66, 127, 255, 127, 255); 0, 91, 50, 100, 50, 100
				vc_binary_dilate(imagemAposProcessamentoDst2, imagemAposProcessamentoDst3, 40);
				vc_binary_erode(imagemAposProcessamentoDst3, imagemAposProcessamentoDst4, 40);
			
					char filename[] = "teste44.pgm";
					vc_write_image(filename, imagemAposProcessamentoDst4);

					memcpy(imagemAposProcessamentoDst2->data, imagemAposProcessamentoDst4->data, imagemAposProcessamentoDst2->bytesperline * imagemAposProcessamentoDst2->height);

				
				int nblobs;

				vc_binary_blob_labelling(imagemAposProcessamentoDst2, imagemAposProcessamentoDst4, &nblobs);
				OVC* blobs;

				blobs = vc_binary_blob_labelling(imagemAposProcessamentoDst2, imagemAposProcessamentoDst4, &nblobs);

				if (blobs != NULL)
				{
					vc_binary_blob_info(imagemAposProcessamentoDst4, blobs, nblobs);
					for (int i = 0; i < nblobs; i++)
					{
						vc_draw_boundingbox(imagemAposProcessamentoDst4, &blobs[i]);
						vc_draw_centerofgravity(imagemAposProcessamentoDst4, &blobs[i]);

						if (blobs[i].area > 10000)
						{
							printf("\n Label %d:\n", blobs[i].label);
							printf("     Area=%-5d Perimetro %-5d x=%-5d y=%-5d w=%-5d h=%-5d xc=%-5d yc=%-5d", blobs[i].area, blobs[i].perimeter, blobs[i].x, blobs[i].y, blobs[i].width, blobs[i].heigh, blobs[i].xc, blobs[i].yc);
							printf("quantidade blobs %d", nblobs);
							getobjectmaskbyboundingbox(imagemOriginal, &blobs[i]);// agora tenho imagem perfeita pronta para voltar a saber informacoes sobre ela
							cv::Mat segmentadaporboundingbox(video.height, video.width, CV_8UC3, imagemOriginal->data);// agora tenho imagem perfeita pronta para voltar a saber informacoes sobre ela

							IVC* imagemseg = vc_image_new(video.width, video.height, 3, 255);
							IVC* imagemseg_grayscale = vc_image_new(video.width, video.height, 1, 255);//imagem a ser usada pelas minhas funcoes em grayscale
							IVC* imagemseg_grayscale2 = vc_image_new(video.width, video.height, 1, 255);
							IVC* imagemseg_grayscale3 = vc_image_new(video.width, video.height, 1, 255);
							IVC* imagemseg_grayscale4 = vc_image_new(video.width, video.height, 1, 255);
							memcpy(imagemseg->data, imagemOriginal->data,video.width * video.height * 3);

							vc_rgb_to_gray(imagemseg, imagemseg_grayscale);

							memcpy(imagemseg_grayscale4->data, imagemseg_grayscale->data, video.width * video.height );
							
							//vc_process_image(imagemseg_grayscale4, imagemseg_grayscale, 3);
							vc_gray_to_binary_bernsen(imagemseg_grayscale4, imagemseg_grayscale, 3, 20);
							//memcpy(imagemseg_grayscale->data, imagemseg_grayscale4->data, video.width * video.height * 3);
							//vc_gray_to_binary_global_mean(imagemIvcAtrataralterada); 
							
							vc_binary_dilate(imagemseg_grayscale, imagemseg_grayscale2, 3);
							//
							vc_binary_erode(imagemseg_grayscale2, imagemseg_grayscale, 3);
							vc_binary_dilate(imagemseg_grayscale, imagemseg_grayscale2, 6);
							memcpy(imagemseg_grayscale->data, imagemseg_grayscale2->data, video.width * video.height);

							cv::Mat teste(video.height, video.width, CV_8UC1, imagemseg_grayscale2->data);// agora tenho imagem perfeita pronta para voltar a saber informacoes sobre ela
							teste = teste * 255;
							cv::imshow("VC tteste", teste);





							//// Verificar se o blob é quadrado
							//if (isSquare(&blobs[i]))
							//{

							//	printf("e quadrado");
							//	// O blob é quadrado
							//	// Faça o que for necessário com o blob quadrado
							//}

							//// Verificar se o blob é redondo
							//if (isRound(&blobs[i]))
							//{
							//	printf("e redondo");
							//	// O blob é redondo
							//	// Faça o que for necessário com o blob redondo
							//}

							//vc_gray_lowpass_median_filter(imagemsegmentada_grayscale, imagemsegmentada_grayscale,);
							//vc_gray_to_binary(imagemseg_grayscale, 127);
							
							//vc_gray_to_binary_global_mean(imagemseg_grayscale);
							
							
							

							int nblobs2;
							IVC* imagemsega_grayscaledst = vc_image_new(video.width, video.height, 1, 1);
							vc_binary_blob_labelling(imagemseg_grayscale, imagemsega_grayscaledst, &nblobs2);
							OVC* blobs2;

							blobs2 = vc_binary_blob_labelling(imagemseg_grayscale, imagemsega_grayscaledst, &nblobs2);

							if (blobs2 != NULL)
							{
								vc_binary_blob_info(imagemsega_grayscaledst, blobs2, nblobs2);
								for (int i = 0; i < nblobs2; i++)
								{
									vc_draw_boundingbox(imagemsega_grayscaledst, &blobs2[i]);
									vc_draw_centerofgravity(imagemsega_grayscaledst, &blobs2[i]);

									if (nblobs2 > 4)
									{
										snprintf(objetoSinal, sizeof(objetoSinal), "SINAL AUTO ESTRADA AZUL %d\n", nblobs2);
									}

									if (nblobs2 ==2)
									{
										int seta;
										seta = determinaSetaEsquerdaOuDireita(imagemsega_grayscaledst, &blobs2[i]);
										if (seta == 1) 
										{
											snprintf(objetoSinal, sizeof(objetoSinal), "SETA ESQUERDA %d\n", nblobs2);
										}
										if (seta == 2)
										{
											snprintf(objetoSinal, sizeof(objetoSinal), "SETA Direita %d\n", nblobs2);
										}
										seta = 0;
										
									}
									if (nblobs2 <2)
									{
										snprintf(objetoSinal, sizeof(objetoSinal), "indefinido %d\n", nblobs2);
									}
								}
							}




							cv::Mat segmentadaporboundingboxbin(video.height, video.width, CV_8UC1, imagemsega_grayscaledst->data);// agora tenho imagem perfeita pronta para voltar a saber informacoes sobre ela
							segmentadaporboundingboxbin = segmentadaporboundingboxbin * 255;
							//vc_rgb_to_hsv(imagemAposProcessamentoDst);
							//vc_hsv_segmentationalterada(imagemAposProcessamentoDst, imagemAposProcessamentoDst2, 47, 66, 127, 255, 127, 255);
						
							//vc_binary_dilate(imagemAposProcessamentoDst2, imagemAposProcessamentoDst3, 40);
							//vc_binary_erode(imagemAposProcessamentoDst3, imagemAposProcessamentoDst4, 40);

							//cv::Mat segmentadaporboundihsv(video.height, video.width, CV_8UC1, imagemAposProcessamentoDst4->data);
							//segmentadaporboundihsv = segmentadaporboundihsv * 255;



							//cv::imshow("VC segmentacao por boundingbox hsv", segmentadaporboundihsv);
							cv::imshow("VC segmentacao por boundingbox", segmentadaporboundingbox);
							cv::putText(segmentadaporboundingboxbin, objetoSinal, cv::Point(20, 50), cv::FONT_HERSHEY_SIMPLEX, 1.0, cv::Scalar(255, 255, 255), 2);
							cv::imshow("VC segmentacao por boundingbox midpoint", segmentadaporboundingboxbin);

						}
						else {

							printf("blob pequena");
						}
					}
					free(blobs);
				}


				cv::Mat framehsvsegeentadablue2(video.height, video.width, CV_8UC1, imagemAposProcessamentoDst4->data);
				framehsvsegeentadablue2 = framehsvsegeentadablue2 * 255;
				cv::imshow("VC hsv segmentada blue", framehsvsegeentadablue2);


				////memcpy(framehsvsegeentadablue.data, imagemAposProcessamentoDst4->data, video.width * video.height );
				char filename2[] = "teste333.pgm";
				vc_write_image(filename2, imagemAposProcessamentoDst4);

				//produzir nova imagem 
				
				//cv::Mat framehsvsegeentadablue2(video.height, video.width, CV_8UC1, imagemAposProcessamentoDst4->data);
				////memcpy(framehsvsegeentadablue.data, imagemAposProcessamentoDst4->data, video.width * video.height );
				//cv::imshow("VC hsv segmentada blue", framehsvsegeentadablue2);








			}
			else
			{

				if (percAreasegmblue <1 && percAreasegmRed < -7)
				{



					//vermelho

					IVC* imagemAposProcessamento = vc_image_new(video.width, video.height, 3, 255);
					IVC* imagemOriginal = vc_image_new(video.width, video.height, 3, 255);
					memcpy(imagemOriginal->data, imagemIvcAtratar->data, video.width* video.height * 3);
					memcpy(imagemAposProcessamento->data, imagemIvcAtrataralterada3->data, video.width* video.height * 3);
					IVC* imagemAposProcessamentoDst = vc_image_new(video.width, video.height, 3, 255);
					IVC* imagemAposProcessamentoDst2 = vc_image_new(video.width, video.height, 1, 1);
					IVC* imagemAposProcessamentoDst3 = vc_image_new(video.width, video.height, 1, 1);
					IVC* imagemAposProcessamentoDst4 = vc_image_new(video.width, video.height, 1, 1);
					//image = vc_read_image("Images/Sinais/Stop.ppm");

					memcpy(imagemAposProcessamentoDst->data, imagemAposProcessamento->data, imagemAposProcessamento->bytesperline* imagemAposProcessamento->height);

					vc_rgb_to_hsv(imagemAposProcessamentoDst);
					vc_hsv_segmentationalterada(imagemAposProcessamentoDst, imagemAposProcessamentoDst2, 200, 260, 50, 255, 50, 255); // 47, 66, 127, 255, 127, 255); 0, 91, 50, 100, 50, 100 //

					vc_binary_dilate(imagemAposProcessamentoDst2, imagemAposProcessamentoDst3, 40);
					vc_binary_erode(imagemAposProcessamentoDst3, imagemAposProcessamentoDst4, 40);

					char filename[] = "teste44.pgm";
					vc_write_image(filename, imagemAposProcessamentoDst4);

					memcpy(imagemAposProcessamentoDst2->data, imagemAposProcessamentoDst4->data, imagemAposProcessamentoDst2->bytesperline* imagemAposProcessamentoDst2->height);


					int nblobs;

					vc_binary_blob_labelling(imagemAposProcessamentoDst2, imagemAposProcessamentoDst4, &nblobs);
					OVC* blobs;

					blobs = vc_binary_blob_labelling(imagemAposProcessamentoDst2, imagemAposProcessamentoDst4, &nblobs);

					if (blobs != NULL)
					{
						vc_binary_blob_info(imagemAposProcessamentoDst4, blobs, nblobs);
						for (int i = 0; i < nblobs; i++)
						{
							vc_draw_boundingbox(imagemAposProcessamentoDst4, &blobs[i]);
							vc_draw_centerofgravity(imagemAposProcessamentoDst4, &blobs[i]);
												
							if (blobs[i].area > 10000)
							{
								printf("\n Label %d:\n", blobs[i].label);
								printf("     Area=%-5d Perimetro %-5d x=%-5d y=%-5d w=%-5d h=%-5d xc=%-5d yc=%-5d", blobs[i].area, blobs[i].perimeter, blobs[i].x, blobs[i].y, blobs[i].width, blobs[i].heigh, blobs[i].xc, blobs[i].yc);
								printf("quantidade blobs %d", nblobs);
								getobjectmaskbyboundingbox(imagemOriginal, &blobs[i]);// agora tenho imagem perfeita pronta para voltar a saber informacoes sobre ela
								cv::Mat segmentadaporboundingbox(video.height, video.width, CV_8UC3, imagemOriginal->data);// agora tenho imagem perfeita pronta para voltar a saber informacoes sobre ela

								IVC* imagemseg = vc_image_new(video.width, video.height, 3, 255);
								IVC* imagemseg_grayscale = vc_image_new(video.width, video.height, 1, 255);//imagem a ser usada pelas minhas funcoes em grayscale
								IVC* imagemseg_grayscale2 = vc_image_new(video.width, video.height, 1, 255);
								IVC* imagemseg_grayscale3 = vc_image_new(video.width, video.height, 1, 255);
								IVC* imagemseg_grayscale4 = vc_image_new(video.width, video.height, 1, 255);
								memcpy(imagemseg->data, imagemOriginal->data, video.width * video.height * 3);

								vc_rgb_to_gray(imagemseg, imagemseg_grayscale);

								memcpy(imagemseg_grayscale4->data, imagemseg_grayscale->data, video.width * video.height);
								vc_gray_to_binary_bernsen(imagemseg_grayscale4, imagemseg_grayscale, 3, 20);
								vc_binary_dilate(imagemseg_grayscale, imagemseg_grayscale2, 3);
								//
								vc_binary_erode(imagemseg_grayscale2, imagemseg_grayscale, 3);
								vc_binary_dilate(imagemseg_grayscale, imagemseg_grayscale2, 6);
								memcpy(imagemseg_grayscale->data, imagemseg_grayscale2->data, video.width * video.height);



							

								//vc_gray_lowpass_median_filter(imagemsegmentada_grayscale, imagemsegmentada_grayscale,);
								//vc_gray_to_binary(imagemseg_grayscale, 127);

								//vc_gray_to_binary_global_mean(imagemseg_grayscale);

									// Verificar se o blob é quadrado
								if (isSquare(&blobs[i]))
								{

									printf("e quadrado");
									// O blob é quadrado
									// Faça o que for necessário com o blob quadrado
								}

								// Verificar se o blob é redondo
								if (isRound(&blobs[i]))
								{
									printf("e redondo");
									// O blob é redondo
									// Faça o que for necessário com o blob redondo
								}




								int nblobs2;
								IVC* imagemsega_grayscaledst = vc_image_new(video.width, video.height, 1, 1);
								vc_binary_blob_labelling(imagemseg_grayscale, imagemsega_grayscaledst, &nblobs2);
								OVC* blobs2;

								blobs2 = vc_binary_blob_labelling(imagemseg_grayscale, imagemsega_grayscaledst, &nblobs2);

								if (blobs2 != NULL)
								{
									vc_binary_blob_info(imagemsega_grayscaledst, blobs2, nblobs2);
									for (int i = 0; i < nblobs2; i++)
									{
										vc_draw_boundingbox(imagemsega_grayscaledst, &blobs2[i]);
										vc_draw_centerofgravity(imagemsega_grayscaledst, &blobs2[i]);

										if (nblobs2 > 3)printf("tem mais de 3 objetos e e vermelho %d \n", nblobs2);
										if (nblobs2 > 4)printf("tem mais de 4 objetos e e vermelho %d \n ", nblobs2);
										if (nblobs2 > 5)printf("tem mais de 5 objetos e e vermelho %d \n ", nblobs2);
									}
								}




								cv::Mat segmentadaporboundingboxbin(video.height, video.width, CV_8UC1, imagemsega_grayscaledst->data);// agora tenho imagem perfeita pronta para voltar a saber informacoes sobre ela
								segmentadaporboundingboxbin = segmentadaporboundingboxbin * 255;
								//vc_rgb_to_hsv(imagemAposProcessamentoDst);
								//vc_hsv_segmentationalterada(imagemAposProcessamentoDst, imagemAposProcessamentoDst2, 47, 66, 127, 255, 127, 255);

								//vc_binary_dilate(imagemAposProcessamentoDst2, imagemAposProcessamentoDst3, 40);
								//vc_binary_erode(imagemAposProcessamentoDst3, imagemAposProcessamentoDst4, 40);

								//cv::Mat segmentadaporboundihsv(video.height, video.width, CV_8UC1, imagemAposProcessamentoDst4->data);
								//segmentadaporboundihsv = segmentadaporboundihsv * 255;



								//cv::imshow("VC segmentacao por boundingbox hsv", segmentadaporboundihsv);
								cv::imshow("VC segmentacao por boundingbox", segmentadaporboundingbox);
								cv::imshow("VC segmentacao por boundingbox midpoint", segmentadaporboundingboxbin);

							}
							else {

								//printf("blob pequena");
							}
						}
						free(blobs);
					}


					cv::Mat framehsvsegeentadablue2(video.height, video.width, CV_8UC1, imagemAposProcessamentoDst4->data);
					framehsvsegeentadablue2 = framehsvsegeentadablue2 * 255;
					cv::imshow("VC hsv segmentada blue", framehsvsegeentadablue2);


					////memcpy(framehsvsegeentadablue.data, imagemAposProcessamentoDst4->data, video.width * video.height );
					char filename2[] = "teste333.pgm";
					vc_write_image(filename2, imagemAposProcessamentoDst4);

					//produzir nova imagem 

					//cv::Mat framehsvsegeentadablue2(video.height, video.width, CV_8UC1, imagemAposProcessamentoDst4->data);
					////memcpy(framehsvsegeentadablue.data, imagemAposProcessamentoDst4->data, video.width * video.height );
					//cv::imshow("VC hsv segmentada blue", framehsvsegeentadablue2);

















































					printf("vemelho");
				}
					
				
				
			}
			
			//memcpy(image3.data, imagemIvcAtrataralterada3->data, video.width* video.height * 3);








			//fim 
			
			
			
			
			
			
			
			
			
			
			char filename[] = "teste33.pgm";
			vc_write_image(filename, imagemIvcAtratar);
			cv::Mat rgbFrame2(video.height, video.width, CV_8UC3, imagemIvcAtrataralterada3->data);


			//vc_binary_erode(imagemIvcAtrataralterada2, imagemIvcAtrataralterada, 9);
			cv::Mat binaryFrame(video.height, video.width, CV_8UC1, imagemIvcAtrataralterada->data);

			cv::Mat bgrFrame;
			cv::cvtColor(binaryFrame, bgrFrame, cv::COLOR_GRAY2BGR);

			// Redimensiona a imagem para 800x600
			cv::resize(rgbFrame2, saida, cv::Size(300, 200));
			cv::resize(frame, saidaoriginal, cv::Size(300, 200));

			/* Exibe a frame */
			cv::imshow("VC - VIDEO", saida);
			cv::imshow("VC - VIDEO2", saidaoriginal);
		
			/* Libera a memória alocada */
			vc_image_free(imagemIvcAtratar);
			vc_image_free(imagemIvcAtrataralterada);
			vc_image_free(imagemIvcAtrataralterada2);

			/* Sai da aplicação, se o utilizador premir a tecla 'q' */
			key = cv::waitKey(1);
		}

		/* Fecha a janela */
		cv::destroyWindow("VC - VIDEO");

		/* Fecha o ficheiro de vídeo */
		capture.release();

		return 0;
	}






	int main_2() {
		
		
		// Vídeo
		cv::VideoCapture capture;
		struct
		{
			int width, height;
			int ntotalframes;
			int fps;
			int nframe;
		} video;
		// Outros
		std::string str;
		int key = 0;

		/* Abrir captura de vídeo pela Webcam #0 */
		capture.open(0);

		/* Verifica se foi possível abrir a captura de vídeo */
		if (!capture.isOpened())
		{
			std::cerr << "Erro ao abrir a captura de vídeo!\n";
			return 1;
		}


		/* Em alternativa, abrir captura de vídeo pela Webcam #0 */
		//capture.open(0, cv::CAP_DSHOW); // Pode-se utilizar apenas capture.open(0);

		/* Verifica se foi possível abrir o ficheiro de vídeo */
		if (!capture.isOpened()) {
			std::cerr << "Erro ao abrir o ficheiro de vídeo!\n";
			return 1;
		}

		/* Número total de frames no vídeo */
		video.ntotalframes = (int)capture.get(cv::CAP_PROP_FRAME_COUNT);
		/* Frame rate do vídeo */
		video.fps = (int)capture.get(cv::CAP_PROP_FPS);
		/* Resolução do vídeo */
		video.width = (int)capture.get(cv::CAP_PROP_FRAME_WIDTH);
		video.height = (int)capture.get(cv::CAP_PROP_FRAME_HEIGHT);

		/* Cria uma janela para exibir o vídeo */
		cv::namedWindow("VC - VIDEO", cv::WINDOW_AUTOSIZE);
		cv::Mat framehsvsegeentadablue;
		cv::Mat frame;
		cv::Mat saida;
		cv::Mat saidaoriginal;
		int contador = 0;
		while (key != 'q') {

			/* Leitura de uma frame do vídeo */
			capture.read(frame);
			cv::cvtColor(frame, frame, cv::COLOR_BGR2RGB);

			//cv::resize(frame, frame, cv::Size(640, 480));
			/* Verifica se conseguiu ler a frame */
			if (frame.empty())
				break;

			/* Número da frame a processar */
			video.nframe = (int)capture.get(cv::CAP_PROP_POS_FRAMES);

			IVC* imagemIvcAtratar = vc_image_new(video.width, video.height, 3, 255); //imagem a ser usada pelas minhas funcoes
			IVC* imagemIvcAtrataralterada = vc_image_new(video.width, video.height, 1, 255);//imagem a ser usada pelas minhas funcoes em grayscale
			IVC* imagemIvcAtratar4 = vc_image_new(video.width, video.height, 3, 255);
			IVC* imagemIvcAtratar5 = vc_image_new(video.width, video.height, 3, 255);
			IVC* imagemIvcAtrataralterada2 = vc_image_new(video.width, video.height, 1, 255);//imagem a ser usada pelas minhas funcoes em grayscale
			IVC* imagemIvcAtrataralterada3 = vc_image_new(video.width, video.height, 3, 255);//imagem a ser usada pelas minhas funcoes em grayscale
			memcpy(imagemIvcAtratar->data, frame.data, video.width * video.height * 3);

			vc_rgb_to_gray(imagemIvcAtratar, imagemIvcAtrataralterada);
			//vc_gray_to_binary(imagemIvcAtrataralterada, 120);
			//vc_gray_to_binary(imagemIvcAtrataralterada,150);
			vc_gray_to_binary_global_mean(imagemIvcAtrataralterada);
			//c_gray_to_binary_bernsen(imagemIvcAtrataralterada4, imagemIvcAtrataralterada, 9, 100);
			//vc_gray_to_binary_global_mean(imagemIvcAtrataralterada);
			vc_binary_dilate(imagemIvcAtrataralterada, imagemIvcAtrataralterada2, 3);
			vc_binary_erode(imagemIvcAtrataralterada2, imagemIvcAtrataralterada, 3);








			//memcpy(imagemIvcAtratar->data, imagemIvcAtrataralterada2->data, video.width* video.height);

			//vc_remove_bg(imagemIvcAtratar, imagemIvcAtrataralterada, imagemIvcAtrataralterada3);
			//memcpy(imagemIvcAtrataralterada->data, imagemIvcAtrataralterada2->data, video.width* video.height);

			//cv::Mat binaryFrame2(video.height, video.width, CV_8UC3, imagemIvcAtrataralterada3->data);
			//
			//cv::Mat bgrFrame2; 
			//cv::cvtColor(binaryFrame2, bgrFrame2, cv::COLOR_RGB2BGR); 

			vc_remove_bg(imagemIvcAtratar, imagemIvcAtrataralterada, imagemIvcAtrataralterada3);



			// ate aqui tenho ja minha imagem apos remover mascara


			memcpy(imagemIvcAtratar4->data, imagemIvcAtrataralterada3->data, video.width * video.height * 3);
			memcpy(imagemIvcAtratar5->data, imagemIvcAtrataralterada3->data, video.width * video.height * 3);
			//int corsinal = getColorCode(imagemIvcAtratar4);
			float percAreasegmblue = calculateNonRedAreaPercentage(imagemIvcAtratar4);
			float percAreasegmRed = calculateNonBlueAreaPercentage(imagemIvcAtratar5);

			printf("	\n %f   blue   %f   Red \n", percAreasegmblue, percAreasegmRed);

			if (percAreasegmblue > 5 && percAreasegmRed > -4)

			{
				printf("azul");

				//imagem imagem original e imagemIvcAtratar
				//imagem apos thresold e tramento e imagemIvcAtrataralterada3


				IVC* imagemAposProcessamento = vc_image_new(video.width, video.height, 3, 255);
				IVC* imagemOriginal = vc_image_new(video.width, video.height, 3, 255);
				memcpy(imagemOriginal->data, imagemIvcAtratar->data, video.width * video.height * 3);
				memcpy(imagemAposProcessamento->data, imagemIvcAtrataralterada3->data, video.width * video.height * 3);
				IVC* imagemAposProcessamentoDst = vc_image_new(video.width, video.height, 3, 255);
				IVC* imagemAposProcessamentoDst2 = vc_image_new(video.width, video.height, 1, 1);
				IVC* imagemAposProcessamentoDst3 = vc_image_new(video.width, video.height, 1, 1);
				IVC* imagemAposProcessamentoDst4 = vc_image_new(video.width, video.height, 1, 1);
				//image = vc_read_image("Images/Sinais/Stop.ppm");
				char objetoSinal[] = "indefenido";
				memcpy(imagemAposProcessamentoDst->data, imagemAposProcessamento->data, imagemAposProcessamento->bytesperline * imagemAposProcessamento->height);

				vc_rgb_to_hsv(imagemAposProcessamentoDst);
				vc_hsv_segmentationalterada(imagemAposProcessamentoDst, imagemAposProcessamentoDst2, 0, 91, 50, 100, 50, 100); // 47, 66, 127, 255, 127, 255); 0, 91, 50, 100, 50, 100
				vc_binary_dilate(imagemAposProcessamentoDst2, imagemAposProcessamentoDst3, 40);
				vc_binary_erode(imagemAposProcessamentoDst3, imagemAposProcessamentoDst4, 40);

				char filename[] = "teste44.pgm";
				vc_write_image(filename, imagemAposProcessamentoDst4);

				memcpy(imagemAposProcessamentoDst2->data, imagemAposProcessamentoDst4->data, imagemAposProcessamentoDst2->bytesperline * imagemAposProcessamentoDst2->height);


				int nblobs;

				vc_binary_blob_labelling(imagemAposProcessamentoDst2, imagemAposProcessamentoDst4, &nblobs);
				OVC* blobs;

				blobs = vc_binary_blob_labelling(imagemAposProcessamentoDst2, imagemAposProcessamentoDst4, &nblobs);

				if (blobs != NULL)
				{
					vc_binary_blob_info(imagemAposProcessamentoDst4, blobs, nblobs);
					for (int i = 0; i < nblobs; i++)
					{
						vc_draw_boundingbox(imagemAposProcessamentoDst4, &blobs[i]);
						vc_draw_centerofgravity(imagemAposProcessamentoDst4, &blobs[i]);

						if (blobs[i].area > 10000)
						{
							printf("\n Label %d:\n", blobs[i].label);
							printf("     Area=%-5d Perimetro %-5d x=%-5d y=%-5d w=%-5d h=%-5d xc=%-5d yc=%-5d", blobs[i].area, blobs[i].perimeter, blobs[i].x, blobs[i].y, blobs[i].width, blobs[i].heigh, blobs[i].xc, blobs[i].yc);
							printf("quantidade blobs %d", nblobs);
							getobjectmaskbyboundingbox(imagemOriginal, &blobs[i]);// agora tenho imagem perfeita pronta para voltar a saber informacoes sobre ela
							cv::Mat segmentadaporboundingbox(video.height, video.width, CV_8UC3, imagemOriginal->data);// agora tenho imagem perfeita pronta para voltar a saber informacoes sobre ela

							IVC* imagemseg = vc_image_new(video.width, video.height, 3, 255);
							IVC* imagemseg_grayscale = vc_image_new(video.width, video.height, 1, 255);//imagem a ser usada pelas minhas funcoes em grayscale
							IVC* imagemseg_grayscale2 = vc_image_new(video.width, video.height, 1, 255);
							IVC* imagemseg_grayscale3 = vc_image_new(video.width, video.height, 1, 255);
							IVC* imagemseg_grayscale4 = vc_image_new(video.width, video.height, 1, 255);
							memcpy(imagemseg->data, imagemOriginal->data, video.width * video.height * 3);

							vc_rgb_to_gray(imagemseg, imagemseg_grayscale);

							memcpy(imagemseg_grayscale4->data, imagemseg_grayscale->data, video.width * video.height);

							//vc_process_image(imagemseg_grayscale4, imagemseg_grayscale, 3);
							vc_gray_to_binary_bernsen(imagemseg_grayscale4, imagemseg_grayscale, 3, 20);
							//memcpy(imagemseg_grayscale->data, imagemseg_grayscale4->data, video.width * video.height * 3);
							//vc_gray_to_binary_global_mean(imagemIvcAtrataralterada); 

							vc_binary_dilate(imagemseg_grayscale, imagemseg_grayscale2, 3);
							//
							vc_binary_erode(imagemseg_grayscale2, imagemseg_grayscale, 3);
							vc_binary_dilate(imagemseg_grayscale, imagemseg_grayscale2, 6);
							memcpy(imagemseg_grayscale->data, imagemseg_grayscale2->data, video.width * video.height);

							cv::Mat teste(video.height, video.width, CV_8UC1, imagemseg_grayscale2->data);// agora tenho imagem perfeita pronta para voltar a saber informacoes sobre ela
							teste = teste * 255;
							cv::imshow("VC tteste", teste);





							//// Verificar se o blob é quadrado
							//if (isSquare(&blobs[i]))
							//{

							//	printf("e quadrado");
							//	// O blob é quadrado
							//	// Faça o que for necessário com o blob quadrado
							//}

							//// Verificar se o blob é redondo
							//if (isRound(&blobs[i]))
							//{
							//	printf("e redondo");
							//	// O blob é redondo
							//	// Faça o que for necessário com o blob redondo
							//}

							//vc_gray_lowpass_median_filter(imagemsegmentada_grayscale, imagemsegmentada_grayscale,);
							//vc_gray_to_binary(imagemseg_grayscale, 127);

							//vc_gray_to_binary_global_mean(imagemseg_grayscale);




							int nblobs2;
							IVC* imagemsega_grayscaledst = vc_image_new(video.width, video.height, 1, 1);
							vc_binary_blob_labelling(imagemseg_grayscale, imagemsega_grayscaledst, &nblobs2);
							OVC* blobs2;

							blobs2 = vc_binary_blob_labelling(imagemseg_grayscale, imagemsega_grayscaledst, &nblobs2);

							if (blobs2 != NULL)
							{
								vc_binary_blob_info(imagemsega_grayscaledst, blobs2, nblobs2);
								for (int i = 0; i < nblobs2; i++)
								{
									vc_draw_boundingbox(imagemsega_grayscaledst, &blobs2[i]);
									vc_draw_centerofgravity(imagemsega_grayscaledst, &blobs2[i]);

									if (nblobs2 > 4)
									{
										snprintf(objetoSinal, sizeof(objetoSinal), "SINAL AUTO ESTRADA AZUL %d\n", nblobs2);
									}

									if (nblobs2 == 2)
									{
										int seta;
										seta = determinaSetaEsquerdaOuDireita(imagemsega_grayscaledst, &blobs2[i]);
										if (seta == 1)
										{
											snprintf(objetoSinal, sizeof(objetoSinal), "SETA ESQUERDA %d\n", nblobs2);
										}
										if (seta == 2)
										{
											snprintf(objetoSinal, sizeof(objetoSinal), "SETA Direita %d\n", nblobs2);
										}
										seta = 0;

									}
									if (nblobs2 < 2)
									{
										snprintf(objetoSinal, sizeof(objetoSinal), "indefinido %d\n", nblobs2);
									}
								}
							}




							cv::Mat segmentadaporboundingboxbin(video.height, video.width, CV_8UC1, imagemsega_grayscaledst->data);// agora tenho imagem perfeita pronta para voltar a saber informacoes sobre ela
							segmentadaporboundingboxbin = segmentadaporboundingboxbin * 255;
							//vc_rgb_to_hsv(imagemAposProcessamentoDst);
							//vc_hsv_segmentationalterada(imagemAposProcessamentoDst, imagemAposProcessamentoDst2, 47, 66, 127, 255, 127, 255);

							//vc_binary_dilate(imagemAposProcessamentoDst2, imagemAposProcessamentoDst3, 40);
							//vc_binary_erode(imagemAposProcessamentoDst3, imagemAposProcessamentoDst4, 40);

							//cv::Mat segmentadaporboundihsv(video.height, video.width, CV_8UC1, imagemAposProcessamentoDst4->data);
							//segmentadaporboundihsv = segmentadaporboundihsv * 255;



							//cv::imshow("VC segmentacao por boundingbox hsv", segmentadaporboundihsv);
							cv::imshow("VC segmentacao por boundingbox", segmentadaporboundingbox);
							cv::putText(segmentadaporboundingboxbin, objetoSinal, cv::Point(20, 50), cv::FONT_HERSHEY_SIMPLEX, 1.0, cv::Scalar(255, 255, 255), 2);
							cv::imshow("VC segmentacao por boundingbox midpoint", segmentadaporboundingboxbin);

						}
						else {

							printf("blob pequena");
						}
					}
					free(blobs);
				}


				cv::Mat framehsvsegeentadablue2(video.height, video.width, CV_8UC1, imagemAposProcessamentoDst4->data);
				framehsvsegeentadablue2 = framehsvsegeentadablue2 * 255;
				cv::imshow("VC hsv segmentada blue", framehsvsegeentadablue2);


				////memcpy(framehsvsegeentadablue.data, imagemAposProcessamentoDst4->data, video.width * video.height );
				char filename2[] = "teste333.pgm";
				vc_write_image(filename2, imagemAposProcessamentoDst4);

				//produzir nova imagem 

				//cv::Mat framehsvsegeentadablue2(video.height, video.width, CV_8UC1, imagemAposProcessamentoDst4->data);
				////memcpy(framehsvsegeentadablue.data, imagemAposProcessamentoDst4->data, video.width * video.height );
				//cv::imshow("VC hsv segmentada blue", framehsvsegeentadablue2);








			}
			else
			{

				if (percAreasegmblue < 1 && percAreasegmRed < -7)
				{



					//vermelho

					IVC* imagemAposProcessamento = vc_image_new(video.width, video.height, 3, 255);
					IVC* imagemOriginal = vc_image_new(video.width, video.height, 3, 255);
					memcpy(imagemOriginal->data, imagemIvcAtratar->data, video.width * video.height * 3);
					memcpy(imagemAposProcessamento->data, imagemIvcAtrataralterada3->data, video.width * video.height * 3);
					IVC* imagemAposProcessamentoDst = vc_image_new(video.width, video.height, 3, 255);
					IVC* imagemAposProcessamentoDst2 = vc_image_new(video.width, video.height, 1, 1);
					IVC* imagemAposProcessamentoDst3 = vc_image_new(video.width, video.height, 1, 1);
					IVC* imagemAposProcessamentoDst4 = vc_image_new(video.width, video.height, 1, 1);
					//image = vc_read_image("Images/Sinais/Stop.ppm");

					memcpy(imagemAposProcessamentoDst->data, imagemAposProcessamento->data, imagemAposProcessamento->bytesperline * imagemAposProcessamento->height);

					vc_rgb_to_hsv(imagemAposProcessamentoDst);
					vc_hsv_segmentationalterada(imagemAposProcessamentoDst, imagemAposProcessamentoDst2, 200, 260, 50, 255, 50, 255); // 47, 66, 127, 255, 127, 255); 0, 91, 50, 100, 50, 100 //

					vc_binary_dilate(imagemAposProcessamentoDst2, imagemAposProcessamentoDst3, 40);
					vc_binary_erode(imagemAposProcessamentoDst3, imagemAposProcessamentoDst4, 40);

					char filename[] = "teste44.pgm";
					vc_write_image(filename, imagemAposProcessamentoDst4);

					memcpy(imagemAposProcessamentoDst2->data, imagemAposProcessamentoDst4->data, imagemAposProcessamentoDst2->bytesperline * imagemAposProcessamentoDst2->height);


					int nblobs;

					vc_binary_blob_labelling(imagemAposProcessamentoDst2, imagemAposProcessamentoDst4, &nblobs);
					OVC* blobs;

					blobs = vc_binary_blob_labelling(imagemAposProcessamentoDst2, imagemAposProcessamentoDst4, &nblobs);

					if (blobs != NULL)
					{
						vc_binary_blob_info(imagemAposProcessamentoDst4, blobs, nblobs);
						for (int i = 0; i < nblobs; i++)
						{
							vc_draw_boundingbox(imagemAposProcessamentoDst4, &blobs[i]);
							vc_draw_centerofgravity(imagemAposProcessamentoDst4, &blobs[i]);

							if (blobs[i].area > 10000)
							{
								printf("\n Label %d:\n", blobs[i].label);
								printf("     Area=%-5d Perimetro %-5d x=%-5d y=%-5d w=%-5d h=%-5d xc=%-5d yc=%-5d", blobs[i].area, blobs[i].perimeter, blobs[i].x, blobs[i].y, blobs[i].width, blobs[i].heigh, blobs[i].xc, blobs[i].yc);
								printf("quantidade blobs %d", nblobs);
								getobjectmaskbyboundingbox(imagemOriginal, &blobs[i]);// agora tenho imagem perfeita pronta para voltar a saber informacoes sobre ela
								cv::Mat segmentadaporboundingbox(video.height, video.width, CV_8UC3, imagemOriginal->data);// agora tenho imagem perfeita pronta para voltar a saber informacoes sobre ela

								IVC* imagemseg = vc_image_new(video.width, video.height, 3, 255);
								IVC* imagemseg_grayscale = vc_image_new(video.width, video.height, 1, 255);//imagem a ser usada pelas minhas funcoes em grayscale
								IVC* imagemseg_grayscale2 = vc_image_new(video.width, video.height, 1, 255);
								IVC* imagemseg_grayscale3 = vc_image_new(video.width, video.height, 1, 255);
								IVC* imagemseg_grayscale4 = vc_image_new(video.width, video.height, 1, 255);
								memcpy(imagemseg->data, imagemOriginal->data, video.width * video.height * 3);

								vc_rgb_to_gray(imagemseg, imagemseg_grayscale);

								memcpy(imagemseg_grayscale4->data, imagemseg_grayscale->data, video.width * video.height);
								vc_gray_to_binary_bernsen(imagemseg_grayscale4, imagemseg_grayscale, 3, 20);
								vc_binary_dilate(imagemseg_grayscale, imagemseg_grayscale2, 3);
								//
								vc_binary_erode(imagemseg_grayscale2, imagemseg_grayscale, 3);
								vc_binary_dilate(imagemseg_grayscale, imagemseg_grayscale2, 6);
								memcpy(imagemseg_grayscale->data, imagemseg_grayscale2->data, video.width * video.height);





								//vc_gray_lowpass_median_filter(imagemsegmentada_grayscale, imagemsegmentada_grayscale,);
								//vc_gray_to_binary(imagemseg_grayscale, 127);

								//vc_gray_to_binary_global_mean(imagemseg_grayscale);

									// Verificar se o blob é quadrado
								if (isSquare(&blobs[i]))
								{

									printf("e quadrado");
									// O blob é quadrado
									// Faça o que for necessário com o blob quadrado
								}

								// Verificar se o blob é redondo
								if (isRound(&blobs[i]))
								{
									printf("e redondo");
									// O blob é redondo
									// Faça o que for necessário com o blob redondo
								}




								int nblobs2;
								IVC* imagemsega_grayscaledst = vc_image_new(video.width, video.height, 1, 1);
								vc_binary_blob_labelling(imagemseg_grayscale, imagemsega_grayscaledst, &nblobs2);
								OVC* blobs2;

								blobs2 = vc_binary_blob_labelling(imagemseg_grayscale, imagemsega_grayscaledst, &nblobs2);

								if (blobs2 != NULL)
								{
									vc_binary_blob_info(imagemsega_grayscaledst, blobs2, nblobs2);
									for (int i = 0; i < nblobs2; i++)
									{
										vc_draw_boundingbox(imagemsega_grayscaledst, &blobs2[i]);
										vc_draw_centerofgravity(imagemsega_grayscaledst, &blobs2[i]);

										if (nblobs2 > 3)printf("tem mais de 3 objetos e e vermelho %d \n", nblobs2);
										if (nblobs2 > 4)printf("tem mais de 4 objetos e e vermelho %d \n ", nblobs2);
										if (nblobs2 > 5)printf("tem mais de 5 objetos e e vermelho %d \n ", nblobs2);
									}
								}




								cv::Mat segmentadaporboundingboxbin(video.height, video.width, CV_8UC1, imagemsega_grayscaledst->data);// agora tenho imagem perfeita pronta para voltar a saber informacoes sobre ela
								segmentadaporboundingboxbin = segmentadaporboundingboxbin * 255;
								//vc_rgb_to_hsv(imagemAposProcessamentoDst);
								//vc_hsv_segmentationalterada(imagemAposProcessamentoDst, imagemAposProcessamentoDst2, 47, 66, 127, 255, 127, 255);

								//vc_binary_dilate(imagemAposProcessamentoDst2, imagemAposProcessamentoDst3, 40);
								//vc_binary_erode(imagemAposProcessamentoDst3, imagemAposProcessamentoDst4, 40);

								//cv::Mat segmentadaporboundihsv(video.height, video.width, CV_8UC1, imagemAposProcessamentoDst4->data);
								//segmentadaporboundihsv = segmentadaporboundihsv * 255;



								//cv::imshow("VC segmentacao por boundingbox hsv", segmentadaporboundihsv);
								cv::imshow("VC segmentacao por boundingbox", segmentadaporboundingbox);
								cv::imshow("VC segmentacao por boundingbox midpoint", segmentadaporboundingboxbin);

							}
							else {

								//printf("blob pequena");
							}
						}
						free(blobs);
					}


					cv::Mat framehsvsegeentadablue2(video.height, video.width, CV_8UC1, imagemAposProcessamentoDst4->data);
					framehsvsegeentadablue2 = framehsvsegeentadablue2 * 255;
					cv::imshow("VC hsv segmentada blue", framehsvsegeentadablue2);


					////memcpy(framehsvsegeentadablue.data, imagemAposProcessamentoDst4->data, video.width * video.height );
					char filename2[] = "teste333.pgm";
					vc_write_image(filename2, imagemAposProcessamentoDst4);

					//produzir nova imagem 

					//cv::Mat framehsvsegeentadablue2(video.height, video.width, CV_8UC1, imagemAposProcessamentoDst4->data);
					////memcpy(framehsvsegeentadablue.data, imagemAposProcessamentoDst4->data, video.width * video.height );
					//cv::imshow("VC hsv segmentada blue", framehsvsegeentadablue2);

















































					printf("vemelho");
				}



			}

			//memcpy(image3.data, imagemIvcAtrataralterada3->data, video.width* video.height * 3);








			//fim 










			char filename[] = "teste33.pgm";
			vc_write_image(filename, imagemIvcAtratar);
			cv::Mat rgbFrame2(video.height, video.width, CV_8UC3, imagemIvcAtrataralterada3->data);


			//vc_binary_erode(imagemIvcAtrataralterada2, imagemIvcAtrataralterada, 9);
			cv::Mat binaryFrame(video.height, video.width, CV_8UC1, imagemIvcAtrataralterada->data);

			cv::Mat bgrFrame;
			cv::cvtColor(binaryFrame, bgrFrame, cv::COLOR_GRAY2BGR);

			// Redimensiona a imagem para 800x600
			cv::resize(rgbFrame2, saida, cv::Size(300, 200));
			cv::resize(frame, saidaoriginal, cv::Size(300, 200));

			/* Exibe a frame */
			cv::imshow("VC - VIDEO", saida);
			cv::imshow("VC - VIDEO2", saidaoriginal);

			/* Libera a memória alocada */
			vc_image_free(imagemIvcAtratar);
			vc_image_free(imagemIvcAtrataralterada);
			vc_image_free(imagemIvcAtrataralterada2);

			/* Sai da aplicação, se o utilizador premir a tecla 'q' */
			key = cv::waitKey(1);
		}

		/* Fecha a janela */
		cv::destroyWindow("VC - VIDEO");

		/* Fecha o ficheiro de vídeo */
		capture.release();

		return 0;
	}


