#version 330 core

// Entrées du shader
in vec3 vColor; // Couleur du sommet

// Sortie du shader
out vec3 fragColor; // Couleur du fragment

void main() {
    // Couleur du fragment
    fragColor = vColor;
}