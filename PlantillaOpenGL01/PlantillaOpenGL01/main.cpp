#include <iostream>
//#include <GL\glew.h>
//#include <GL\freeglut.h>
#include <math.h>
#include <stdlib.h>
#include <ctime>

#include <GL/freeglut.h>
#include <GL/gl.h>

/*
Agregue unas opciones si la pelota pega de las paredes dentro de dibujarPelota, 
la pelota se mueve y reconoce las colisiones con las paredes y la plataforma
los angulos debemos pensarlos mejor, no se ven naturales. Debemos usar un poco de fisica
pero para ir probando sirven
bonus[6][3] nos va a servir para ir moviendo el bono cuando cae
*/

bool inicial = true, //true para inicializar los bonus y espaciales una sola vez
     isLeftKeyPressed = false, isRightKeyPressed = false, isUpKeyPressed = false,
     velocidad = false, //bonus de velocidad activado o desactivado
     baseLarga = false,  //bonus de base activado o desactivado
     moviendose = false,
     gameOver = false;

float plataforma = 0.0,      //posicion de la plataforma
      tam = 2.0,             //tamano de la plataforma
      //matriz de bloques
      // variables para cuando el bloque se rompe
      posInicial[8][2] = {{1,0},{0,0},{0,1},{0,0},{1,0},{1,0},{1,0},{0,0}}, 
      anguloPedazos[8] = {-330,-280,-90,-180,220,-60,320,-100},
      explota[2],
      // variables relacionadas a la pelota
      pelota[2] = {0.0,0.0}, //posicion de la pelota
      velocidadP = 0.1, //velocidad de la pelota
      anguloP,          //angulo con el que se mueve la pelota
      radioP = 0.3;     //radio de la pelota

int bloques[5][7] = {{0,0,0,0,0,0,0},{0,0,0,0,0,0,0},{0,0,0,0,0,0,0},
                       {0,0,0,0,0,0,0},{0,0,0,0,0,0,0}},
    especiales[5] = {}, //arreglo para los bloques especiales 
    bonus[6][3]= {};    /*{posicion x, posicion y, tipo de bono} 
                          (0 tama�o de la plataforma, 1 velocidad de la pelota} */

using namespace std;

#define DEF_floorGridScale  1.0f
#define DEF_floorGridXSteps 10.0f
#define DEF_floorGridZSteps 10.0f

void ejesCoordenada(float w) {
  
  glLineWidth(w);
  glBegin(GL_LINES);
    glColor3f(1.0,0.0,0.0);
    glVertex2f(0,10);
    glVertex2f(0,-10);
    glColor3f(0.0,0.0,1.0);
    glVertex2f(10,0);
    glVertex2f(-10,0);
  glEnd();

  glLineWidth(w-1.0);
  int i;
  glColor3f(0.0,1.0,0.0);
  glBegin(GL_LINES);
    for(i = -10; i <=10; i++){
      if (i!=0) {   
        if ((i%2)==0){  
          glVertex2f(i,0.4);
          glVertex2f(i,-0.4);

          glVertex2f(0.4,i);
          glVertex2f(-0.4,i);
        }else{
          glVertex2f(i,0.2);
          glVertex2f(i,-0.2);

          glVertex2f(0.2,i);
          glVertex2f(-0.2,i);
        }
      }
    }
    
  glEnd();
  glLineWidth(1.0);
}

void changeViewport(int w, int h) {
  float aspectradio;
  glViewport(0,0,w,h);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  aspectradio = (float) w / (float) h;
  if (w <= h)
    glOrtho(-10,10,-10/aspectradio,10/aspectradio,1.0,-1.0);
  else
    glOrtho(-10*aspectradio,10*aspectradio,-10,10,1.0,-1.0);
}

/********************** ESPECIALES Y BONUS **********************/
// Busca si un bloque es especial o es un bonus 
bool buscarEspeciales(int x){
  bool esta = false;

  for (int i = 0; i < 5; i++){
    if (x == especiales[i]) {
      esta = true;
      break;
    }
  }
  return esta;
}

void generarEspeciales(){
    int r;
    for (int i = 0; i < 5; i++){
        r = rand()%35;
        while ( buscarEspeciales(r) ) r = rand()%35;
        especiales[i] = r;
    }
}

bool buscarBonus(int x, int y){
  bool esta = false;

  for (int i = 0; i < 6; i++){
    if (x == bonus[i][0] && y == bonus[i][1]) {
      esta = true;
      break;
    }
  }
  return esta;
}

void generarBonus(){
    int i,j;

    for (int k = 0; k < 6; k++){  
      do{
          i = rand()%5;
          j = rand()%7;
      }while ( buscarBonus(i,j) );
        bonus[k][0] = i;
        bonus[k][1] = j;
        bonus[k][2] = rand()%2;
    }   
}


/*********************** FUNCIONES PARA DIBUJAR ***********************/
// -------- DIBUJOS PARA LA FORMA DE LOS BONUS  --------

void dibujarBonusVelocidad(float x, float y){ //
  glColor3f(1.0,0.5,0.0);
  glBegin(GL_LINE_LOOP);
    glVertex2f(x,y);
    glVertex2f(x+0.7,y);
    glVertex2f(x+0.6,y-0.5);
    glVertex2f(x+0.8,y-0.5);
    glVertex2f(x+0.5,y-1.0);
    glVertex2f(x+0.6,y-0.6);
    glVertex2f(x+0.3,y-0.6);
    glVertex2f(x+0.5,y-0.2);
  glEnd();
}

void dibujarBonusTamBase(float x, float y){ // largo 0.8 en X, alto 0.2 en Y
   
    glColor3f(1.0,0.0,0.5);
    glBegin(GL_LINE_LOOP);
      glVertex2f(x,y-0.2);
      glVertex2f(x,y-0.4);
      glVertex2f(x+0.8,y-0.4);
      glVertex2f(x+0.8,y-0.2);
    glEnd();
}

// INCOMPLETA
void lanzarBonus(){
    float x,y;

    for(int i; i<6; i++){
      if(bloques[bonus[i][0]][bonus[i][1]] == -1 ){ 
        x = bonus[i][0];
        y = bonus[i][1];

        if ( bonus[i][2]==0 && bonus[i][1] >= -0.7) dibujarBonusTamBase(x,y);
        else if( bonus[i][2]==1 && bonus[i][1] >= 0 ) dibujarBonusVelocidad(x,y);

        bonus[i][1] += -0.1;
      }
    }
}

// -------------------- DIBUJOS BASE  ---------------------- 
void dibujarPlataforma() {

    glPushMatrix();
        glTranslatef(0.0,-8.2,0.0); 
        glColor3f(0.0,0.0,1.0);

        if(baseLarga){
          glBegin(GL_LINES);
              glVertex2f(-tam+plataforma, -0.3);
              glVertex2f(-tam+plataforma, -0.8);
              glVertex2f(tam+plataforma, -0.3);
              glVertex2f(tam+plataforma, -0.8);
          glEnd();
          tam += 0.3;
        } // base = 4 -> 15% = 0.6 -> base = 4.6

        glBegin(GL_LINE_LOOP);
            glVertex2f(-tam+plataforma,-0.3);
            glVertex2f(tam+plataforma,-0.3);
            glVertex2f(tam+plataforma,-0.8);
            glVertex2f(-tam+plataforma,-0.8);
        glEnd();
    glPopMatrix();
}

void calcularAngulo(float angulo = 0){
  anguloP = 2 * (angulo - M_PI/2) - anguloP;
}

void dibujarPelota(float r) {

    glPushMatrix();
        glTranslatef(0.0,-8.2,0.0); 
        glColor3f(1.0,1.0,1.0);
        glPointSize(3.0);
        glBegin(GL_LINE_LOOP);
            for (float angulo = 0.0; angulo<6.0; angulo+=0.0001){
                glVertex2f(r*cos(angulo) + pelota[0],r*sin(angulo) + pelota[1]);
            }
        glEnd();

        //los angulos son una prueba
        if( pelota[1] < 0.0 && moviendose
                 && -tam+plataforma <= pelota[0]
                 && pelota[0] <= tam+plataforma){   // pega de la plataforma
            calcularAngulo(anguloP + 90);
        }
        else if(pelota[1]-r < 0) gameOver = true;        //el jugador pierde
        else if(pelota[0]+r >= 8.9) calcularAngulo();  //choca con la pared derecha
        else if(pelota[0]-r <= -8.9) calcularAngulo(); //choca con la pared izquierda
        else if(pelota[1]+r >= 17.1){               //pelota pega del techo
            if(anguloP >= 90) calcularAngulo(anguloP - 90);
            else calcularAngulo(anguloP + 90);
        }
        else if((pow ((-tam+plataforma-pelota[0]),2) + pow(0,0) == pow (radioP,2))          //  choca con la esquina sup izq bloque
                || (pow((tam+plataforma+1.5-pelota[0]),2) + pow (0,0)  == pow(radioP,2))){
            if(anguloP >= 180) calcularAngulo(anguloP -180);
            else calcularAngulo(anguloP + 180);
        }


    glPopMatrix();

}

void dibujarMarcoVerde() {
    
    glPushMatrix();
        glLineWidth(2.0);
        glColor3f(0.0,1.0,0.0);
        
        glBegin(GL_LINE_LOOP);
            glVertex2f(-9.0,9.5);
            glVertex2f(9.0,9.5);
            glVertex2f(9.0,9.0);
            glVertex2f(-9.0,9.0);
        glEnd();
        
        glBegin(GL_LINE_LOOP);
            glVertex2f(9.0,9.5);
            glVertex2f(9.5,9.5);
            glVertex2f(9.5,-9.0);
            glVertex2f(9.0,-9.0);
        glEnd();
        
        glBegin(GL_LINE_LOOP);
            glVertex2f(-9.0,9.5);
            glVertex2f(-9.5,9.5);
            glVertex2f(-9.5,-9.0);
            glVertex2f(-9.0,-9.0);
        glEnd();

    glPopMatrix();

}

// --------DIBUJO CUANDO EL BLOQUE SE ROMPE--------
void dibujarCirculo(float px, float py) {
    float x,y,radio = 0.14;
    glPointSize(2.0);
    glBegin(GL_POINTS);
        for(double i=0.0; i<10; i+=0.001){
            x=radio*cos(i)+px;
            y=radio*sin(i)+py;
            glVertex2f(x,y);
        }
    glEnd();


}

void dibujarExplosion(int h){

    glPushMatrix();
        glTranslatef(explota[0]+0.75,explota[1]-0.25,0.0); 
        for (int i = 0; i < 8; i++){
            dibujarCirculo(posInicial[i][0], posInicial[i][1]);
            posInicial[i][0] = 0.1*cos(anguloPedazos[i])+ posInicial[i][0];
            posInicial[i][1] = 0.1*sin(anguloPedazos[i])+ posInicial[i][1];
        }

    glutPostRedisplay();
    glPopMatrix();
}
// --------FIN DIBUJO CUANDO EL BLOQUE SE ROMPE--------

void dibujarBloque(float cx, float cy, float color){

    glColor3f(1.0,color,0.0);
    glBegin(GL_LINE_LOOP);
        glVertex2f(cx,cy);
        glVertex2f(cx+1.5,cy);
        glVertex2f(cx+1.5,cy-0.5);
        glVertex2f(cx,cy-0.5);
    glEnd();

}

void dibujarBloqueRoto(float cx, float cy){ //hay que revisarlo

  glColor3f(0.8,1.0,0.8);
  glBegin(GL_LINE_LOOP);
    glVertex2f(cx,cy);
    glVertex2f(cx+1.5/2,cy);
    glVertex2f(cx+1.5/2-0.25,cy-0.15);
    glVertex2f(cx+1.5/2+0.15,cy-0.4);
    glVertex2f(cx+1.5/2,cy-0.5);
    glVertex2f(cx,cy-0.5);
  glEnd();
  glBegin(GL_LINE_LOOP);
    glVertex2f(cx+1.5/2+0.1,cy);
    glVertex2f(cx+1.5,cy);
    glVertex2f(cx+1.5,cy-0.5);
    glVertex2f(cx+1.5/2+0.15,cy-0.5);
    glVertex2f(cx+1.5/2+0.25,cy-0.4);
    glVertex2f(cx+1.5/2-0.1,cy-0.15);
  glEnd();

}

bool hayChoque(float x, float y){
    bool choca = false;
    float px = pelota[0], py = pelota[1];

    if(px-radioP <= x+1.5 && px-radioP > x && py <= y && py >= y-0.5){// choca del lado der del bloque
        choca = true;
        calcularAngulo(anguloP + 90);
    }
    else if(px+radioP >= x && px+radioP < x +1.5 && py <= y && py >= y-0.5){ // choca del lado izq del bloque
        choca = true;
        calcularAngulo(anguloP - 90);
    }
    else if((px >= x && px < x+1.5 && y >= py-radioP && py-radioP >= y-0.5) //  choca de la parte de arriba del bloque
            || (x <= px && px <= x+1.5 &&  py+radioP <= y && y-0.5 <= py+radioP)// choca de la parte de abajo del bloque
            || (pow ((x-px),2) + pow(y-0.5-py,2) == pow (radioP,2))          //  choca con la esquina inf izq bloque
            || (pow((x+1.5-px),2) + pow (y-0.5-py,2) == pow(radioP,2))  // choca con la esquina inf der bloque
            || (pow ((x-px),2) + pow(y-py,2) == pow (radioP,2))          //  choca con la esquina sup izq bloque
            || (pow((x+1.5-px),2) + pow (y-py,2) == pow(radioP,2))){   // choca con la esquina sup der bloque
        choca = true;
        if(anguloP >= 180 ) calcularAngulo(anguloP - 180);
        else calcularAngulo(anguloP + 180);
    }
    return choca;
}

void dibujarBloques() {

    int esBonus;

    glPushMatrix();
        
        glTranslatef(0.0,-8.2,0.0); //comparte eje con la pelota para revisar las colisiones mas facil
        glLineWidth(2.0);
        float cx = -8.2, cy = 15;

        for (int i = 0; i < 5; i++){
            for (int j = 0;j < 7;j++){

                if(bloques[i][j] > -1)   //revisa si hay un choque
                  if(hayChoque(cx,cy)) bloques[i][j] +=1;

                switch (bloques[i][j]) {
                  case 0:
                    if(buscarEspeciales((i*7+j))) dibujarBloque(cx, cy, 1); //dibuja especiales 
                    else dibujarBloque(cx, cy, 0);
                  break;
                  case 1:
                    if(buscarEspeciales(i*7+j)) dibujarBloqueRoto(cx, cy); //dibuja especiales golpeados una vez
                    else{
                      //esBonus = buscarBonus(i,j);
                      //if (esBonus > -1) lanzarBonus(i,j,esBonus);
                      //dibujarExplosion(2);
                      bloques[i][j] = -1;
                    }
                  break;
                  case 2:
                    //esBonus = buscarBonus(i,j);
                    //if (esBonus > -1) lanzarBonus(i,j,esBonus);
                    //dibujarExplosion(2);
                    bloques[i][j] = -1;
                  break;
                }

                cx += 2.5;
            }
            cx = -8.2;
            cy -= 1.25;
        }

    glPopMatrix();

}

/***************** FIN FUNCIONES PARA DIBUJAR *****************/

/************************* MOVIMIENTO *************************/
void moverPelota(int h){
  float v;
  if (h > 0){
    if(velocidad){
      v = velocidadP + velocidadP * 0.4;
      pelota[0] = v*cos(anguloP)+ pelota[0];
      pelota[1] = v*sin(anguloP)+ pelota[1];
    }else{      
      pelota[0] = velocidadP*cos(anguloP)+ pelota[0];
      pelota[1] = velocidadP*sin(anguloP)+ pelota[1];
    }
    
    lanzarBonus();
      
    glutTimerFunc(10,moverPelota,1);
    glutPostRedisplay();
  }
}

void handleSpecialKeypress(int key, int x, int y) {
    switch (key) {
        case GLUT_KEY_LEFT:
            isLeftKeyPressed = true;
            if (!isRightKeyPressed) {
                if (plataforma > -6.6 && !baseLarga) plataforma -= 0.2;
                else if (plataforma > -6.8 && !baseLarga) plataforma -= 0.1;
                else if (plataforma > -6.4 && baseLarga) plataforma -= 0.2;
                if (!moviendose){
                  anguloP = 95 + rand()% 25;
                  glutTimerFunc(10,moverPelota,1);
                  moviendose = true;
                } 
            }
        break;
        case GLUT_KEY_RIGHT:
            isRightKeyPressed = true;
            if (!isLeftKeyPressed) {
                if (plataforma < 6.6 && !baseLarga) plataforma += 0.2;
                else if (plataforma < 6.8 && !baseLarga) plataforma += 0.1;
                else if (plataforma < 6.4 && baseLarga) plataforma += 0.2;
                if (!moviendose){
                  anguloP = 40 + rand()% 39;
                  glutTimerFunc(10,moverPelota,1);
                  moviendose = true;
                } 
            }
        break;
    }

    glutPostRedisplay();
}

void handleSpecialKeyReleased(int key, int x, int y) {
    switch (key) {
        case GLUT_KEY_LEFT:
            isLeftKeyPressed = false;
        break;
        case GLUT_KEY_RIGHT:
            isRightKeyPressed = false;
        break;
    
    }
  // glutPostRedisplay(); Creo que en este caso no es necesario
}

void render(){
  float aspectradio;
  glClearColor(0.0f, 0.0f, 0.0f ,1.0f); 
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  if(inicial) {
    srand(time(NULL)); //genera semilla basada en el reloj del sistema
    generarEspeciales();
    generarBonus();
    inicial = false;
  }

//------------- Dibujamos PLATAFORMA -------------          
    dibujarPlataforma();

//------------- Dibujamos PELOTA -------------
    dibujarPelota(0.3);
    //explota[0] = 3;
    //explota[1] = 1;
    //dibujarExplosion(2);

//------------- Dibujamos MARCO -------------
    dibujarMarcoVerde();

//------------- Dibujamos BLOQUES -------------
    
    dibujarBloques();

//------------- Dibujo Bonus para probar -------------

  //dibujarBonusTamBase(0.4,-0.3,0.5);
  //dibujarBonusVelocidad(0.4,-0.3,0.5);
  
  glFlush();
  glutSwapBuffers();
}

int main (int argc, char** argv) {

    glutInit(&argc, argv);

    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);

    glutInitWindowSize(800,600);

    glutCreateWindow("Opengl");

    glutReshapeFunc(changeViewport);
    glutDisplayFunc(render);

    /*
    GLenum err = glewInit();
    if (GLEW_OK != err) {
    f//printf(stderr, "GLEW error");
    return 1;
    }
    */


    glutSpecialFunc(handleSpecialKeypress);
    glutSpecialUpFunc(handleSpecialKeyReleased);


    glutMainLoop();
    return 0;

}