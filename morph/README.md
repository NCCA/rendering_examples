### Animating Morph Targets in Graphics Hardware
This demo cycles between 5 morph targets of a face. It is a good example of doing vector math on Geometry using libigl, and also of blending between geometry states on the Vertex Shader. It also uses catmull rom splines for the blending for super-smoothness and creates a new VAO type in NGL to store the animated geometry.

- Richard Southern 
  15/01/2018