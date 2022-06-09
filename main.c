#define STBI_NO_SIMD
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"
#include<strings.h>
#include<stdio.h>
#include<stdlib.h>


typedef struct {
	int width;
	int height;
	unsigned char *imageData;
} PGM;

int dx[4] = {0, 0, 1, -1};
int dy[4] = {1, -1, 0, 0};

int is_valid(int row, int col, PGM* image) {
	if (row < 0 || col < 0 || row >= image->height || col >= image->width) return 0;
	if (image->imageData[row * image->width + col] != 0) return 0; 
	return 1;
}

void dfs(int row, int col, PGM* image, PGM* colorful_image, int color) {
	int cord = row * image->width + col;
	colorful_image->imageData[cord] = color;

	for (int i = 0; i < 4; ++i) {
		int new_row = row + dx[i];
		int new_col = col + dy[i];
		int new_cord = new_row * image->width + new_col;

		if (is_valid(new_row, new_col, colorful_image)) {
			printf("go to x = %d, y = %d newcord = %d\n", new_row, new_col, new_cord);
			dfs(new_row, new_col, image, colorful_image, color);
		}
	}
}


void padding(PGM* image) {
	for (int i = 0; i < image->width; i++) {
		image->imageData[i] = 0;
		image->imageData[(image->height - 1) * image->width + i] = 0;
	}
	
	for (int i = 0; i < image->height; i++) {
		image->imageData[i * image->width] = 0;
		image->imageData[(i + 1) * image->width - 1] = 0;
	} 
}

int convolution(PGM* image, int kernel[3][3], int row, int col, int dim) {
	int i, j, sum = 0;
	for (i = 0; i < dim; i++) {
		for (j = 0; j < dim; j++) {
			sum += image->imageData[(i + row) * image->width + j + col] * kernel[i][j];
		}
	}
	return sum;
}

void sobel_edge_detector(PGM* image, PGM* out_image) {
	int mx[3][3] = {
		{-1, 0, 1},
		{-2, 0, 2},
		{-1, 0, 1}
	};
	int my[3][3] = {
		{-3, -10, -3},
		{0, 0, 0},
		{3, 10, 3}
	};
	
	for (int i = 1; i < image->height - 2; i++) {
		for (int j = 1; j < image->width - 2; j++) {
			int gx = convolution(image, mx, i, j, 3);
			int gy = convolution(image, my, i, j, 3);
			// printf("CHECK FOR I = %d J = %d\n", i, j);
			out_image->imageData[i * image->width + j] = sqrt(gx * gx + gy * gy);
		}
	}
}


void blur(PGM* image, PGM* out_image) {
	int blur3[3][3] = {
		{1, 2, 1},
		{2, 4, 2},
		{1, 2, 1}
	};


	for (int i = 1; i < image->height - 2; i++) {
		for (int j = 1; j < image->width - 2; j++) {
			out_image->imageData[i * image->width + j] = convolution(image, blur3, i, j, 3) / 16;
		}
	}
}


void copy(PGM* image, PGM* out_image) {
	for (int i = 0; i < image->height; ++i) {
		for (int j = 0; j < image->width; ++j) {
			image->imageData[i * image->width + j] = out_image->imageData[i * image->width + j];
		}
	}
}

FILE * file;

int main() {
	file = fopen("test.txt", "w+");

	char *inputPath = "hampster.png";
	int InputWidth, InputHeight, n;
	unsigned char *idata = stbi_load(inputPath, &InputWidth, &InputHeight, &n, 0);

	if (!idata) {
		printf("ERROR: can't read file \n");
		return 1;
	}
	printf("Loaded image with a width of %dpx,a height of %dpx and %d channels\n",InputWidth,InputHeight,n);

	char *pixel = idata;
	unsigned char* odata = calloc(InputWidth * InputHeight, sizeof(unsigned char));

	PGM image;
	image.width = InputWidth;
	image.height = InputHeight;
	image.imageData = calloc(InputWidth * InputHeight, sizeof(unsigned char));

	PGM out_image;
	out_image.width = InputWidth;
	out_image.height = InputHeight;
	out_image.imageData = calloc(InputWidth * InputHeight, sizeof(unsigned char));
	
	PGM colorful_image;
	colorful_image.width = InputWidth;
	colorful_image.height = InputHeight;
	colorful_image.imageData = calloc(InputWidth * InputHeight, sizeof(unsigned char));

	PGM final_image;
	final_image.width = InputWidth;
	final_image.height = InputHeight;
	final_image.imageData = calloc(InputWidth * InputHeight * 4, sizeof(unsigned char));

	for (int i = 0; i < InputWidth * InputHeight * n; i += 4) {
		image.imageData[i / 4] = (pixel[i] * 11 + pixel[i + 1] * 16 + 5 * pixel[i + 2]) / 32;	
	}

	
	padding(&image);
	blur(&image, &out_image);
	copy(&image, &out_image);
	sobel_edge_detector(&image, &out_image);	
	dfs(0, 0, &out_image, &colorful_image, 1);


	char *outputPath = "output.png";
	char *outputPathSobel = "outputSobel.png";
	char *final="final.png";
	stbi_write_png(outputPath, InputWidth, InputHeight, 1, image.imageData, 0);
	stbi_write_png(outputPathSobel, InputWidth, InputHeight, 1, out_image.imageData, 0);
    	 
	for (int i = 0; i < InputHeight * InputWidth; ++i) {
		fprintf(file, "%d ", colorful_image.imageData[i]);
		if ((i + 1) % InputWidth == 0) fprintf(file, "\n");
	}

    for (int i=0; i < InputWidth * InputHeight; i++) { 
        int c=colorful_image.imageData[i]; 
        final_image.imageData[4*i]=c; 
        final_image.imageData[4*i+1]=c; 
        final_image.imageData[4*i+2]=c; 
        final_image.imageData[4*i+3]=255; 
        
    } 
	stbi_write_png(final, InputWidth, InputHeight, 4, final_image.imageData, 0);
	stbi_image_free(idata);
	stbi_image_free(image.imageData);
	stbi_image_free(out_image.imageData);
	stbi_image_free(final_image.imageData);
	fclose(file);	
	return 0;
}