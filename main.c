// To compile (linux/mac): gcc main.c -o main.out -std=c99
// To run (linux/mac): ./main.out "samples/easy/1EASY.bmp" "results/easy/1EASY_RESULT.bmp"

// To compile (win): gcc cbmp.c main.c -o main.exe -std=c99
// gcc main.c -o main.exe
// To run (win): main.exe example.bmp example_inv.bmp

#include <stdlib.h>
#include <stdio.h>
#include "cbmp.c"
#include <time.h>

#define testsize 12
#define BINARY_COLOUR_THRESHOLD 270

// Get the colour of the RGB image at pixel x, y.
char getColourIntensity(unsigned char bmp_image[BMP_WIDTH][BMP_HEIGTH][BMP_CHANNELS], unsigned int x, unsigned int y) {
    // Return 1 if pixel is white and 0 if the pixel is black.
    short int sum = bmp_image[x][y][0] + bmp_image[x][y][1] + bmp_image[x][y][2];
    return sum > BINARY_COLOUR_THRESHOLD;
}


// Write a 2D list binary_image from the RGB bitmap bmp_image.
void rgbToBinary (unsigned char bmp_image[BMP_WIDTH][BMP_HEIGTH][BMP_CHANNELS], unsigned char binary_image[BMP_WIDTH][BMP_HEIGTH]) {
    for (short int x = 0; x < BMP_WIDTH; x++) {
        for (short int y = 0; y < BMP_HEIGTH; y++) {
            if (getColourIntensity(bmp_image, x, y)) {
                binary_image[x][y] = 1;
            } else {
                binary_image[x][y] = 0;
            }
        }
    }
}

// Apply the erosion algorithm to the binary image using a structuring element.
char erode (unsigned char binary_image[BMP_WIDTH][BMP_HEIGTH], unsigned char bmp_image[BMP_WIDTH][BMP_HEIGTH][BMP_CHANNELS]) {
    char wasEroded = 0;

    // 1 means the pixel is part of the structuring element, 0 means it's ignored.
    // Define size of structuring element.
    char se_size = 3;

    // Define center of the structuring element.
    char se_center = se_size>>1; // Used for making sure the pixel is not at the border. Dividing integers automatically rounds down.

    // Define the structuring element itself.
    char structuringElement[3][3] = {
        {1, 1, 0},
        {1, 1, 1},
        {1, 1, 0}
    };
    
    // Create a temporary array to store the result.
    unsigned char temp_image[BMP_WIDTH][BMP_HEIGTH];
    
    // Copy the original binary image to the temporary array.
    for (short int x = 0; x < BMP_WIDTH; x++) {
        for (short int y = 0; y < BMP_HEIGTH; y++) {
            temp_image[x][y] = binary_image[x][y];
        }
    }

    printf("Starting erosion\n");

    // Apply the erosion algorithm for non-border pixels
    for (short int x = 1; x < BMP_WIDTH - 1; x++) {
        for (short int y = 1; y < BMP_HEIGTH - 1; y++) {

            if (temp_image[x][y]) {

                // Assume erosion passes initially, i.e. that there are no black pixels within the structuring element.
            
                // Check if the selected pixel should be eroding by comparing the surrounding grid with the structuring element.
                
                // Check each position in the structuring element projected on the binary image.
                for (char i = 0; i < se_size; i++) {
                    for (char j = 0; j < se_size; j++) {

                        // Only check positions where the structuring element has a 1.
                        if (structuringElement[i][j] == 1) {

                            

                            // Calculate the position of the selected entry of the structuring element relative to the selected pixel.
                            short int entry_x = x + i - 1;
                            short int entry_y = y + j - 1;
                            
                            // If any black pixel is contain on an entry of the structuring element equal to 1, the erosion happens.
                            if (temp_image[entry_x][entry_y] == 0) {
                                binary_image[x][y] = 0;
                                wasEroded = 1;
                            }
                        }
                    }
                }             
            }

            // Set the pixel to the result of the erosion, i.e. if the erosion failed (0), then set the pixel to 0 (black). If it succeeded set it to 1 (white).
        }
    }
    
    // Set border pixels to black to avoid boundary issues.

    // Horizontal borders:
    for (short int x = 0; x < BMP_WIDTH; x++) {
        for (short int i = 0; i < se_center; i++) {
            binary_image[x][i] = 0;  // Top border
            binary_image[x][BMP_HEIGTH - 1 - i] = 0;  // Bottom border
        }
    }

    // Vertical borders:
    for (short int y = 0; y < BMP_HEIGTH; y++) {
        for (short int i = 0; i < se_center; i++) {
            binary_image[i][y] = 0;  // Left border
            binary_image[BMP_WIDTH - 1 - i][y] = 0;  // Right border
        }
    }
    return wasEroded;
}


// Detect cells in the binary image using sliding window approach
short int detect(unsigned char binary_image[BMP_WIDTH][BMP_HEIGTH], unsigned char bmp_image[BMP_WIDTH][BMP_HEIGTH][BMP_CHANNELS]) {    
    short int cells = 0;
    for (short int x = 1; x < BMP_WIDTH - testsize - 1; x++) {
        for (short int y = 1; y < BMP_HEIGTH - testsize - 1; y++) {
            
            // Check if at least one pixel is white in the capturing area (12x12 square)
            char has_white_pixel = 0;
            for (int dx = 0; dx < testsize; dx++) {
                for (int dy = 0; dy < testsize; dy++) {
                    if (binary_image[x + dx][y + dy] == 1) {
                        has_white_pixel = 1;
                        break;
                    }
                }
                if (has_white_pixel) break;
            }
            
            // If at least one pixel is white in the capturing area
            if (has_white_pixel) {
                
                // Check if all pixels in the exclusion frame are black
                char exclusion_frame_clear = 1;
                
                // Check top and bottom rows of exclusion frame
                for (char dy = -1; dy <= testsize; dy++) {
                    if (y + dy >= 0 && y + dy < BMP_HEIGTH) {
                        if (binary_image[x - 1][y + dy] == 1 || binary_image[x + testsize][y + dy] == 1) {
                            exclusion_frame_clear = 0;
                            break;
                        }
                    }
                }
                
                // Check left and right columns of exclusion frame (excluding corners already checked)
                if (exclusion_frame_clear) {
                    for (char dx = 0; dx < testsize; dx++) {
                        if (x + dx >= 0 && x + dx < BMP_WIDTH) {
                            if (binary_image[x + dx][y - 1] == 1 || binary_image[x + dx][y + testsize] == 1) {
                                exclusion_frame_clear = 0;
                                break;
                            }
                        }
                    }
                }
                
                // If all pixels in the exclusion frame are black, register a cell detection
                if (exclusion_frame_clear) {
                    cells++;
                    for (char i = 1; i <= testsize; i++) {
                        for (char j = 1; j <= testsize; j++) {
                            bmp_image[x + i][y + j][0] = 255;
                            bmp_image[x + i][y + j][1] = 0;
                            bmp_image[x + i][y + j][2] = 0;
                        }
                    }
                    
                    // Set all pixels inside the capturing area to black to prevent detecting the same cell twice
                    for (char dx = 0; dx < testsize; dx++) {
                        for (char dy = 0; dy < testsize; dy++) {
                            binary_image[x + dx][y + dy] = 0;
                        }
                    }
                }
            }
        }
    }
    return cells;
}


//Declare the array to store the RGB image.
unsigned char bmp_image[BMP_WIDTH][BMP_HEIGTH][BMP_CHANNELS];

// Declare the array to store the binary images.
unsigned char binary_image[BMP_WIDTH][BMP_HEIGTH];

// Main function.
int main(int argc, char** argv) {
    //argc counts how may arguments are passed
    //argv[0] is a string with the name of the program
    //argv[1] is the first command line argument (input image)
    //argv[2] is the second command line argument (output image)
    clock_t start, end;
    double cpu_time_used;
    //Checking that 2 arguments are passed
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <output file path> <output file path>\n", argv[0]);
        exit(1);
    }

    start = clock();

    // Load image from file.
    read_bitmap(argv[1], bmp_image);

    // Write the binary image.
    rgbToBinary(bmp_image, binary_image);
    
    // Apply limited erosion and detect cells
    unsigned char erosion_iterations = 0;
    unsigned char max_erosions = 100; // Limit erosions to prevent removing all pixels
    short int total_cells = 0;
    
    while (erode(binary_image, bmp_image)) {
        erosion_iterations++;
        
        printf("Erosion %d done\n", 
               erosion_iterations);
        
        // Stop if max erosions reached
        if (erosion_iterations > max_erosions) {
            break;
        }
        
        total_cells += detect(binary_image, bmp_image);  
    }

    // Save image to file
    write_bitmap(bmp_image, argv[2]);

    printf("Total cells detected: %d\n", total_cells);

    end = clock();

    printf("Done!\n");
    cpu_time_used = end - start;
    printf("Total time: %f ms\n", cpu_time_used * 1000.0 /CLOCKS_PER_SEC);

    return 0;
}
