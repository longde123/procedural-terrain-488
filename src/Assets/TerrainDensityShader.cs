#version 430

// One work group = 1 slice
layout(local_size_x = 32, local_size_y = 32, local_size_z = 1) in;

void main() {
}
