#version 330 core

// Entrées du shader
in vec3 vPosition_vs; // Position du sommet transformé dans l'espace View
in vec3 vNormal_vs; // Normale du sommet transformé dans l'espace View
in vec4 vColor; // Couleur du sommet
in vec2 vTexCoords; // Coordonnées de texture du sommet

// Sortie du shader
out vec4 fragColor; // Couleur du fragment

void main() {
    // Couleur du fragment
    fragColor = vec4(normalize(vNormal_vs), 1.0);
}