#version 330 core

// Attributs de sommet
layout(location = 0) in vec3 aVertexPosition; // Position du sommet
layout(location = 2) in vec4 aVertexColor; // Couleur du sommet

// Matrices de transformations reçues en uniform
uniform mat4 uMVPMatrix;

// Sorties du shader
out vec4 vColor; // Couleur du sommet

void main() {
    // Passage en coordonnées homogènes
    vec4 vertexPosition = vec4(aVertexPosition, 1);

    // Calcul des valeurs de sortie
    vColor = aVertexColor;

    // Calcul de la position projetée
    gl_Position = uMVPMatrix * vertexPosition;
}