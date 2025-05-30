#include "Render.h"
#include <Windows.h>
#include <GL\GL.h>
#include <GL\GLU.h>
#include <iostream>
#include <iomanip>
#include <sstream>
#include "GUItextRectangle.h"

#ifdef _DEBUG
#include <Debugapi.h> 
struct debug_print
{
    template<class C>
    debug_print& operator<<(const C& a)
    {
        OutputDebugStringA((std::stringstream() << a).str().c_str());
        return *this;
    }
} debout;
#else
struct debug_print
{
    template<class C>
    debug_print& operator<<(const C& a)
    {
        return *this;
    }
} debout;
#endif

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "MyOGL.h"
extern OpenGL gl;
#include "Light.h"
Light light;
#include "Camera.h"
Camera camera;


bool texturing = true;
bool lightning = true;
bool alpha = false;


void switchModes(OpenGL* sender, KeyEventArg arg)
{
    auto key = LOWORD(MapVirtualKeyA(arg.key, MAPVK_VK_TO_CHAR));

    switch (key)
    {
    case 'L':
        lightning = !lightning;
        break;
    case 'T':
        texturing = !texturing;
        break;
    case 'A':
        alpha = !alpha;
        break;
    }
}
GuiTextRectangle text;

GLuint texId;
void initRender()
{
    glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
    glGenTextures(1, &texId);
    glBindTexture(GL_TEXTURE_2D, texId);

    int x, y, n;
    unsigned char* data = stbi_load("texture.png", &x, &y, &n, 4);
    unsigned char* _tmp = new unsigned char[x * 4];
    for (int i = 0; i < y / 2; ++i)
    {
        std::memcpy(_tmp, data + i * x * 4, x * 4);
        std::memcpy(data + i * x * 4, data + (y - 1 - i) * x * 4, x * 4);
        std::memcpy(data + (y - 1 - i) * x * 4, _tmp, x * 4);
    }
    delete[] _tmp;

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, x, y, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);

    stbi_image_free(data);

    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    camera.caclulateCameraPos();

    gl.WheelEvent.reaction(&camera, &Camera::Zoom);
    gl.MouseMovieEvent.reaction(&camera, &Camera::MouseMovie);
    gl.MouseLeaveEvent.reaction(&camera, &Camera::MouseLeave);
    gl.MouseLdownEvent.reaction(&camera, &Camera::MouseStartDrag);
    gl.MouseLupEvent.reaction(&camera, &Camera::MouseStopDrag);
    gl.MouseMovieEvent.reaction(&light, &Light::MoveLight);
    gl.KeyDownEvent.reaction(&light, &Light::StartDrug);
    gl.KeyUpEvent.reaction(&light, &Light::StopDrug);
    gl.KeyDownEvent.reaction(switchModes);
    text.setSize(512, 180);

    camera.setPosition(2, 1.5, 1.5);
}


const int height = 5;
const double M_PI = std::acos(-1);

double A[]{ -5, 2, 0 };
double B[]{ -2, 8, 0 };
double C[]{ 2, 3, 0 };
double D[]{ 0, 0, 0 };
double E[]{ 6, 5, 0 };
double F[]{ 2, 0, 0 };
double G[]{ 5, -5, 0 };
double H[]{ -3, -7, 0 };

double A_top[]{ -5, 2, height };
double B_top[]{ -2, 8, height };
double C_top[]{ 2, 3, height };
double D_top[]{ 0, 0, height };
double E_top[]{ 6, 5, height };
double F_top[]{ 2, 0, height };
double G_top[]{ 5, -5, height };
double H_top[]{ -3, -7, height };

void setNormal(const double* p1, const double* p2, const double* p3) {
    double u[3] = { p2[0] - p1[0], p2[1] - p1[1], p2[2] - p1[2] };
    double v[3] = { p3[0] - p1[0], p3[1] - p1[1], p3[2] - p1[2] };
    double n[3] = {
        u[1] * v[2] - u[2] * v[1],
        u[2] * v[0] - u[0] * v[2],
        u[0] * v[1] - u[1] * v[0]
    };
    double length = std::sqrt(n[0] * n[0] + n[1] * n[1] + n[2] * n[2]);
    if (length > 0)
        glNormal3d(n[0] / length, n[1] / length, n[2] / length);
}

void DrawQuadNormal(const double* p1, const double* p2, const double* p3, const double* p4, double length = 5.0) {
    // Вычисляем нормаль к плоскости четырёхугольника
    double u[3] = { p2[0] - p1[0], p2[1] - p1[1], p2[2] - p1[2] };
    double v[3] = { p3[0] - p1[0], p3[1] - p1[1], p3[2] - p1[2] };
    double n[3] = {
        u[1] * v[2] - u[2] * v[1],
        u[2] * v[0] - u[0] * v[2],
        u[0] * v[1] - u[1] * v[0]
    };

    // Нормализуем вектор
    double norm_length = std::sqrt(n[0] * n[0] + n[1] * n[1] + n[2] * n[2]);
    if (norm_length > 0) {
        n[0] = n[0] / norm_length * length;
        n[1] = n[1] / norm_length * length;
        n[2] = n[2] / norm_length * length;

        // Вычисляем центр четырёхугольника
        double center[3] = {
            (p1[0] + p2[0] + p3[0] + p4[0]) / 4.0,
            (p1[1] + p2[1] + p3[1] + p4[1]) / 4.0,
            (p1[2] + p2[2] + p3[2] + p4[2]) / 4.0
        };

        // Сохраняем текущее состояние освещения
        GLboolean lightingEnabled;
        glGetBooleanv(GL_LIGHTING, &lightingEnabled);

        // Рисуем нормаль (красная линия из центра)
        glDisable(GL_LIGHTING);
        glBegin(GL_LINES);
        glColor3d(1, 0, 0); // Красный цвет
        glVertex3dv(center);
        glVertex3d(center[0] + n[0], center[1] + n[1], center[2] + n[2]);

        // Добавляем небольшой маркер направления (две короткие линии)
        glVertex3d(center[0] + n[0], center[1] + n[1], center[2] + n[2]);
        glVertex3d(center[0] + n[0] * 0.9 + length * 0.1, center[1] + n[1] * 0.9, center[2] + n[2] * 0.9);

        glVertex3d(center[0] + n[0], center[1] + n[1], center[2] + n[2]);
        glVertex3d(center[0] + n[0] * 0.9, center[1] + n[1] * 0.9 + length * 0.1, center[2] + n[2] * 0.9);
        glEnd();

        // Восстанавливаем состояние освещения
        if (lightingEnabled) glEnable(GL_LIGHTING);
    }
}

void ShapeABCD() {
    // base
    glBegin(GL_QUADS);
    glColor3d(1, 0, 0);
    setNormal(A, B, C);
    glVertex3dv(A); glVertex3dv(B); glVertex3dv(C); glVertex3dv(D);
    glEnd();
    DrawQuadNormal(A, B, C, D);

    // top
    glBegin(GL_QUADS);
    glColor3d(0.5, 0, 0.5);
    setNormal(A_top, C_top, B_top);
    glVertex3dv(A_top); glVertex3dv(B_top); glVertex3dv(C_top); glVertex3dv(D_top);
    glEnd();
    DrawQuadNormal(A_top, C_top, B_top, D_top);

    // wall AB
    glBegin(GL_QUADS);
    glColor3d(0.8, 0.8, 0.8);
    setNormal(A, A_top, B_top);
    glVertex3dv(A); glVertex3dv(A_top); glVertex3dv(B_top); glVertex3dv(B);
    glEnd();
    DrawQuadNormal(A, A_top, B_top, B);
    //DrawQuadNormal(A_top, A, B, B_top);

    // wall BC
    glBegin(GL_QUADS);
    glColor3d(0.4, 0.8, 0.4);
    setNormal(B, B_top, C_top);
    glVertex3dv(B); glVertex3dv(B_top); glVertex3dv(C_top); glVertex3dv(C);
    glEnd();
    DrawQuadNormal(B, B_top, C_top, C);
    //DrawQuadNormal(B_top, B, C, C_top);

    // wall AD
    glBegin(GL_QUADS);
    glColor3d(0.8, 0.4, 0.8);
    setNormal(A, D_top, A_top);
    glVertex3dv(A); glVertex3dv(A_top); glVertex3dv(D_top); glVertex3dv(D);
    glEnd();
   // DrawQuadNormal(A, A_top, D_top, D);
    DrawQuadNormal(A_top, A, D, D_top);

}

void ShapeCDEF() {
    // base
    glBegin(GL_QUADS);
    glColor3d(0, 1, 0);
    setNormal(C, F, D);
    glVertex3dv(C); glVertex3dv(D); glVertex3dv(F); glVertex3dv(E);
    glEnd();
    DrawQuadNormal(C, E, D, F);

    // top
    glBegin(GL_QUADS);
    glColor3d(0.5, 0.5, 0);
    setNormal(C_top, D_top, F_top);
    glVertex3dv(C_top); glVertex3dv(D_top); glVertex3dv(F_top); glVertex3dv(E_top);
    glEnd();
    DrawQuadNormal(C_top, D_top, F_top, E_top);

    // wall CE
    glBegin(GL_QUADS);
    glColor3d(0.5, 0.9, 0.5);
    setNormal(C, C_top, E_top);
    glVertex3dv(C); glVertex3dv(C_top); glVertex3dv(E_top); glVertex3dv(E);
    glEnd();
    DrawQuadNormal(C, C_top, E_top, E);

    // wall EF
    glBegin(GL_QUADS);
    glColor3d(1, 0.1, 1);
    setNormal(E, E_top, F_top);
    glVertex3dv(E); glVertex3dv(E_top); glVertex3dv(F_top); glVertex3dv(F);
    glEnd();
    DrawQuadNormal(E, E_top, F_top, F);
}

void ShapeDFGH() {
    // base
    glBegin(GL_QUADS);
    glColor3d(0, 0, 1);
    setNormal(D, F, G);
    glVertex3dv(D); glVertex3dv(F); glVertex3dv(G); glVertex3dv(H);
    glEnd();
    DrawQuadNormal(D, F, G, H);

    // top
    glBegin(GL_QUADS);
    glColor3d(0, 0.5, 0.5);
    setNormal(C_top, F_top, G_top);
    glVertex3dv(D_top); glVertex3dv(F_top); glVertex3dv(G_top); glVertex3dv(H_top);
    glEnd();
    DrawQuadNormal(D_top, G_top, F_top, H_top);

    // wall FG
    glBegin(GL_QUADS);
    glColor3d(0.4, 0.8, 0.6);
    setNormal(F, F_top, G_top);
    glVertex3dv(F); glVertex3dv(F_top); glVertex3dv(G_top); glVertex3dv(G);
    glEnd();
    DrawQuadNormal(F, F_top, G_top, G);
    //DrawQuadNormal(F_top, F, G, G_top);

    // wall GH
    glBegin(GL_QUADS);
    glColor3d(0.0, 1.0, 0.5);
    setNormal(G, G_top, H_top);
    glVertex3dv(G); glVertex3dv(G_top); glVertex3dv(H_top); glVertex3dv(H);
    glEnd();
    DrawQuadNormal(G, G_top, H_top, H);
    //DrawQuadNormal(G_top, G, H, H_top);

    // wall DH
    glBegin(GL_QUADS);
    glColor3d(0.6, 0.0, 0.6);
    setNormal(D, H_top, D_top);
    glVertex3dv(D); glVertex3dv(D_top); glVertex3dv(H_top); glVertex3dv(H);
    glEnd();
   // DrawQuadNormal(D, D_top, H_top, H);
    DrawQuadNormal(D_top, D, H, H_top);
}

void RoundedSection() {
    const int segments = 20;
    double vecX = A[0] - B[0];
    double vecY = A[1] - B[1];
    double length = sqrt(vecX * vecX + vecY * vecY);
    double radius = length / 2.0;

    double centerX = (B[0] + A[0]) / 2.0;
    double centerY = (B[1] + A[1]) / 2.0;
    double centerZ_bottom = B[2];
    double centerZ_top = B_top[2];

    double startAngle = atan2(A[1] - centerY, A[0] - centerX) + M_PI;

    glBegin(GL_QUAD_STRIP);
    glColor3d(0.8, 0.6, 0.4);
    for (int i = 0; i <= segments; i++) {
        double angle = startAngle + M_PI * i / segments;
        double x = centerX + radius * cos(angle);
        double y = centerY + radius * sin(angle);
        glVertex3d(x, y, centerZ_bottom);
        glVertex3d(x, y, centerZ_top);
    }
    glEnd();
    DrawQuadNormal(A, B, B_top, A_top);


    // Округлый низ
    glBegin(GL_TRIANGLE_FAN);
    glColor3d(0.3, 0.8, 0.9);
    glNormal3d(0, 0, -1);
    glVertex3d(centerX, centerY, centerZ_bottom);
    for (int i = 0; i <= segments; i++) {
        double angle = startAngle + M_PI * i / segments;
        double x = centerX + radius * cos(angle);
        double y = centerY + radius * sin(angle);
        glVertex3d(x, y, centerZ_bottom);
    }
    glEnd();

    // Округлый верх
    glBegin(GL_TRIANGLE_FAN);
    glColor3d(1, 0.4, 0.1);
    glNormal3d(0, 0, 1);
    glVertex3d(centerX, centerY, centerZ_top);
    for (int i = 0; i <= segments; i++) {
        double angle = startAngle + M_PI * i / segments;
        double x = centerX + radius * cos(angle);
        double y = centerY + radius * sin(angle);
        glVertex3d(x, y, centerZ_top);
    }
    glEnd();
}

void shape() {
    ShapeABCD();
    ShapeCDEF();
    ShapeDFGH();
    RoundedSection();
}

void Render(double delta_time)
{
    glEnable(GL_DEPTH_TEST);


    if (gl.isKeyPressed('F'))
    {
        light.SetPosition(camera.x(), camera.y(), camera.z());
    }
    camera.SetUpCamera();
    light.SetUpLight();

    gl.DrawAxes();

    glDisable(GL_LIGHTING);
    glDisable(GL_TEXTURE_2D);
    glDisable(GL_BLEND);


    if (lightning)
        glEnable(GL_LIGHTING);
    if (texturing)
    {
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, texId);
    }

    if (alpha)
    {
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    }

    float  amb[] = { 0.2, 0.2, 0.1, 1. };
    float dif[] = { 0.4, 0.65, 0.5, 1. };
    float spec[] = { 0.9, 0.8, 0.3, 1. };
    float sh = 0.2f * 256;

    glMaterialfv(GL_FRONT, GL_AMBIENT, amb);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, dif);
    glMaterialfv(GL_FRONT, GL_SPECULAR, spec);
    glMaterialf(GL_FRONT, GL_SHININESS, sh);
    
    glShadeModel(GL_SMOOTH);
    shape();

    light.DrawLightGizmo();
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();

    glOrtho(0, gl.getWidth() - 1, 0, gl.getHeight() - 1, 0, 1);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    std::wstringstream ss;
    ss << std::fixed << std::setprecision(3);
    ss << "T - " << (texturing ? L"[вкл]выкл  " : L" вкл[выкл] ") << L"текстур" << std::endl;
    ss << "L - " << (lightning ? L"[вкл]выкл  " : L" вкл[выкл] ") << L"освещение" << std::endl;
    ss << "A - " << (alpha ? L"[вкл]выкл  " : L" вкл[выкл] ") << L"альфа-наложение" << std::endl;
    ss << L"F - Свет из камеры" << std::endl;
    ss << L"G - двигать свет по горизонтали" << std::endl;
    ss << L"H - двигать свет по вертикали" << std::endl;
    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glEnable(GL_DEPTH_TEST);
}
