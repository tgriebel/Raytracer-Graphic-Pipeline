#ifndef ___TEXTURELOADER___
#define ___TEXTURELOADER___
#include <vector>
#include "TextureStructs.h"
#include "bitmap.h"
void loadTexture32(raw_bmp *image, TextureID& text_tag) {

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	glGenTextures(1, &text_tag.ID); //Make room for our texture
	glBindTexture(GL_TEXTURE_2D,  text_tag.ID); //Tell OpenGL which texture to edit
	//Map the image to the texture
	glTexImage2D(GL_TEXTURE_2D, 0,GL_RGBA, image->width, image->height,
							0,GL_RGBA,GL_UNSIGNED_BYTE,image->data);

	text_tag.height = image->height;
	text_tag.width = image ->width;


}
#endif
