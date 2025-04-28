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
Visao_tp/
│
├── vc.h                      # Header com funções de processamento de imagem
├── vc.cpp                    # Implementação das funções de processamento
├── main.cpp                  # Ponto de entrada da aplicação
│
├── Visao_tp.sln             # Solução do Visual Studio
├── Visao_tp.vcxproj         # Arquivo de projeto do Visual Studio
├── Visao_tp.vcxproj.filters # Filtros do projeto
│
├── .vscode/                  # Configurações do VS Code (se utilizado)
│   └── c_cpp_properties.json # Configurações do IntelliSense
│
└── x64/                      # Arquivos de build (ignorados no git)
```

## ⚙️ Parâmetros Configuráveis

O sistema permite ajustar vários parâmetros para otimizar a detecção:
- Intervalos HSV para segmentação de cor
- Parâmetros da Transformada de Hough
- Limiares de área para filtrar moedas
- Parâmetros do detector de bordas Canny 