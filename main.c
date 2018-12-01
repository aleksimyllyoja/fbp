#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <math.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <linux/fb.h>
#include <cjson/cJSON.h>

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

	if(argc < 2) {
		printf("Please provide a filename\n");
		return -1;
	}
	
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

	// printf("%dX%d, %dbpp\n", vinfo.xres, vinfo.yres, vinfo.bits_per_pixel);

	screensize = vinfo.xres*vinfo.yres*vinfo.bits_per_pixel/8;

	// printf("mapping %dmb\n", screensize/1024/1014);
	
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
	
	int fd = open(argv[1], O_RDONLY);
	int len = lseek(fd, 0, SEEK_END);
	char *data = mmap(0, len, PROT_READ, MAP_PRIVATE, fd, 0);

	if((int)data == -1) {
		printf("Couldn't mmap file\n");
		return -1;
	}
	
	cJSON *elem, *subelem;
	cJSON *name;

	cJSON *root = cJSON_Parse(data);

	int n = cJSON_GetArraySize(root);
	int sn, i, j;
	double x0, y0, x1, y1;
	
	clear(300, 218);

	for(i=0; i<n; i++) {
		elem = cJSON_GetArrayItem(root, i);
		sn = cJSON_GetArraySize(elem);

		for(j=0; j<sn; j++) {
			subelem = cJSON_GetArrayItem(elem, j);
			x1 = cJSON_GetArrayItem(subelem, 0)->valuedouble;
			y1 = cJSON_GetArrayItem(subelem, 1)->valuedouble;
			
			if(j>0) {
				line(x0, y0, x1, y1);	
			}
			
			x0 = x1;
			y0 = y1;
		}	
	}

	munmap(data, len);
	munmap(fbp, screensize);

	close(fbfd);

	return 0;
}
