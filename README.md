# Tiaguinho-the-Runner
### Somos alunos do 2° periodo de ciência da computação da Cesar School e desenvolvemos um jogo de plataforma que se chama Tiaguinho the runner, baseado em nosso professor Tiago Barros, e que tem como objetivo principal fazer com que o boneco passe o máximo de tempo vivo, desviando dos obstáculos/monstros.

## Participantes:

### Caio Fonseca (cmaf@cesar.school)
### Rodrigo Lucena (rlc2@cesar.school)
### Paulo Portella (phcp@cesar.school)

### Slide de apresentação:

#### https://www.canva.com/design/DAGW9PVu1fE/NX5JsESOhSJJM9hvN10DAA/edit?utm_content=DAGW9PVu1fE&utm_campaign=designshare&utm_medium=link2&utm_source=sharebutton

## Colocar instrucao de baixar o raylib para linux e mac

Instale na sua maquina os pacotes Make e gcc com os seguintes comandos: 

 sudo apt install make
 sudo apt install gcc

Use esses comandos para rodar o jogo:
   make
  ./tiaguinho

!! Importante falar que você DEVE ter as seguintes bibliotecas !!

  #include <stdlib.h>
  #include <stdbool.h>
  #include <stdio.h>
  #include <unistd.h>
  #include <time.h>

Para instalar o Raylib no linux você deve usar os comandos:

  git clone https://github.com/raysan5/raylib
  cd raylib
  mkdir build && cd build
  cmake -DBUILD_SHARED_LIBS=ON ..
  make
  sudo make install
  sudo cp /usr/local/lib/libraylib.so.420 /usr/lib/

Para instalar o Raylib no Mac você deve usar os comandos:

  brew install raylib
  pkg-config --cflags --libs raylib


  
