//========================================================================
// OpenGL - Motor 3D Base: Primitivas Geométricas para Scene Graph
// + Cámara (con permisos) + GestorCamaras + Órbitas + Espirales
//========================================================================

#define GLAD_GL_IMPLEMENTATION
#include <glad/gl.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <iostream>
#include <vector>
#include <cmath>
#include <map>
#include <cstdlib>
#include <ctime>
#include <algorithm>

using namespace std;

const float PI = 3.14159265359f;

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow *window);

const unsigned int SCR_WIDTH = 1000;
const unsigned int SCR_HEIGHT = 800;

// Proporción actual de la ventana (ancho/alto). La usa la cámara.
float g_aspecto = (float)SCR_WIDTH / (float)SCR_HEIGHT;

// =========================================================================
// SHADERS
// vista y proyeccion valen identidad por defecto: sin cámara todo funciona.
// =========================================================================
const char *vertexShaderSource = "#version 330 core\n"
    "layout (location = 0) in vec3 aPos;\n"
    "uniform mat4 transform = mat4(1.0);\n"
    "uniform mat4 vista = mat4(1.0);\n"
    "uniform mat4 proyeccion = mat4(1.0);\n"
    "void main()\n"
    "{\n"
    "   gl_Position = proyeccion * vista * transform * vec4(aPos, 1.0);\n"
    "}\0";

const char *fragmentShaderSource = "#version 330 core\n"
    "out vec4 FragColor;\n"
    "uniform vec4 ourColor = vec4(1.0, 1.0, 1.0, 1.0);\n"
    "void main()\n"
    "{\n"
    "   FragColor = ourColor;\n"
    "}\n\0";

// =========================================================================
// 1. GESTOR DE COLORES
// =========================================================================
enum class NombreColor {
    Rojo, Verde, Azul, Amarillo, Celeste, Marron, Morado, Cyan,
    Naranja, Rosa, Blanco, Negro, Gris, Beige, Fucsia, VerdeFluorescente, Violeta
};

class GestorColor {
private:
    unsigned int shaderID;
    int colorLoc;
public:
    GestorColor(unsigned int shaderProgram) {
        shaderID = shaderProgram;
        colorLoc = glGetUniformLocation(shaderID, "ourColor");
    }
    void establecer(NombreColor color, float alpha = 1.0f) {
        glUseProgram(shaderID);
        switch (color) {
            case NombreColor::Rojo:               glUniform4f(colorLoc, 1.0f, 0.0f, 0.0f, alpha); break;
            case NombreColor::Verde:              glUniform4f(colorLoc, 0.0f, 1.0f, 0.0f, alpha); break;
            case NombreColor::Azul:               glUniform4f(colorLoc, 0.0f, 0.0f, 1.0f, alpha); break;
            case NombreColor::Amarillo:           glUniform4f(colorLoc, 1.0f, 1.0f, 0.0f, alpha); break;
            case NombreColor::Celeste:            glUniform4f(colorLoc, 0.53f, 0.81f, 0.98f, alpha); break;
            case NombreColor::Marron:             glUniform4f(colorLoc, 0.55f, 0.27f, 0.07f, alpha); break;
            case NombreColor::Morado:             glUniform4f(colorLoc, 0.5f, 0.0f, 0.5f, alpha); break;
            case NombreColor::Cyan:               glUniform4f(colorLoc, 0.0f, 1.0f, 1.0f, alpha); break;
            case NombreColor::Naranja:            glUniform4f(colorLoc, 1.0f, 0.5f, 0.0f, alpha); break;
            case NombreColor::Rosa:               glUniform4f(colorLoc, 1.0f, 0.75f, 0.8f, alpha); break;
            case NombreColor::Blanco:             glUniform4f(colorLoc, 1.0f, 1.0f, 1.0f, alpha); break;
            case NombreColor::Negro:              glUniform4f(colorLoc, 0.0f, 0.0f, 0.0f, alpha); break;
            case NombreColor::Gris:               glUniform4f(colorLoc, 0.5f, 0.5f, 0.5f, alpha); break;
            case NombreColor::Beige:              glUniform4f(colorLoc, 0.96f, 0.96f, 0.86f, alpha); break;
            case NombreColor::Fucsia:             glUniform4f(colorLoc, 1.0f, 0.0f, 1.0f, alpha); break;
            case NombreColor::VerdeFluorescente:  glUniform4f(colorLoc, 0.2f, 1.0f, 0.2f, alpha); break;
            case NombreColor::Violeta:            glUniform4f(colorLoc, 0.54f, 0.17f, 0.89f, alpha); break;
        }
    }
};

// =========================================================================
// 2. TRANSFORMADOR 3D (ACUMULATIVO)
// =========================================================================
class Transformador3D {
private:
    unsigned int shaderID;
    int transformLoc;
    float matrizActual[16];

    void acumular(const float* nuevaMatriz) {
        float temp[16];
        for (int i = 0; i < 4; i++) {
            for (int j = 0; j < 4; j++) {
                temp[i * 4 + j] = 0.0f;
                for (int k = 0; k < 4; k++) {
                    temp[i * 4 + j] += matrizActual[i * 4 + k] * nuevaMatriz[k * 4 + j];
                }
            }
        }
        for (int i = 0; i < 16; i++) matrizActual[i] = temp[i];
    }

public:
    Transformador3D(unsigned int shaderProgram) {
        shaderID = shaderProgram;
        transformLoc = glGetUniformLocation(shaderID, "transform");
        reiniciar();
    }

    void reiniciar() {
        float identidad[16] = {
            1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
            0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f
        };
        for (int i = 0; i < 16; i++) matrizActual[i] = identidad[i];
    }

    void trasladar(float tx, float ty, float tz) {
        float mat[16] = { 1.f,0.f,0.f,tx,  0.f,1.f,0.f,ty,  0.f,0.f,1.f,tz,  0.f,0.f,0.f,1.f };
        acumular(mat);
    }
    void escalar(float sx, float sy, float sz) {
        float mat[16] = { sx,0.f,0.f,0.f,  0.f,sy,0.f,0.f,  0.f,0.f,sz,0.f,  0.f,0.f,0.f,1.f };
        acumular(mat);
    }
    void rotarX(float g) {
        float r = g * PI / 180.0f, c = cosf(r), s = sinf(r);
        float mat[16] = { 1.f,0.f,0.f,0.f, 0.f,c,-s,0.f, 0.f,s,c,0.f, 0.f,0.f,0.f,1.f };
        acumular(mat);
    }
    void rotarY(float g) {
        float r = g * PI / 180.0f, c = cosf(r), s = sinf(r);
        float mat[16] = { c,0.f,s,0.f, 0.f,1.f,0.f,0.f, -s,0.f,c,0.f, 0.f,0.f,0.f,1.f };
        acumular(mat);
    }
    void rotarZ(float g) {
        float r = g * PI / 180.0f, c = cosf(r), s = sinf(r);
        float mat[16] = { c,-s,0.f,0.f, s,c,0.f,0.f, 0.f,0.f,1.f,0.f, 0.f,0.f,0.f,1.f };
        acumular(mat);
    }

    // ORBITAR (versión simple): gira alrededor de (cx,cy,cz) a "radio".
    // Dos llamadas seguidas sin reiniciar() quedan anidadas (luna del planeta).
    void orbitar(float cx, float cy, float cz, float radio, float angulo, char eje = 'Y') {
        trasladar(cx, cy, cz);
        switch (eje) {
            case 'X': case 'x': rotarX(angulo); break;
            case 'Y': case 'y': rotarY(angulo); break;
            case 'Z': case 'z': rotarZ(angulo); break;
            default:            rotarY(angulo); break;
        }
        trasladar(radio, 0.0f, 0.0f);
    }

    void aplicar() {
        glUseProgram(shaderID);
        glUniformMatrix4fv(transformLoc, 1, GL_TRUE, matrizActual);
    }
};

// =========================================================================
// 3. ANIMADOR DE RUTAS (LERP)
// =========================================================================
struct Punto3D { float x, y, z; };
class AnimadorRuta {
private:
    std::vector<Punto3D> puntos;
public:
    void agregarPunto(float x, float y, float z) { puntos.push_back({x, y, z}); }
    void hacerIdaYVuelta() {
        int n = puntos.size();
        for (int i = n - 2; i >= 0; i--) puntos.push_back(puntos[i]);
    }
    Punto3D obtenerPosicionActual(float tiempoBase, float velocidad) {
        if (puntos.empty()) return {0.0f, 0.0f, 0.0f};
        if (puntos.size() == 1) return puntos[0];

        float tiempoMod = tiempoBase * velocidad;
        int tramo = (int)tiempoMod % (puntos.size() - 1);
        float pct = tiempoMod - (int)tiempoMod;

        Punto3D A = puntos[tramo];
        Punto3D B = puntos[tramo + 1];
        return { A.x + (B.x - A.x)*pct, A.y + (B.y - A.y)*pct, A.z + (B.z - A.z)*pct };
    }
};

// =========================================================================
// 4. CLASE BASE FIGURA Y PRIMITIVAS
// =========================================================================
class Figura {
protected:
    // Malla de relleno (triángulos)
    unsigned int VAO, VBO;
    int cantidadVertices;

    // Malla de líneas (aristas reales, sin diagonales de triangulación)
    unsigned int VAO_lineas = 0, VBO_lineas = 0;
    int cantidadVerticesLineas = 0;
    bool tieneLineas = false;

    // Borde de la figura 2D, en orden (para convertirPizza)
    std::vector<Punto3D> bordePoligono;

    // Rango de vértices de cada "cara" de una figura 3D
    std::vector<int> caraInicio;
    std::vector<int> caraCantidadVertices;

    // Color "base" de la figura (lo usa cambiarColorCaras para las caras no indicadas)
    NombreColor colorBase = NombreColor::Blanco;

    // Posición de la figura en el mundo (la actualizan orbitar*/espiral*
    // si les pasas la figura, o tú con establecerPosicion()).
    Punto3D posicion = {0.0f, 0.0f, 0.0f};

    void configurarMalla(const std::vector<float>& vertices) {
        cantidadVertices = vertices.size() / 3;
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);

        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
    }

    void configurarLineas(const std::vector<float>& verticesLineas) {
        cantidadVerticesLineas = verticesLineas.size() / 3;
        glGenVertexArrays(1, &VAO_lineas);
        glGenBuffers(1, &VBO_lineas);

        glBindVertexArray(VAO_lineas);
        glBindBuffer(GL_ARRAY_BUFFER, VBO_lineas);
        glBufferData(GL_ARRAY_BUFFER, verticesLineas.size() * sizeof(float), verticesLineas.data(), GL_STATIC_DRAW);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
        tieneLineas = true;
    }

    void configurarBordePizza(const std::vector<Punto3D>& borde) {
        bordePoligono = borde;
    }

    void configurarCaras(const std::vector<int>& verticesPorCara) {
        int acumulado = 0;
        caraInicio.clear();
        caraCantidadVertices.clear();
        for (int cantidad : verticesPorCara) {
            caraInicio.push_back(acumulado);
            caraCantidadVertices.push_back(cantidad);
            acumulado += cantidad;
        }
    }

    // Dónde el rayo desde "centro" en dirección "angulo" choca con el borde.
    Punto3D interseccionConBorde(Punto3D centro, float angulo) const {
        float dx = cosf(angulo), dy = sinf(angulo);
        float mejorU = -1.0f;
        Punto3D resultado = centro;
        for (size_t i = 0; i < bordePoligono.size(); i++) {
            Punto3D p1 = bordePoligono[i];
            Punto3D p2 = bordePoligono[(i + 1) % bordePoligono.size()];
            float x1 = p1.x, y1 = p1.y, x2 = p2.x, y2 = p2.y;
            float x3 = centro.x, y3 = centro.y, x4 = centro.x + dx, y4 = centro.y + dy;
            float denom = (x1 - x2) * (y3 - y4) - (y1 - y2) * (x3 - x4);
            if (fabs(denom) < 1e-6f) continue;
            float t = ((x1 - x3) * (y3 - y4) - (y1 - y3) * (x3 - x4)) / denom;
            float u = -((x1 - x2) * (y1 - y3) - (y1 - y2) * (x1 - x3)) / denom;
            if (t >= 0.0f && t <= 1.0f && u > mejorU) {
                mejorU = u;
                resultado = { x3 + u * dx, y3 + u * dy, 0.0f };
            }
        }
        return resultado;
    }

public:
    virtual ~Figura() {
        glDeleteVertexArrays(1, &VAO);
        glDeleteBuffers(1, &VBO);
        if (tieneLineas) {
            glDeleteVertexArrays(1, &VAO_lineas);
            glDeleteBuffers(1, &VBO_lineas);
        }
    }

    void establecerPosicion(float x, float y, float z) { posicion = {x, y, z}; }
    void establecerPosicion(Punto3D p) { posicion = p; }
    Punto3D obtenerPosicion() const { return posicion; }

    // Solo relleno (usa el color activo)
    virtual void dibujar() {
        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, cantidadVertices);
        glBindVertexArray(0);
    }

    // Solo aristas reales (usa el color activo)
    virtual void dibujarLineas() {
        if (!tieneLineas) return;
        glBindVertexArray(VAO_lineas);
        glDrawArrays(GL_LINES, 0, cantidadVerticesLineas);
        glBindVertexArray(0);
    }

    // Relleno + líneas con el mismo color activo
    virtual void dibujarRellenoYLineas() {
        glEnable(GL_POLYGON_OFFSET_FILL);
        glPolygonOffset(1.0f, 1.0f);
        dibujar();
        glDisable(GL_POLYGON_OFFSET_FILL);
        dibujarLineas();
    }

    // Relleno de un color + líneas de otro
    virtual void dibujarRellenoYLineas(GestorColor& gestor, NombreColor colorRelleno, NombreColor colorLineas) {
        colorBase = colorRelleno;
        gestor.establecer(colorRelleno);
        glEnable(GL_POLYGON_OFFSET_FILL);
        glPolygonOffset(1.0f, 1.0f);
        dibujar();
        glDisable(GL_POLYGON_OFFSET_FILL);
        gestor.establecer(colorLineas);
        dibujarLineas();
    }

    // Relleno con caras personalizadas + líneas, en una sola llamada
    virtual void dibujarRellenoYLineas(GestorColor& gestor, NombreColor colorBaseFigura,
                                        const std::map<int, NombreColor>& coloresPorCara,
                                        NombreColor colorLineas) {
        colorBase = colorBaseFigura;
        glEnable(GL_POLYGON_OFFSET_FILL);
        glPolygonOffset(1.0f, 1.0f);
        cambiarColorCaras(gestor, coloresPorCara);
        glDisable(GL_POLYGON_OFFSET_FILL);
        gestor.establecer(colorLineas);
        dibujarLineas();
    }

    // CONVERTIR PIZZA (solo 2D): líneas desde "centro" hasta el borde real.
    // Se usa DESPUÉS de dibujar, con el color puesto antes.
    void convertirPizza(int divisiones, Punto3D centro = {0.0f, 0.0f, 0.0f}) {
        if (bordePoligono.empty() || divisiones < 2) return;

        std::vector<float> lineas;
        for (int i = 0; i < divisiones; i++) {
            float angulo = 2.0f * PI * i / divisiones;
            Punto3D borde = interseccionConBorde(centro, angulo);
            lineas.push_back(centro.x); lineas.push_back(centro.y); lineas.push_back(centro.z);
            lineas.push_back(borde.x);  lineas.push_back(borde.y);  lineas.push_back(borde.z);
        }

        unsigned int vaoTmp, vboTmp;
        glGenVertexArrays(1, &vaoTmp);
        glGenBuffers(1, &vboTmp);
        glBindVertexArray(vaoTmp);
        glBindBuffer(GL_ARRAY_BUFFER, vboTmp);
        glBufferData(GL_ARRAY_BUFFER, lineas.size() * sizeof(float), lineas.data(), GL_DYNAMIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);

        glDrawArrays(GL_LINES, 0, (int)(lineas.size() / 3));

        glBindVertexArray(0);
        glDeleteBuffers(1, &vboTmp);
        glDeleteVertexArrays(1, &vaoTmp);
    }

    int cantidadCaras() const { return (int)caraInicio.size(); }

    void establecerColorBase(NombreColor color) { colorBase = color; }
    NombreColor obtenerColorBase() const { return colorBase; }

    // Cada cara de un color al azar (se re-sortea en cada llamada;
    // usa srand(n) antes para congelar los colores).
    void pintarCarasAleatorio(GestorColor& gestor,
                               const std::vector<NombreColor>& paleta = {
                                   NombreColor::Rojo, NombreColor::Verde, NombreColor::Azul,
                                   NombreColor::Amarillo, NombreColor::Naranja, NombreColor::Morado,
                                   NombreColor::Cyan, NombreColor::Rosa, NombreColor::Fucsia,
                                   NombreColor::Celeste, NombreColor::VerdeFluorescente, NombreColor::Violeta }) {
        if (caraInicio.empty() || paleta.empty()) { dibujar(); return; }
        glBindVertexArray(VAO);
        for (size_t i = 0; i < caraInicio.size(); i++) {
            gestor.establecer(paleta[rand() % paleta.size()]);
            glDrawArrays(GL_TRIANGLES, caraInicio[i], caraCantidadVertices[i]);
        }
        glBindVertexArray(0);
    }

    // Como cambiarColorCaras pero pasando el color base en la llamada
    void pintarCarasEspecificas(GestorColor& gestor,
                                 const std::map<int, NombreColor>& coloresPorCara,
                                 NombreColor colorBaseParametro) {
        colorBase = colorBaseParametro;
        cambiarColorCaras(gestor, coloresPorCara);
    }

    // Elige el color de las caras que quieras; las demás usan colorBase.
    //   figura.cambiarColorCaras(gestorColor, { {0, NombreColor::Rojo}, {3, NombreColor::Violeta} });
    // Solo pinta relleno (las líneas las dibujas tú con dibujarLineas()).
    void cambiarColorCaras(GestorColor& gestor, const std::map<int, NombreColor>& coloresPorCara) {
        if (caraInicio.empty()) {
            gestor.establecer(colorBase);
            dibujar();
            return;
        }
        glBindVertexArray(VAO);
        for (size_t i = 0; i < caraInicio.size(); i++) {
            auto it = coloresPorCara.find((int)i);
            NombreColor color = (it != coloresPorCara.end()) ? it->second : colorBase;
            gestor.establecer(color);
            glDrawArrays(GL_TRIANGLES, caraInicio[i], caraCantidadVertices[i]);
        }
        glBindVertexArray(0);
    }
};

// --- PRIMITIVAS 2D ---
class Triangulo : public Figura {
public:
    Triangulo() {
        float A[3] = {-0.5f, -0.5f, 0.0f};
        float B[3] = { 0.5f, -0.5f, 0.0f};
        float C[3] = { 0.0f,  0.5f, 0.0f};

        std::vector<float> relleno = {
            A[0],A[1],A[2],  B[0],B[1],B[2],  C[0],C[1],C[2]
        };
        configurarMalla(relleno);

        std::vector<float> lineas = {
            A[0],A[1],A[2],  B[0],B[1],B[2],
            B[0],B[1],B[2],  C[0],C[1],C[2],
            C[0],C[1],C[2],  A[0],A[1],A[2]
        };
        configurarLineas(lineas);

        configurarBordePizza({
            {A[0],A[1],A[2]}, {B[0],B[1],B[2]}, {C[0],C[1],C[2]}
        });
    }
};

class Cuadrado : public Figura {
public:
    Cuadrado() {
        float TL[3] = {-0.5f,  0.5f, 0.0f};
        float BL[3] = {-0.5f, -0.5f, 0.0f};
        float BR[3] = { 0.5f, -0.5f, 0.0f};
        float TR[3] = { 0.5f,  0.5f, 0.0f};

        std::vector<float> relleno = {
            TL[0],TL[1],TL[2],  BL[0],BL[1],BL[2],  BR[0],BR[1],BR[2],
            TL[0],TL[1],TL[2],  BR[0],BR[1],BR[2],  TR[0],TR[1],TR[2]
        };
        configurarMalla(relleno);

        std::vector<float> lineas = {
            TL[0],TL[1],TL[2],  BL[0],BL[1],BL[2],
            BL[0],BL[1],BL[2],  BR[0],BR[1],BR[2],
            BR[0],BR[1],BR[2],  TR[0],TR[1],TR[2],
            TR[0],TR[1],TR[2],  TL[0],TL[1],TL[2]
        };
        configurarLineas(lineas);

        configurarBordePizza({
            {TL[0],TL[1],TL[2]}, {BL[0],BL[1],BL[2]}, {BR[0],BR[1],BR[2]}, {TR[0],TR[1],TR[2]}
        });
    }
};

class Circulo : public Figura {
public:
    Circulo(int segmentos = 36, float radio = 0.5f) {
        std::vector<float> relleno;
        std::vector<Punto3D> borde;
        for(int i = 0; i < segmentos; i++) {
            float angulo1 = 2.0f * PI * i / segmentos;
            float angulo2 = 2.0f * PI * (i + 1) / segmentos;
            relleno.push_back(0.0f); relleno.push_back(0.0f); relleno.push_back(0.0f);
            relleno.push_back(radio * cos(angulo1)); relleno.push_back(radio * sin(angulo1)); relleno.push_back(0.0f);
            relleno.push_back(radio * cos(angulo2)); relleno.push_back(radio * sin(angulo2)); relleno.push_back(0.0f);
            borde.push_back({radio * cosf(angulo1), radio * sinf(angulo1), 0.0f});
        }
        configurarMalla(relleno);

        std::vector<float> lineas;
        for (size_t i = 0; i < borde.size(); i++) {
            Punto3D a = borde[i];
            Punto3D b = borde[(i + 1) % borde.size()];
            lineas.push_back(a.x); lineas.push_back(a.y); lineas.push_back(a.z);
            lineas.push_back(b.x); lineas.push_back(b.y); lineas.push_back(b.z);
        }
        configurarLineas(lineas);

        configurarBordePizza(borde);
    }
};

class Trapecio : public Figura {
public:
    Trapecio(float baseInferior = 1.0f, float baseSuperior = 0.5f, float altura = 0.6f) {
        float bi = baseInferior / 2.0f;
        float bs = baseSuperior / 2.0f;
        float h  = altura / 2.0f;
        float A[3] = {-bi, -h, 0.0f};
        float B[3] = { bi, -h, 0.0f};
        float C[3] = { bs,  h, 0.0f};
        float D[3] = {-bs,  h, 0.0f};

        std::vector<float> relleno = {
            A[0],A[1],A[2],  B[0],B[1],B[2],  C[0],C[1],C[2],
            A[0],A[1],A[2],  C[0],C[1],C[2],  D[0],D[1],D[2]
        };
        configurarMalla(relleno);

        std::vector<float> lineas = {
            A[0],A[1],A[2],  B[0],B[1],B[2],
            B[0],B[1],B[2],  C[0],C[1],C[2],
            C[0],C[1],C[2],  D[0],D[1],D[2],
            D[0],D[1],D[2],  A[0],A[1],A[2]
        };
        configurarLineas(lineas);

        configurarBordePizza({
            {A[0],A[1],A[2]}, {B[0],B[1],B[2]}, {C[0],C[1],C[2]}, {D[0],D[1],D[2]}
        });
    }
};

class Rombo : public Figura {
public:
    Rombo(float diagonalHorizontal = 1.0f, float diagonalVertical = 0.7f) {
        float dh = diagonalHorizontal / 2.0f;
        float dv = diagonalVertical / 2.0f;
        float Arriba[3]   = {0.0f,  dv, 0.0f};
        float Izquierda[3]= {-dh, 0.0f, 0.0f};
        float Abajo[3]    = {0.0f, -dv, 0.0f};
        float Derecha[3]  = { dh, 0.0f, 0.0f};

        std::vector<float> relleno = {
            Arriba[0],Arriba[1],Arriba[2],  Izquierda[0],Izquierda[1],Izquierda[2],  Abajo[0],Abajo[1],Abajo[2],
            Arriba[0],Arriba[1],Arriba[2],  Abajo[0],Abajo[1],Abajo[2],  Derecha[0],Derecha[1],Derecha[2]
        };
        configurarMalla(relleno);

        std::vector<float> lineas = {
            Arriba[0],Arriba[1],Arriba[2],   Izquierda[0],Izquierda[1],Izquierda[2],
            Izquierda[0],Izquierda[1],Izquierda[2],  Abajo[0],Abajo[1],Abajo[2],
            Abajo[0],Abajo[1],Abajo[2],      Derecha[0],Derecha[1],Derecha[2],
            Derecha[0],Derecha[1],Derecha[2],Arriba[0],Arriba[1],Arriba[2]
        };
        configurarLineas(lineas);

        configurarBordePizza({
            {Arriba[0],Arriba[1],Arriba[2]}, {Izquierda[0],Izquierda[1],Izquierda[2]},
            {Abajo[0],Abajo[1],Abajo[2]},    {Derecha[0],Derecha[1],Derecha[2]}
        });
    }
};

class Semicirculo : public Figura {
public:
    Semicirculo(int segmentos = 36, float radio = 0.5f) {
        std::vector<float> relleno;
        std::vector<Punto3D> arco;
        for (int i = 0; i <= segmentos; i++) {
            float ang = PI * i / segmentos;
            arco.push_back({radio * cosf(ang), radio * sinf(ang), 0.0f});
        }
        for (int i = 0; i < segmentos; i++) {
            relleno.push_back(0.0f); relleno.push_back(0.0f); relleno.push_back(0.0f);
            relleno.push_back(arco[i].x);   relleno.push_back(arco[i].y);   relleno.push_back(arco[i].z);
            relleno.push_back(arco[i+1].x); relleno.push_back(arco[i+1].y); relleno.push_back(arco[i+1].z);
        }
        configurarMalla(relleno);

        std::vector<float> lineas;
        for (int i = 0; i < segmentos; i++) {
            lineas.push_back(arco[i].x);   lineas.push_back(arco[i].y);   lineas.push_back(arco[i].z);
            lineas.push_back(arco[i+1].x); lineas.push_back(arco[i+1].y); lineas.push_back(arco[i+1].z);
        }
        lineas.push_back(arco[segmentos].x); lineas.push_back(arco[segmentos].y); lineas.push_back(arco[segmentos].z);
        lineas.push_back(arco[0].x);         lineas.push_back(arco[0].y);         lineas.push_back(arco[0].z);
        configurarLineas(lineas);

        configurarBordePizza(arco);
    }
};

// --- PRIMITIVAS 3D ---

// Piramide: caras [0]=base, [1]=frontal, [2]=derecha, [3]=trasera, [4]=izquierda
class Piramide : public Figura {
public:
    Piramide() {
        float BA[3] = {-0.5f, -0.5f, -0.5f};
        float BB[3] = { 0.5f, -0.5f, -0.5f};
        float BC[3] = { 0.5f, -0.5f,  0.5f};
        float BD[3] = {-0.5f, -0.5f,  0.5f};
        float P[3]  = { 0.0f,  0.5f,  0.0f};

        std::vector<float> relleno = {
            BA[0],BA[1],BA[2],  BB[0],BB[1],BB[2],  BC[0],BC[1],BC[2],
            BC[0],BC[1],BC[2],  BD[0],BD[1],BD[2],  BA[0],BA[1],BA[2],
            BD[0],BD[1],BD[2],  BC[0],BC[1],BC[2],  P[0],P[1],P[2],
            BC[0],BC[1],BC[2],  BB[0],BB[1],BB[2],  P[0],P[1],P[2],
            BB[0],BB[1],BB[2],  BA[0],BA[1],BA[2],  P[0],P[1],P[2],
            BA[0],BA[1],BA[2],  BD[0],BD[1],BD[2],  P[0],P[1],P[2]
        };
        configurarMalla(relleno);

        auto E = [](std::vector<float>& out, const float* a, const float* b) {
            out.push_back(a[0]); out.push_back(a[1]); out.push_back(a[2]);
            out.push_back(b[0]); out.push_back(b[1]); out.push_back(b[2]);
        };
        std::vector<float> lineas;
        E(lineas, BA, BB); E(lineas, BB, BC); E(lineas, BC, BD); E(lineas, BD, BA);
        E(lineas, BA, P);  E(lineas, BB, P);  E(lineas, BC, P);  E(lineas, BD, P);
        configurarLineas(lineas);

        configurarCaras({6, 3, 3, 3, 3});
    }
};

// Cubo: caras [0]=Frontal, [1]=Trasera, [2]=Izquierda, [3]=Derecha, [4]=Inferior, [5]=Superior
class Cubo : public Figura {
public:
    Cubo() {
        float FTL[3] = {-0.5f,  0.5f,  0.5f}, FTR[3] = { 0.5f,  0.5f,  0.5f};
        float FBR[3] = { 0.5f, -0.5f,  0.5f}, FBL[3] = {-0.5f, -0.5f,  0.5f};
        float BTL[3] = {-0.5f,  0.5f, -0.5f}, BTR[3] = { 0.5f,  0.5f, -0.5f};
        float BBR[3] = { 0.5f, -0.5f, -0.5f}, BBL[3] = {-0.5f, -0.5f, -0.5f};

        std::vector<float> v = {
            FBL[0],FBL[1],FBL[2],  FBR[0],FBR[1],FBR[2],  FTR[0],FTR[1],FTR[2],
            FTR[0],FTR[1],FTR[2],  FTL[0],FTL[1],FTL[2],  FBL[0],FBL[1],FBL[2],
            BBL[0],BBL[1],BBL[2],  BBR[0],BBR[1],BBR[2],  BTR[0],BTR[1],BTR[2],
            BTR[0],BTR[1],BTR[2],  BTL[0],BTL[1],BTL[2],  BBL[0],BBL[1],BBL[2],
            FTL[0],FTL[1],FTL[2],  BTL[0],BTL[1],BTL[2],  BBL[0],BBL[1],BBL[2],
            BBL[0],BBL[1],BBL[2],  FBL[0],FBL[1],FBL[2],  FTL[0],FTL[1],FTL[2],
            FTR[0],FTR[1],FTR[2],  BTR[0],BTR[1],BTR[2],  BBR[0],BBR[1],BBR[2],
            BBR[0],BBR[1],BBR[2],  FBR[0],FBR[1],FBR[2],  FTR[0],FTR[1],FTR[2],
            BBL[0],BBL[1],BBL[2],  BBR[0],BBR[1],BBR[2],  FBR[0],FBR[1],FBR[2],
            FBR[0],FBR[1],FBR[2],  FBL[0],FBL[1],FBL[2],  BBL[0],BBL[1],BBL[2],
            FTL[0],FTL[1],FTL[2],  FTR[0],FTR[1],FTR[2],  BTR[0],BTR[1],BTR[2],
            BTR[0],BTR[1],BTR[2],  BTL[0],BTL[1],BTL[2],  FTL[0],FTL[1],FTL[2]
        };
        configurarMalla(v);

        auto E = [](std::vector<float>& out, const float* a, const float* b) {
            out.push_back(a[0]); out.push_back(a[1]); out.push_back(a[2]);
            out.push_back(b[0]); out.push_back(b[1]); out.push_back(b[2]);
        };
        std::vector<float> lineas;
        E(lineas, FTL, FTR); E(lineas, FTR, FBR); E(lineas, FBR, FBL); E(lineas, FBL, FTL);
        E(lineas, BTL, BTR); E(lineas, BTR, BBR); E(lineas, BBR, BBL); E(lineas, BBL, BTL);
        E(lineas, FTL, BTL); E(lineas, FTR, BTR); E(lineas, FBR, BBR); E(lineas, FBL, BBL);
        configurarLineas(lineas);

        configurarCaras({6, 6, 6, 6, 6, 6});
    }
};

// Esfera: las "caras" son franjas de latitud (0 = polo sur ... paralelos-1 = polo norte).
// El eje de los polos es Z.
class Esfera : public Figura {
public:
    Esfera(float radio = 0.5f, int paralelos = 20, int meridianos = 20) {
        std::vector<float> relleno;
        for(int i = 0; i < paralelos; ++i) {
            float lat0 = PI * (-0.5f + (float)(i) / paralelos);
            float z0  = sin(lat0)*radio;
            float zr0 = cos(lat0)*radio;

            float lat1 = PI * (-0.5f + (float)(i+1) / paralelos);
            float z1 = sin(lat1)*radio;
            float zr1 = cos(lat1)*radio;

            for(int j = 0; j < meridianos; ++j) {
                float lng0 = 2 * PI * (float)(j) / meridianos;
                float x0 = cos(lng0);
                float y0 = sin(lng0);

                float lng1 = 2 * PI * (float)(j+1) / meridianos;
                float x1 = cos(lng1);
                float y1 = sin(lng1);

                relleno.push_back(x0*zr0); relleno.push_back(y0*zr0); relleno.push_back(z0);
                relleno.push_back(x1*zr0); relleno.push_back(y1*zr0); relleno.push_back(z0);
                relleno.push_back(x0*zr1); relleno.push_back(y0*zr1); relleno.push_back(z1);

                relleno.push_back(x1*zr0); relleno.push_back(y1*zr0); relleno.push_back(z0);
                relleno.push_back(x1*zr1); relleno.push_back(y1*zr1); relleno.push_back(z1);
                relleno.push_back(x0*zr1); relleno.push_back(y0*zr1); relleno.push_back(z1);
            }
        }
        configurarMalla(relleno);

        auto punto = [&](int i, int j) -> Punto3D {
            float lat = PI * (-0.5f + (float)i / paralelos);
            float lng = 2 * PI * (float)j / meridianos;
            float zr = cos(lat) * radio;
            float z  = sin(lat) * radio;
            return { cos(lng) * zr, sin(lng) * zr, z };
        };
        std::vector<float> lineas;
        auto E = [&](Punto3D a, Punto3D b) {
            lineas.push_back(a.x); lineas.push_back(a.y); lineas.push_back(a.z);
            lineas.push_back(b.x); lineas.push_back(b.y); lineas.push_back(b.z);
        };
        for (int i = 1; i < paralelos; i++) {
            for (int j = 0; j < meridianos; j++) {
                E(punto(i, j), punto(i, (j + 1) % meridianos));
            }
        }
        for (int j = 0; j < meridianos; j++) {
            for (int i = 0; i < paralelos; i++) {
                E(punto(i, j), punto(i + 1, j));
            }
        }
        configurarLineas(lineas);

        std::vector<int> caras;
        for (int i = 0; i < paralelos; i++) caras.push_back(meridianos * 6);
        configurarCaras(caras);
    }
};

// Cilindro: 3 "caras" por segmento i: (3i)=tapa sup, (3i+1)=tapa inf, (3i+2)=lateral
class Cilindro : public Figura {
public:
    Cilindro(float radio = 0.5f, float altura = 1.0f, int segmentos = 36) {
        std::vector<float> relleno;
        float h = altura / 2.0f;
        std::vector<Punto3D> top, bottom;
        for (int i = 0; i < segmentos; i++) {
            float a0 = 2.0f * PI * i / segmentos;
            float a1 = 2.0f * PI * (i + 1) / segmentos;
            float x0 = radio * cos(a0), z0 = radio * sin(a0);
            float x1 = radio * cos(a1), z1 = radio * sin(a1);

            relleno.push_back(0.0f); relleno.push_back(h); relleno.push_back(0.0f);
            relleno.push_back(x0);   relleno.push_back(h); relleno.push_back(z0);
            relleno.push_back(x1);   relleno.push_back(h); relleno.push_back(z1);

            relleno.push_back(0.0f); relleno.push_back(-h); relleno.push_back(0.0f);
            relleno.push_back(x1);   relleno.push_back(-h); relleno.push_back(z1);
            relleno.push_back(x0);   relleno.push_back(-h); relleno.push_back(z0);

            relleno.push_back(x0); relleno.push_back(h);  relleno.push_back(z0);
            relleno.push_back(x0); relleno.push_back(-h); relleno.push_back(z0);
            relleno.push_back(x1); relleno.push_back(-h); relleno.push_back(z1);

            relleno.push_back(x0); relleno.push_back(h);  relleno.push_back(z0);
            relleno.push_back(x1); relleno.push_back(-h); relleno.push_back(z1);
            relleno.push_back(x1); relleno.push_back(h);  relleno.push_back(z1);

            top.push_back({x0, h, z0});
            bottom.push_back({x0, -h, z0});
        }
        configurarMalla(relleno);

        std::vector<float> lineas;
        auto E = [&](Punto3D a, Punto3D b) {
            lineas.push_back(a.x); lineas.push_back(a.y); lineas.push_back(a.z);
            lineas.push_back(b.x); lineas.push_back(b.y); lineas.push_back(b.z);
        };
        for (int i = 0; i < segmentos; i++) {
            E(top[i], top[(i + 1) % segmentos]);
            E(bottom[i], bottom[(i + 1) % segmentos]);
        }
        int verticales = 8;
        int paso = std::max(1, segmentos / verticales);
        for (int i = 0; i < segmentos; i += paso) {
            E(top[i], bottom[i]);
        }
        configurarLineas(lineas);

        std::vector<int> caras;
        for (int i = 0; i < segmentos; i++) {
            caras.push_back(3);
            caras.push_back(3);
            caras.push_back(6);
        }
        configurarCaras(caras);
    }
};

// Cono: 2 "caras" por segmento i: (2i)=base, (2i+1)=lateral
class Cono : public Figura {
public:
    Cono(float radio = 0.5f, float altura = 1.0f, int segmentos = 36) {
        std::vector<float> relleno;
        float h = altura / 2.0f;
        std::vector<Punto3D> base;
        Punto3D apice = {0.0f, h, 0.0f};
        for (int i = 0; i < segmentos; i++) {
            float a0 = 2.0f * PI * i / segmentos;
            float a1 = 2.0f * PI * (i + 1) / segmentos;
            float x0 = radio * cos(a0), z0 = radio * sin(a0);
            float x1 = radio * cos(a1), z1 = radio * sin(a1);

            relleno.push_back(0.0f); relleno.push_back(-h); relleno.push_back(0.0f);
            relleno.push_back(x1);   relleno.push_back(-h); relleno.push_back(z1);
            relleno.push_back(x0);   relleno.push_back(-h); relleno.push_back(z0);

            relleno.push_back(apice.x); relleno.push_back(apice.y); relleno.push_back(apice.z);
            relleno.push_back(x0);      relleno.push_back(-h);      relleno.push_back(z0);
            relleno.push_back(x1);      relleno.push_back(-h);      relleno.push_back(z1);

            base.push_back({x0, -h, z0});
        }
        configurarMalla(relleno);

        std::vector<float> lineas;
        auto E = [&](Punto3D a, Punto3D b) {
            lineas.push_back(a.x); lineas.push_back(a.y); lineas.push_back(a.z);
            lineas.push_back(b.x); lineas.push_back(b.y); lineas.push_back(b.z);
        };
        for (int i = 0; i < segmentos; i++) {
            E(base[i], base[(i + 1) % segmentos]);
        }
        int lineasApice = 8;
        int paso = std::max(1, segmentos / lineasApice);
        for (int i = 0; i < segmentos; i += paso) {
            E(base[i], apice);
        }
        configurarLineas(lineas);

        std::vector<int> caras;
        for (int i = 0; i < segmentos; i++) {
            caras.push_back(3);
            caras.push_back(3);
        }
        configurarCaras(caras);
    }
};

// -------------------------------------------------------------------------
// 5. UTILIDADES DE VECTORES
// -------------------------------------------------------------------------
inline Punto3D vSumar(Punto3D a, Punto3D b)    { return {a.x + b.x, a.y + b.y, a.z + b.z}; }
inline Punto3D vRestar(Punto3D a, Punto3D b)   { return {a.x - b.x, a.y - b.y, a.z - b.z}; }
inline float   vPunto(Punto3D a, Punto3D b)    { return a.x * b.x + a.y * b.y + a.z * b.z; }
inline Punto3D vCruz(Punto3D a, Punto3D b)     { return {a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x}; }
inline float   vLongitud(Punto3D a)            { return sqrtf(vPunto(a, a)); }
inline Punto3D vNormalizar(Punto3D a) {
    float l = vLongitud(a);
    if (l < 1e-8f) return {0.0f, 0.0f, 0.0f};
    return {a.x / l, a.y / l, a.z / l};
}
inline Punto3D vRotarZ(Punto3D p, float grados) {
    float r = grados * PI / 180.0f, c = cosf(r), s = sinf(r);
    return {p.x * c - p.y * s, p.x * s + p.y * c, p.z};
}
inline Punto3D vRotarY(Punto3D p, float grados) {
    float r = grados * PI / 180.0f, c = cosf(r), s = sinf(r);
    return {p.x * c + p.z * s, p.y, -p.x * s + p.z * c};
}

// -------------------------------------------------------------------------
// 6. OBJETIVO: un punto del mundo o una figura
// -------------------------------------------------------------------------
// Sirve para dos cosas:
//   - el CENTRO alrededor del cual se orbita (objetos y cámaras)
//   - el punto AL QUE MIRA una cámara (camara.mirarA(...))
// Puede ser:
//   - un punto fijo:   Objetivo(0, 0, 0)  u  Objetivo(Punto3D{...})
//   - una figura:      Objetivo(sol)  -> usa sol.obtenerPosicion() cada vez
//     que se pide, así que si la figura se mueve, el objetivo se mueve con ella.
struct Objetivo {
    Punto3D fijo = {0.0f, 0.0f, 0.0f};
    const Figura* figura = nullptr;

    Objetivo(Punto3D p) : fijo(p) {}
    Objetivo(float x, float y, float z) : fijo{x, y, z} {}
    Objetivo(const Figura& f) : figura(&f) {}

    Punto3D obtener() const { return figura ? figura->obtenerPosicion() : fijo; }
};

// Rotar un punto alrededor del eje X (en grados). (Z e Y ya existen arriba)
inline Punto3D vRotarX(Punto3D p, float grados) {
    float r = grados * PI / 180.0f, c = cosf(r), s = sinf(r);
    return {p.x, p.y * c - p.z * s, p.y * s + p.z * c};
}

// -------------------------------------------------------------------------
// 7. CÁMARA
// -------------------------------------------------------------------------
// Cada cámara tiene TODO esto propio:
//   - posición y hacia dónde mira (fijar / mirarA)
//   - PERMISOS: qué movimientos tiene permitidos (puedeOrbitar, puedeEspiral,
//     puedeRuta, puedeRespirar, puedeRotar)
//   - su propia RUTA (animador): camara.ruta.agregarPunto(...)
//   - su propio RESPIRO (acercarse/alejarse) y su propia ROTACIÓN en X, Y, Z
//
// ORDEN CORRECTO CADA FRAME (ver el main):
//     cam.reiniciar();                    // 1) siempre primero
//     orbitarCamara / espiralCamara / rutaCamara(...)   // 2) mueve la posición (opcional)
//     cam.aplicarEfectos(tiempo);         // 3) mira a su objetivo + respira + rota
//     camaras.aplicarActiva(g_aspecto);   // 4) sube la cámara al shader
class Camara {
private:
    unsigned int shaderID;
    int vistaLoc, proyLoc;
public:
    Punto3D posicion = {0.0f, 1.0f, 4.5f};
    Punto3D objetivo = {0.0f, 0.0f, 0.0f};
    Punto3D arriba   = {0.0f, 1.0f, 0.0f};
    float fov   = 45.0f;
    float cerca = 0.1f;
    float lejos = 100.0f;

    // PERMISOS: si están en false, esa función no mueve esta cámara.
    bool puedeOrbitar  = true;   // orbitarCamara
    bool puedeEspiral  = true;   // espiralCamara
    bool puedeRuta     = true;   // rutaCamara
    bool puedeRespirar = true;   // respiro (acercarse/alejarse)
    bool puedeRotar    = true;   // rotación X, Y, Z
    bool puedeTrasladar = true;  // trasladarCamara (desplazarse en línea recta)

    // Posición "de fábrica" de ESTA cámara (a la que vuelve reiniciar())
    Punto3D posInicial = {0.0f, 1.0f, 4.5f};
    Punto3D objInicial = {0.0f, 0.0f, 0.0f};

    // A DÓNDE MIRA (opcional). Si tieneMira es true, la cámara apunta SIEMPRE
    // a este Objetivo (punto o figura), sin importar cómo se mueva.
    Objetivo mira = Objetivo(0.0f, 0.0f, 0.0f);
    bool tieneMira = false;

    // RUTA PROPIA de esta cámara (se usa con rutaCamara)
    AnimadorRuta ruta;

    // RESPIRO: la cámara se acerca y se aleja de su objetivo.
    //   respiroAmplitud : fracción de la distancia (0.2 = ±20 %). 0 = no respira.
    //   respiroVelocidad: rapidez (radianes/seg; ~2 es tranquilo)
    float respiroAmplitud  = 0.0f;
    float respiroVelocidad = 2.0f;

    // ROTACIÓN: la cámara gira alrededor de su objetivo sobre los ejes X, Y, Z
    // del mundo (grados por segundo, negativo = al revés). 0 = no gira.
    //   X : voltereta vertical (pasa por arriba y por abajo)
    //   Y : vuelta horizontal (como un plato giratorio)
    //   Z : la imagen gira sobre sí misma (la cámara "se ladea")
    float rotVelX = 0.0f, rotVelY = 0.0f, rotVelZ = 0.0f;

    Camara(unsigned int shaderProgram) {
        shaderID = shaderProgram;
        vistaLoc = glGetUniformLocation(shaderID, "vista");
        proyLoc  = glGetUniformLocation(shaderID, "proyeccion");
    }

    void establecerPosicion(float x, float y, float z) { posicion = {x, y, z}; }
    void establecerArriba(float x, float y, float z)   { arriba = {x, y, z}; }

    // DÓNDE está y HACIA DÓNDE mira (punto fijo), en una sola llamada.
    // También la guarda como posición inicial.
    void fijar(Punto3D pos, Punto3D mirando) {
        posicion = posInicial = pos;
        objetivo = objInicial = mirando;
        arriba = {0.0f, 1.0f, 0.0f};
    }

    // A QUÉ APUNTAR (se queda guardado, no hace falta repetirlo cada frame):
    //   cam.mirarA(Objetivo(sol));            // a una figura (la sigue)
    //   cam.mirarA(Objetivo(0, 0.5f, 0));     // a un punto fijo
    //   cam.mirarA(0, 0.5f, 0);               // lo mismo, más corto
    //   cam.dejarDeMirar();                   // vuelve al punto de fijar()
    void mirarA(const Objetivo& o)          { mira = o; tieneMira = true; }
    void mirarA(float x, float y, float z)  { mira = Objetivo(x, y, z); tieneMira = true; }
    void dejarDeMirar()                     { tieneMira = false; }

    void configurarRespiro(float amplitud, float velocidad = 2.0f) {
        respiroAmplitud = amplitud;
        respiroVelocidad = velocidad;
    }
    void configurarRotacion(float velX, float velY, float velZ) {
        rotVelX = velX; rotVelY = velY; rotVelZ = velZ;
    }

    // Vuelve a la posición/orientación de fábrica (llamar al inicio de cada frame)
    void reiniciar() {
        posicion = posInicial;
        objetivo = objInicial;
        arriba   = {0.0f, 1.0f, 0.0f};
    }

    // Aplica "mirar a", respiro y rotación sobre la posición que ya tenga la
    // cámara (la de fábrica, o la que dejó orbitarCamara/espiralCamara/rutaCamara).
    // Llamar DESPUÉS del movimiento y ANTES de aplicar(). Requiere reiniciar()
    // al inicio del frame; si no, los efectos se acumularían.
    void aplicarEfectos(float tiempo) {
        if (tieneMira) objetivo = mira.obtener();

        Punto3D offset = vRestar(posicion, objetivo);   // posición de la cámara respecto a su objetivo
        if (vLongitud(offset) < 1e-6f) return;
        Punto3D up = arriba;

        if (puedeRotar && (rotVelX != 0.0f || rotVelY != 0.0f || rotVelZ != 0.0f)) {
            float ax = tiempo * rotVelX, ay = tiempo * rotVelY, az = tiempo * rotVelZ;
            offset = vRotarZ(vRotarY(vRotarX(offset, ax), ay), az);
            up     = vRotarZ(vRotarY(vRotarX(up, ax), ay), az);   // el "arriba" gira con la cámara
        }

        if (puedeRespirar && respiroAmplitud != 0.0f) {
            float k = 1.0f + respiroAmplitud * sinf(tiempo * respiroVelocidad);
            offset = {offset.x * k, offset.y * k, offset.z * k};
        }

        posicion = vSumar(objetivo, offset);
        arriba = up;
    }

    void aplicar(float aspecto) {
        Punto3D f = vRestar(objetivo, posicion);
        if (vLongitud(f) < 1e-6f) f = {0.0f, 0.0f, -1.0f};
        f = vNormalizar(f);

        Punto3D up = arriba;
        Punto3D s = vCruz(f, up);
        if (vLongitud(s) < 1e-6f) {
            up = (fabsf(f.y) > 0.99f) ? Punto3D{0.0f, 0.0f, -1.0f} : Punto3D{0.0f, 1.0f, 0.0f};
            s = vCruz(f, up);
        }
        s = vNormalizar(s);
        Punto3D u = vCruz(s, f);

        float vista[16] = {
             s.x,  s.y,  s.z, -vPunto(s, posicion),
             u.x,  u.y,  u.z, -vPunto(u, posicion),
            -f.x, -f.y, -f.z,  vPunto(f, posicion),
             0.0f, 0.0f, 0.0f, 1.0f
        };

        float t = 1.0f / tanf(fov * PI / 360.0f);
        float proy[16] = {
            t / aspecto, 0.0f, 0.0f, 0.0f,
            0.0f, t, 0.0f, 0.0f,
            0.0f, 0.0f, (lejos + cerca) / (cerca - lejos), (2.0f * lejos * cerca) / (cerca - lejos),
            0.0f, 0.0f, -1.0f, 0.0f
        };

        glUseProgram(shaderID);
        glUniformMatrix4fv(vistaLoc, 1, GL_TRUE, vista);
        glUniformMatrix4fv(proyLoc,  1, GL_TRUE, proy);
    }
};

// ANIMADOR DE LA CÁMARA: la cámara recorre SU PROPIA ruta (cam.ruta), igual
// que los objetos recorren una AnimadorRuta. Respeta el permiso puedeRuta.
// velocidad = tramos por segundo. La cámara sigue mirando a su objetivo.
inline Punto3D rutaCamara(Camara& cam, float tiempo, float velocidad) {
    if (!cam.puedeRuta) return cam.posicion;
    Punto3D p = cam.ruta.obtenerPosicionActual(tiempo, velocidad);
    cam.posicion = p;
    return p;
}

// TRASLADAR LA CÁMARA: la desplaza en línea recta (dx, dy, dz) respecto a su
// posición de fábrica. Si moverObjetivo es true, el punto al que mira se
// desplaza igual (como un "travelling": la dirección de la mirada no cambia).
// Si es false, la cámara se mueve pero sigue apuntando al mismo punto.
// Respeta el permiso puedeTrasladar. Va DESPUÉS de reiniciar().
inline Punto3D trasladarCamara(Camara& cam, float dx, float dy, float dz, bool moverObjetivo = true) {
    if (!cam.puedeTrasladar) return cam.posicion;
    cam.posicion = vSumar(cam.posicion, {dx, dy, dz});
    if (moverObjetivo) cam.objetivo = vSumar(cam.objetivo, {dx, dy, dz});
    return cam.posicion;
}

// Maneja 1 o N cámaras. Solo UNA es la "activa" (la que se ve en pantalla).
class GestorCamaras {
private:
    unsigned int shaderID;
    std::vector<Camara> camaras;
    int indiceActivo = 0;
public:
    GestorCamaras(unsigned int shaderProgram) : shaderID(shaderProgram) {}

    // Agrega una cámara y devuelve su índice (0, 1, 2...).
    //   pos, mira     : dónde está y hacia dónde mira (punto fijo)
    //   puedeOrbitar  : ¿puede usar orbitarCamara?
    //   puedeEspiral  : ¿puede usar espiralCamara?
    // El resto de permisos (ruta, respiro, rotación) se cambian después:
    //   gestor.obtener(i).puedeRuta = false;
    // OJO: agrega TODAS las cámaras antes de pedir referencias con obtener(),
    // porque agregar una nueva puede invalidar las referencias anteriores.
    int agregar(Punto3D pos, Punto3D mira, bool puedeOrbitar = true, bool puedeEspiral = true) {
        Camara c(shaderID);
        c.fijar(pos, mira);
        c.puedeOrbitar = puedeOrbitar;
        c.puedeEspiral = puedeEspiral;
        camaras.push_back(c);
        return (int)camaras.size() - 1;
    }

    int  cantidad() const { return (int)camaras.size(); }
    int  indiceActiva() const { return indiceActivo; }
    Camara& obtener(int i) { return camaras[i]; }
    Camara& activa()       { return camaras[indiceActivo]; }

    void activar(int i) { if (i >= 0 && i < cantidad()) indiceActivo = i; }
    void siguiente()    { if (cantidad() > 0) indiceActivo = (indiceActivo + 1) % cantidad(); }

    void reiniciarTodas() { for (auto& c : camaras) c.reiniciar(); }

    // Sube al shader SOLO la cámara activa
    void aplicarActiva(float aspecto) {
        if (!camaras.empty()) camaras[indiceActivo].aplicar(aspecto);
    }
};

// -------------------------------------------------------------------------
// 8. TIPOS DE ÓRBITA / ESPIRAL Y SUS PARÁMETROS
// -------------------------------------------------------------------------
// Convencional      : círculo horizontal (plano XZ).
// DiagonalIzquierda : ese círculo inclinado con el lado izquierdo ARRIBA.
// DiagonalDerecha   : ese círculo inclinado con el lado derecho ARRIBA.
// Medio             : aro VERTICAL (órbita "polar").
enum class TipoOrbita { Convencional, DiagonalIzquierda, DiagonalDerecha, Medio };

// Normal / DiagonalIzquierda / DiagonalDerecha: espiral plana (el radio cambia).
// Arriba: hélice que sube por el eje Y mientras da vueltas.
enum class TipoEspiral { Normal, DiagonalIzquierda, DiagonalDerecha, Arriba };

struct ParamsOrbita {
    float inclinacion = 45.0f; // inclinación de las órbitas/espirales diagonales
    float anguloAro   = 30.0f; // giro (sobre Y) del aro vertical de "Medio"
};

// Configuración de una ÓRBITA
struct ConfigOrbita {
    TipoOrbita tipo;
    Objetivo centro;
    float radio;
    float velocidad;   // grados por segundo (negativo = al revés)
    float fase;        // ángulo inicial en grados
    ParamsOrbita params;

    ConfigOrbita(TipoOrbita t, Objetivo c, float r, float vel = 60.0f, float f = 0.0f,
                 ParamsOrbita p = ParamsOrbita())
        : tipo(t), centro(c), radio(r), velocidad(vel), fase(f), params(p) {}
};

// Configuración de una ESPIRAL (al terminar las vueltas, reinicia)
struct ConfigEspiral {
    TipoEspiral tipo;
    Objetivo centro;
    float radioInicial;
    float radioFinal;
    float vueltas;
    float alturaTotal;
    float velocidad;
    float fase;
    ParamsOrbita params;

    ConfigEspiral(TipoEspiral t, Objetivo c, float rIni, float rFin, float v = 3.0f,
                  float altura = 0.0f, float vel = 90.0f, float f = 0.0f,
                  ParamsOrbita p = ParamsOrbita())
        : tipo(t), centro(c), radioInicial(rIni), radioFinal(rFin), vueltas(v),
          alturaTotal(altura), velocidad(vel), fase(f), params(p) {}
};

// -------------------------------------------------------------------------
// 9. MATEMÁTICA INTERNA
// -------------------------------------------------------------------------
inline Punto3D puntoOrbita(TipoOrbita tipo, float radio, float angGrados, const ParamsOrbita& p) {
    float a = angGrados * PI / 180.0f;
    float c = cosf(a), s = sinf(a);
    switch (tipo) {
        case TipoOrbita::Convencional:
            return {radio * c, 0.0f, radio * s};
        case TipoOrbita::DiagonalIzquierda:
            return vRotarZ({radio * c, 0.0f, radio * s}, -p.inclinacion);
        case TipoOrbita::DiagonalDerecha:
            return vRotarZ({radio * c, 0.0f, radio * s}, p.inclinacion);
        case TipoOrbita::Medio:
            return vRotarY({0.0f, radio * c, radio * s}, p.anguloAro);
    }
    return {0.0f, 0.0f, 0.0f};
}

inline Punto3D arribaOrbita(TipoOrbita tipo, float angGrados, const ParamsOrbita& p) {
    switch (tipo) {
        case TipoOrbita::Convencional:      return {0.0f, 1.0f, 0.0f};
        case TipoOrbita::DiagonalIzquierda: return vRotarZ({0.0f, 1.0f, 0.0f}, -p.inclinacion);
        case TipoOrbita::DiagonalDerecha:   return vRotarZ({0.0f, 1.0f, 0.0f},  p.inclinacion);
        case TipoOrbita::Medio: {
            Punto3D t = puntoOrbita(TipoOrbita::Medio, 1.0f, angGrados + 90.0f, p);
            return {-t.x, -t.y, -t.z};
        }
    }
    return {0.0f, 1.0f, 0.0f};
}

struct EstadoEspiral { float angulo, radio, altura; };

inline EstadoEspiral calcularEstadoEspiral(const ConfigEspiral& c, float tiempo) {
    float vueltas = (c.vueltas > 0.0f) ? c.vueltas : 1.0f;
    float total = 360.0f * vueltas;
    float a = fmodf(tiempo * c.velocidad + c.fase, total);
    if (a < 0.0f) a += total;
    float progreso = a / total;
    float radio  = c.radioInicial + (c.radioFinal - c.radioInicial) * progreso;
    float altura = c.alturaTotal * (progreso - 0.5f);
    return {a, radio, altura};
}

inline Punto3D puntoEspiral(TipoEspiral tipo, float radio, float angGrados, float altura,
                            const ParamsOrbita& p) {
    float a = angGrados * PI / 180.0f;
    float c = cosf(a), s = sinf(a);
    switch (tipo) {
        case TipoEspiral::Normal:
            return {radio * c, 0.0f, radio * s};
        case TipoEspiral::DiagonalIzquierda:
            return vRotarZ({radio * c, 0.0f, radio * s}, -p.inclinacion);
        case TipoEspiral::DiagonalDerecha:
            return vRotarZ({radio * c, 0.0f, radio * s}, p.inclinacion);
        case TipoEspiral::Arriba:
            return {radio * c, altura, radio * s};
    }
    return {0.0f, 0.0f, 0.0f};
}

inline Punto3D arribaEspiral(TipoEspiral tipo, const ParamsOrbita& p) {
    switch (tipo) {
        case TipoEspiral::DiagonalIzquierda: return vRotarZ({0.0f, 1.0f, 0.0f}, -p.inclinacion);
        case TipoEspiral::DiagonalDerecha:   return vRotarZ({0.0f, 1.0f, 0.0f},  p.inclinacion);
        default:                             return {0.0f, 1.0f, 0.0f};
    }
}

// -------------------------------------------------------------------------
// 10. FUNCIONES DE ÓRBITA
// -------------------------------------------------------------------------
// Para objetos NO llaman a reiniciar() ni aplicar(). Haz tú:
//     transformador.reiniciar();
//     orbitarObjeto(transformador, cfg, tiempo, &figura);
//     transformador.rotarY(...);  transformador.escalar(...);   // opcional
//     transformador.aplicar();
//     figura.dibujar...();

inline Punto3D orbitarObjeto(Transformador3D& t, const ConfigOrbita& cfg, float tiempo,
                             Figura* figura = nullptr) {
    float ang = tiempo * cfg.velocidad + cfg.fase;
    Punto3D pos = vSumar(cfg.centro.obtener(), puntoOrbita(cfg.tipo, cfg.radio, ang, cfg.params));
    t.trasladar(pos.x, pos.y, pos.z);
    if (figura) figura->establecerPosicion(pos);
    return pos;
}

// Solo CÁMARA (orbita y siempre mira al centro). Respeta el permiso puedeOrbitar.
inline Punto3D orbitarCamara(Camara& cam, const ConfigOrbita& cfg, float tiempo) {
    if (!cam.puedeOrbitar) return cam.posicion;   // <- NUEVO
    float ang = tiempo * cfg.velocidad + cfg.fase;
    Punto3D centro = cfg.centro.obtener();
    Punto3D pos = vSumar(centro, puntoOrbita(cfg.tipo, cfg.radio, ang, cfg.params));
    cam.posicion = pos;
    cam.objetivo = centro;
    cam.arriba   = arribaOrbita(cfg.tipo, ang, cfg.params);
    return pos;
}

// AMBOS: primero el objeto (actualiza su posición) y luego la cámara.
inline Punto3D orbitarAmbos(Camara& cam, Transformador3D& t, const ConfigOrbita& cfgObjeto,
                            const ConfigOrbita& cfgCamara, float tiempo, Figura* figura = nullptr) {
    Punto3D posObjeto = orbitarObjeto(t, cfgObjeto, tiempo, figura);
    orbitarCamara(cam, cfgCamara, tiempo);
    return posObjeto;
}

// -------------------------------------------------------------------------
// 11. FUNCIONES DE ESPIRAL
// -------------------------------------------------------------------------
inline Punto3D espiralObjeto(Transformador3D& t, const ConfigEspiral& cfg, float tiempo,
                             Figura* figura = nullptr) {
    EstadoEspiral e = calcularEstadoEspiral(cfg, tiempo);
    Punto3D pos = vSumar(cfg.centro.obtener(), puntoEspiral(cfg.tipo, e.radio, e.angulo, e.altura, cfg.params));
    t.trasladar(pos.x, pos.y, pos.z);
    if (figura) figura->establecerPosicion(pos);
    return pos;
}

// Respeta el permiso puedeEspiral.
inline Punto3D espiralCamara(Camara& cam, const ConfigEspiral& cfg, float tiempo) {
    if (!cam.puedeEspiral) return cam.posicion;   // <- NUEVO
    EstadoEspiral e = calcularEstadoEspiral(cfg, tiempo);
    Punto3D centro = cfg.centro.obtener();
    Punto3D pos = vSumar(centro, puntoEspiral(cfg.tipo, e.radio, e.angulo, e.altura, cfg.params));
    cam.posicion = pos;
    cam.objetivo = centro;
    cam.arriba   = arribaEspiral(cfg.tipo, cfg.params);
    return pos;
}

inline Punto3D espiralAmbos(Camara& cam, Transformador3D& t, const ConfigEspiral& cfgObjeto,
                            const ConfigEspiral& cfgCamara, float tiempo, Figura* figura = nullptr) {
    Punto3D posObjeto = espiralObjeto(t, cfgObjeto, tiempo, figura);
    espiralCamara(cam, cfgCamara, tiempo);
    return posObjeto;
}


int main()
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Figuras", NULL, NULL);
    if (!window) { glfwTerminate(); return -1; }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    if (!gladLoadGL(glfwGetProcAddress)) return -1;
    glEnable(GL_DEPTH_TEST);

    unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);
    unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);
    unsigned int shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    {
        Transformador3D transformador(shaderProgram);
        GestorColor gestorColor(shaderProgram);

        // =====================================================================
        // FIGURAS: aquí van las que crees (antes del while). Ahora no hay ninguna.
        // =====================================================================
        Cubo cubo;   // el cubo de las piezas (base y pared). Aquí declaras las demás figuras que necesites.

        // =====================  CARPA DE CIRCO: figuras (se crean UNA vez)  =====================
        // Todas las piezas salen de las primitivas que ya tienes (Cilindro, Cono, Esfera, Triangulo).
        Cilindro paredCarpa(0.5f, 1.0f, 36);   // paredes: 36 segmentos = 18 franjas (2 segmentos por franja)
        Cono     techoCarpa(0.5f, 1.0f, 36);   // techo: mismos 36 segmentos, así sus franjas calzan con las de la pared
        Cilindro pisoCarpa(0.5f, 1.0f, 36);    // base plana donde se apoya la carpa
        Cilindro mastil(0.5f, 1.0f, 12);       // palito de la banderita
        Esfera   feston(0.5f, 8, 12);          // "medias lunas" del borde del techo (se achatan al dibujarlas)
        Triangulo puertaCarpa;                 // entrada
        Triangulo bandera;                     // banderín

        // COLORES POR CARA (map: número de cara -> color). Franja k = segmentos 2k y 2k+1; k par = rojo, k impar = blanco.
        // Cilindro: (3i)=tapa superior, (3i+1)=tapa inferior, (3i+2)=lateral del segmento i
        // Cono:     (2i)=base,          (2i+1)=lateral del segmento i
        std::map<int, NombreColor> carasPared, carasTecho;
        for (int i = 0; i < 36; i++) {
            NombreColor franja = ((i / 2) % 2 == 0) ? NombreColor::Rojo : NombreColor::Blanco;
            carasPared[3 * i]     = NombreColor::Blanco;   // tapa superior (queda tapada por el techo)
            carasPared[3 * i + 1] = NombreColor::Gris;     // tapa inferior (queda tapada por el piso)
            carasPared[3 * i + 2] = franja;                // lateral a franjas
            carasTecho[2 * i]     = NombreColor::Blanco;   // base del cono (por abajo)
            carasTecho[2 * i + 1] = franja;                // lateral a franjas
        }

        // =====================================================================
        // CÁMARA: solo la de DEBUG (la mueves tú con el teclado)
        // =====================================================================
        //   W / S = adelante / atrás      A / D = izquierda / derecha
        //   E o ESPACIO = subir           Q = bajar
        //   FLECHAS = girar la vista      SHIFT = ir 3 veces más rápido      R = volver al inicio
        GestorCamaras camaras(shaderProgram);
        int camDebug = camaras.agregar({0.0f, 0.8f, 2.6f}, {0.0f, 0.0f, 0.0f}, false, false);
        Camara& cDebug = camaras.obtener(camDebug);
        // Le quitamos todo lo automático: solo la mueves tú.
        cDebug.puedeOrbitar = false;  cDebug.puedeEspiral = false;  cDebug.puedeRuta = false;
        cDebug.puedeRespirar = false; cDebug.puedeRotar = false;    cDebug.puedeTrasladar = false;

        Punto3D dbgPos   = {0.0f, 0.8f, 2.6f};
        float   dbgYaw   = 0.0f;       // giro horizontal (grados)
        float   dbgPitch = -17.0f;     // giro vertical (grados), limitado a ±89
        float   tiempoPrev = 0.0f;     // para calcular el tiempo entre frames
        auto tecla = [&](int k) { return glfwGetKey(window, k) == GLFW_PRESS; };

        while (!glfwWindowShouldClose(window))
        {
            processInput(window);
            glClearColor(0.15f, 0.55f, 0.75f, 1.0f);   // fondo azul (como la imagen de la carpa)
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            float tiempo = (float)glfwGetTime();
            float dt = tiempo - tiempoPrev;  tiempoPrev = tiempo;   // segundos desde el frame anterior

            // =====================  CÁMARA DEBUG (va ANTES de dibujar)  =====================
            Camara& cam = camaras.activa();
            cam.reiniciar();                                 // 1) SIEMPRE primero en cada frame

            float paso = 2.0f * dt * (tecla(GLFW_KEY_LEFT_SHIFT) ? 3.0f : 1.0f);
            float giro = 90.0f * dt;
            if (tecla(GLFW_KEY_LEFT))  dbgYaw   -= giro;
            if (tecla(GLFW_KEY_RIGHT)) dbgYaw   += giro;
            if (tecla(GLFW_KEY_UP))    dbgPitch += giro;
            if (tecla(GLFW_KEY_DOWN))  dbgPitch -= giro;
            if (dbgPitch >  89.0f) dbgPitch =  89.0f;
            if (dbgPitch < -89.0f) dbgPitch = -89.0f;
            if (tecla(GLFW_KEY_R)) { dbgPos = {0.0f, 0.8f, 2.6f}; dbgYaw = 0.0f; dbgPitch = -17.0f; }

            float yR = dbgYaw * PI / 180.0f, pR = dbgPitch * PI / 180.0f;
            Punto3D dir = { sinf(yR) * cosf(pR), sinf(pR), -cosf(yR) * cosf(pR) };  // hacia dónde mira
            Punto3D der = { cosf(yR), 0.0f, sinf(yR) };                              // su derecha
            auto mover = [&](Punto3D v, float k) { dbgPos = vSumar(dbgPos, {v.x * k, v.y * k, v.z * k}); };
            if (tecla(GLFW_KEY_W)) mover(dir,  paso);
            if (tecla(GLFW_KEY_S)) mover(dir, -paso);
            if (tecla(GLFW_KEY_D)) mover(der,  paso);
            if (tecla(GLFW_KEY_A)) mover(der, -paso);
            if (tecla(GLFW_KEY_E) || tecla(GLFW_KEY_SPACE)) dbgPos.y += paso;
            if (tecla(GLFW_KEY_Q)) dbgPos.y -= paso;

            cam.posicion = dbgPos;
            cam.objetivo = vSumar(dbgPos, dir);

            cam.aplicarEfectos(tiempo);                      // 3) (sin efectos: no cambia nada)
            camaras.aplicarActiva(g_aspecto);                // 4) sube la cámara al shader

            // =====================  AQUÍ DIBUJAS TUS FIGURAS  =====================
            // ---- CARPA DE CIRCO ----
            // MEDIDAS (en "unidades de carpa", antes de escalar todo el conjunto):
            //   pared : radio 0.80, alto 0.80  (de y=0 a y=0.8)
            //   techo : radio 0.95, alto 1.00  (de y=0.8 a y=1.8)   -> sobresale 0.15 de la pared
            //   mástil: de y=1.8 a y=2.15
            // La puerta mira hacia +Z (hacia la cámara inicial).
            const float TAM  = 0.7f;    // tamaño de TODA la carpa (1.0 = medidas de arriba)
            float giroCarpa  = 0.0f;    // giro de TODO el conjunto (grados)
            // giroCarpa = tiempo * 20.0f;   // <- descomenta para que la carpa gire sola

            // pieza(...) = las líneas del CONJUNTO (iguales para todas) + las de cada pieza:
            //   trasladar/rotarY/escalar(TAM) = dónde está y cuánto mide TODA la carpa
            //   trasladar(px,py,pz)           = dónde está la pieza DENTRO de la carpa
            //   rotarZ(rz) y escalar(sx,sy,sz)= giro y forma de ESTA pieza
            auto pieza = [&](float px, float py, float pz, float sx, float sy, float sz, float rz) {
                transformador.reiniciar();
                transformador.trasladar(0.0f, -0.7f, 0.0f);
                transformador.rotarY(giroCarpa);
                transformador.escalar(TAM, TAM, TAM);
                transformador.trasladar(px, py, pz);
                if (rz != 0.0f) transformador.rotarZ(rz);
                transformador.escalar(sx, sy, sz);
                transformador.aplicar();
            };

            // PISO (cilindro muy bajito, radio 1.3)
            pieza(0.0f, -0.02f, 0.0f,  2.6f, 0.04f, 2.6f, 0.0f);
            pisoCarpa.dibujarRellenoYLineas(gestorColor, NombreColor::Celeste, NombreColor::Negro);

            // PAREDES (cilindro radio 0.8, alto 0.8) con franjas rojo/blanco
            pieza(0.0f, 0.4f, 0.0f,  1.6f, 0.8f, 1.6f, 0.0f);
            paredCarpa.dibujarRellenoYLineas(gestorColor, NombreColor::Blanco, carasPared, NombreColor::Negro);

            // TECHO (cono radio 0.95, alto 1.0): centro en y = 0.8 + 1.0/2 = 1.3
            pieza(0.0f, 1.3f, 0.0f,  1.9f, 1.0f, 1.9f, 0.0f);
            techoCarpa.dibujarRellenoYLineas(gestorColor, NombreColor::Blanco, carasTecho, NombreColor::Negro);

            // FESTONES: una media luna por franja, en el borde del techo (y = 0.8, radio 0.95).
            // El centro de la franja k está en el ángulo 2*PI*(k+0.5)/18.
            for (int k = 0; k < 18; k++) {
                float ang = 2.0f * PI * (k + 0.5f) / 18.0f;
                pieza(0.95f * cosf(ang), 0.8f, 0.95f * sinf(ang),  0.3f, 0.14f, 0.3f, 0.0f);
                feston.dibujarRellenoYLineas(gestorColor, (k % 2 == 0) ? NombreColor::Rojo : NombreColor::Blanco, NombreColor::Negro);
            }

            // PUERTA: triángulo pegado a la pared, en el centro de una franja roja (ángulo 90 grados = +Z).
            // z = 0.81 para quedar apenas por delante de la pared (radio 0.8).
            pieza(0.0f, 0.3f, 0.81f,  0.4f, 0.6f, 1.0f, 0.0f);
            puertaCarpa.dibujarRellenoYLineas(gestorColor, NombreColor::Negro, NombreColor::Negro);

            // MÁSTIL (cilindro finito, de y=1.8 a y=2.15)
            pieza(0.0f, 1.975f, 0.0f,  0.03f, 0.35f, 0.03f, 0.0f);
            mastil.dibujarRellenoYLineas(gestorColor, NombreColor::Marron, NombreColor::Negro);

            // BANDERÍN: triángulo girado -90 grados en Z para que la punta apunte hacia +X
            pieza(0.115f, 2.06f, 0.0f,  0.14f, 0.22f, 1.0f, -90.0f);
            bandera.dibujarRellenoYLineas(gestorColor, NombreColor::Rojo, NombreColor::Negro);

            // ---- ESCENA ANTERIOR (base + paredes de práctica): desactivada, no se borró ----
            // Cambia "#if 0" por "#if 1" para volver a dibujarla (junto con la carpa).
#if 0
            // Conjunto: base + pared. Las 3 primeras líneas (trasladar, rotarX, rotarY) son del
            // CONJUNTO y se repiten igual en cada pieza; lo que va después es de cada pieza.

            // BASE
            transformador.reiniciar();
            transformador.trasladar(0.0f, -0.5f, 0.0f);   // 1) dónde está TODO el conjunto
            transformador.rotarX(20.0f);                  // 2) giro de TODO el conjunto
            transformador.rotarY(30.0f);                  //    (igual en todas las piezas)
            transformador.escalar(1.5f, 0.05f, 1.0f);     // 3) forma de ESTA pieza
            transformador.aplicar();
            cubo.dibujarRellenoYLineas(gestorColor, NombreColor::Beige, NombreColor::Negro);

            // PARED
            transformador.reiniciar();
            transformador.trasladar(0.0f, -0.5f, 0.0f);   // las MISMAS 3 líneas del conjunto
            transformador.rotarX(20.0f);
            transformador.rotarY(30.0f);
            transformador.trasladar(0.60f, 0.375f, 0.0f); // 4) dónde está la pared DENTRO del conjunto
            transformador.escalar(0.05f, 0.7f, 0.5f);     // 5) forma de la pared
            transformador.aplicar();
            cubo.dibujarRellenoYLineas(gestorColor, NombreColor::Beige, NombreColor::Negro);

			transformador.reiniciar();
            transformador.trasladar(0.0f, -0.5f, 0.0f);   // las MISMAS 3 líneas del conjunto
            transformador.rotarX(20.0f);
            transformador.rotarY(30.0f);
            transformador.trasladar(-0.60f, 0.375f, 0.0f); // 4) dónde está la pared DENTRO del conjunto
            transformador.escalar(0.05f, 0.7f, 0.5f);     // 5) forma de la pared
            transformador.aplicar();
            cubo.dibujarRellenoYLineas(gestorColor, NombreColor::Beige, NombreColor::Negro);


			transformador.reiniciar();
            transformador.trasladar(0.0f, -0.5f, 0.0f);   // las MISMAS 3 líneas del conjunto
            transformador.rotarX(20.0f);
            transformador.rotarY(90.0f);
            transformador.trasladar(0.60f, 0.375f, 0.0f); // 4) dónde está la pared DENTRO del conjunto
            transformador.escalar(0.05f, 0.7f, 0.9f);     // 5) forma de la pared
            transformador.aplicar();
            cubo.dibujarRellenoYLineas(gestorColor, NombreColor::Beige, NombreColor::Negro);
#endif

            glfwSwapBuffers(window);
            glfwPollEvents();
        }
    }

    glDeleteProgram(shaderProgram);
    glfwTerminate();
    return 0;
}


void processInput(GLFWwindow *window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
}

// Además del viewport, actualiza la proporción para la cámara
void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
    if (height > 0) g_aspecto = (float)width / (float)height;
}