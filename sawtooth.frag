// John Salame
// I originally tried to make a cardioid, but it didn't work out.
// After a few attempts, I ended up making a sawtooth pattern by accident.
#version 120

uniform int dim;

void main() {
	vec2 p = gl_FragCoord.xy;
	// vec2 center = mod(p, dim) * dim;
	// vec2 r = p - center;
	vec2 r = mod(p, 2*dim);
	r.x = r.x>dim? r.x-2*dim : r.x; // allow for negative offsets from the center
	r.y = r.y>dim? r.y-2*dim : r.y; // allow for negative offsets from the center
	float th = atan(r.y, r.x);
	// float eqn = dim - length(r)*cos(th);
	// float eqn = length(r) - dim;
	float eqn = r.x/dim - (1 - cos(th));
	if(eqn < 0) {
		// green outside the shape
		gl_FragColor = vec4(0.0, 1.0, 0.0, 1.0);
	}
	else {
		// red inside shape
		gl_FragColor = vec4(1.0, 0.0, 0.0, 1.0);
	}
}
