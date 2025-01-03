#version 330 core

// Entrées du shader
in vec4 vColor; // Couleur du sommet

// Sortie du shader
out vec4 fragColor; // Couleur du fragment

void main() {
    // Couleur du fragment
    fragColor = vColor;
}