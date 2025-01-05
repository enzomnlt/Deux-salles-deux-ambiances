#version 330 core

// Entrées du shader
in vec4 vColor; // Couleur du sommet

// Sortie du shader
out vec4 fragColor; // Couleur du fragment

uniform bool isCone;

void main() {
    if (isCone) {
        fragColor = vec4(1.0, 1.0, 1.0, 1.0); // White for cone
    } else {
        fragColor = vColor;
    }
}