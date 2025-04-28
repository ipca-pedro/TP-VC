# 🪙 Detector de Moedas em Vídeo

Este projeto implementa um sistema de detecção de moedas em tempo real utilizando OpenCV e C++, desenvolvido como parte de um trabalho prático de Visão por Computador.

## 📋 Pré-requisitos

- Microsoft Visual Studio 2022
- OpenCV (última versão estável)

## 🛠️ Instalação e Configuração

1. Para uma configuração detalhada do OpenCV com Visual Studio, siga este tutorial em vídeo:
   [Tutorial de Instalação do OpenCV](https://www.youtube.com/watch?v=YUjamcyuKT4)

2. Configuração do Projeto no Visual Studio:
   - Adicione os diretórios de include do OpenCV em: `Project > Properties > C/C++ > General > Additional Include Directories`
   - Adicione os diretórios das bibliotecas em: `Project > Properties > Linker > General > Additional Library Directories`
   - Adicione as dependências do OpenCV em: `Project > Properties > Linker > Input > Additional Dependencies`

## 🚀 Como Executar

1. Abra o projeto no Visual Studio
2. Selecione a configuração "Release" e plataforma "x64"
3. Compile o projeto (F7)
4. Execute (F5)

## 🎯 Funcionalidades

- Detecção de moedas em tempo real através da webcam
- Processamento de imagem utilizando técnicas de:
  - Segmentação por cor (HSV)
  - Detecção de contornos (Canny Edge Detection)
  - Filtragem por área
  - Transformada de Hough Circular
- Identificação e contagem de moedas

## 🔧 Estrutura do Projeto

```
.
├── src/
│   ├── main.cpp
│   ├── coin_detector.cpp
│   └── coin_detector.h
└── README.md
```

## ⚙️ Parâmetros Configuráveis

O sistema permite ajustar vários parâmetros para otimizar a detecção:
- Intervalos HSV para segmentação de cor
- Parâmetros da Transformada de Hough
- Limiares de área para filtrar moedas
- Parâmetros do detector de bordas Canny

## 📝 Licença

Este projeto está sob a licença MIT.

## ✉️ Contacto

Para questões ou sugestões, por favor abra uma issue no repositório. 