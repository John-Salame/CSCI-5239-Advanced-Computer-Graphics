//
// John Salame
// Movement for firefly swarms
// Note: My computer does not support the GL_ARB_compute_variable_group_size extension.
//

#version 440 compatibility

//  Array positions
layout(binding = 4) buffer posbuf { vec4 pos []; };
layout(binding = 5) buffer velbuf { vec4 vel []; };
//  Work group size
layout(local_size_x = 20, local_size_y = 1, local_size_z = 1) in; // 100 threads per work group

void main()
{

}
