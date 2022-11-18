//João Gabriel Fernandes Brandão
//Daniel de Alencar Carvalho

#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>
#include <math.h>
#include <stdio.h>

#define ESC 27
#define WIDTH 640
#define HEIGHT 480
#define DOOR 1
#define FAKEDOOR 4
#define WALL1 2
#define WALL2 3
#define FLOOR 0
#define MAPCOL 10
#define MAPROW 10
#define MAXLEVELS 3
#define CELLSIZEX WIDTH/MAPCOL
#define CELLSIZEY HEIGHT/MAPROW
#define PLAYERWIDTH 10
#define PLAYERHEIGHT 10
#define PLAYERSPEED 2
#define DEGREE M_PI/180
#define radtodeg(x) x*57.29578

//#include "sky.ppm"
//float skyMovement;
//#include "testTexture.txt"
#include "textureMap.ppm"

typedef struct player{
	float x, y, speed, angleX, angleY;
}PLAYER;
PLAYER player;

typedef struct entity{
	float x, y, z;
}ENTITY;
ENTITY enemy;

typedef struct keys{
	bool heldW;
	bool heldS;
	bool heldA;
	bool heldD;
	bool heldCAML;
	bool heldCAMR;
	bool heldShiftP;
}KEYS;
KEYS keys;

//Desenhando o mapa, 3 - Parede, 4 - Porta Falsa, 1 - Porta Correta, 0 - piso
char maps[MAXLEVELS][MAPROW][MAPCOL] = {{
							{3, 4, 3, 3, 3, 3, 3, 3, 1, 3},
							{3, 0, 0, 0, 0, 0, 0, 3, 0, 3},
							{3, 3, 3, 0, 3, 3, 0, 3, 0, 3},
							{3, 0, 0, 0, 3, 0, 0, 3, 0, 3},
							{3, 0, 3, 3, 3, 0, 3, 3, 0, 3},
							{3, 0, 0, 0, 3, 0, 0, 0, 0, 3},
							{3, 3, 3, 0, 3, 0, 3, 3, 0, 3},
							{3, 0, 0, 0, 3, 0, 3, 0, 0, 3},
							{3, 0, 3, 0, 0, 0, 3, 0, 0, 3},
							{3, 3, 3, 3, 3, 3, 3, 3, 3, 3}},
							{
                            {3, 1, 3, 3, 3, 3, 3, 3, 3, 3},
							{3, 0, 0, 0, 0, 0, 0, 0, 0, 3},
							{3, 3, 3, 3, 3, 3, 3, 0, 0, 3},
							{3, 0, 0, 0, 3, 0, 0, 0, 0, 3},
							{3, 0, 3, 0, 0, 0, 3, 3, 3, 3},
							{3, 0, 3, 3, 3, 0, 3, 0, 0, 3},
							{3, 0, 0, 0, 3, 0, 0, 0, 0, 3},
							{3, 3, 3, 0, 3, 0, 3, 0, 0, 3},
							{3, 0, 0, 0, 0, 0, 3, 0, 0, 4},
							{3, 3, 3, 3, 3, 3, 3, 3, 3, 3}},
                            {
                            {3, 3, 3, 3, 3, 3, 3, 3, 3, 3},
							{3, 0, 0, 0, 0, 0, 0, 0, 0, 4},
							{3, 0, 3, 0, 3, 3, 3, 3, 0, 3},
							{3, 0, 3, 0, 0, 0, 0, 3, 0, 3},
							{3, 0, 3, 0, 3, 3, 3, 3, 3, 3},
							{3, 0, 3, 0, 3, 0, 0, 0, 0, 3},
							{3, 0, 3, 0, 3, 0, 3, 0, 0, 3},
							{3, 0, 3, 0, 3, 0, 3, 0, 0, 3},
							{3, 0, 3, 0, 0, 0, 3, 0, 0, 1},
							{3, 3, 3, 3, 3, 3, 3, 3, 3, 3}}
							};

float lookSens;
float lastFrame = 0, currentFrame, deltaT;
float fps;
int   gameOriginX;
enum  PERSPECTIVE{Default, Double, TopDown};
PERSPECTIVE gamePerspective;
enum  STATE{Running, Paused};
STATE gameState;
int   currentLevel = 0;

void init();
void resize(int width, int height);
void keyboardUp(unsigned char key, int x, int y);
void keyboard(unsigned char key, int x, int y);
void mouse(int x, int y);
void display();
void idle();
void resetMouse();

int main(int argc, char** argv){
	//Window setup
	glutInit(&argc, argv);
	glutInitWindowPosition(glutGet(GLUT_SCREEN_WIDTH)/2 - WIDTH/2, glutGet(GLUT_SCREEN_HEIGHT)/2 - HEIGHT/2);
	glutInitWindowSize(WIDTH,HEIGHT);
	glutInitDisplayMode(GLUT_RGB | GLUT_SINGLE);
	glutCreateWindow("RaycasterEngine");
	init();
	
	//Callback functions
	glutReshapeFunc(resize);
    glutDisplayFunc(display);
    glutKeyboardUpFunc(keyboardUp);
    glutKeyboardFunc(keyboard);
    glutPassiveMotionFunc(mouse);
	glutIdleFunc(idle);
	
	//Glut main loop
	glutMainLoop();
	return 0;
}

void idle(){
	int delay = (int)fps/30;
	Sleep(delay);
	glutPostRedisplay();
}

//Set Player Position
void setPlayerPos(int x, int y){
	player.x = x*CELLSIZEX + CELLSIZEX/2;
	player.y = y*CELLSIZEY + CELLSIZEY/2;
}

void init() {
	glClearColor(0, 0, 0, 0);
	
	setPlayerPos(1, 1);
	player.speed = PLAYERSPEED*0.6;
	player.angleX = M_PI/2;
	
	lookSens = 2*M_PI/100;
	gamePerspective = Default;
	resetMouse();
	gameState = Running;
}

void resize(int width, int height) {
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    
    //Projection setup
    int currentWidth, currentHeight;
    if(gamePerspective == Default){
        currentWidth = WIDTH;
        currentHeight = HEIGHT;
        gameOriginX = 0;
    }
    else if(gamePerspective == Double){
        currentWidth = WIDTH*2;
        currentHeight = HEIGHT;
        gameOriginX = WIDTH;
    }
    else if(gamePerspective == TopDown){
        currentWidth = WIDTH;
        currentHeight = HEIGHT;
    }
    glViewport(0, 0, width, height);
    glOrtho (0, currentWidth, 0, currentHeight, -1 ,1);
    glutReshapeWindow(currentWidth, currentHeight);
    
    glMatrixMode(GL_MODELVIEW);
}

//Check Key Release
void keyboardUp(unsigned char key, int x, int y){
    if(key == 'w') keys.heldW = false;
    if(key == 's') keys.heldS = false;
    if(key == 'a') keys.heldA = false;
    if(key == 'd') keys.heldD = false;
    if(key == '1') keys.heldCAML = false;
    if(key == '3') keys.heldCAMR = false;
    if(key == 'P') keys.heldShiftP = false;
}

void keyboard (unsigned char key, int x, int y){
    // Comandos do jogo
    if(key == 'w') keys.heldW = true;
    if(key == 's') keys.heldS = true;
    if(key == 'a') keys.heldA = true;
    if(key == 'd') keys.heldD = true;
    if(key == '1') keys.heldCAML = true;
    if(key == '3') keys.heldCAMR = true;
    
    if(key == 'P' && !keys.heldShiftP){//Mudando a perspectiva do jogo
        if(gamePerspective == Default) gamePerspective = Double;
        else if(gamePerspective == Double) gamePerspective = TopDown;
        else if(gamePerspective == TopDown) gamePerspective = Default;
        resize(WIDTH, HEIGHT);
        keys.heldShiftP = true;
    }
    if(key == 'e'){
        //Interação de abrir porta para ir para proxima estágio
        float playerNextX, playerNextY;
        int nextTileCol, nextTileRow;
        
        playerNextX = player.x + player.speed*cos(player.angleX) + 3;
        playerNextY = player.y + player.speed*sin(player.angleX) + 3;
        nextTileCol = 100*playerNextX/CELLSIZEX;
        nextTileRow = MAPROW - 100*playerNextY/CELLSIZEY;
        
        //Mudando de nivel, checando se a interação é com uma porta
        if(maps[currentLevel][nextTileRow][nextTileCol] == DOOR && currentLevel + 1 < MAXLEVELS){
            //maps[currentLevel][nextTileRow][nextTileCol] = FLOOR;
            currentLevel++;
            setPlayerPos(1, 1);
        }
        else if(maps[currentLevel][nextTileRow][nextTileCol] == FAKEDOOR){
            setPlayerPos(1, 1);
        }
    }
    
    if(key == 'p' && gameState == Running) gameState = Paused;
    else if(key == 'p' && gameState == Paused) gameState = Running;
    if(key == ESC) exit(0);
}

//Controles do mouse
float mouseDX = 0, mouseDY = 0;
void mouse(int x, int y){
    if(gameState != Paused){
        mouseDX = x - WIDTH/2;
        mouseDY = -(y - HEIGHT/2);
        player.angleX -= mouseDX/1500;
        //player.angleY -= mouseDY/2;
        if(player.angleX > 2*M_PI) player.angleX = 0;
        else if(player.angleX < 0) player.angleX = 2*M_PI;
    }
}
void resetMouse(){
    glutWarpPointer(WIDTH/2, HEIGHT/2);
    mouseDX = WIDTH/2;
}

//Desenhar mira
void drawCrosshair(){
    int length = 5;
    glColor3f(0.5, 0.5, 0.5);
    glLineWidth(2);
    glBegin(GL_LINES);
        glVertex2f(gameOriginX + WIDTH/2, HEIGHT/2 + length);
        glVertex2f(gameOriginX + WIDTH/2, HEIGHT/2 - length);
        
        glVertex2f(gameOriginX + WIDTH/2 - length, HEIGHT/2);
        glVertex2f(gameOriginX + WIDTH/2 + length, HEIGHT/2);
    glEnd();
}

//Desenhar pause
void drawPause(){
    glColor3f(0.5, 0.5, 0.5);
    glBegin(GL_QUADS);
        glVertex2f(gameOriginX + WIDTH/2 - 20      - 5, HEIGHT/2 + 20);
        glVertex2f(gameOriginX + WIDTH/2 - 20      - 5, HEIGHT/2 + 20 - 40);
        glVertex2f(gameOriginX + WIDTH/2 - 20 + 10 - 5, HEIGHT/2 + 20 - 40);
        glVertex2f(gameOriginX + WIDTH/2 - 20 + 10 - 5, HEIGHT/2 + 20);
        
        glVertex2f(gameOriginX + WIDTH/2 + 20      - 5, HEIGHT/2 + 20);
        glVertex2f(gameOriginX + WIDTH/2 + 20      - 5, HEIGHT/2 + 20 - 40);
        glVertex2f(gameOriginX + WIDTH/2 + 20 + 10 - 5, HEIGHT/2 + 20 - 40);
        glVertex2f(gameOriginX + WIDTH/2 + 20 + 10 - 5, HEIGHT/2 + 20);
    glEnd();
}

bool checkCollision(){
    float playerNextX, playerNextY;
    int nextTileCol, nextTileRow;
    
    if(keys.heldW){
        playerNextX = player.x + player.speed*cos(player.angleX);
        playerNextX += (playerNextX < player.x) ? -3: 3;
        playerNextY = player.y + player.speed*sin(player.angleX);
        playerNextY += (playerNextY < player.y) ? -3: 3;
        
        nextTileCol = 100*playerNextX/CELLSIZEX;
        nextTileRow = MAPROW - 100*playerNextY/CELLSIZEY;
        
        if(maps[currentLevel][nextTileRow][nextTileCol] != FLOOR) return true;
    }
    if(keys.heldS){
        playerNextX = player.x - player.speed*cos(player.angleX);
        playerNextX += (playerNextX < player.x) ? -3: 3;
        playerNextY = player.y - player.speed*sin(player.angleX);
        playerNextY += (playerNextY < player.y) ? -3: 3;
        
        nextTileCol = 100*playerNextX/CELLSIZEX;
        nextTileRow = MAPROW - 100*playerNextY/CELLSIZEY;
        
        if(maps[currentLevel][nextTileRow][nextTileCol] != FLOOR) return true;
    }
    if(keys.heldA){
        playerNextX = player.x + player.speed*cos(player.angleX + M_PI/2);
        playerNextX += (playerNextX < player.x) ? -3: 3;
        playerNextY = player.y + player.speed*sin(player.angleX + M_PI/2);
        playerNextY += (playerNextY < player.y) ? -3: 3;
        
        nextTileCol = 100*playerNextX/CELLSIZEX;
        nextTileRow = MAPROW - 100*playerNextY/CELLSIZEY;
        
        if(maps[currentLevel][nextTileRow][nextTileCol] != FLOOR) return true;
    }
    if(keys.heldD){
        playerNextX = player.x - player.speed*cos(player.angleX + M_PI/2);
        playerNextX += (playerNextX < player.x) ? -3: 3;
        playerNextY = player.y - player.speed*sin(player.angleX + M_PI/2);
        playerNextY += (playerNextY < player.y) ? -3: 3;
        
        nextTileCol = 100*playerNextX/CELLSIZEX;
        nextTileRow = MAPROW - 100*playerNextY/CELLSIZEY;
        
        if(maps[currentLevel][nextTileRow][nextTileCol] != FLOOR) return true;
    }
    
    return false;
}

void movePlayer(){
    if(keys.heldW){
        if(!checkCollision()){
            player.x += player.speed*cos(player.angleX);
            player.y += player.speed*sin(player.angleX);
        }
    }
    if(keys.heldS) {
        if(!checkCollision()){
            player.x -= player.speed*cos(player.angleX);
            player.y -= player.speed*sin(player.angleX);
        }
    }
    if(keys.heldA) {
        if(!checkCollision()){
            player.x += player.speed*cos(player.angleX + M_PI/2);
            player.y += player.speed*sin(player.angleX + M_PI/2);
        }
    }
    if(keys.heldD) {
        if(!checkCollision()){
            player.x -= player.speed*cos(player.angleX + M_PI/2);
            player.y -= player.speed*sin(player.angleX + M_PI/2);
        }
    }
    if(keys.heldCAML){
        player.angleX += lookSens;
        if(player.angleX > 2*M_PI) player.angleX = 0;
    }
    if(keys.heldCAMR){
        player.angleX -= lookSens;
        if(player.angleX < 0) player.angleX = 2*M_PI;
    }
}

void drawPlayer(){
	//Player
	glColor3f(0, 1, 0);
    glBegin(GL_QUADS);
	    glVertex2f(player.x - PLAYERWIDTH/2, player.y - PLAYERHEIGHT/2);
	    glVertex2f(player.x + PLAYERWIDTH/2, player.y - PLAYERHEIGHT/2);
	    glVertex2f(player.x + PLAYERWIDTH/2, player.y + PLAYERHEIGHT/2);
	    glVertex2f(player.x - PLAYERWIDTH/2, player.y + PLAYERHEIGHT/2);
    glEnd();
    
    //Player Camera Vector
    int rayLen = 20;
    int rayX = player.x + rayLen*cos(player.angleX);
	int rayY = player.y + rayLen*sin(player.angleX);
    glColor3f(0, 1, 0);
    glBegin(GL_LINES);
    	glVertex2f(player.x, player.y);
    	glVertex2f(rayX, rayY);
	glEnd();
}

void drawMap(){
    float cellOriginX = 0;
    float cellOriginY = HEIGHT;
    
    //Draw Cells
    for(int row = 0; row < MAPROW; row++){
        for(int col = 0; col < MAPCOL; col++){
            if(maps[currentLevel][row][col] != FLOOR){
                if(maps[currentLevel][row][col] == DOOR) glColor3f(0, 1, 0);
                else if(maps[currentLevel][row][col] == FAKEDOOR) glColor3f(1, 0, 0);
                else glColor3f(1, 1, 1);
                glBegin(GL_QUADS);
                    glVertex2f(cellOriginX + 1           , cellOriginY -1);
                    glVertex2f(cellOriginX + 1           , cellOriginY - CELLSIZEY +1);
                    glVertex2f(cellOriginX + CELLSIZEX -1, cellOriginY - CELLSIZEY +1);
                    glVertex2f(cellOriginX + CELLSIZEX -1, cellOriginY -1);
                glEnd();
            }
    
               cellOriginX += CELLSIZEX;
        }
        
        cellOriginX = 0;
        cellOriginY -= CELLSIZEY;
    }
    
    //Grid
    int x, y;
    glColor3f(0.5, 0.5, 0.5);
    glLineWidth(2);
    glBegin(GL_LINES);
    for(x = 0; x <= WIDTH; x += CELLSIZEX){
        glVertex2f(x, 0);
        glVertex2f(x, HEIGHT);
    }
    for(y = 0; y <= HEIGHT; y += CELLSIZEY){
        glVertex2f(0, y);
        glVertex2f(WIDTH, y);
    }
    glEnd();
    
    //Draw Ground and Sky
    float groundColorR = 0.2, groundColorG = 0.15, groundColorB = 0.1;
    float skyColorR = 0.07, skyColorG = 0.07, skyColorB = 0.12;
    if(gamePerspective != TopDown){
        //- Ground
        glColor3f(groundColorR, groundColorG, groundColorB);
        if(gameState == Paused) glColor3f(0.3, 0.2, 0);
        glBegin(GL_QUADS);
            glVertex2f(gameOriginX, 0);
            glVertex2f(gameOriginX + WIDTH, 0);
            glColor3f(0, 0, 0);
            if(gameState == Paused) glColor3f(groundColorR/4, groundColorG/4, groundColorB/4);
            glVertex2f(gameOriginX + WIDTH, HEIGHT/2 + player.angleY);
            glVertex2f(gameOriginX, HEIGHT/2 + player.angleY);
        glEnd();
        
        //- Flat Sky
        glColor3f(skyColorR, skyColorG, skyColorB);
        if(gameState == Paused) glColor3f(0.3, 0.2, 0);
        glBegin(GL_QUADS);
            glVertex2f(gameOriginX, HEIGHT);
            glVertex2f(gameOriginX + WIDTH, HEIGHT);
            glColor3f(skyColorR/2, skyColorG/2, skyColorB/2);
            if(gameState == Paused) glColor3f(skyColorR/4, skyColorG/4, skyColorB/4);
            glVertex2f(gameOriginX + WIDTH, HEIGHT/2 + player.angleY);
            glVertex2f(gameOriginX, HEIGHT/2 + player.angleY);
        glEnd();
        
        //- Sky Texture
        // int y, x;
        // int pixelIndex = skyMovement + ((WIDTH - 1)*player.angleX)/(2*M_PI);
        // pixelIndex = (WIDTH - 1) - pixelIndex;
        // pixelIndex = pixelIndex - pixelIndex%3;
        // printf("%i\n", pixelIndex);
        //
        // glPointSize(1);
        // glBegin(GL_POINTS);
        // for(y = HEIGHT; y > HEIGHT/2; y--){
        //     for(x = 0; x < WIDTH; x++){
        //         glColor3ub(sky[pixelIndex]/10, sky[pixelIndex + 1]/10, sky[pixelIndex + 2]/10);
        //         glVertex2f(gameOriginX + x, y + player.angleY);
        //         pixelIndex += 3;
        //     }
        // }
        // glEnd();
        //
        // skyMovement += 0.3;
        // if(skyMovement > WIDTH - 1) skyMovement = 0;
    }
    
    //Raycasting
    float rayXV, rayYV,rayLenV;
    float rayXH, rayYH, rayLenH;
    float rayFinalX, rayFinalY;
    float rayLength = 1;
    int   distanceToLeft;
    int     distanceToRight;
    int   distanceToBot;
    int   distanceToTop;
    
    float rayAngle = 0;
    int   lastIteration = 15;
    int   columns = 80;
    float fov = columns*DEGREE;
    
    int colWidth = WIDTH/columns;
    int colHeight;
    int colX = gameOriginX;
    
    //std::cout << "-------------First Ray-----------" << '\n';
    for(float rayAngleOffset = fov/2; rayAngleOffset > -fov/2; rayAngleOffset -= DEGREE){
        rayAngle = player.angleX + rayAngleOffset;
        if(rayAngle > 2*M_PI) rayAngle -= 2*M_PI;
        if(rayAngle < 0) rayAngle = 2*M_PI + rayAngle;
        
        distanceToLeft = player.x - CELLSIZEX*floor(100*player.x/CELLSIZEX);
        distanceToRight = CELLSIZEX - distanceToLeft;
        distanceToBot = player.y - CELLSIZEY*floor(100*player.y/CELLSIZEY);
        distanceToTop = CELLSIZEY - distanceToBot;
        
        int tileType;
        for(int i = 0; i <= lastIteration; i++){
            //- Check Vertical
            if(rayAngle < M_PI/2 || rayAngle > 3*M_PI/2){//Right
                rayXV = player.x + distanceToRight;
                rayYV = player.y + (distanceToRight)*tan(rayAngle);
                   rayLenV = sqrt(pow(distanceToRight, 2) + pow(distanceToRight*tan(rayAngle), 2));
            }
            else if(rayAngle > M_PI/2 && rayAngle < 3*M_PI/2){//Left
                rayXV = player.x - distanceToLeft;
                rayYV = player.y - (distanceToLeft)*tan(rayAngle);
                   rayLenV = sqrt(pow(distanceToLeft, 2) + pow(distanceToLeft*tan(rayAngle), 2));
            }
            
            //- Check Horizontal
            if(rayAngle < M_PI && rayAngle > 0){//Up
                rayXH = player.x + (distanceToTop)/tan(rayAngle);
                rayYH = player.y + distanceToTop;
                rayLenH = sqrt(pow(distanceToTop/tan(rayAngle), 2) + pow(distanceToTop, 2));
            }
            else if(rayAngle > M_PI && rayAngle < 2*M_PI){//Down
                rayXH = player.x - (distanceToBot)/tan(rayAngle);
                rayYH = player.y - distanceToBot;
                rayLenH = sqrt(pow(distanceToBot/tan(rayAngle), 2) + pow(distanceToBot, 2));
            }
            
            //- Check Shortest Hit
            if(rayLenV < rayLenH){
                rayLength = rayLenV;
                rayFinalX = rayXV;
                rayFinalY = rayYV;
                
                //Convert Coordinates to Matrix
                int tileHitRow = MAPROW - 1 - floor(100*rayYV/CELLSIZEY);
                int tileHitCol = floor(100*rayXV/CELLSIZEX);
                tileHitCol = (rayXV < player.x) ? tileHitCol - 1: tileHitCol;
                
                tileType = maps[currentLevel][tileHitRow][tileHitCol];
                
                if(maps[currentLevel][tileHitRow][tileHitCol] != FLOOR){
                    if(gamePerspective != Default){
                        //Draw Ray
                        glColor3f(1, 1, 0);
                        glLineWidth(1);
                        glBegin(GL_LINES);
                            glVertex2f(player.x, player.y);
                            glVertex2f(rayXV, rayYV);
                        glEnd();
                        
                        //Hit Marker
                        glColor3f(1, 0, 0);
                        glBegin(GL_QUADS);
                            glVertex2f(rayXV - 3, rayYV - 3);
                            glVertex2f(rayXV + 3, rayYV - 3);
                            glVertex2f(rayXV + 3, rayYV + 3);
                            glVertex2f(rayXV - 3, rayYV + 3);
                        glEnd();
                    }
                    break;
                }
                if(rayAngle < M_PI/2 || rayAngle > 3*M_PI/2) distanceToRight += CELLSIZEX;//If Right
                else if(rayAngle > M_PI/2 && rayAngle < 3*M_PI/2) distanceToLeft += CELLSIZEX;//If Left
                
                glColor3f(0.5, 0.5, 0.5);
            }
            else if(rayLenV > rayLenH){
                rayLength = rayLenH;
                rayFinalX = rayXH;
                rayFinalY = rayYH;
                
                int tileHitRow = MAPROW - 1 - floor(100*rayYH/CELLSIZEY);
                int tileHitCol = floor(100*rayXH/CELLSIZEX);
                tileHitRow = (rayYH < player.y) ? tileHitRow + 1: tileHitRow;
                
                tileType = maps[currentLevel][tileHitRow][tileHitCol];
                
                if(maps[currentLevel][tileHitRow][tileHitCol] != FLOOR){
                    if(gamePerspective != Default){
                        //Draw Ray
                        glColor3f(1, 0.8, 0);
                        glLineWidth(1);
                        glBegin(GL_LINES);
                            glVertex2f(player.x, player.y);
                            glVertex2f(rayXH, rayYH);
                        glEnd();
                        
                        //Hit Marker
                        glColor3f(1, 0, 0);
                        glBegin(GL_QUADS);
                            glVertex2f(rayXH - 3, rayYH - 3);
                            glVertex2f(rayXH + 3, rayYH - 3);
                            glVertex2f(rayXH + 3, rayYH + 3);
                            glVertex2f(rayXH - 3, rayYH + 3);
                        glEnd();
                    }
                    break;
                }
                if(rayAngle < M_PI && rayAngle > 0) distanceToTop += CELLSIZEY;
                else if(rayAngle > M_PI && rayAngle < 2*M_PI) distanceToBot += CELLSIZEY;
                
                glColor3f(0.3, 0.3, 0.3);
            }
        }
            
        if(gamePerspective != TopDown){
            //Render Game World
            rayLength *= cos(rayAngleOffset);
            colHeight = (float)45*HEIGHT/rayLength;
            float shade = 30/rayLength; if(shade > 1) shade = 1;
            //shade = 1;
            
            //Calculate Texture Pixel
            int tX;
            if(rayLenV < rayLenH){
                tX = (int)rayYV%(CELLSIZEY);
                tX = 32*tX/(CELLSIZEY);
                if(player.angleX >= 3*M_PI/2 && player.angleX <= M_PI/2) tX = 31 - tX;
            }
            else{
                shade *= 0.5;//Natural Shade
                
                tX = (int)rayXH%(CELLSIZEX);
                tX = 32*tX/(CELLSIZEX);
                if(player.angleX >= M_PI && player.angleX <= 2*M_PI) tX = 31 - tX;
            }
            tX *= 3;
            
            //Render Walls
            int y;
            float tR, tG, tB, tY = 0, tZ, tIndex;
            tZ = tileType - 1;
            if(tileType == FAKEDOOR) tZ = 0;
            float tOffset = 32/(float)colHeight;
            glPointSize(colWidth);
            glBegin(GL_POINTS);
            for(y = colHeight/2 + HEIGHT/2; y - colWidth/2 >= -colHeight/2 + HEIGHT/2; y--){
                tIndex = (int)tZ*32*32*3 + (int)tY*32*3 + (int)tX;
                
                tR = textureMap[(int)tIndex];
                tG = textureMap[(int)tIndex + 1];
                tB = textureMap[(int)tIndex + 2];
                
                glColor3ub(tR*shade, tG*shade, tB*shade);
                if(gameState == Paused) glColor3ub(tR*shade/2, tG*shade/2, tB*shade/2);
                
                glVertex2f(colX + colWidth/2, y + player.angleY);
            
                tY += tOffset;
            }
            
            //- Wall Reflections
            // tY = 0;
            // int colBottomY = y;
            // tOffset = 32/(float)colHeight;
            // for(y = -colHeight/2 + HEIGHT/2; y - colWidth >= colBottomY - colHeight; y--){
            //     tIndex = (int)tZ*32*32*3 + (int)tY*32*3 + (int)tX;
            //     tR = textureMap[(int)tIndex];
            //     tG = textureMap[(int)tIndex + 1];
            //     tB = textureMap[(int)tIndex + 2];
   //               glColor3ub(shade*(tR + 255*groundColorR)/2, shade*(tG + 255*groundColorG)/2, shade*(tB + 255*groundColorB)/2);
            //     glVertex2f(colX + colWidth/2, -(y - player.angleY - colBottomY) + 1 + colBottomY - colHeight);
            //     tY += tOffset;
            // }
            
            //- Render Floor
            // rayLength /= cos(rayAngleOffset);
            // int horizonY = HEIGHT/2;
            //
            // float dY = rayLength/horizonY;
            // float floorY = horizonY - dY;
            // tY = (int)floorY%32;
            //
            // float dX;
            // if(rayAngle > 0 && rayAngle < M_PI/2) dX = dY/tan(rayAngle);
            // else if(rayAngle > M_PI/2 && rayAngle < M_PI) dX = dY/tan(M_PI - rayAngle);
            // else if(rayAngle > M_PI && rayAngle < 3*M_PI/2) dX = dY/tan(3*M_PI/2 - rayAngle);
            // else if(rayAngle > 3*M_PI/2 && rayAngle < 2*M_PI) dX = dY/tan(rayAngle - 3*M_PI/1);
            // float floorX;
            // if(rayAngle > 2*M_PI || rayAngle < M_PI/2) floorX = rayFinalX - dX;
            // else if(rayAngle > M_PI/2 && rayAngle < 3*M_PI/2) floorX = rayFinalX + dX;
            // tX = (int)floorX%32;
            //
            // for(y = -colHeight/2 + HEIGHT/2; y - colWidth/2 >= 0; y--){
            //     shade = 100/sqrt(pow(player.x - floorX, 2) + pow(floorY - player.y, 2));
            //
            //     tIndex = (int)tY*32*1 + (int)tX;
            //     tR = textureMapTest[(int)tIndex]*255;
            //     tG = textureMapTest[(int)tIndex + 0]*255;
            //     tB = textureMapTest[(int)tIndex + 0]*255;
   //              glColor3ub(tR, tG, tB);
            //     glVertex2f(colX + colWidth/2, y + player.angleY);
            //
            //     if(rayAngle > 0 && rayAngle < M_PI) floorY -= dY;
            //     else if(rayAngle > M_PI && rayAngle < 2*M_PI) floorY += dY;
            //     tY = (int)floorY%32;
            //
            //     if(rayAngle > 2*M_PI || rayAngle < M_PI/2) floorX -= dX;
            //     else if(rayAngle > M_PI/2 && rayAngle < 3*M_PI/2) floorX += dX;
            //     tX = (int)floorX%32;
            // }
        
            glEnd();
        }
            
        colX += colWidth;
    }
    //std::cout << "-------------Last Ray-----------" << '\n';
}

void display(){
	if(gameState == Running){
		currentFrame = glutGet(GLUT_ELAPSED_TIME);
		deltaT = currentFrame - lastFrame;
		lastFrame = currentFrame;
		fps = 1000/deltaT;
		
		glClear (GL_COLOR_BUFFER_BIT);
	    glLoadIdentity();
	    
	    movePlayer();
	    resetMouse();
		glutSetCursor(GLUT_CURSOR_NONE);
	    drawMap();
	    drawCrosshair();
		if(gamePerspective != Default) drawPlayer();
		//drawEntity();
	}
	else if(gameState == Paused){
		drawPause();
		glutSetCursor(GLUT_CURSOR_LEFT_ARROW); 
	}
		
	glFlush();
}
