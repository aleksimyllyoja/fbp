#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <math.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <linux/fb.h>

#define PI2 2*M_PI

char *fbp=0;
int xo;
int yo;
int bpp;
int line_length;

void plot(int x, int y, int r, int g, int b) {

	long int location = (x+xo) * (bpp/8) +
				           (y+yo) * line_length;

	*(fbp + location) = b;
	*(fbp + location + 1) = g;
	*(fbp + location + 2) = r;
}

void clear(int w, int h) {
	
	int x;
	int y;

	for(y=0;y<h;y++)
		for(x=0;x<w;x++) 
			plot(x, y, 0, 0, 0);
}

void line(int x0, int y0, int x1, int y1) {
	
	int dx = abs(x1-x0), sx = x0<x1 ? 1 : -1;
	int dy = abs(y1-y0), sy = y0<y1 ? 1 : -1;
	int err = (dx>dy ? dx : -dy)/2, e2;
	
	for(;;) {
		plot(x0, y0, 123, 32, 230);
		
		if(x0==x1 && y0==y1) break;
		e2 = err;
		
		if(e2 >-dx) { err -= dy; x0 += sx; }
		if(e2 < dy) { err += dx; y0 += sy; } 
	}
}

int main(int argc, char **argv) {

	long int screensize;	
	struct fb_var_screeninfo vinfo;
	struct fb_fix_screeninfo finfo;
	int fbfd = 0;

	fbfd = open("/dev/fb0", O_RDWR);
	
	if(fbfd == -1) {
		perror("ERROR");	
		exit(1);
	}
	
	if(ioctl(fbfd, FBIOGET_FSCREENINFO, &finfo) == -1) {
		printf("err\n");
		return -1;
	}
	if(ioctl(fbfd, FBIOGET_VSCREENINFO, &vinfo) == -1) {
		printf("err\n");
		return -1;
	}

	printf("%dX%d, %dbpp\n", vinfo.xres, vinfo.yres, vinfo.bits_per_pixel);

	screensize = vinfo.xres*vinfo.yres*vinfo.bits_per_pixel/8;

	printf("mapping %dmb\n", screensize/1024/1014);
	
	fbp = (char *)mmap(
		0, screensize, PROT_READ | PROT_WRITE,
		MAP_SHARED, fbfd, 0);

	xo = vinfo.xoffset;
	yo = vinfo.yoffset;
	bpp = vinfo.bits_per_pixel;
	line_length = finfo.line_length;

	if((int)fbp == -1) {
		printf("fb not initalized\n"); 
		return -1;
	}
	
	clear(300, 300);

	
	double i=0;
	double r=60;
	float phase=0.5;
	int cx=150;
	int cy=150;
	int p=5;
	int x1, y1, x2, y2;
	
	for(i=0; i<p; i++) {
		
		x1 = cx+r*cos(PI2/p*i+phase);
		y1 = cy+r*sin(PI2/p*i+phase);

		x2 = cx+r*cos(PI2/p*(i+1)+phase);
		y2 = cy+r*sin(PI2/p*(i+1)+phase);

		line(x1, y1, x2, y2);
	}

	
	
	munmap(fbp, screensize);
	close(fbfd);

	return 0;

}
