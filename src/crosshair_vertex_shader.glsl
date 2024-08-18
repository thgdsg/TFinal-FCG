#version 330 core
layout(location = 0) in vec2 aPos; // Posicao do vertice

uniform mat4 model;      // Matriz de modelagem
uniform mat4 view;       // Matriz de visao
uniform mat4 projection;  // Matriz de projecao

void main() {
    // Transformando a posicaoo do vertice pela matriz de modelagem, visao e projecao
    gl_Position = projection * view * model * vec4(aPos, 0.0, 1.0);
    gl_PointSize = 10.0; // Tamanho do ponto, se necesserio
}