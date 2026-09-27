//========================================================================
// OpenGL - Motor 3D Base: Primitivas Geométricas para Scene Graph
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

using namespace std;

const float PI = 3.14159265359f;

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow *window);

const unsigned int SCR_WIDTH = 1000;
const unsigned int SCR_HEIGHT = 800;

// =========================================================================
// SHADERS
// =========================================================================
const char *vertexShaderSource = "#version 330 core\n"
    "layout (location = 0) in vec3 aPos;\n"
    "uniform mat4 transform = mat4(1.0);\n" 
    "void main()\n"
    "{\n"
    "   gl_Position = transform * vec4(aPos, 1.0);\n"
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

    // ---------------------------------------------------------------
    // ORBITAR: hace que una figura gire alrededor de un punto (cx,cy,cz)
    // a una distancia "radio", como un planeta alrededor del sol.
    //   cx,cy,cz -> punto/centro de la órbita
    //   radio    -> distancia de la figura al centro de la órbita
    //   angulo   -> ángulo actual de la órbita en grados (usar tiempo*velocidad)
    //   eje      -> 'X', 'Y' o 'Z' -> eje sobre el que gira la órbita
    //
    // TRUCO PARA ÓRBITAS ANIDADAS (tipo sistema solar con lunas):
    // Si llamas a orbitar() dos veces seguidas SIN llamar reiniciar() en
    // medio, la segunda órbita queda anidada dentro de la primera
    // (la luna orbita al planeta, y el planeta orbita al sol).
    // ---------------------------------------------------------------
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
// 4. ESTRUCTURA DEL GRAFO DE ESCENA (CLASE BASE FIGURA Y PRIMITIVAS)
// =========================================================================

// CLASE BASE ABSTRACTA
class Figura {
protected:
    // Malla de relleno (triángulos)
    unsigned int VAO, VBO;
    int cantidadVertices;

    // Malla de líneas (ARISTAS REALES, sin diagonales de triangulación)
    unsigned int VAO_lineas = 0, VBO_lineas = 0;
    int cantidadVerticesLineas = 0;
    bool tieneLineas = false;

    // Borde de la figura en 2D, en orden (para la función "pizza" más abajo).
    // Solo lo usan las figuras 2D; las 3D simplemente no lo configuran.
    std::vector<Punto3D> bordePoligono;

    // Rango de vértices [inicio, inicio+cantidad) dentro del buffer de
    // RELLENO que le corresponde a cada "cara" de una figura 3D. Con esto
    // se puede dibujar/pintar cada cara por separado.
    std::vector<int> caraInicio;
    std::vector<int> caraCantidadVertices;

    // Color "base" o "de fábrica" de la figura: el que se supone que debe
    // tener el relleno cuando no se le hace ninguna personalización de
    // caras. Se actualiza solo cada vez que se llama a
    // dibujarRellenoYLineas(...) con un color de relleno, así que
    // cambiarColorCaras() siempre sabe qué color usar para las caras que
    // tú NO le indiques explícitamente, sin que tengas que repetirlo.
    NombreColor colorBase = NombreColor::Blanco;

    // Sube la geometría de RELLENO (para dibujar con GL_TRIANGLES)
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

    // Sube la geometría de LÍNEAS: son las aristas reales de la figura,
    // en pares consecutivos (inicio,fin, inicio,fin, ...) para dibujarse
    // con GL_LINES. Como aquí NO se pasa por la triangulación de relleno,
    // no aparece la diagonal que se ve al poner un cuadrado/cubo/rombo/
    // trapecio en modo alambre con GL_TRIANGLES.
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

    // Guarda el contorno (en orden) de una figura 2D, usado por
    // convertirPizza() para saber hasta dónde llega cada "rebanada".
    void configurarBordePizza(const std::vector<Punto3D>& borde) {
        bordePoligono = borde;
    }

    // Guarda, para una figura 3D, cuántos vértices ocupa cada cara dentro
    // del buffer de relleno, en el mismo orden en que se subieron en el
    // constructor. Ej: un cubo son 6 caras de 6 vértices cada una. TODAS
    // las figuras 3D (Piramide, Cubo, Esfera, Cilindro, Cono) llaman a
    // esto en su constructor, así que TODAS pueden usar
    // cambiarColorCaras() más abajo.
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

    // Busca dónde el rayo que sale de "centro" en dirección "angulo" choca
    // contra el borde de la figura (bordePoligono), probando cada arista
    // del polígono. Es lo que le da a convertirPizza() el punto exacto
    // hasta donde debe llegar cada rebanada.
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
            if (fabs(denom) < 1e-6f) continue; // segmento paralelo al rayo
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
    
    // Dibuja la figura RELLENA (como antes)
    virtual void dibujar() {
        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, cantidadVertices);
        glBindVertexArray(0);
    }

    // Dibuja SOLO el contorno/aristas reales de la figura (GL_LINES).
    // Esta función se debe llamar explícitamente donde se quiera ver la
    // figura en modo líneas; si nunca la llamas para una figura, esa
    // figura jamás se ve en líneas (no depende de un estado global que
    // afecte a todo el programa).
    virtual void dibujarLineas() {
        if (!tieneLineas) return; // esta figura no definió aristas propias
        glBindVertexArray(VAO_lineas);
        glDrawArrays(GL_LINES, 0, cantidadVerticesLineas);
        glBindVertexArray(0);
    }

    // Dibuja el RELLENO y las LÍNEAS juntos, con el mismo color: primero
    // el relleno, empujado levemente "hacia atrás" con glPolygonOffset
    // para que no compita en profundidad con las líneas (si no se hace
    // esto, las aristas parpadean/se pierden por estar exactamente a la
    // misma distancia de la cámara que la malla rellena), y luego las
    // aristas reales encima. Así se ve el color de adentro Y el contorno
    // al mismo tiempo.
    virtual void dibujarRellenoYLineas() {
        glEnable(GL_POLYGON_OFFSET_FILL);
        glPolygonOffset(1.0f, 1.0f);
        dibujar();
        glDisable(GL_POLYGON_OFFSET_FILL);
        dibujarLineas();
    }

    // Misma idea, pero permite usar un color distinto para el relleno y
    // para las líneas del contorno (por ejemplo: figura celeste con
    // bordes negros), usando el mismo GestorColor del programa.
    // De paso, recuerda "colorRelleno" como el colorBase de la figura,
    // que es justo lo que después usa cambiarColorCaras() para las caras
    // que no le indiques explícitamente.
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

    // ---------------------------------------------------------------
    // SOBRECARGA "TODO EN UNO": RELLENO CON CARAS PERSONALIZADAS + LÍNEAS.
    // Es la forma más cómoda de usar la función que pediste: en una sola
    // llamada dibuja la figura completa (relleno + contorno), pero
    // dejándote elegir el color de las caras que quieras.
    //   colorBaseFigura -> el color "de fábrica" de la figura (el que
    //                      tendrían TODAS sus caras si no tocaras nada)
    //   coloresPorCara  -> mapa {índice de cara -> color que tú elijas}.
    //                      Las caras que NO pongas aquí se quedan con
    //                      colorBaseFigura automáticamente.
    //   colorLineas     -> color del contorno/aristas
    // ---------------------------------------------------------------
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

    // ---------------------------------------------------------------
    // CONVERTIR PIZZA (solo figuras 2D): parte la figura como una pizza,
    // dibujando "divisiones" líneas repartidas uniformemente en ángulo,
    // desde un centro hasta el borde REAL de la figura (usando
    // interseccionConBorde). No modifica la figura, solo dibuja encima
    // las líneas de corte con el color que esté activo en ese momento
    // (llama a gestorColor.establecer(...) antes de usarla).
    //   divisiones -> en cuántas "porciones" se corta (cuántas líneas)
    //   centro     -> desde dónde salen los cortes (por defecto el origen,
    //                 que es el centro de todas las figuras 2D de este motor)
    // ---------------------------------------------------------------
    void convertirPizza(int divisiones, Punto3D centro = {0.0f, 0.0f, 0.0f}) {
        if (bordePoligono.empty() || divisiones < 2) return; // esta figura no tiene borde 2D configurado

        std::vector<float> lineas;
        for (int i = 0; i < divisiones; i++) {
            float angulo = 2.0f * PI * i / divisiones;
            Punto3D borde = interseccionConBorde(centro, angulo);
            lineas.push_back(centro.x); lineas.push_back(centro.y); lineas.push_back(centro.z);
            lineas.push_back(borde.x);  lineas.push_back(borde.y);  lineas.push_back(borde.z);
        }

        // Se sube a un buffer temporal (GL_DYNAMIC_DRAW) porque "divisiones"
        // lo elige quien llama a la función, puede cambiar en cualquier
        // momento, y no vale la pena guardarlo de forma permanente.
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

    // ---------------------------------------------------------------
    // PINTAR CARAS (solo figuras 3D con configurarCaras ya llamado en su
    // constructor: Piramide, Cubo, Esfera, Cilindro y Cono): en vez de un
    // solo color para toda la figura, dibuja cada cara por separado con
    // su propio color.
    // ---------------------------------------------------------------
    int cantidadCaras() const { return (int)caraInicio.size(); }

    // Establece manualmente el color base de la figura, por si quieres
    // fijarlo sin pasar por dibujarRellenoYLineas(...) primero.
    void establecerColorBase(NombreColor color) { colorBase = color; }
    NombreColor obtenerColorBase() const { return colorBase; }

    // Multicolor ALEATORIO: cada cara se pinta con un color al azar de la
    // paleta dada (por defecto usa una paleta variada). Cada vez que se
    // llama, se vuelve a sortear el color de cada cara.
    void pintarCarasAleatorio(GestorColor& gestor,
                               const std::vector<NombreColor>& paleta = {
                                   NombreColor::Rojo, NombreColor::Verde, NombreColor::Azul,
                                   NombreColor::Amarillo, NombreColor::Naranja, NombreColor::Morado,
                                   NombreColor::Cyan, NombreColor::Rosa, NombreColor::Fucsia,
                                   NombreColor::Celeste, NombreColor::VerdeFluorescente, NombreColor::Violeta }) {
        if (caraInicio.empty() || paleta.empty()) { dibujar(); return; } // sin caras configuradas: relleno normal
        glBindVertexArray(VAO);
        for (size_t i = 0; i < caraInicio.size(); i++) {
            gestor.establecer(paleta[rand() % paleta.size()]);
            glDrawArrays(GL_TRIANGLES, caraInicio[i], caraCantidadVertices[i]);
        }
        glBindVertexArray(0);
    }

    // Colores ESPECÍFICOS (versión "clásica"): tú eliges el color de las
    // caras que te interesen, y todas las que NO menciones se pintan con
    // "colorBase" que TÚ pasas explícitamente en cada llamada.
    // Se mantiene por compatibilidad; para el caso normal (que las caras
    // no elegidas usen el color de fábrica de la figura sin tener que
    // repetirlo) usa mejor cambiarColorCaras() de aquí abajo.
    void pintarCarasEspecificas(GestorColor& gestor,
                                 const std::map<int, NombreColor>& coloresPorCara,
                                 NombreColor colorBaseParametro) {
        colorBase = colorBaseParametro;
        cambiarColorCaras(gestor, coloresPorCara);
    }

    // =================================================================
    // *** CAMBIAR COLOR DE CARAS ***
    // ---------------------------------------------------------------
    // Esta es la función pedida: te deja elegir el color de las caras
    // que quieras, UNA POR UNA, para CUALQUIER figura 3D del motor
    // (Piramide, Cubo, Esfera, Cilindro, Cono... todas las que llamen a
    // configurarCaras() en su constructor).
    //
    // A las caras que NO menciones en "coloresPorCara" se les asigna
    // automáticamente "colorBase": el color de relleno que se supone que
    // debería tener la figura sin ninguna modificación (el mismo que le
    // diste la última vez que llamaste a dibujarRellenoYLineas(...), o el
    // que hayas fijado a mano con establecerColorBase(...)). Así NUNCA
    // tienes que repetir el color base cada vez que solo quieres tocar
    // un par de caras.
    //
    // Cómo usarla:
    //   figura.cambiarColorCaras(gestorColor, {
    //       {0, NombreColor::Rojo},      // cara número 0 -> roja
    //       {3, NombreColor::Violeta}    // cara número 3 -> violeta
    //   });                              // el resto de caras -> colorBase
    //
    // El índice de cada cara depende del orden en que la figura las armó
    // en su constructor (revisa el comentario de cada clase más abajo
    // para saber qué índice corresponde a qué cara concreta).
    //
    // Nota: esta función solo pinta el RELLENO. Si además quieres ver el
    // contorno en líneas, dibuja tú mismo dibujarLineas() después (o usa
    // directamente la sobrecarga de 4 argumentos de
    // dibujarRellenoYLineas(...) de más arriba, que ya hace las dos cosas
    // en un solo llamado).
    // =================================================================
    void cambiarColorCaras(GestorColor& gestor, const std::map<int, NombreColor>& coloresPorCara) {
        if (caraInicio.empty()) {
            // Esta figura no tiene caras configuradas (p.ej. es 2D):
            // simplemente se dibuja entera con el color base.
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

        // Borde para la función convertirPizza()
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
            TL[0],TL[1],TL[2],  BL[0],BL[1],BL[2],  BR[0],BR[1],BR[2], // Triángulo 1
            TL[0],TL[1],TL[2],  BR[0],BR[1],BR[2],  TR[0],TR[1],TR[2]  // Triángulo 2
        };
        configurarMalla(relleno);

        // Solo el contorno real (4 aristas), SIN la diagonal TL-BR que
        // usa el relleno para formar los 2 triángulos.
        std::vector<float> lineas = {
            TL[0],TL[1],TL[2],  BL[0],BL[1],BL[2],
            BL[0],BL[1],BL[2],  BR[0],BR[1],BR[2],
            BR[0],BR[1],BR[2],  TR[0],TR[1],TR[2],
            TR[0],TR[1],TR[2],  TL[0],TL[1],TL[2]
        };
        configurarLineas(lineas);

        // Borde para la función convertirPizza()
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

        // Líneas = solo el borde circular (sin los radios/spokes al centro)
        std::vector<float> lineas;
        for (size_t i = 0; i < borde.size(); i++) {
            Punto3D a = borde[i];
            Punto3D b = borde[(i + 1) % borde.size()];
            lineas.push_back(a.x); lineas.push_back(a.y); lineas.push_back(a.z);
            lineas.push_back(b.x); lineas.push_back(b.y); lineas.push_back(b.z);
        }
        configurarLineas(lineas);

        // Borde para la función convertirPizza() (el mismo borde circular)
        configurarBordePizza(borde);
    }
};

// Trapecio (isósceles) — 4 vértices, 2 triángulos de relleno
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

        // Contorno real (4 aristas), sin la diagonal A-C del relleno
        std::vector<float> lineas = {
            A[0],A[1],A[2],  B[0],B[1],B[2],
            B[0],B[1],B[2],  C[0],C[1],C[2],
            C[0],C[1],C[2],  D[0],D[1],D[2],
            D[0],D[1],D[2],  A[0],A[1],A[2]
        };
        configurarLineas(lineas);

        // Borde para la función convertirPizza()
        configurarBordePizza({
            {A[0],A[1],A[2]}, {B[0],B[1],B[2]}, {C[0],C[1],C[2]}, {D[0],D[1],D[2]}
        });
    }
};

// Rombo — 4 vértices, 2 triángulos de relleno
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

        // Contorno real (4 aristas), sin la diagonal Arriba-Abajo del relleno
        std::vector<float> lineas = {
            Arriba[0],Arriba[1],Arriba[2],   Izquierda[0],Izquierda[1],Izquierda[2],
            Izquierda[0],Izquierda[1],Izquierda[2],  Abajo[0],Abajo[1],Abajo[2],
            Abajo[0],Abajo[1],Abajo[2],      Derecha[0],Derecha[1],Derecha[2],
            Derecha[0],Derecha[1],Derecha[2],Arriba[0],Arriba[1],Arriba[2]
        };
        configurarLineas(lineas);

        // Borde para la función convertirPizza()
        configurarBordePizza({
            {Arriba[0],Arriba[1],Arriba[2]}, {Izquierda[0],Izquierda[1],Izquierda[2]},
            {Abajo[0],Abajo[1],Abajo[2]},    {Derecha[0],Derecha[1],Derecha[2]}
        });
    }
};

// Semicírculo — abanico de triángulos cubriendo 180°
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

        // Líneas = arco curvo + el diámetro recto que cierra la figura
        // (sin los radios internos hacia el centro que usa el relleno)
        std::vector<float> lineas;
        for (int i = 0; i < segmentos; i++) {
            lineas.push_back(arco[i].x);   lineas.push_back(arco[i].y);   lineas.push_back(arco[i].z);
            lineas.push_back(arco[i+1].x); lineas.push_back(arco[i+1].y); lineas.push_back(arco[i+1].z);
        }
        // Diámetro: del último punto del arco de vuelta al primero
        lineas.push_back(arco[segmentos].x); lineas.push_back(arco[segmentos].y); lineas.push_back(arco[segmentos].z);
        lineas.push_back(arco[0].x);         lineas.push_back(arco[0].y);         lineas.push_back(arco[0].z);
        configurarLineas(lineas);

        // Borde para la función convertirPizza(): el arco completo
        // (el cierre con el diámetro queda implícito por el módulo)
        configurarBordePizza(arco);
    }
};

// --- PRIMITIVAS 3D ---

// Piramide de base cuadrada — 5 caras: [0]=base, [1]=frontal, [2]=derecha,
// [3]=trasera, [4]=izquierda (en ese orden, según se arman en el relleno).
class Piramide : public Figura {
public:
    Piramide() {
        float BA[3] = {-0.5f, -0.5f, -0.5f};
        float BB[3] = { 0.5f, -0.5f, -0.5f};
        float BC[3] = { 0.5f, -0.5f,  0.5f};
        float BD[3] = {-0.5f, -0.5f,  0.5f};
        float P[3]  = { 0.0f,  0.5f,  0.0f};

        std::vector<float> relleno = {
            // Base cuadrada (2 triángulos)                -> cara 0
            BA[0],BA[1],BA[2],  BB[0],BB[1],BB[2],  BC[0],BC[1],BC[2],
            BC[0],BC[1],BC[2],  BD[0],BD[1],BD[2],  BA[0],BA[1],BA[2],
            // Cara Frontal                                -> cara 1
            BD[0],BD[1],BD[2],  BC[0],BC[1],BC[2],  P[0],P[1],P[2],
            // Cara Derecha                                -> cara 2
            BC[0],BC[1],BC[2],  BB[0],BB[1],BB[2],  P[0],P[1],P[2],
            // Cara Trasera                                -> cara 3
            BB[0],BB[1],BB[2],  BA[0],BA[1],BA[2],  P[0],P[1],P[2],
            // Cara Izquierda                              -> cara 4
            BA[0],BA[1],BA[2],  BD[0],BD[1],BD[2],  P[0],P[1],P[2]
        };
        configurarMalla(relleno);

        // Contorno real: 4 aristas de la base + 4 aristas hacia el ápice
        // (sin la diagonal BA-BC que usa el relleno para la base)
        auto E = [](std::vector<float>& out, const float* a, const float* b) {
            out.push_back(a[0]); out.push_back(a[1]); out.push_back(a[2]);
            out.push_back(b[0]); out.push_back(b[1]); out.push_back(b[2]);
        };
        std::vector<float> lineas;
        E(lineas, BA, BB); E(lineas, BB, BC); E(lineas, BC, BD); E(lineas, BD, BA);
        E(lineas, BA, P);  E(lineas, BB, P);  E(lineas, BC, P);  E(lineas, BD, P);
        configurarLineas(lineas);

        // Caras para cambiarColorCaras()/pintarCarasAleatorio():
        // índice 0 = base (6 vértices), 1..4 = las 4 caras triangulares
        // laterales (3 vértices cada una), en el mismo orden del relleno.
        configurarCaras({6, 3, 3, 3, 3});
    }
};

// Cubo — 6 caras, todas de 6 vértices (2 triángulos c/u):
// [0]=Frontal, [1]=Trasera, [2]=Izquierda, [3]=Derecha, [4]=Inferior, [5]=Superior
class Cubo : public Figura {
public:
    Cubo() {
        float FTL[3] = {-0.5f,  0.5f,  0.5f}, FTR[3] = { 0.5f,  0.5f,  0.5f};
        float FBR[3] = { 0.5f, -0.5f,  0.5f}, FBL[3] = {-0.5f, -0.5f,  0.5f};
        float BTL[3] = {-0.5f,  0.5f, -0.5f}, BTR[3] = { 0.5f,  0.5f, -0.5f};
        float BBR[3] = { 0.5f, -0.5f, -0.5f}, BBL[3] = {-0.5f, -0.5f, -0.5f};

        std::vector<float> v = {
            // Cara Frontal                                        -> cara 0
            FBL[0],FBL[1],FBL[2],  FBR[0],FBR[1],FBR[2],  FTR[0],FTR[1],FTR[2],
            FTR[0],FTR[1],FTR[2],  FTL[0],FTL[1],FTL[2],  FBL[0],FBL[1],FBL[2],
            // Cara Trasera                                        -> cara 1
            BBL[0],BBL[1],BBL[2],  BBR[0],BBR[1],BBR[2],  BTR[0],BTR[1],BTR[2],
            BTR[0],BTR[1],BTR[2],  BTL[0],BTL[1],BTL[2],  BBL[0],BBL[1],BBL[2],
            // Cara Izquierda                                      -> cara 2
            FTL[0],FTL[1],FTL[2],  BTL[0],BTL[1],BTL[2],  BBL[0],BBL[1],BBL[2],
            BBL[0],BBL[1],BBL[2],  FBL[0],FBL[1],FBL[2],  FTL[0],FTL[1],FTL[2],
            // Cara Derecha                                        -> cara 3
            FTR[0],FTR[1],FTR[2],  BTR[0],BTR[1],BTR[2],  BBR[0],BBR[1],BBR[2],
            BBR[0],BBR[1],BBR[2],  FBR[0],FBR[1],FBR[2],  FTR[0],FTR[1],FTR[2],
            // Cara Inferior                                       -> cara 4
            BBL[0],BBL[1],BBL[2],  BBR[0],BBR[1],BBR[2],  FBR[0],FBR[1],FBR[2],
            FBR[0],FBR[1],FBR[2],  FBL[0],FBL[1],FBL[2],  BBL[0],BBL[1],BBL[2],
            // Cara Superior                                       -> cara 5
            FTL[0],FTL[1],FTL[2],  FTR[0],FTR[1],FTR[2],  BTR[0],BTR[1],BTR[2],
            BTR[0],BTR[1],BTR[2],  BTL[0],BTL[1],BTL[2],  FTL[0],FTL[1],FTL[2]
        };
        configurarMalla(v);

        // Contorno real: las 12 aristas del cubo, SIN las diagonales que
        // el relleno usa en cada cara para formar sus 2 triángulos.
        auto E = [](std::vector<float>& out, const float* a, const float* b) {
            out.push_back(a[0]); out.push_back(a[1]); out.push_back(a[2]);
            out.push_back(b[0]); out.push_back(b[1]); out.push_back(b[2]);
        };
        std::vector<float> lineas;
        E(lineas, FTL, FTR); E(lineas, FTR, FBR); E(lineas, FBR, FBL); E(lineas, FBL, FTL); // cara frontal
        E(lineas, BTL, BTR); E(lineas, BTR, BBR); E(lineas, BBR, BBL); E(lineas, BBL, BTL); // cara trasera
        E(lineas, FTL, BTL); E(lineas, FTR, BTR); E(lineas, FBR, BBR); E(lineas, FBL, BBL); // aristas que unen
        configurarLineas(lineas);

        // Caras para cambiarColorCaras()/pintarCarasAleatorio(): 6 caras,
        // cada una con 6 vértices (2 triángulos), en el orden de arriba:
        // Frontal, Trasera, Izquierda, Derecha, Inferior, Superior.
        configurarCaras({6, 6, 6, 6, 6, 6});
    }
};

// Esfera — sus "caras" para cambiarColorCaras() son bandas de latitud
// completas (como los husos/franjas horizontales de un globo terráqueo):
// cara 0 = la franja más cercana al polo sur, cara (paralelos-1) = la más
// cercana al polo norte. Cada franja está hecha de "meridianos" cuadros
// (2 triángulos = 6 vértices cada uno), así que ocupa meridianos*6
// vértices dentro del buffer de relleno.
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

        // Líneas = una grilla real de paralelos (latitud) y meridianos
        // (longitud), como un globo terráqueo, en vez de la diagonal de
        // cada triángulo de relleno.
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
        // Anillos de latitud (se saltan los polos, i=0 e i=paralelos, porque
        // ahí el anillo tiene radio 0 y no se ve nada)
        for (int i = 1; i < paralelos; i++) {
            for (int j = 0; j < meridianos; j++) {
                E(punto(i, j), punto(i, (j + 1) % meridianos));
            }
        }
        // Líneas de meridiano (de polo a polo)
        for (int j = 0; j < meridianos; j++) {
            for (int i = 0; i < paralelos; i++) {
                E(punto(i, j), punto(i + 1, j));
            }
        }
        configurarLineas(lineas);

        // Caras para cambiarColorCaras(): una por cada franja de latitud
        // (en el mismo orden en que el bucle de arriba las va generando,
        // de polo sur a polo norte), cada una con meridianos*6 vértices.
        std::vector<int> caras;
        for (int i = 0; i < paralelos; i++) caras.push_back(meridianos * 6);
        configurarCaras(caras);
    }
};

// Cilindro — tapa superior, tapa inferior y superficie lateral.
// Sus "caras" para cambiarColorCaras() van de 3 en 3 por cada segmento i:
//   cara (3*i+0) = triángulo de la tapa SUPERIOR del segmento i
//   cara (3*i+1) = triángulo de la tapa INFERIOR del segmento i
//   cara (3*i+2) = el panel LATERAL (rectángulo) del segmento i
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

        // Líneas = círculo superior + círculo inferior + unas pocas
        // verticales (aristas reales de la "silueta" del cilindro), sin
        // triangular ni rellenar nada.
        std::vector<float> lineas;
        auto E = [&](Punto3D a, Punto3D b) {
            lineas.push_back(a.x); lineas.push_back(a.y); lineas.push_back(a.z);
            lineas.push_back(b.x); lineas.push_back(b.y); lineas.push_back(b.z);
        };
        for (int i = 0; i < segmentos; i++) {
            E(top[i], top[(i + 1) % segmentos]);
            E(bottom[i], bottom[(i + 1) % segmentos]);
        }
        int verticales = 8; // cantidad de líneas verticales a mostrar
        int paso = std::max(1, segmentos / verticales);
        for (int i = 0; i < segmentos; i += paso) {
            E(top[i], bottom[i]);
        }
        configurarLineas(lineas);

        // Caras para cambiarColorCaras(): por cada segmento se generaron,
        // en este orden, 3 vértices de la tapa superior, 3 de la tapa
        // inferior y 6 del panel lateral (2 triángulos) -> 3 "caras" por
        // segmento: {3, 3, 6}.
        std::vector<int> caras;
        for (int i = 0; i < segmentos; i++) {
            caras.push_back(3); // tapa superior de este segmento
            caras.push_back(3); // tapa inferior de este segmento
            caras.push_back(6); // panel lateral de este segmento
        }
        configurarCaras(caras);
    }
};

// Cono — base circular y superficie lateral hacia una punta.
// Sus "caras" para cambiarColorCaras() van de 2 en 2 por cada segmento i:
//   cara (2*i+0) = triángulo de la BASE del segmento i
//   cara (2*i+1) = triángulo LATERAL (hacia el ápice) del segmento i
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

        // Líneas = círculo de la base + unas pocas líneas hacia el ápice
        // (aristas reales de la silueta), sin diagonales de relleno.
        std::vector<float> lineas;
        auto E = [&](Punto3D a, Punto3D b) {
            lineas.push_back(a.x); lineas.push_back(a.y); lineas.push_back(a.z);
            lineas.push_back(b.x); lineas.push_back(b.y); lineas.push_back(b.z);
        };
        for (int i = 0; i < segmentos; i++) {
            E(base[i], base[(i + 1) % segmentos]);
        }
        int lineasApice = 8; // cantidad de líneas hacia la punta a mostrar
        int paso = std::max(1, segmentos / lineasApice);
        for (int i = 0; i < segmentos; i += paso) {
            E(base[i], apice);
        }
        configurarLineas(lineas);

        // Caras para cambiarColorCaras(): por cada segmento se generaron,
        // en este orden, 3 vértices de la base y 3 del triángulo lateral
        // -> 2 "caras" por segmento: {3, 3}.
        std::vector<int> caras;
        for (int i = 0; i < segmentos; i++) {
            caras.push_back(3); // base de este segmento
            caras.push_back(3); // lateral de este segmento (hacia el ápice)
        }
        configurarCaras(caras);
    }
};

// =========================================================================
// MAIN - PRUEBA DE TODAS LAS FIGURAS
// =========================================================================
int main()
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "OpenGL - Scene Graph Primitivas", NULL, NULL);
    if (!window) { glfwTerminate(); return -1; }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    if (!gladLoadGL(glfwGetProcAddress)) return -1;
    glEnable(GL_DEPTH_TEST); // Vital para 3D (Esfera, Cubo, Pirámide, Cilindro, Cono)

    // Compilación de Shaders
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

    Transformador3D transformador(shaderProgram);
    GestorColor gestorColor(shaderProgram);

    // INICIALIZACIÓN DE FIGURAS (Memoria Dinámica)
    Esfera esfera;
    Piramide piramide;
    Cubo cubo;
    Triangulo triangulo;

    // ---------------------------------------------------------------
    // DEMOSTRACIÓN de cambiarColorCaras(): a cada figura 3D se le arma,
    // UNA sola vez (fuera del bucle, porque no cambia frame a frame), un
    // mapa con los índices de las caras que queremos personalizar. Todo
    // índice de cara que NO aparezca en el mapa se queda con el color
    // base de la figura (Amarillo en la pirámide, Cyan en el cubo,
    // Naranja en la esfera), sin que tengamos que repetirlo.
    // ---------------------------------------------------------------

    // Pirámide: caras 0=base, 1=frontal, 2=derecha, 3=trasera, 4=izquierda
    std::map<int, NombreColor> coloresPiramide = {
        {0, NombreColor::Marron},  // la base, de color marrón (tierra)
        {2, NombreColor::Rojo}     // la cara derecha, roja
        // el resto (1, 3 y 4) se quedan Amarillas (color base)
    };

    // Cubo: caras 0=Frontal, 1=Trasera, 2=Izquierda, 3=Derecha, 4=Inferior, 5=Superior
    std::map<int, NombreColor> coloresCubo = {
        {0, NombreColor::Rojo},      // frontal, roja
        {5, NombreColor::Blanco},    // superior, blanca
        {2, NombreColor::Violeta}    // izquierda, violeta
        // el resto (1, 3, 4) se quedan Cyan (color base)
    };

    // Esfera (paralelos = 20 por defecto): cada cara es una franja de
    // latitud completa, de la 0 (polo sur) a la 19 (polo norte).
    std::map<int, NombreColor> coloresEsfera = {
        {2,  NombreColor::Rojo},       // franja cerca del polo sur
        {9,  NombreColor::Blanco},     // franja del "ecuador"
        {10, NombreColor::Blanco},
        {17, NombreColor::Celeste}     // franja cerca del polo norte
        // el resto de franjas se quedan Naranja (color base)
    };

    // Ruta cuadrada para el triángulo (cierra el ciclo en el 5to punto)
    AnimadorRuta rutaCuadrada;
    rutaCuadrada.agregarPunto(-0.5f,  0.5f, 0.0f); // 1. Esquina superior izquierda
    rutaCuadrada.agregarPunto( 0.5f,  0.5f, 0.0f); // 2. Esquina superior derecha
    rutaCuadrada.agregarPunto( 0.5f, -0.5f, 0.0f); // 3. Esquina inferior derecha
    rutaCuadrada.agregarPunto(-0.5f, -0.5f, 0.0f); // 4. Esquina inferior izquierda
    rutaCuadrada.agregarPunto(-0.5f,  0.5f, 0.0f); // 5. Cierra el ciclo arriba a la izquierda

    cout << "Controles: ESC para salir" << endl;

    while (!glfwWindowShouldClose(window))
    {
        processInput(window);
        glClearColor(0.15f, 0.15f, 0.15f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        float tiempo = (float)glfwGetTime();

        // ESFERA: rotando sobre sí misma, fija en su lugar.
        // Antes se pintaba entera de Naranja; ahora se usa la sobrecarga
        // de dibujarRellenoYLineas(...) con mapa de caras, así que se ve
        // Naranja EXCEPTO en las franjas que elegimos en coloresEsfera.
        transformador.reiniciar();
        transformador.trasladar(-0.5f, 0.5f, 0.0f);
        transformador.rotarY(tiempo * 60.0f);
        transformador.rotarX(tiempo * 40.0f);
        transformador.escalar(0.3f, 0.3f, 0.3f);
        transformador.aplicar();
        esfera.dibujarRellenoYLineas(gestorColor, NombreColor::Naranja, coloresEsfera, NombreColor::Negro);

        // PIRÁMIDE: escalando (efecto de pulso) y rotando.
        // Mismo caso: Amarilla salvo la base (marrón) y la cara derecha (roja).
        transformador.reiniciar();
        transformador.trasladar(0.5f, 0.5f, 0.0f);
        transformador.rotarY(tiempo * 50.0f);
        float escalaPiramide = 0.25f + 0.1f * sinf(tiempo * 2.0f);
        transformador.escalar(escalaPiramide, escalaPiramide, escalaPiramide);
        transformador.aplicar();
        piramide.dibujarRellenoYLineas(gestorColor, NombreColor::Amarillo, coloresPiramide, NombreColor::Negro);

        // CUBO: rotando y trasladándose de lado a lado.
        // Cyan salvo frontal (roja), izquierda (violeta) y superior (blanca).
        transformador.reiniciar();
        float posXCubo = sinf(tiempo) * 0.6f;
        transformador.trasladar(posXCubo, -0.5f, 0.0f);
        transformador.rotarX(tiempo * 45.0f);
        transformador.rotarY(tiempo * 45.0f);
        transformador.escalar(0.25f, 0.25f, 0.25f);
        transformador.aplicar();
        cubo.dibujarRellenoYLineas(gestorColor, NombreColor::Cyan, coloresCubo, NombreColor::Negro);

        // TRIÁNGULO: recorre la ruta cuadrada (AnimadorRuta). Es 2D, así
        // que no tiene "caras" que personalizar; se dibuja como siempre.
        Punto3D posTriangulo = rutaCuadrada.obtenerPosicionActual(tiempo, 0.3f);
        transformador.reiniciar();
        transformador.trasladar(posTriangulo.x, posTriangulo.y, posTriangulo.z);
        transformador.escalar(0.15f, 0.15f, 0.15f);
        transformador.aplicar();
        gestorColor.establecer(NombreColor::VerdeFluorescente);
        triangulo.dibujar();

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteProgram(shaderProgram);
    glfwTerminate();
    return 0;
}

void processInput(GLFWwindow *window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
}
void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}