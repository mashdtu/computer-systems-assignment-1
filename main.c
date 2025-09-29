// To compile (linux/mac): gcc cbmp.c main.c -o main.out -std=c99
// To run (linux/mac): ./main.out example.bmp example_gray.bmp

// To compile (win): gcc cbmp.c main.c -o main.exe -std=c99
// gcc main.c -o main.exe
// To run (win): main.exe example.bmp example_inv.bmp

#include <stdlib.h>
#include <stdio.h>
#include "cbmp.c"

// Force pixel (x,y) to be white.
void forceWhite(unsigned char binary_image[BMP_WIDTH][BMP_HEIGTH], unsigned int x, unsigned int y) {
    binary_image[x][y] = 1;
}

// Force pixel (x,y) to be black.
void forceBlack(unsigned char binary_image[BMP_WIDTH][BMP_HEIGTH], unsigned int x, unsigned int y) {
    binary_image[x][y] = 0; 
}

// Get the colour of the binary image at pixel x, y.
int getColour(unsigned char binary_image[BMP_WIDTH][BMP_HEIGTH], unsigned int x, unsigned int y) {
    // Return 1 if pixel is white and 0 if the pixel is black.
    return binary_image[x][y];
}

// Get the colour of the RGB image at pixel x, y.
int getColourRGB(unsigned char input_image[BMP_WIDTH][BMP_HEIGTH][BMP_CHANNELS], unsigned int x, unsigned int y) {
    // Return 1 if pixel is white and 0 if the pixel is black.
    int sum = input_image[x][y][0] + input_image[x][y][1] + input_image[x][y][2];
    return sum > 270;
}

// Switch colour of pixel (x,y) between black and white.
void switchColour(unsigned char binary_image[BMP_WIDTH][BMP_HEIGTH], unsigned int x, unsigned int y) {
    // Switch between black and white based on current pixel.
    if (getColour(binary_image, x, y)) {
        forceBlack(binary_image, x, y);
    } else {
        forceWhite(binary_image, x, y);
    }
}


// Write a 2D list binary_image from the RGB bitmap input_image.
void rgbToBinary (unsigned char input_image[BMP_WIDTH][BMP_HEIGTH][BMP_CHANNELS], unsigned char binary_image[BMP_WIDTH][BMP_HEIGTH]) {
    for (int x = 0; x < BMP_WIDTH; x++) {
        for (int y = 0; y < BMP_HEIGTH; y++) {
            if (getColourRGB(input_image, x, y)) {
                forceWhite(binary_image, x, y);
            } else {
                forceBlack(binary_image, x, y);
            }
        }
    }
}


// Write an RGB bitmap input_image from the 2D list binary_image.
// Note that this image will neccisarily be polarised, any pixel will be either completely black or completely white.
void binaryToRGB (unsigned char binary_image[BMP_WIDTH][BMP_HEIGTH], unsigned char output_image[BMP_WIDTH][BMP_HEIGTH][BMP_CHANNELS]){
  for (int x = 0; x < BMP_WIDTH; x++) {
    for (int y = 0; y < BMP_HEIGTH; y++) {
      for (int c = 0; c < BMP_CHANNELS; c++) {
        output_image[x][y][c] = binary_image[x][y] * 255;
      }
    }
  }
}


void erode;






void detect;






// Declare the arrays to store the RGB images.
unsigned char input_image[BMP_WIDTH][BMP_HEIGTH][BMP_CHANNELS];
unsigned char output_image[BMP_WIDTH][BMP_HEIGTH][BMP_CHANNELS];

// Declare the array to store the binary images.
unsigned char binary_image[BMP_WIDTH][BMP_HEIGTH];


// Main function.
int main(int argc, char** argv)
{
  //argc counts how may arguments are passed
  //argv[0] is a string with the name of the program
  //argv[1] is the first command line argument (input image)
  //argv[2] is the second command line argument (output image)

  //Checking that 2 arguments are passed
  if (argc != 3)
  {
      fprintf(stderr, "Usage: %s <output file path> <output file path>\n", argv[0]);
      exit(1);
  }

  printf("Example program - 02132 - A1\n");

  // Load image from file.
  read_bitmap(argv[1], input_image);

  // Write the binary image.
  rgbToBinary(input_image, binary_image);

  binaryToRGB(binary_image, output_image);

  //Save image to file
  write_bitmap(output_image, argv[2]);

  printf("Done!\n");
  return 0;
}
