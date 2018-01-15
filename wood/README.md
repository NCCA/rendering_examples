### Procedural noise textures
This wood grain texture is really an example of using my templated procedural noise generating class. It generates a 3D texture as a preprocess using a basic Perlin noise rather than computing this on the GPU in every frame. There is a performance overhead upfront to do this, but rendering is quicker.

Note that the 3D texture is used both for colour and for bump mapping to give the idea of different reflective properties based on the wood grain (not sure how successful this is).

The 3D noise only applies between 0 and 1, and the teapot coordinates lie slightly outside these bounds which is why there are seams in the final render.

 - Richard Southern 
   15/01/2018