// To compile (linux/mac): gcc cbmp.c main.c -o main.out -std=c99
// To run (linux/mac): ./main.out example.bmp example_gray.bmp

// To compile (win): gcc cbmp.c main.c -o main.exe -std=c99
// gcc main.c -o main.exe
// To run (win): main.exe example.bmp example_inv.bmp

#include <stdlib.h>
#include <stdio.h>
#include "cbmp.c"
#include <time.h>

#define testsize 12

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
        binary_image[x][y] = 0;
    } else {
        binary_image[x][y] = 1;
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
void binaryToRGB (unsigned char binary_image[BMP_WIDTH][BMP_HEIGTH], unsigned char output_image[BMP_WIDTH][BMP_HEIGTH][BMP_CHANNELS]) {
    for (int x = 0; x < BMP_WIDTH; x++) {
        for (int y = 0; y < BMP_HEIGTH; y++) {
            for (int c = 0; c < BMP_CHANNELS; c++) {
                output_image[x][y][c] = binary_image[x][y] * 255;
            }
        }
    }
}


// Apply the erosion algorithm to the binary image using a structuring element.
void erode (unsigned char binary_image[BMP_WIDTH][BMP_HEIGTH]) {

    // Define a 3x3 structuring element (cross shape for cell detection).
    // 1 means the pixel is part of the structuring element, 0 means it's ignored.

    // Define size of structuring element.
    int se_size = 3;

    // Define center of the structuring element.
    int se_center = se_size / 2; // Used for making sure the pixel is not at the border. Dividing integers automatically rounds down.

    // Define the structuring element itself.
    int structuringElement[3][3] = {
        {0, 1, 0},
        {1, 1, 1},
        {0, 1, 0}
    };
    
    // Create a temporary array to store the result.
    unsigned char temp_image[BMP_WIDTH][BMP_HEIGTH];
    
    // Copy the original binary image to the temporary array.
    for (int x = 0; x < BMP_WIDTH; x++) {
        for (int y = 0; y < BMP_HEIGTH; y++) {
            temp_image[x][y] = binary_image[x][y];
        }
    }
    
    // Apply the erosion algorithm for non-border pixels
    for (int x = se_center; x < BMP_WIDTH - se_center; x++) {
        for (int y = se_center; y < BMP_HEIGTH - se_center; y++) {
            
            // Check if the selected pixel should be eroding by comparing the surrounding grid with the structuring element.

            // Assume erosion passes initially, i.e. that there are no black pixels within the structuring element.
            int erosion_result = 1;
            
            // Check each position in the structuring element projected on the binary image.
            for (int i = 0; i < se_size; i++) {
                for (int j = 0; j < se_size; j++) {

                    // Only check positions where the structuring element has a 1.
                    if (structuringElement[i][j] == 1) {

                        // Calculate the position of the selected entry of the structuring element relative to the selected pixel.
                        int entry_x = x + (i - se_center);
                        int entry_y = y + (j - se_center);
                        
                        // If any black pixel is contain on an entry of the structuring element equal to 1, the erosion fails.
                        if (temp_image[entry_x][entry_y] == 0) {
                            erosion_result = 0;
                            break;
                        }
                    }
                }

                // Skip the loop if the erosion already fails.
                if (erosion_result == 0) break;
            }
            
            // Set the pixel to the result of the erosion, i.e. if the erosion failed (0), then set the pixel to 0 (black). If it succeeded set it to 1 (white).
            binary_image[x][y] = erosion_result;
        }
    }
    
    // Set border pixels to black to avoid boundary issues.

    // Horizontal borders:
    for (int x = 0; x < BMP_WIDTH; x++) {
        for (int i = 0; i < se_center; i++) {
            binary_image[x][i] = 0;  // Top border
            binary_image[x][BMP_HEIGTH - 1 - i] = 0;  // Bottom border
        }
    }

    // Vertical borders:
    for (int y = 0; y < BMP_HEIGTH; y++) {
        for (int i = 0; i < se_center; i++) {
            binary_image[i][y] = 0;  // Left border
            binary_image[BMP_WIDTH - 1 - i][y] = 0;  // Right border
        }
    }

    // Check if the eroded image is identical to the binary image. Set the boolean value wasEroded to false if they are identical.
    wasEroded = (temp_image == binary_image) ? 0 : 1;
}






void detect(unsigned char binary_image[BMP_WIDTH][BMP_HEIGTH], unsigned char found_spots[BMP_WIDTH-testsize][BMP_HEIGTH-testsize]){
    unsigned char testsquare [testsize][testsize];
    for(int i = 0; i < 950-testsize; i++){
        for (int j = 0; j < 950-testsize; j++){
            for (int h = 0; h < testsize; h++){
                for (int w = 0; w < testsize; w++){
                    testsquare [h][w] = binary_image[i+h][j+w];
                }
            }
            if (test(testsquare)){
                if (exclusion(binary_image, i, j)){
                    found_spots[i][j]=1;
                    for (int h = 0; h < testsize; h++){
                        for (int w = 0; w < testsize; w++){
                            binary_image[i+h][j+w] = 0;
                        }
                    }
                }
            }
        }
    }
}

int test(unsigned char testsquare[testsize][testsize]){
    for (int h = 0; h < testsize; h++){
        for (int w = 0; w < testsize; w++){
            if (testsquare[h][w]){
                return 1;
            }     
        }
    }
    return 0;
}

int exclusion(unsigned char binary_image[BMP_WIDTH][BMP_HEIGTH], int i, int j){
    if(i){
        for(int w = 0; w < testsize; w++){
            if (binary_image[i-1][j+w]){
                return 0;
            }
        }
    }
    if(i!=(BMP_WIDTH-testsize-1)){
        for(int w = 0; w < testsize; w++){
            if (binary_image[i+1][j+w]){
                return 0;
            }
        }
    }
    if (j){
        for(int h = 0; h < testsize; h++){
            if (binary_image[i+h][j-1]){
                return 0;
            }
        }
    }
    if(j!=(BMP_HEIGTH-testsize-1)){
        for(int h = 0; h < testsize; h++){
            if (binary_image[i+h][j+1]){
                return 0;
            }
        }
    }
    if (i&&j){
        if (binary_image[i-1][j-1]){
                return 0;
            }
    }
    if (i&&j!=(BMP_HEIGTH-testsize-1)){
        if (binary_image[i-1][j+1]){
                return 0;
            }
    }
    if (i!=(BMP_WIDTH-testsize-1)&&j){
        if (binary_image[i-1][j-1]){
                return 0;
            }
    }
    if (i!=(BMP_WIDTH-testsize-1)&&j!=(BMP_HEIGTH-testsize-1)){
        if (binary_image[i-1][j+1]){
                return 0;
            }
    }
}



// Declare the arrays to store the RGB images.
unsigned char input_image[BMP_WIDTH][BMP_HEIGTH][BMP_CHANNELS];
unsigned char output_image[BMP_WIDTH][BMP_HEIGTH][BMP_CHANNELS];

// Declare the array to store the binary images.
unsigned char binary_image[BMP_WIDTH][BMP_HEIGTH];

// Declare list of found spots
unsigned char found_spots[BMP_WIDTH-testsize][BMP_HEIGTH-testsize];

// Declare boolean used to check erosion.
char wasEroded;

// Main function.
int main(int argc, char** argv) {
    //argc counts how may arguments are passed
    //argv[0] is a string with the name of the program
    //argv[1] is the first command line argument (input image)
    //argv[2] is the second command line argument (output image)
    clock_t start, end;
    double cpu_time_used;
    //Checking that 2 arguments are passed
    if (argc != 4) {
        fprintf(stderr, "Usage: %s <output file path> <output file path>\n", argv[0]);
        exit(1);
    }

    printf("Example program - 02132 - A1\n");
    start = clock();
    // Load image from file.
    read_bitmap(argv[1], input_image);

    // Write the binary image.
    rgbToBinary(input_image, binary_image);

    binaryToRGB(binary_image, output_image);

    // Save image to file
    write_bitmap(output_image, argv[2]);

    // erode image
    erode(binary_image);

    detect(binary_image, found_spots);
    //binaryToRGB(binary_image, output_image);

    // Save image to file
    //write_bitmap(output_image, argv[3]);

    end = clock();

    printf("Done!\n");
    cpu_time_used = end - start;
    printf("Total time: %f ms\n", cpu_time_used * 1000.0 /CLOCKS_PER_SEC);
    return 0;
}
