class raw_bmp;
struct TextureID;

//defined in billboarding
void billboard_identity();
void billboard_render_at(float x, float y, float z);
void draw2DModeON();
void draw2DModeOFF();

//defined in texture.h
void loadTexture32(raw_bmp *image, TextureID& text_tag);