#include <windows.h>
#include <gl/glu.h>
#include <gl/gl.h>
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>

#ifndef GL_BGR
#define GL_BGR 0x80E0
#endif

#ifndef GL_BGRA
#define GL_BGRA 0x80E1
#endif

GLuint texturaParede;
GLuint texturaChao;
GLuint texturaSteve;
GLuint texturaInimigo;

void desenhaPlayer(float x, float z);
void desenhaCubo(float x, float y, float z);
void desenhaInimigo(float x, float z);
void desenhaBarraTempo(int tempoRestante, int tempoLimite);
void desenhaChave(float x, float z);
GLuint carregarTextura(const char* arquivo);
int podeAndar(float x, float z);

#define MAP_WIDTH  15
#define MAP_HEIGHT 15

int labirinto[MAP_HEIGHT][MAP_WIDTH] = {
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
    {1,0,0,0,0,0,1,0,0,0,0,0,0,0,1},
    {1,0,1,1,1,0,1,0,1,1,1,1,0,1,1},
    {1,0,1,0,0,0,0,0,1,0,0,1,0,0,1},
    {1,0,1,0,1,1,1,1,1,0,1,1,1,0,1},
    {1,0,0,0,1,0,0,0,0,0,1,0,0,0,1},
    {1,1,1,0,0,0,1,1,1,0,1,0,1,0,1},
    {1,0,0,0,1,0,1,0,0,0,1,0,1,0,1},
    {1,0,1,1,1,0,0,0,1,1,1,0,1,0,1},
    {1,0,1,0,0,0,1,0,0,0,0,0,1,0,1},
    {1,0,1,0,1,1,1,1,1,0,1,1,1,0,1},
    {1,0,0,0,1,0,0,0,1,0,0,0,0,0,1},
    {1,1,1,0,1,0,1,0,1,1,1,1,0,1,1},
    {1,0,0,0,0,0,1,0,0,0,0,0,0,0,1},
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1}
};

// Valores de localização do personagem, inimigo, chave, tempo e saída; além de booleans que mostram se a chave foi coletada e qual o modo de jogo

int exitX = 13;
int exitZ = 13;

int modoJogo = 0;
int estadoJogo = 0;

int chaveColetada = 0;
float chaveX = 7.5f;
float chaveZ = 7.5f;
float rotacaoChave = 0.0f;

float playerX = 1.5f;
float playerZ = 1.5f;
float playerSpeed = 0.1f;

int inimigoVivo = 1;
float inimigoX;
float inimigoZ;
float velocidadeInimigo = 0.7f;

DWORD tempoInicio;
int tempoLimite = 45;

GLuint carregarTextura(const char* arquivo)
{
    GLuint textura;
    HBITMAP hBMP;
    BITMAP bmp;

    glGenTextures(1, &textura);
    hBMP = (HBITMAP)LoadImageA(
        GetModuleHandle(NULL),
        arquivo,
        IMAGE_BITMAP,
        0, 0,
        LR_LOADFROMFILE | LR_CREATEDIBSECTION
    );

    GetObject(hBMP, sizeof(bmp), &bmp);

    glBindTexture(GL_TEXTURE_2D, textura);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glTexImage2D(
        GL_TEXTURE_2D,
        0,
        GL_RGB,
        bmp.bmWidth,
        bmp.bmHeight,
        0,
        GL_BGR,
        GL_UNSIGNED_BYTE,
        bmp.bmBits
    );


    DeleteObject(hBMP);
    return textura;
}

int podeAndar(float x, float z)
{
    int mapaX = (int)floor(x);
    int mapaZ = (int)floor(z);

    if (mapaX < 0 || mapaX >= MAP_WIDTH ||
        mapaZ < 0 || mapaZ >= MAP_HEIGHT)
        return 0;

    if (labirinto[mapaZ][mapaX] == 0)
        return 1;

    return 0;
}

LRESULT CALLBACK WindowProc(HWND, UINT, WPARAM, LPARAM);
void EnableOpenGL(HWND hwnd, HDC*, HGLRC*);
void DisableOpenGL(HWND, HDC, HGLRC);

void ResizeGLScene(int width, int height)
{
    if (height == 0) height = 1;

    glViewport(0, 0, width, height);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(60.0f, (float)width / (float)height, 1.0f, 100.0f);
    glMatrixMode(GL_MODELVIEW);
}

void desenhaCubo(float x, float y, float z)
{
    float s = 1.0f;

    glPushMatrix();
    glTranslatef(x, y, z);

    glBindTexture(GL_TEXTURE_2D, texturaParede);
    glColor3f(1.0f, 1.0f, 1.0f);

    glBegin(GL_QUADS);

    // FRENTE
    glTexCoord2f(0, 0); glVertex3f(0, 0, s);
    glTexCoord2f(1, 0); glVertex3f(s, 0, s);
    glTexCoord2f(1, 1); glVertex3f(s, s, s);
    glTexCoord2f(0, 1); glVertex3f(0, s, s);

    // TRÁS
    glTexCoord2f(0, 0); glVertex3f(s, 0, 0);
    glTexCoord2f(1, 0); glVertex3f(0, 0, 0);
    glTexCoord2f(1, 1); glVertex3f(0, s, 0);
    glTexCoord2f(0, 1); glVertex3f(s, s, 0);

    // ESQUERDA
    glTexCoord2f(0, 0); glVertex3f(0, 0, 0);
    glTexCoord2f(1, 0); glVertex3f(0, 0, s);
    glTexCoord2f(1, 1); glVertex3f(0, s, s);
    glTexCoord2f(0, 1); glVertex3f(0, s, 0);

    // DIREITA
    glTexCoord2f(0, 0); glVertex3f(s, 0, s);
    glTexCoord2f(1, 0); glVertex3f(s, 0, 0);
    glTexCoord2f(1, 1); glVertex3f(s, s, 0);
    glTexCoord2f(0, 1); glVertex3f(s, s, s);

    // TOPO
    glTexCoord2f(0, 0); glVertex3f(0, s, s);
    glTexCoord2f(1, 0); glVertex3f(s, s, s);
    glTexCoord2f(1, 1); glVertex3f(s, s, 0);
    glTexCoord2f(0, 1); glVertex3f(0, s, 0);

    // BASE
    glTexCoord2f(0, 0); glVertex3f(0, 0, 0);
    glTexCoord2f(1, 0); glVertex3f(s, 0, 0);
    glTexCoord2f(1, 1); glVertex3f(s, 0, s);
    glTexCoord2f(0, 1); glVertex3f(0, 0, s);

    glEnd();
    glPopMatrix();
}

int WINAPI WinMain(HINSTANCE hInstance,
                   HINSTANCE hPrevInstance,
                   LPSTR lpCmdLine,
                   int nCmdShow)
{
    WNDCLASSEX wcex;
    HWND hwnd;
    HDC hDC;
    HGLRC hRC;
    MSG msg;
    BOOL bQuit = FALSE;
    float theta = 0.0f;

    /* register window class */
    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style = CS_OWNDC;
    wcex.lpfnWndProc = WindowProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
    wcex.lpszMenuName = NULL;
    wcex.lpszClassName = "GLSample";
    wcex.hIconSm = LoadIcon(NULL, IDI_APPLICATION);;


    if (!RegisterClassEx(&wcex))
        return 0;

    /* create main window */
    hwnd = CreateWindowEx(0,
                          "GLSample",
                          "OpenGL Sample",
                          WS_OVERLAPPEDWINDOW,
                          CW_USEDEFAULT,
                          CW_USEDEFAULT,
                          256,
                          256,
                          NULL,
                          NULL,
                          hInstance,
                          NULL);

    ShowWindow(hwnd, nCmdShow);

    /* enable OpenGL for the window */
    EnableOpenGL(hwnd, &hDC, &hRC);
    glEnable(GL_TEXTURE_2D);
    texturaParede = carregarTextura("stone.bmp");
    texturaChao = carregarTextura("grass.bmp");
    texturaSteve = carregarTextura("steve_head.bmp");
    texturaInimigo = carregarTextura("zombie_head.bmp");
    glEnable(GL_DEPTH_TEST);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(60.0f, 1.0f, 1.0f, 100.0f);
    glMatrixMode(GL_MODELVIEW);
    tempoInicio = GetTickCount();
    srand(time(NULL));

    inimigoX = 3.5f;
    inimigoZ = 10.5f;

    if (!podeAndar(inimigoX, inimigoZ)) {
        int encontrou = 0;
        for (int i = -1; i <= 1 && !encontrou; i++) {
            for (int j = -1; j <= 1; j++) {
                float testX = exitX + i + 0.5f;
                float testZ = exitZ + j + 0.5f;
                if (podeAndar(testX, testZ)) {
                    inimigoX = testX;
                    inimigoZ = testZ;
                    encontrou = 1;
                    break;
                }
            }
        }
    }

    /* program main loop */
    while (!bQuit)
    {
        DWORD tempoAtual = GetTickCount();
        int tempoDecorrido = (tempoAtual - tempoInicio) / 1000;
        int tempoRestante = tempoLimite - tempoDecorrido;
        if (tempoRestante < 0) tempoRestante = 0;


        if (tempoDecorrido >= tempoLimite)
        {
            MessageBox(NULL, "Tempo esgotado! Você perdeu!", "Fim de Jogo", MB_OK);
            PostQuitMessage(0);
        }

        /* check for messages */
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            /* handle or dispatch messages */
            if (msg.message == WM_QUIT)
            {
                bQuit = TRUE;
            }
            else
            {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
        }
        else
        {
            /* OpenGL animation code goes here */

            glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            if (estadoJogo == 0) {
                    // Menu
                    desenhaMenu();
                } else {
                    if (modoJogo == 1) {
                        configurarLuzModoDark();

                        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

                        glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

                         float centroPlayer = 0.3f;
                         GLfloat luzPosicao[] = { playerX, 0.3f, playerZ, 1.0f };
                         glLightfv(GL_LIGHT0, GL_POSITION, luzPosicao);
                    } else {
                        glDisable(GL_LIGHTING);
                        glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
                        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
                    }

                    static DWORD ultimoMovimento = 0;
                    DWORD agora = GetTickCount();

                    int intervaloMovimento = 100;
                    if (agora - ultimoMovimento > intervaloMovimento) {
                    // ===== MOVIMENTO DO INIMIGO =====
                    if (inimigoVivo) {
                        ultimoMovimento = agora;
                        float dx = playerX - inimigoX;
                        float dz = playerZ - inimigoZ;

                        float distancia = sqrt(dx*dx + dz*dz);

                        if (distancia > 0.8f) {
                            float dirX = dx / distancia;
                            float dirZ = dz / distancia;

                            float velocidadeAtual;
                            if (distancia > 5.0f) {
                                velocidadeAtual = 0.03f;
                            } else if (distancia > 3.0f) {
                                velocidadeAtual = 0.05f;
                            } else if (distancia > 1.5f) {
                                velocidadeAtual = 0.08f;
                            } else {
                                velocidadeAtual = 0.1f;
                            }

                            float novoX = inimigoX + dirX * velocidadeAtual;
                            float novoZ = inimigoZ + dirZ * velocidadeAtual;

                            if (novoX >= 0.0f && novoX <= MAP_WIDTH - 1 &&
                                podeAndar(novoX, inimigoZ)) {
                                inimigoX = novoX;
                            }

                            if (novoZ >= 0.0f && novoZ <= MAP_HEIGHT - 1 &&
                                podeAndar(inimigoX, novoZ)) {
                                inimigoZ = novoZ;
                            }
                        }
                    }
                   }
                    // COLISÃO
                    float distanciaColisao = sqrt(
                        (playerX - inimigoX)*(playerX - inimigoX) +
                        (playerZ - inimigoZ)*(playerZ - inimigoZ)
                    );

                    if (distanciaColisao < 0.8f)
                    {
                        MessageBox(NULL, "O inimigo te pegou!", "Derrota", MB_OK);
                        PostQuitMessage(0);
                    }


                    /* posição da câmera */
                    glMatrixMode(GL_MODELVIEW);
                    glLoadIdentity();


                    float camX = playerX;
                    float camY = 12.0f;
                    float camZ = playerZ + 12.0f;

                    gluLookAt(
                        camX, camY, camZ,
                        playerX, 0.0f, playerZ,
                        0.0f, 1.0f, 0.0f
                    );

                    if (!chaveColetada) {
                        float distChave = sqrt(pow(playerX - chaveX, 2) + pow(playerZ - chaveZ, 2));
                        if (distChave < 0.8f) {
                            chaveColetada = 1;
                            Beep(1000, 200);
                        }
                    }

                    /* chão com textura */
                    glBindTexture(GL_TEXTURE_2D, texturaChao);
                    glColor3f(1.0f, 1.0f, 1.0f);

                    // No desenho do chão
                    glBegin(GL_QUADS);
                        glTexCoord2f(0, 0);  glVertex3f(0.0f, 0.0f, 0.0f);
                        glTexCoord2f(MAP_WIDTH, 0); glVertex3f(MAP_WIDTH, 0.0f, 0.0f);
                        glTexCoord2f(MAP_WIDTH, MAP_HEIGHT); glVertex3f(MAP_WIDTH, 0.0f, MAP_HEIGHT);
                        glTexCoord2f(0, MAP_HEIGHT);  glVertex3f(0.0f, 0.0f, MAP_HEIGHT);
                    glEnd();


                    glPushMatrix();
                    glTranslatef(exitX, 0.01f, exitZ);

                    if (chaveColetada) {
                        glColor3f(0.0f, 1.0f, 0.0f);
                    } else {
                        glColor3f(1.0f, 0.0f, 0.0f);
                    }

                    glBegin(GL_QUADS);
                        glVertex3f(0, 0, 0);
                        glVertex3f(1, 0, 0);
                        glVertex3f(1, 0, 1);
                        glVertex3f(0, 0, 1);
                    glEnd();
                    glPopMatrix();

                    /* labirinto */
                    for (int z = 0; z < MAP_HEIGHT; z++)
                    {
                        for (int x = 0; x < MAP_WIDTH; x++)
                        {
                            if (labirinto[z][x] == 1)
                            {
                                desenhaCubo((float)x, 0.0f, (float)z);
                            }
                        }
                    }
                    /* player */
                        desenhaPlayer(playerX, playerZ);
                        if (inimigoX >= 0 && inimigoX < MAP_WIDTH &&
                            inimigoZ >= 0 && inimigoZ < MAP_HEIGHT) {
                            desenhaInimigo(inimigoX, inimigoZ);
                        }
                        desenhaChave(chaveX, chaveZ);
                        glDisable(GL_LIGHTING);
                        desenhaBarraTempo(tempoRestante, tempoLimite);
                        desenhaBarraTempo(tempoRestante, tempoLimite);

                    // Tela Vermelha quando o inimigo chega perto
                    if(modoJogo == 0){
                        float distInimigo = sqrt((playerX - inimigoX)*(playerX - inimigoX) +
                                            (playerZ - inimigoZ)*(playerZ - inimigoZ));

                        if (distInimigo < 3.0f) {
                            float intensidade = 1.0f - (distInimigo / 3.0f);

                            glEnable(GL_BLEND);
                            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

                            glDisable(GL_TEXTURE_2D);
                            glDisable(GL_DEPTH_TEST);

                            glMatrixMode(GL_PROJECTION);
                            glPushMatrix();
                            glLoadIdentity();
                            gluOrtho2D(0, 800, 0, 600);

                            glMatrixMode(GL_MODELVIEW);
                            glPushMatrix();
                            glLoadIdentity();

                            glColor4f(1.0f, 0.0f, 0.0f, intensidade * 0.3f);
                            glBegin(GL_QUADS);
                                glVertex2f(0, 0);
                                glVertex2f(800, 0);
                                glVertex2f(800, 600);
                                glVertex2f(0, 600);
                            glEnd();

                            glPopMatrix();
                            glMatrixMode(GL_PROJECTION);
                            glPopMatrix();
                            glMatrixMode(GL_MODELVIEW);

                            glEnable(GL_DEPTH_TEST);
                            glEnable(GL_TEXTURE_2D);
                        }

                            glDisable(GL_LIGHTING);
                            desenhaBarraTempo(tempoRestante, tempoLimite);
                        }
                }

            SwapBuffers(hDC);

            theta += 1.0f;
            Sleep (1);
        }
    }

    /* shutdown OpenGL */
    DisableOpenGL(hwnd, hDC, hRC);

    /* destroy the window explicitly */
    DestroyWindow(hwnd);

    return msg.wParam;
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
        case WM_CLOSE:
            PostQuitMessage(0);
        break;

        case WM_DESTROY:
            return 0;

        case WM_SIZE:
        {
            int width  = LOWORD(lParam);
            int height = HIWORD(lParam);
            ResizeGLScene(width, height);
        }
        break;

        // Mapeamento de teclas do player e tambem do menu
        case WM_KEYDOWN:
        {
            if (estadoJogo == 0) {
                switch (wParam) {
                    case '1':
                        modoJogo = 0;
                        estadoJogo = 1;
                        tempoInicio = GetTickCount();
                        break;
                    case '2':
                        modoJogo = 1;
                        estadoJogo = 1;
                        tempoInicio = GetTickCount();
                        break;
                }
                break;
            }
            float novoX = playerX;
            float novoZ = playerZ;
            int moveu = 0;

            switch (wParam)
            {
                case 'W': {
                    novoZ = playerZ - playerSpeed;
                    if (podeAndar(playerX, novoZ)) {
                        playerZ = novoZ;
                        moveu = 1;
                    }
                    break;
                }
                case 'S': {
                    novoZ = playerZ + playerSpeed;
                    if (podeAndar(playerX, novoZ)) {
                        playerZ = novoZ;
                        moveu = 1;
                    }
                    break;
                }
                case 'A': {
                    novoX = playerX - playerSpeed;
                    if (podeAndar(novoX, playerZ)) {
                        playerX = novoX;
                        moveu = 1;
                    }
                    break;
                }
                case 'D': {
                    novoX = playerX + playerSpeed;
                    if (podeAndar(novoX, playerZ)) {
                        playerX = novoX;
                        moveu = 1;
                    }
                    break;
                }
                case VK_ESCAPE:
                    PostQuitMessage(0);
                    break;
            }

            if (moveu) {
                int pX = (int)(playerX + 0.5f);
                int pZ = (int)(playerZ + 0.5f);

                if (pX == exitX && pZ == exitZ) {
                    if (chaveColetada) {
                        MessageBox(NULL, "Você venceu o labirinto!", "Vitória", MB_OK);
                        PostQuitMessage(0);
                    } else {
                        MessageBox(NULL, "A porta está trancada! Encontre a chave primeiro!", "Porta Trancada", MB_OK);
                        playerX = playerX - (playerSpeed * (wParam == 'A' ? -1 : 1));
                        playerZ = playerZ - (playerSpeed * (wParam == 'W' ? -1 : 1));
                    }
                }
            }
        }
        break;

        default:
            return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }

    return 0;
}

void EnableOpenGL(HWND hwnd, HDC* hDC, HGLRC* hRC)
{
    PIXELFORMATDESCRIPTOR pfd;

    int iFormat;

    /* get the device context (DC) */
    *hDC = GetDC(hwnd);

    /* set the pixel format for the DC */
    ZeroMemory(&pfd, sizeof(pfd));

    pfd.nSize = sizeof(pfd);
    pfd.nVersion = 1;
    pfd.dwFlags = PFD_DRAW_TO_WINDOW |
                  PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
    pfd.iPixelType = PFD_TYPE_RGBA;
    pfd.cColorBits = 24;
    pfd.cDepthBits = 16;
    pfd.iLayerType = PFD_MAIN_PLANE;

    iFormat = ChoosePixelFormat(*hDC, &pfd);

    SetPixelFormat(*hDC, iFormat, &pfd);

    /* create and enable the render context (RC) */
    *hRC = wglCreateContext(*hDC);

    wglMakeCurrent(*hDC, *hRC);
}

void DisableOpenGL (HWND hwnd, HDC hDC, HGLRC hRC)
{
    wglMakeCurrent(NULL, NULL);
    wglDeleteContext(hRC);
    ReleaseDC(hwnd, hDC);
}

void desenhaPlayer(float x, float z)
{
    float size = 0.6f;

    glPushMatrix();
    glTranslatef(x + 0.1f, 0.0f, z + 0.1f);

    glBindTexture(GL_TEXTURE_2D, texturaSteve);
    glColor3f(1.0f, 1.0f, 1.0f);

    glBegin(GL_QUADS);

    // Frente
    glTexCoord2f(0,0); glVertex3f(0, 0, size);
    glTexCoord2f(1,0); glVertex3f(size, 0, size);
    glTexCoord2f(1,1); glVertex3f(size, size, size);
    glTexCoord2f(0,1); glVertex3f(0, size, size);

    // Trás
    glTexCoord2f(0,0); glVertex3f(0, 0, 0);
    glTexCoord2f(1,0); glVertex3f(size, 0, 0);
    glTexCoord2f(1,1); glVertex3f(size, size, 0);
    glTexCoord2f(0,1); glVertex3f(0, size, 0);

    // Esquerda
    glTexCoord2f(0,0); glVertex3f(0, 0, 0);
    glTexCoord2f(1,0); glVertex3f(0, 0, size);
    glTexCoord2f(1,1); glVertex3f(0, size, size);
    glTexCoord2f(0,1); glVertex3f(0, size, 0);

    // Direita
    glTexCoord2f(0,0); glVertex3f(size, 0, 0);
    glTexCoord2f(1,0); glVertex3f(size, 0, size);
    glTexCoord2f(1,1); glVertex3f(size, size, size);
    glTexCoord2f(0,1); glVertex3f(size, size, 0);

    // Topo
    glTexCoord2f(0,0); glVertex3f(0, size, 0);
    glTexCoord2f(1,0); glVertex3f(size, size, 0);
    glTexCoord2f(1,1); glVertex3f(size, size, size);
    glTexCoord2f(0,1); glVertex3f(0, size, size);

    // Base
    glTexCoord2f(0,0); glVertex3f(0, 0, 0);
    glTexCoord2f(1,0); glVertex3f(size, 0, 0);
    glTexCoord2f(1,1); glVertex3f(size, 0, size);
    glTexCoord2f(0,1); glVertex3f(0, 0, size);

    glEnd();
    glPopMatrix();
}

void desenhaBarraTempo(int tempoRestante, int tempoLimite)
{
    float proporcao = (float)tempoRestante / tempoLimite;

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(0, 800, 0, 600);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    glDisable(GL_TEXTURE_2D);
    glDisable(GL_DEPTH_TEST);

    if (proporcao > 0.5f)
        glColor3f(0.0f, 1.0f, 0.0f);
    else if (proporcao > 0.2f)
        glColor3f(1.0f, 1.0f, 0.0f);
    else
        glColor3f(1.0f, 0.0f, 0.0f);

    float larguraMax = 300.0f;
    float larguraAtual = larguraMax * proporcao;

    float x = 20.0f;
    float y = 550.0f;

    glBegin(GL_QUADS);
        glVertex2f(x, y);
        glVertex2f(x + larguraAtual, y);
        glVertex2f(x + larguraAtual, y - 20);
        glVertex2f(x, y - 20);
    glEnd();

    // MOLDURA DO ÍCONE DA CHAVE
    glColor3f(0.2f, 0.2f, 0.2f);
    glBegin(GL_QUADS);
        glVertex2f(745, 560);
        glVertex2f(795, 560);
        glVertex2f(795, 525);
        glVertex2f(745, 525);
    glEnd();

    // ÍCONE DA CHAVE
    if (chaveColetada) {
    // Cabeça da chave
    glColor3f(1.0f, 0.85f, 0.0f);  // Dourado

    // Círculo
    glBegin(GL_TRIANGLE_FAN);
        glVertex2f(760, 542);
        for (int i = 0; i <= 360; i += 20) {
            float ang = i * 3.14159f / 180.0f;
            glVertex2f(760 + cos(ang) * 8, 542 + sin(ang) * 8);
        }
    glEnd();

    // Buraco da chave
    glColor3f(0.2f, 0.2f, 0.2f);
    glBegin(GL_TRIANGLE_FAN);
        glVertex2f(760, 542);
        for (int i = 0; i <= 360; i += 20) {
            float ang = i * 3.14159f / 180.0f;
            glVertex2f(760 + cos(ang) * 3, 542 + sin(ang) * 3);
        }
    glEnd();

    // Haste
    glColor3f(1.0f, 0.8f, 0.0f);
    glBegin(GL_QUADS);
        glVertex2f(767, 545);
        glVertex2f(782, 545);
        glVertex2f(782, 535);
        glVertex2f(767, 535);
    glEnd();

    // Dentes da chave
    glColor3f(0.9f, 0.7f, 0.0f);

    // Dente 1 (topo)
    glBegin(GL_QUADS);
        glVertex2f(780, 543);
        glVertex2f(785, 543);
        glVertex2f(785, 540);
        glVertex2f(780, 540);
    glEnd();

    // Dente 2 (meio)
    glBegin(GL_QUADS);
        glVertex2f(780, 540);
        glVertex2f(785, 540);
        glVertex2f(785, 537);
        glVertex2f(780, 537);
    glEnd();

    // Dente 3 (baixo)
    glBegin(GL_QUADS);
        glVertex2f(780, 537);
        glVertex2f(785, 537);
        glVertex2f(785, 534);
        glVertex2f(780, 534);
    glEnd();

    // Detalhe na haste
    glColor3f(0.7f, 0.5f, 0.0f);
    glBegin(GL_LINE_LOOP);
        glVertex2f(767, 542);
        glVertex2f(782, 542);
    glEnd();

} else {
    // Chave NÃO coletada - mostra contorno com cadeado

    // Fundo escuro
    glColor3f(0.2f, 0.2f, 0.2f);
    glBegin(GL_QUADS);
        glVertex2f(755, 545);
        glVertex2f(785, 545);
        glVertex2f(785, 530);
        glVertex2f(755, 530);
    glEnd();

    // Cadeado (corpo)
    glColor3f(0.7f, 0.7f, 0.7f);
    glBegin(GL_QUADS);
        glVertex2f(760, 540);
        glVertex2f(780, 540);
        glVertex2f(780, 530);
        glVertex2f(760, 530);
    glEnd();

    // Cadeado (arco)
    glColor3f(0.8f, 0.8f, 0.8f);
    glBegin(GL_LINE_STRIP);
        glVertex2f(765, 540);
        glVertex2f(765, 545);
        glVertex2f(775, 545);
        glVertex2f(775, 540);
    glEnd();

    // X vermelho
    glColor3f(1.0f, 0.0f, 0.0f);
    glBegin(GL_LINES);
        glVertex2f(768, 537);
        glVertex2f(772, 533);

        glVertex2f(772, 537);
        glVertex2f(768, 533);
    glEnd();
}

    // BORDA BRANCA DA MOLDURA
    glColor3f(1.0f, 1.0f, 1.0f);
    glBegin(GL_LINE_LOOP);
        glVertex2f(745, 560);
        glVertex2f(795, 560);
        glVertex2f(795, 525);
        glVertex2f(745, 525);
    glEnd();

    glEnable(GL_TEXTURE_2D);
    glEnable(GL_DEPTH_TEST);

    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
}

void desenhaInimigo(float x, float z)
{
    float size = 0.6f;

    glPushMatrix();

    // CENTRALIZA IGUAL AO PLAYER
    glTranslatef(x, 0.5f, z);

    glBindTexture(GL_TEXTURE_2D, texturaInimigo);
    glColor3f(1.0f, 1.0f, 1.0f);

    glBegin(GL_QUADS);

    // Frente (olhando para -z)
    glTexCoord2f(0, 0); glVertex3f(0, 0, size);
    glTexCoord2f(1, 0); glVertex3f(size, 0, size);
    glTexCoord2f(1, 1); glVertex3f(size, size, size);
    glTexCoord2f(0, 1); glVertex3f(0, size, size);

    // Trás
    glTexCoord2f(0, 0); glVertex3f(0, 0, 0);
    glTexCoord2f(1, 0); glVertex3f(size, 0, 0);
    glTexCoord2f(1, 1); glVertex3f(size, size, 0);
    glTexCoord2f(0, 1); glVertex3f(0, size, 0);

    // Esquerda
    glTexCoord2f(0, 0); glVertex3f(0, 0, 0);
    glTexCoord2f(1, 0); glVertex3f(0, 0, size);
    glTexCoord2f(1, 1); glVertex3f(0, size, size);
    glTexCoord2f(0, 1); glVertex3f(0, size, 0);

    // Direita
    glTexCoord2f(0, 0); glVertex3f(size, 0, 0);
    glTexCoord2f(1, 0); glVertex3f(size, 0, size);
    glTexCoord2f(1, 1); glVertex3f(size, size, size);
    glTexCoord2f(0, 1); glVertex3f(size, size, 0);

    // Topo
    glTexCoord2f(0, 0); glVertex3f(0, size, 0);
    glTexCoord2f(1, 0); glVertex3f(size, size, 0);
    glTexCoord2f(1, 1); glVertex3f(size, size, size);
    glTexCoord2f(0, 1); glVertex3f(0, size, size);

    // Base
    glTexCoord2f(0, 0); glVertex3f(0, 0, 0);
    glTexCoord2f(1, 0); glVertex3f(size, 0, 0);
    glTexCoord2f(1, 1); glVertex3f(size, 0, size);
    glTexCoord2f(0, 1); glVertex3f(0, 0, size);

    glEnd();
    glPopMatrix();
}

void desenhaChave(float x, float z)
{
    if (chaveColetada) return;  // Não desenha se já foi coletada

    glPushMatrix();
    glTranslatef(x, 0.3f, z);
    // Faz a chave girar
    rotacaoChave += 2.0f;
    glRotatef(rotacaoChave, 0.0f, 1.0f, 0.0f);

    glDisable(GL_TEXTURE_2D);

    // ===== CABEÇA DA CHAVE (argola) =====
    glColor3f(1.0f, 0.84f, 0.0f);  // Dourado

    // Desenha um anel usando 8 cubos pequenos
    for (int i = 0; i < 8; i++) {
        float angulo = i * 45.0f * 3.14159f / 180.0f;
        float raio = 0.15f;
        float cx = cos(angulo) * raio;
        float cz = sin(angulo) * raio;

        glPushMatrix();
        glTranslatef(cx, 0.0f, cz);
        glScalef(0.05f, 0.1f, 0.05f);

        // Desenha um cubo
        glBegin(GL_QUADS);
            // Frente
            glVertex3f(-0.5f, -0.5f, 0.5f);
            glVertex3f(0.5f, -0.5f, 0.5f);
            glVertex3f(0.5f, 0.5f, 0.5f);
            glVertex3f(-0.5f, 0.5f, 0.5f);

            // Trás
            glVertex3f(-0.5f, -0.5f, -0.5f);
            glVertex3f(-0.5f, 0.5f, -0.5f);
            glVertex3f(0.5f, 0.5f, -0.5f);
            glVertex3f(0.5f, -0.5f, -0.5f);

            // Cima
            glVertex3f(-0.5f, 0.5f, -0.5f);
            glVertex3f(-0.5f, 0.5f, 0.5f);
            glVertex3f(0.5f, 0.5f, 0.5f);
            glVertex3f(0.5f, 0.5f, -0.5f);

            // Baixo
            glVertex3f(-0.5f, -0.5f, -0.5f);
            glVertex3f(0.5f, -0.5f, -0.5f);
            glVertex3f(0.5f, -0.5f, 0.5f);
            glVertex3f(-0.5f, -0.5f, 0.5f);

            // Esquerda
            glVertex3f(-0.5f, -0.5f, -0.5f);
            glVertex3f(-0.5f, -0.5f, 0.5f);
            glVertex3f(-0.5f, 0.5f, 0.5f);
            glVertex3f(-0.5f, 0.5f, -0.5f);

            // Direita
            glVertex3f(0.5f, -0.5f, -0.5f);
            glVertex3f(0.5f, 0.5f, -0.5f);
            glVertex3f(0.5f, 0.5f, 0.5f);
            glVertex3f(0.5f, -0.5f, 0.5f);
        glEnd();
        glPopMatrix();
    }

    // ===== HASTE DA CHAVE =====
    glColor3f(1.0f, 0.84f, 0.0f);  // Dourado
    glPushMatrix();
    glTranslatef(0.2f, 0.0f, 0.0f);
    glScalef(0.4f, 0.05f, 0.05f);

    glBegin(GL_QUADS);
        // Frente
        glVertex3f(-0.5f, -0.5f, 0.5f);
        glVertex3f(0.5f, -0.5f, 0.5f);
        glVertex3f(0.5f, 0.5f, 0.5f);
        glVertex3f(-0.5f, 0.5f, 0.5f);

        // Trás
        glVertex3f(-0.5f, -0.5f, -0.5f);
        glVertex3f(-0.5f, 0.5f, -0.5f);
        glVertex3f(0.5f, 0.5f, -0.5f);
        glVertex3f(0.5f, -0.5f, -0.5f);

        // Cima
        glVertex3f(-0.5f, 0.5f, -0.5f);
        glVertex3f(-0.5f, 0.5f, 0.5f);
        glVertex3f(0.5f, 0.5f, 0.5f);
        glVertex3f(0.5f, 0.5f, -0.5f);

        // Baixo
        glVertex3f(-0.5f, -0.5f, -0.5f);
        glVertex3f(0.5f, -0.5f, -0.5f);
        glVertex3f(0.5f, -0.5f, 0.5f);
        glVertex3f(-0.5f, -0.5f, 0.5f);

        // Esquerda
        glVertex3f(-0.5f, -0.5f, -0.5f);
        glVertex3f(-0.5f, -0.5f, 0.5f);
        glVertex3f(-0.5f, 0.5f, 0.5f);
        glVertex3f(-0.5f, 0.5f, -0.5f);

        // Direita
        glVertex3f(0.5f, -0.5f, -0.5f);
        glVertex3f(0.5f, 0.5f, -0.5f);
        glVertex3f(0.5f, 0.5f, 0.5f);
        glVertex3f(0.5f, -0.5f, 0.5f);
    glEnd();
    glPopMatrix();

    // ===== DENTES DA CHAVE =====
    glColor3f(1.0f, 0.5f, 0.0f);

    // Dente de cima
    glPushMatrix();
    glTranslatef(0.4f, 0.05f, 0.0f);
    glScalef(0.1f, 0.1f, 0.05f);
    glBegin(GL_QUADS);
        glVertex3f(-0.5f, -0.5f, 0.5f);
        glVertex3f(0.5f, -0.5f, 0.5f);
        glVertex3f(0.5f, 0.5f, 0.5f);
        glVertex3f(-0.5f, 0.5f, 0.5f);

        glVertex3f(-0.5f, -0.5f, -0.5f);
        glVertex3f(-0.5f, 0.5f, -0.5f);
        glVertex3f(0.5f, 0.5f, -0.5f);
        glVertex3f(0.5f, -0.5f, -0.5f);
    glEnd();
    glPopMatrix();

    // Dente de baixo
    glPushMatrix();
    glTranslatef(0.4f, -0.05f, 0.0f);
    glScalef(0.1f, 0.1f, 0.05f);
    glBegin(GL_QUADS);
        glVertex3f(-0.5f, -0.5f, 0.5f);
        glVertex3f(0.5f, -0.5f, 0.5f);
        glVertex3f(0.5f, 0.5f, 0.5f);
        glVertex3f(-0.5f, 0.5f, 0.5f);

        glVertex3f(-0.5f, -0.5f, -0.5f);
        glVertex3f(-0.5f, 0.5f, -0.5f);
        glVertex3f(0.5f, 0.5f, -0.5f);
        glVertex3f(0.5f, -0.5f, -0.5f);
    glEnd();
    glPopMatrix();

    glEnable(GL_TEXTURE_2D);
    glPopMatrix();
}

void desenhaMenu() {
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(0, 800, 0, 600);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    glDisable(GL_TEXTURE_2D);
    glDisable(GL_DEPTH_TEST);

    // Fundo preto
    glColor3f(0.0f, 0.0f, 0.0f);
    glBegin(GL_QUADS);
        glVertex2f(0, 0);
        glVertex2f(800, 0);
        glVertex2f(800, 600);
        glVertex2f(0, 600);
    glEnd();

    HDC hDC = wglGetCurrentDC();

    HFONT hTitulo = CreateFontA(48, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Arial");
    SelectObject(hDC, hTitulo);
    SetTextColor(hDC, RGB(255, 255, 255));
    SetBkMode(hDC, TRANSPARENT);

    TextOutA(hDC, 160, 50, "LABIRINTO MINECRAFT", 20);

    HFONT hOpcao = CreateFontA(36, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Arial");
    SelectObject(hDC, hOpcao);

    SetTextColor(hDC, RGB(0, 255, 0));
    TextOutA(hDC, 265, 150, "1 - MODO NORMAL", 15);

    SetTextColor(hDC, RGB(255, 0, 0));
    TextOutA(hDC, 283, 250, "2 - MODO DARK", 13);

    HFONT hInstrucao = CreateFontA(24, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Arial");
    SelectObject(hDC, hInstrucao);
    SetTextColor(hDC, RGB(255, 255, 0));

    TextOutA(hDC, 250, 400, "Pressione 1 ou 2 para começar", 30);

    SelectObject(hDC, hTitulo);
    DeleteObject(hTitulo);
    DeleteObject(hOpcao);
    DeleteObject(hInstrucao);

    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_TEXTURE_2D);
}

void configurarLuzModoDark() {
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);

    GLfloat luzPosicao[] = { playerX, 0.5f, playerZ, 1.0f };
    glLightfv(GL_LIGHT0, GL_POSITION, luzPosicao);

    GLfloat luzAmbiente[] = { 0.3f, 0.3f, 0.3f, 1.0f };
    GLfloat luzDifusa[] = { 1.2f, 1.1f, 0.8f, 1.0f };
    GLfloat luzEspecular[] = { 0.5f, 0.5f, 0.5f, 1.0f };

    glLightfv(GL_LIGHT0, GL_AMBIENT, luzAmbiente);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, luzDifusa);
    glLightfv(GL_LIGHT0, GL_SPECULAR, luzEspecular);

    glLightf(GL_LIGHT0, GL_SPOT_CUTOFF, 180.0f);

    glLightf(GL_LIGHT0, GL_CONSTANT_ATTENUATION, 0.7f);
    glLightf(GL_LIGHT0, GL_LINEAR_ATTENUATION, 0.05f);
    glLightf(GL_LIGHT0, GL_QUADRATIC_ATTENUATION, 0.01f);

    glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER, GL_TRUE);

    GLfloat materialAmbiente[] = { 0.6f, 0.6f, 0.6f, 1.0f };
    GLfloat materialDifusa[] = { 0.8f, 0.8f, 0.8f, 1.0f };
    glMaterialfv(GL_FRONT, GL_AMBIENT, materialAmbiente);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, materialDifusa);
}

