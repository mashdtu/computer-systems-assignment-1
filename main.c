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
#define BINARY_COLOUR_THRESHOLD 270

// Get the colour of the RGB image at pixel x, y.
int getColourRGB(unsigned char bmp_image[BMP_WIDTH][BMP_HEIGTH][BMP_CHANNELS], unsigned int x, unsigned int y) {
    // Return 1 if pixel is white and 0 if the pixel is black.
    int sum = bmp_image[x][y][0] + bmp_image[x][y][1] + bmp_image[x][y][2];
    return sum > BINARY_COLOUR_THRESHOLD;
}

// Switch colour of pixel (x,y) between black and white.
void switchColour(unsigned char binary_image[BMP_WIDTH][BMP_HEIGTH], unsigned int x, unsigned int y) {
    // Switch between black and white.
    binary_image[x][y] = !(binary_image[x][y]);
}


// Write a 2D list binary_image from the RGB bitmap bmp_image.
void rgbToBinary (unsigned char bmp_image[BMP_WIDTH][BMP_HEIGTH][BMP_CHANNELS], unsigned char binary_image[BMP_WIDTH][BMP_HEIGTH]) {
    for (int x = 0; x < BMP_WIDTH; x++) {
        for (int y = 0; y < BMP_HEIGTH; y++) {
            if (getColourRGB(bmp_image, x, y)) {
                binary_image[x][y] = 1;
            } else {
                binary_image[x][y] = 0;
            }
        }
    }
}


// Write an RGB bitmap bmp_image from the 2D list binary_image.
// Note that this image will neccisarily be polarised, any pixel will be either completely black or completely white.
void binaryToRGB (unsigned char binary_image[BMP_WIDTH][BMP_HEIGTH], unsigned char bmp_image[BMP_WIDTH][BMP_HEIGTH][BMP_CHANNELS]) {
    for (int x = 0; x < BMP_WIDTH; x++) {
        for (int y = 0; y < BMP_HEIGTH; y++) {
            for (int c = 0; c < BMP_CHANNELS; c++) {
                bmp_image[x][y][c] = binary_image[x][y] * 255;
            }
        }
    }
}

// Save a snapshot of the current binary image as a BMP under results/step_<step>.bmp
void saveErosionStepImage(unsigned char binary_image[BMP_WIDTH][BMP_HEIGTH], int step) {
    static unsigned char tmp[BMP_WIDTH][BMP_HEIGTH][BMP_CHANNELS];
    binaryToRGB(binary_image, tmp);
    char path[260];
    snprintf(path, sizeof(path), "results/step_%d.bmp", step);
    write_bitmap(tmp, path);
    printf("Saved erosion snapshot: %s\n", path);
}

// Apply the erosion algorithm to the binary image using a structuring element.
void erode (unsigned char binary_image[BMP_WIDTH][BMP_HEIGTH], unsigned char bmp_image[BMP_WIDTH][BMP_HEIGTH][BMP_CHANNELS]) {

    // Define a 3x3 structuring element (cross shape for cell detection).
    // 1 means the pixel is part of the structuring element, 0 means it's ignored.

    // Define size of structuring element.
    int se_size = 3;

    // Define center of the structuring element.
    int se_center = 1; // Used for making sure the pixel is not at the border. Dividing integers automatically rounds down.

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

    printf("Starting erosion with cross-shaped structuring element...\n");

    // Apply the erosion algorithm for non-border pixels
    for (int x = 0; x < BMP_WIDTH; x++) {
        for (int y = 0; y < BMP_HEIGTH; y++) {


            int erosion_result = 0;
            

            if (temp_image[x][y]) {

                // Assume erosion passes initially, i.e. that there are no black pixels within the structuring element.
                erosion_result = 1;
            
                // Check if the selected pixel should be eroding by comparing the surrounding grid with the structuring element.
                
                // Check each position in the structuring element projected on the binary image.
                for (int i = 0; i < se_size; i++) {
                    for (int j = 0; j < se_size; j++) {

                        // Only check positions where the structuring element has a 1.
                        if (structuringElement[i][j] == 1) {

                            

                            // Calculate the position of the selected entry of the structuring element relative to the selected pixel.
                            int entry_x = x + i;
                            int entry_y = y + j;
                            
                            // If any black pixel is contain on an entry of the structuring element equal to 1, the erosion fails.
                            if (temp_image[entry_x][entry_y] == 0) {
                                erosion_result = 0;
                            }
                        }
                    }
                }
                
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
}



// Detect cells in the binary image using sliding window approach
void detect(unsigned char binary_image[BMP_WIDTH][BMP_HEIGTH], unsigned char found_spots[BMP_WIDTH-testsize][BMP_HEIGTH-testsize]) {    
    
    for (int x = 1; x < BMP_WIDTH - testsize - 1; x++) {
        for (int y = 1; y < BMP_HEIGTH - testsize - 1; y++) {
            
            // Check if at least one pixel is white in the capturing area (12x12 square)
            int has_white_pixel = 0;
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
                int exclusion_frame_clear = 1;
                
                // Check top and bottom rows of exclusion frame
                for (int dy = -1; dy <= testsize; dy++) {
                    if (y + dy >= 0 && y + dy < BMP_HEIGTH) {
                        if (binary_image[x - 1][y + dy] == 1 || binary_image[x + testsize][y + dy] == 1) {
                            exclusion_frame_clear = 0;
                            break;
                        }
                    }
                }
                
                // Check left and right columns of exclusion frame (excluding corners already checked)
                if (exclusion_frame_clear) {
                    for (int dx = 0; dx < testsize; dx++) {
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
                    found_spots[x - 1][y - 1] = 1;  // Adjust for the exclusion frame offset
                    
                    // Set all pixels inside the capturing area to black to prevent detecting the same cell twice
                    for (int dx = 0; dx < testsize; dx++) {
                        for (int dy = 0; dy < testsize; dy++) {
                            binary_image[x + dx][y + dy] = 0;
                        }
                    }
                    
                    printf("Cell detected at position (%d, %d)\n", x, y);
                }
            }
        }
    }
}


// Function to create the output image with red + signs over each cell.
void createOutputImage (unsigned char bmp_image[BMP_WIDTH][BMP_HEIGTH][BMP_CHANNELS], unsigned char found_spots[BMP_WIDTH-testsize][BMP_HEIGTH-testsize]) {

    // Define the offset from the values in found_spots to the actual positions in the output image.
    char offset = testsize;

    // Check through all values in found_spots and.
    for (int x = 0; x < BMP_WIDTH-testsize; x++) {
        for (int y = 0; y < BMP_HEIGTH-testsize; y++) {

            // Draw the + sign for found spots.
            if (found_spots[x][y]) {
                for (int i = -6; i < 6; i++) {
                    for (int j = -6; j < 6; j++) {
                        bmp_image[x + offset + i][y + offset + j][0] = 255;
                        bmp_image[x + offset + i][y + offset + j][1] = 0;
                        bmp_image[x + offset + i][y + offset + j][2] = 0;
                    }
                }
                
                printf("Cell written at position (%d, %d)\n", x + offset, y + offset);
            }
        }
    }
}



//Declare the array to store the RGB image.
unsigned char bmp_image[BMP_WIDTH][BMP_HEIGTH][BMP_CHANNELS];

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
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <output file path> <output file path>\n", argv[0]);
        exit(1);
    }

    printf("Example program - 02132 - A1\n");
    start = clock();

    // Initialize found_spots array to all zeros.
    for (int x = 0; x < BMP_WIDTH - testsize; x++) {
        for (int y = 0; y < BMP_HEIGTH - testsize; y++) {
            found_spots[x][y] = 0;
        }
    }

    // Load image from file.
    read_bitmap(argv[1], bmp_image);

    // Write the binary image.
    rgbToBinary(bmp_image, binary_image);

    // Save the initial binary image as step 0 (before any erosion)
    saveErosionStepImage(binary_image, 0);

    // Count initial white pixels
    int initial_white_pixels = 0;
    for (int x = 0; x < BMP_WIDTH; x++) {
        for (int y = 0; y < BMP_HEIGTH; y++) {
            if (binary_image[x][y] == 1) {
                initial_white_pixels++;
            }
        }
    }
    printf("Initial white pixels after binary conversion: %d\n", initial_white_pixels);
    
    // Apply limited erosion and detect cells
    int erosion_iterations = 0;
    int max_erosions = 100; // Limit erosions to prevent removing all pixels
    
    do {
        // Create a copy to check if erosion changed anything
        unsigned char temp_copy[BMP_WIDTH][BMP_HEIGTH];
        for (int x = 0; x < BMP_WIDTH; x++) {
            for (int y = 0; y < BMP_HEIGTH; y++) {
                temp_copy[x][y] = binary_image[x][y];
            }
        }
        
    erode(binary_image, bmp_image);
    erosion_iterations++;

    // Save snapshot after this erosion iteration
    saveErosionStepImage(binary_image, erosion_iterations);
        
        // Check if erosion actually changed anything
        wasEroded = 0;
        for (int x = 0; x < BMP_WIDTH && !wasEroded; x++) {
            for (int y = 0; y < BMP_HEIGTH && !wasEroded; y++) {
                if (temp_copy[x][y] != binary_image[x][y]) {
                    wasEroded = 1;
                }
            }
        }
        
        // Count remaining white pixels
        int white_pixels = 0;
        for (int x = 0; x < BMP_WIDTH; x++) {
            for (int y = 0; y < BMP_HEIGTH; y++) {
                if (binary_image[x][y] == 1) {
                    white_pixels++;
                }
            }
        }
        
        printf("After erosion %d: %d white pixels remaining, wasEroded=%d\n", 
               erosion_iterations, white_pixels, wasEroded);
        
        // Stop if no white pixels remain or max erosions reached
        if (white_pixels == 0 || erosion_iterations > max_erosions) {
            break;
        }
        
        detect(binary_image, found_spots);
        
    } while (wasEroded);

    // Create output image
    createOutputImage(bmp_image, found_spots);

    // Save image to file
    write_bitmap(bmp_image, argv[2]);

    // Count and display total number of cells found
    int total_cells = 0;
    for (int x = 0; x < BMP_WIDTH-testsize; x++) {
        for (int y = 0; y < BMP_HEIGTH-testsize; y++) {
            if (found_spots[x][y]) {
                total_cells++;
            }
        }
    }
    printf("Total cells detected: %d\n", total_cells);

    end = clock();

    printf("Done!\n");
    cpu_time_used = end - start;
    printf("Total time: %f ms\n", cpu_time_used * 1000.0 /CLOCKS_PER_SEC);
    return 0;
}
