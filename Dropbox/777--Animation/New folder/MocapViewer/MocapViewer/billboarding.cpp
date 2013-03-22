#include <gl\glut.h>
void billboard_identity(){
	float modelview[16];
	glGetFloatv(GL_MODELVIEW_MATRIX , modelview);

	for(int i=0; i<3; i++ ) {
	    for(int j=0; j<3; j++ ) {
			if ( i==j ){
				modelview[i*4+j] = 1.0;
			}else{
				modelview[i*4+j] = 0.0;
			}
		}
	}
	glLoadMatrixf(modelview);
}
void billboard_render_at(float x, float y, float z){
	glTranslatef(x,y,z);
	billboard_identity();
}
void draw2DModeON(){

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();

	glEnable(GL_BLEND);

	glDisable(GL_CULL_FACE);
	glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable( GL_TEXTURE_2D );
	glDepthMask(false);

}
void draw2DModeOFF(){
	
	glEnable(GL_CULL_FACE);
	glDepthMask(true);
	glDisable(GL_BLEND);
	glDisable( GL_TEXTURE_2D );

	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();


}
