### Loading multiple images into a 3D cycling texture
This relatively simple demo loads a "stack" of separate images into a 3D texture and renders the layers, smoothly blending between them. 

The work is performed in TexScene::load3DTex() where each layer is loaded from the "data/" directory. The shaders are pretty straightforward, with just a 3D lookup with the z-position based on the time. Try switching the dimensions for a whacky effect!

If you're interested, the patterns are simulations generated of Random Plane Waves, which describe the patters which form on vibrating Chladni plates at different frequencies. 

- Richard Southern 
  15/01/2018