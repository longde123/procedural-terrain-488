#version 430

uniform int texture_size;
in uint z6_y6_x6_case8_in;

out int index;

void main() {
    index = int(z6_y6_x6_case8_in) + texture_size;
}
